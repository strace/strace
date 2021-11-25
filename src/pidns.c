/*
 * Copyright (c) 2020-2021 Ákos Uzonyi <uzonyi.akos@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"


#include <dirent.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <asm/unistd.h>

#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/nsfs.h>
#include "largefile_wrappers.h"
#include "number_set.h"
#include "trie.h"
#include "xmalloc.h"
#include "xstring.h"

/**
 * Key:   PID NS ID
 * Value: a trie:
 *           Key:   a process PID in NS
 *           Value: the process's PID as present in /proc
 */
static struct trie *ns_pid_to_proc_pid[PT_COUNT];

/**
 * Key:   Proc PID
 * Value: struct proc_data
 */
static struct trie *proc_data_cache;

static bool ns_get_parent_enotty = false;

static const char tid_str[]  = "NSpid:\t";
static const char tgid_str[] = "NStgid:\t";
static const char pgid_str[] = "NSpgid:\t";
static const char sid_str[]  = "NSsid:\t";

static const struct {
	const char *str;
	size_t size;
} id_strs[PT_COUNT] = {
	[PT_TID] =  { tid_str,  sizeof(tid_str)  - 1 },
	[PT_TGID] = { tgid_str, sizeof(tgid_str) - 1 },
	[PT_PGID] = { pgid_str, sizeof(pgid_str) - 1 },
	[PT_SID] =  { sid_str,  sizeof(sid_str)  - 1 },
};


/**
 * Limit on PID NS hierarchy depth, imposed since Linux 3.7. NS traversal
 * is not possible before Linux 4.9, so we consider this limit pretty universal.
 */
#define MAX_NS_DEPTH 32

static const size_t ns_id_size = sizeof(unsigned int) * 8;
static const uint8_t ptr_sz_lg = (sizeof(void *) == 8 ? 6 : 5);

static int pid_max;
static uint8_t pid_max_size, pid_max_size_lg;

struct proc_data {
	int proc_pid;
	int ns_count;
	unsigned int ns_hierarchy[MAX_NS_DEPTH];
	int id_count[PT_COUNT];
	int id_hierarchy[PT_COUNT][MAX_NS_DEPTH];
};

/**
 * Helper function for creating a trie.
 *
 * For node_key_bits and data_block_key_bits 4 is used (so trie height is 32 / 4
 * = 8, and node sizes are 8 byte * 2^4 = 128 bytes), which seems to be a good
 * tradeoff between memory usage and lookup time. It should not be too large,
 * since there can be large holes between PIDs, and it would be just a waste of
 * memory having large nodes with lot of NULL pointers in them.
 */
static struct trie *
create_trie_4(uint8_t key_size, uint8_t item_size_lg, uint64_t empty_value)
{
	struct trie *t = trie_create(key_size, item_size_lg, 4, 4, empty_value);
	if (!t)
		error_msg_and_die("creating trie failed");

	return t;
}

void
pidns_init(void)
{
	if (proc_data_cache)
		return;

	pid_max = INT_MAX;
	if (read_int_from_file("/proc/sys/kernel/pid_max", &pid_max) < 0)
		debug_func_perror_msg("reading /proc/sys/kernel/pid_max");
	pid_max_size = ilog2_32(pid_max - 1) + 1;
	pid_max_size_lg = ilog2_32(pid_max_size - 1) + 1;

	for (int i = 0; i < PT_COUNT; i++)
		ns_pid_to_proc_pid[i] = create_trie_4(ns_id_size, ptr_sz_lg, 0);

	proc_data_cache = create_trie_4(pid_max_size, ptr_sz_lg, 0);
}

static void
put_proc_pid(unsigned int ns, int ns_pid, enum pid_type type, int proc_pid)
{
	struct trie *b = (struct trie *) (uintptr_t) trie_get(ns_pid_to_proc_pid[type], ns);
	if (!b) {
		b = create_trie_4(pid_max_size, pid_max_size_lg, 0);
		trie_set(ns_pid_to_proc_pid[type], ns, (uint64_t) (uintptr_t) b);
	}
	trie_set(b, ns_pid, proc_pid);
}

