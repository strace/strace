#include "defs.h"
#include <dirent.h>

#define D_NAME_LEN_MAX 256

struct kernel_dirent {
	unsigned long   d_ino;
	unsigned long   d_off;
	unsigned short  d_reclen;
	char            d_name[1];
};

static void
print_old_dirent(struct tcb *tcp, long addr)
{
#ifdef SH64
	typedef struct kernel_dirent old_dirent_t;
#else
	typedef struct {
		uint32_t	d_ino;
		uint32_t	d_off;
		unsigned short  d_reclen;
		char            d_name[1];
	} old_dirent_t;
#endif
	old_dirent_t d;

	if (!verbose(tcp) || umove(tcp, addr, &d) < 0) {
		tprintf("%#lx", addr);
		return;
	}

	tprintf("{d_ino=%lu, d_off=%lu, d_reclen=%u, d_name=",
		(unsigned long) d.d_ino, (unsigned long) d.d_off, d.d_reclen);
	if (d.d_reclen > D_NAME_LEN_MAX)
		d.d_reclen = D_NAME_LEN_MAX;
	printpathn(tcp, addr + offsetof(old_dirent_t, d_name), d.d_reclen);
	tprints("}");
}

SYS_FUNC(readdir)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		if (syserror(tcp) || tcp->u_rval == 0 || !verbose(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			print_old_dirent(tcp, tcp->u_arg[1]);
		/* Not much point in printing this out, it is always 1. */
		if (tcp->u_arg[2] != 1)
			tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

#include "xlat/direnttypes.h"

SYS_FUNC(getdents)
{
	unsigned int i, len, dents = 0;
	char *buf;

	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		return 0;
	}
	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx, %lu", tcp->u_arg[1], tcp->u_arg[2]);
		return 0;
	}

	/* Beware of insanely large or too small values in tcp->u_rval */
	if (tcp->u_rval > 1024*1024)
		len = 1024*1024;
	else if (tcp->u_rval < (int) sizeof(struct kernel_dirent))
		len = 0;
	else
		len = tcp->u_rval;

	if (len) {
		buf = malloc(len);
		if (!buf)
			die_out_of_memory();
		if (umoven(tcp, tcp->u_arg[1], len, buf) < 0) {
			tprintf("%#lx, %lu", tcp->u_arg[1], tcp->u_arg[2]);
			free(buf);
			return 0;
		}
	} else {
		buf = NULL;
	}

	if (!abbrev(tcp))
		tprints("{");
	for (i = 0; len && i <= len - sizeof(struct kernel_dirent); ) {
		struct kernel_dirent *d = (struct kernel_dirent *) &buf[i];

		if (!abbrev(tcp)) {
			int oob = d->d_reclen < sizeof(struct kernel_dirent) ||
				  i + d->d_reclen - 1 >= len;
			int d_name_len = oob ? len - i : d->d_reclen;
			d_name_len -= offsetof(struct kernel_dirent, d_name) + 1;
			if (d_name_len > D_NAME_LEN_MAX)
				d_name_len = D_NAME_LEN_MAX;

			tprintf("%s{d_ino=%lu, d_off=%lu, d_reclen=%u, d_name=",
				i ? " " : "", d->d_ino, d->d_off, d->d_reclen);

			if (print_quoted_string(d->d_name, d_name_len,
					        QUOTE_0_TERMINATED) > 0) {
				tprints("...");
			}

			tprints(", d_type=");
			if (oob)
				tprints("?");
			else
				printxval(direnttypes, buf[i + d->d_reclen - 1], "DT_???");
			tprints("}");
		}
		dents++;
		if (d->d_reclen < sizeof(struct kernel_dirent)) {
			tprints("/* d_reclen < sizeof(struct kernel_dirent) */");
			break;
		}
		i += d->d_reclen;
	}
	if (!abbrev(tcp))
		tprints("}");
	else
		tprintf("/* %u entries */", dents);
	tprintf(", %lu", tcp->u_arg[2]);
	free(buf);
	return 0;
}

SYS_FUNC(getdents64)
{
	/* the minimum size of a valid dirent64 structure */
	const unsigned int d_name_offset = offsetof(struct dirent64, d_name);

	unsigned int i, len, dents = 0;
	char *buf;

	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		return 0;
	}
	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx, %lu", tcp->u_arg[1], tcp->u_arg[2]);
		return 0;
	}

	/* Beware of insanely large or too small tcp->u_rval */
	if (tcp->u_rval > 1024*1024)
		len = 1024*1024;
	else if (tcp->u_rval < (int) d_name_offset)
		len = 0;
	else
		len = tcp->u_rval;

	if (len) {
		buf = malloc(len);
		if (!buf)
			die_out_of_memory();
		if (umoven(tcp, tcp->u_arg[1], len, buf) < 0) {
			tprintf("%#lx, %lu", tcp->u_arg[1], tcp->u_arg[2]);
			free(buf);
			return 0;
		}
	} else {
		buf = NULL;
	}

	if (!abbrev(tcp))
		tprints("{");
	for (i = 0; len && i <= len - d_name_offset; ) {
		struct dirent64 *d = (struct dirent64 *) &buf[i];
		if (!abbrev(tcp)) {
			int d_name_len;
			if (d->d_reclen >= d_name_offset
			    && i + d->d_reclen <= len) {
				d_name_len = d->d_reclen - d_name_offset;
			} else {
				d_name_len = len - i - d_name_offset;
			}
			if (d_name_len > D_NAME_LEN_MAX)
				d_name_len = D_NAME_LEN_MAX;

			tprintf("%s{d_ino=%" PRIu64 ", d_off=%" PRId64
				", d_reclen=%u, d_type=",
				i ? " " : "",
				d->d_ino,
				d->d_off,
				d->d_reclen);
			printxval(direnttypes, d->d_type, "DT_???");

			tprints(", d_name=");
			if (print_quoted_string(d->d_name, d_name_len,
					        QUOTE_0_TERMINATED) > 0) {
				tprints("...");
			}

			tprints("}");
		}
		if (d->d_reclen < d_name_offset) {
			tprints("/* d_reclen < offsetof(struct dirent64, d_name) */");
			break;
		}
		i += d->d_reclen;
		dents++;
	}
	if (!abbrev(tcp))
		tprints("}");
	else
		tprintf("/* %u entries */", dents);
	tprintf(", %lu", tcp->u_arg[2]);
	free(buf);
	return 0;
}