static int
get_cached_proc_pid(unsigned int ns, int ns_pid, enum pid_type type)
{
	struct trie *b = (struct trie *) (uintptr_t)
		trie_get(ns_pid_to_proc_pid[type], ns);
	if (!b)
		return 0;

	return trie_get(b, ns_pid);
}

/**
 * Returns a list of PID NS IDs for the specified PID.
 *
 * @param proc_pid PID (as present in /proc) to get information for.
 * @param ns_buf   Pointer to buffer that is able to contain at least
 *                 ns_buf_size items.
 * @return         Amount of NS in list. 0 indicates error.
 */
static size_t
get_ns_hierarchy(int proc_pid, unsigned int *ns_buf, size_t ns_buf_size)
{
	char path[PATH_MAX + 1];
	xsprintf(path, "/proc/%s/ns/pid", pid_to_str(proc_pid));

	int fd = open_file(path, O_RDONLY);
	if (fd < 0)
		return 0;

	size_t n = 0;
	while (n < ns_buf_size) {
		strace_stat_t st;
		if (fstat_fd(fd, &st))
			break;

		ns_buf[n++] = st.st_ino;
		if (n >= ns_buf_size)
			break;

		if (ns_get_parent_enotty)
			break;

		int parent_fd = ioctl(fd, NS_GET_PARENT);
		if (parent_fd < 0) {
			switch (errno) {
			case EPERM:
				break;

			case ENOTTY:
				ns_get_parent_enotty = true;
				error_msg("NS_* ioctl commands are not "
					  "supported by the kernel");
				break;

			default:
				perror_func_msg("ioctl(NS_GET_PARENT)");
				break;
			}

			break;
		}

		close(fd);
		fd = parent_fd;
	}

	close(fd);

	return n;
}

/**
 * Get list of IDs present in NS* proc status record. IDs are placed as they are
 * stored in /proc (from top to bottom of NS hierarchy).
 *
 * @param proc_pid    PID (as present in /proc) to get information for.
 * @param id_buf      Pointer to buffer that is able to contain at least
 *                    MAX_NS_DEPTH items. Can be NULL.
 * @param type        Type of ID requested.
 * @return            Number of items stored in id_list. 0 indicates error.
 */
static size_t
get_id_list(int proc_pid, int *id_buf, enum pid_type type)
{
	return proc_status_get_id_list(proc_pid, id_buf, MAX_NS_DEPTH,
				       id_strs[type].str, id_strs[type].size);
}

/**
 * Returns whether the /proc filesystem's PID namespace is the same as strace's.
 */
static bool
is_proc_ours(void)
{
	static int cached_val = -1;

	if (cached_val < 0)
		cached_val = get_id_list(0, NULL, PT_TID) <= 1;

	return cached_val;
}

/**
 * Returns the PID namespace of the tracee
 */
static unsigned int
get_ns(struct tcb *tcp)
{
	if (!tcp->pid_ns) {
		int proc_pid = 0;
		translate_pid(NULL, tcp->pid, PT_TID, &proc_pid);

		if (proc_pid)
			get_ns_hierarchy(proc_pid, &tcp->pid_ns, 1);
	}

	return tcp->pid_ns;
}

/**
 * Returns the PID namespace of strace
 */
static unsigned int
get_our_ns(void)
{
	static unsigned int our_ns = 0;
	static bool our_ns_initialised = false;

	if (!our_ns_initialised) {
		get_ns_hierarchy(0, &our_ns, 1);
		our_ns_initialised = true;
	}

	return our_ns;
}

/**
 * Returns the cached proc_data struct associated with proc_pid.
 * If none found, allocates a new proc_data.
 */
static struct proc_data *
get_or_create_proc_data(int proc_pid)
{
	struct proc_data *pd = (struct proc_data *) (uintptr_t)
		trie_get(proc_data_cache, proc_pid);

	if (!pd) {
		pd = calloc(1, sizeof(*pd));
		if (!pd)
			return NULL;

		pd->proc_pid = proc_pid;
		trie_set(proc_data_cache, proc_pid, (uint64_t) (uintptr_t) pd);
	}

	return pd;
}

/**
 * Updates the proc_data from /proc
 * If the process does not exists, returns false, and frees the proc_data
 */
static bool
update_proc_data(struct proc_data *pd, enum pid_type type)
{
	pd->ns_count = get_ns_hierarchy(pd->proc_pid,
		pd->ns_hierarchy, MAX_NS_DEPTH);
	if (!pd->ns_count)
		goto fail;

	pd->id_count[type] = get_id_list(pd->proc_pid,
		pd->id_hierarchy[type], type);
	if (!pd->id_count[type])
		goto fail;

	return true;

fail:
	trie_set(proc_data_cache, pd->proc_pid, (uint64_t) (uintptr_t) NULL);
	free(pd);
	return false;
}

/**
 * Parameters for id translation
 */
struct translate_id_params {
	/* The namespace to be translated from */
	unsigned int from_ns;
	/* The id to be translated */
	int from_id;
	/* The type of the id */
	enum pid_type type;

	/* The result (output) */
	int result_id;
	/* The proc data of the process (output) */
	struct proc_data *pd;
};

/**
 * Translates an id to our namespace, given the proc_pid of the process,
 * by reading files in /proc.
 *
 * @param tip      The parameters
 * @param proc_pid The proc pid of the process.
 *                 If 0, use the cached values in tip->pd.
 */
static void
translate_id_proc_pid(struct translate_id_params *tip, int proc_pid)
{
	struct proc_data *pd = proc_pid ?
		get_or_create_proc_data(proc_pid) :
		tip->pd;

	tip->result_id = 0;
	tip->pd = NULL;

	if (!pd)
		return;

	if (proc_pid && !update_proc_data(pd, tip->type))
		return;

	if (!pd->ns_count || pd->id_count[tip->type] < pd->ns_count)
		return;

	int *id_hierarchy = pd->id_hierarchy[tip->type];
	int id_count = pd->id_count[tip->type];

	for (int i = 0; i < pd->ns_count; i++) {
		unsigned int ns = pd->ns_hierarchy[i];
		int ns_id = id_hierarchy[id_count - i - 1];
		int our_id = id_hierarchy[id_count - pd->ns_count];

		if (ns != tip->from_ns)
			continue;

		if (ns_id != tip->from_id)
			return;

		tip->result_id = our_id;
		tip->pd = pd;
		return;
	}
}

/**
 * Translates an id to our namespace by reading all proc entries in a directory.
 * The directory is either /proc or /proc/<pid>/task.
 *
 *
 * @param tip            The parameters
 * @param path           The path of the directory to be read.
 * @param read_task_dir  Whether recurse to "task" subdirectory.
 */
static void
translate_id_dir(struct translate_id_params *tip, const char *path,
                 bool read_task_dir)
{
	DIR *dir = opendir(path);
	if (!dir) {
		debug_func_perror_msg("opening dir: %s", path);
		return;
	}

	while (!tip->result_id) {
		errno = 0;
		struct_dirent *entry = read_dir(dir);
		if (!entry) {
			if (errno)
				perror_func_msg("readdir");

			break;
		}

		if (entry->d_type != DT_DIR)
			continue;

		errno = 0;
		long proc_pid = strtol(entry->d_name, NULL, 10);
		if (proc_pid < 1 || proc_pid > INT_MAX || errno)
			continue;

		if (read_task_dir) {
			char task_dir_path[PATH_MAX + 1];
			xsprintf(task_dir_path, "/proc/%ld/task", proc_pid);
			translate_id_dir(tip, task_dir_path, false);
		}

		if (tip->result_id)
			break;

		translate_id_proc_pid(tip, proc_pid);
	}

	closedir(dir);
}

/**
 * Iterator function of the proc_data_cache for id translation.
 * If the cache contains the id we are looking for, reads the corresponding
 * directory in /proc, and if cache is valid, saves the result.
 */
static void
proc_data_cache_iterator_fn(void* fn_data, uint64_t key, uint64_t val)
{
	struct translate_id_params *tip = (struct translate_id_params *)fn_data;
	struct proc_data *pd = (struct proc_data *) (uintptr_t) val;

	if (!pd)
		return;

	/* Result already found in an earlier iteration */
	if (tip->result_id)
		return;

	/* Translate from cache */
	tip->pd = pd;
	translate_id_proc_pid(tip, 0);
	if (!tip->result_id)
		return;

	/* Now translate from actual data in /proc, to check cache validity */
	translate_id_proc_pid(tip, pd->proc_pid);
}

int
translate_pid(struct tcb *tcp, int from_id, enum pid_type type,
              int *proc_pid_ptr)
{
	if (from_id <= 0 || type < 0 || type >= PT_COUNT)
		return 0;

	/* If translation is trivial */
	if ((!tcp || get_ns(tcp) == get_our_ns()) &&
	    (!proc_pid_ptr || is_proc_ours())) {
		if (proc_pid_ptr)
			*proc_pid_ptr = from_id;

		return from_id;
	}

	struct translate_id_params tip = {
		.from_ns = tcp ? get_ns(tcp) : get_our_ns(),
		.from_id = from_id,
		.type = type,
		.result_id = 0,
		.pd = NULL,
	};

	if (!tip.from_ns)
		return 0;

	if (ns_get_parent_enotty)
		return 0;

	/* Look for a cached proc_pid for this (from_ns, from_id) pair */
	int cached_proc_pid = get_cached_proc_pid(tip.from_ns, tip.from_id,
		tip.type);
	if (cached_proc_pid) {
		translate_id_proc_pid(&tip, cached_proc_pid);
		if (tip.result_id)
			goto exit;
	}

	/* Iterate through the cache, find potential proc_data */
	trie_iterate_keys(proc_data_cache, 0, pid_max - 1,
		proc_data_cache_iterator_fn, &tip);
	/* (proc_data_cache_iterator_fn takes care about updating proc_data) */
	if (tip.result_id)
		goto exit;

	/* No cache helped, read all entries in /proc */
	translate_id_dir(&tip, "/proc", true);

exit:
	if (tip.pd) {
		if (tip.pd->proc_pid)
			put_proc_pid(tip.from_ns, tip.from_id, tip.type,
				tip.pd->proc_pid);

		if (proc_pid_ptr)
			*proc_pid_ptr = tip.pd->proc_pid;
	}

	return tip.result_id;
}

int
get_proc_pid(int pid)
{
	int proc_pid = 0;
	translate_pid(NULL, pid, PT_TID, &proc_pid);
	return proc_pid;
}

static void
printpid_translation(struct tcb *tcp, int pid, enum pid_type type)
{
	bool print_ns_translation =
		is_number_in_set(DECODE_PID_NS_TRANSLATION, decode_pid_set);
	bool print_comm =
		is_number_in_set(DECODE_PID_COMM, decode_pid_set) &&
		(type == PT_TID || type == PT_TGID);

	if (print_ns_translation || print_comm) {
		int strace_pid = translate_pid(tcp, pid, type, NULL);
		if (strace_pid) {
			if (print_comm)
				print_pid_comm(strace_pid);
			if (print_ns_translation && strace_pid != pid)
				tprintf_comment("%d in strace's PID NS",
						strace_pid);
		}
	}
}

void
printpid(struct tcb *tcp, int pid, enum pid_type type)
{
	PRINT_VAL_D(pid);
	printpid_translation(tcp, pid, type);
}

void
printpid_tgid_pgid(struct tcb *tcp, int pid)
{
	PRINT_VAL_D(pid);
	if (pid > 0)
		printpid_translation(tcp,  pid, PT_TGID);
	else if (pid < -1)
		printpid_translation(tcp, -pid, PT_PGID);
}
