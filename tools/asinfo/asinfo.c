/*
 * The asinfo main source. The asinfo tool is purposed to operate
 * with system calls and provide information about it.
 *
 * Copyright (c) 2017 Edgar A. Kaziakhmedov <edgar.kaziakhmedov@virtuozzo.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch_interface.h"
#include "dispatchers.h"
#include "error_interface.h"
#include "error_prints.h"
#include "macros.h"
#include "request_msgs.h"
#include "syscall_interface.h"
#include "xmalloc.h"

#ifndef HAVE_PROGRAM_INVOCATION_NAME
char *program_invocation_name;
#endif

static void
usage(void)
{
	puts(
		"usage: asinfo (--set-arch arch | --get-arch | --list-arch)\n"
		"              [--set-abi abi | --list-abi] [--raw]\n"
		"   or: asinfo [(--set-arch arch | --get-arch) [--set-abi abi | --list-abi]]\n"
		"              ((--get-sname expr | --get-snum expr) [--nargs]) [--raw]\n"
		"\n"
		"Architecture:\n"
		"  --set-arch arch  use architecture ARCH for further work\n"
		"                   argument format: arch1,arch2,...\n"
		"  --get-arch       use architecture returned by uname for further work\n"
		"  --list-arch      print out all architectures supported by strace\n"
		"                   (combined use list-arch and any ABI option is permitted)\n"
		"\n"
		"ABI:\n"
		"  --set-abi abi    use application binary interface ABI for further work\n"
		"                   ('all' can be used as ABI to use all compatible personalities\n"
		"                   for corresponding architecture)\n"
		"                   argument format: abi1,abi2,...\n"
		"  --list-abi       use all ABIs for specified architecture\n"
		"\n"
		"System call:\n"
		"  --get-sname expr print all system calls that satisfy a filtering expression:\n"
		"                   [!]all or [!][?]val1[,[?]val2]...\n"
		"                   with the following format:\n"
		"                   | N | syscall name | snum1 | snum2 | ...\n"
		"  --get-snum expr  print all system calls that satisfy a filtering expression:\n"
		"                   [!]all or [!][?]val1[,[?]val2]...\n"
		"                   with the following format:\n"
		"                   | N | syscall number | sname1 | sname2 | ...\n"
		"  --nargs          change output format as follows:\n"
		"                   | N | syscall name or number | nargs1 | sname2 | ...\n"
		"\n"
		"Output formatting:\n"
		"  --raw            reset alignment and remove titles, use ';' as a delimiter\n"
		"\n"
		"Miscellaneous:\n"
		"  -h               print help message\n"
		"  -v               print version");
	exit(0);
}

static void
print_version(void)
{
	printf("asinfo (%s package) -- version %s\n"
	       "Copyright (c) 1991-%s The strace developers <%s>.\n"
	       "This is free software; see the source for copying conditions.  There is NO\n"
	       "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	       PACKAGE_NAME, PACKAGE_VERSION, COPYRIGHT_YEAR, PACKAGE_URL);
	exit(0);
}

void
die(void)
{
	exit(1);
}

static int
is_more1bit(unsigned int num)
{
	return !(num & (num - 1));
}

static unsigned
strpar2req(char *option)
{
	/* Convertion table to store string with options */
	static const char *options[] = {
		[SD_REQ_GET_SNAME_BIT]	= "--get-sname",
		[SD_REQ_GET_SNUM_BIT]	= "--get-snum",
		[SD_REQ_NARGS_BIT]	= "--nargs",
		[AD_REQ_SET_ARCH_BIT]	= "--set-arch",
		[AD_REQ_GET_ARCH_BIT]	= "--get-arch",
		[AD_REQ_LIST_ARCH_BIT]	= "--list-arch",
		[ABD_REQ_SET_ABI_BIT]	= "--set-abi",
		[ABD_REQ_LIST_ABI_BIT]	= "--list-abi",
		[SERV_REQ_RAW_BIT]	= "--raw",
		[SERV_REQ_HELP_BIT]	= "-h",
		[SERV_REQ_VERSION_BIT]	= "-v"
	};
	unsigned i;

	for (i = 0; i < ARRAY_SIZE(options); i++) {
		if (options[i] && strcmp(option, options[i]) == 0)
			return i;
	}
	return SERV_REQ_ERROR_BIT;
}

static char **
arg2list(char *argument)
{
	int i;
	int len = strlen(argument);
	int occur = 1;
	char **arg_list;

	for (i = 0; i < len; i++) {
		if (argument[i] == ',') {
			if (i == 0 || i == len - 1 || argument[i + 1] == ',')
				return NULL;
			occur++;
		}
	}
	arg_list = xcalloc(sizeof(*arg_list), occur + 1);
	for (i = 0; i < occur; i++) {
		arg_list[i] = argument;
		argument = strchr(argument, ',');
		if (argument) {
			*argument = '\0';
			argument++;
		}
	}
	return arg_list;
}

/* The purpose of this function is to convert input parameters to number with
   set bits, where each bit means specific work mode. Moreover, it checks input
   for correctness and outputs error messages in case of wrong input */
static unsigned
command_dispatcher(int argc, char *argv[], char **args[])
{
	int i;
	unsigned final_req = 0;
	unsigned temp_req = 0;
	int mult_arch = 0;
	int mult_abi = 0;
	unsigned non_req_arg = AD_REQ_GET_ARCH	| AD_REQ_LIST_ARCH	|
			       ABD_REQ_LIST_ABI	| SD_REQ_NARGS		|
			       SERV_REQ_RAW;

	if (!program_invocation_name || !*program_invocation_name) {
		static char name[] = "asinfo";
		program_invocation_name =
			(argv[0] && *argv[0]) ? argv[0] : name;
	}

	/* Try to find help or version parameter first */
	for (i = 1; i < argc; i++) {
		if (strpar2req(argv[i]) == SERV_REQ_HELP_BIT)
			usage();
		if (strpar2req(argv[i]) == SERV_REQ_VERSION_BIT)
			print_version();
	}
	/* For now, is is necessary to convert string parameter to number of
	   request and make basic check */
	for (i = 1; i < argc; i++) {
		if ((temp_req = strpar2req(argv[i])) == SERV_REQ_ERROR_BIT)
			error_msg_and_help("unrecognized option '%s'",
					   argv[i]);
		if (final_req & 1 << temp_req)
			error_msg_and_help("parameter '%s' has been used "
					   "more than once", argv[i]);
		if (!((1 << temp_req) & non_req_arg) &&
		     (i + 1 >= argc || strlen(argv[i + 1]) == 0 ||
		      strpar2req(argv[i + 1]) != SERV_REQ_ERROR_BIT))
			error_msg_and_help("parameter '%s' requires "
					   "argument", argv[i]);
		final_req |= 1 << temp_req;
		if (!((1 << temp_req) & non_req_arg)) {
			if ((1 << temp_req) & SD_REQ_MASK) {
				args[temp_req] = &argv[i + 1];
				i++;
				continue;
			}
			if ((args[temp_req] = arg2list(argv[i + 1])) != NULL) {
				i++;
				continue;
			}
			error_msg_and_help("argument '%s' of '%s' parameter "
					   "has a wrong format",
					   argv[i + 1], argv[i]);
		}
	}
	/* Count our multuarchness */
	if (args[AD_REQ_SET_ARCH_BIT])
		while (args[AD_REQ_SET_ARCH_BIT][mult_arch] != NULL)
			mult_arch++;
	if (args[ABD_REQ_SET_ABI_BIT])
		while (args[ABD_REQ_SET_ABI_BIT][mult_abi] != NULL)
			mult_abi++;
	/* final_req should be logically checked */
	/* More than one option from one request group couldn't be set */
	if ((is_more1bit(final_req & SD_REQ_MASK & ~SD_REQ_NARGS) == 0) ||
	    (is_more1bit(final_req & AD_REQ_MASK) == 0) ||
	    (is_more1bit(final_req & ABD_REQ_MASK) == 0))
		error_msg_and_help("exclusive parameters");
	/* Check on mutually exclusive options chain */
	/* If at least one syscall option has been typed, therefore
	   arch_options couldn't be list-arch and
	   abi_option couldn't be list-abi */
	if ((final_req & SD_REQ_MASK) &&
	    (((final_req & AD_REQ_MASK) && (final_req & AD_REQ_LIST_ARCH))))
		error_msg_and_help("wrong parameters");
	/* list-arch couldn't be used with any abi options */
	if ((final_req & AD_REQ_LIST_ARCH) &&
	    (final_req & ABD_REQ_MASK))
		error_msg_and_help("'--list-arch' cannot be used with any "
				   "ABI parameters");
	/* ABI requests could be used just in a combination with arch
	   requests */
	if ((final_req & ABD_REQ_MASK) &&
	    !(final_req & AD_REQ_MASK))
		error_msg_and_help("ABI parameters could be used only with "
				   "architecture parameter");
	/* set-abi must be used in case of multiple arch */
	if ((mult_arch > 1) && !(final_req & ABD_REQ_MASK))
		error_msg_and_help("ABI modes cannot be automatically "
				   "detected for multiple "
				   "architectures");
	/* set-abi and set-arch have to take the same number of args */
	if ((final_req & AD_REQ_SET_ARCH) && (final_req & ABD_REQ_SET_ABI) &&
	    (mult_arch != mult_abi))
		error_msg_and_help("each architecture needs respective "
				   "ABI mode, and vice versa");
	/* --nargs cannot be used alone */
	if ((final_req & SD_REQ_NARGS) &&
	    !(final_req & SD_REQ_MASK & ~SD_REQ_NARGS))
		error_msg_and_help("first set main output syscall "
				   "characteristics");
	/* raw should not be single */
	if (final_req == SERV_REQ_RAW)
		error_msg_and_help("raw data implies existing data");
	return final_req;
}

static char *
seek_sc_arg(char **input_args[])
{
	int i;

	for (i = SD_REQ_GET_SNAME_BIT; i < SYSCALL_REQ_BIT_LAST; i++)
		if (input_args[i] != NULL)
			return input_args[i][0];
	return NULL;
}

int
main(int argc, char *argv[])
{
	ARCH_LIST_DEFINE(arch_list);
	SYSCALL_LIST_DEFINE(sc_list);
	/* This array is purposed to store arguments for options in the
	   most convenient way */
	char ***in_args = xcalloc(sizeof(*in_args), REQ_LAST_BIT);
	unsigned reqs;

	/* command_dispatcher turn */
	reqs = command_dispatcher(argc, argv, in_args);
	if (reqs == 0)
		error_msg_and_help("must have OPTIONS");

	/* arch_dispatcher turn */
	arch_list = arch_dispatcher(reqs, in_args[AD_REQ_SET_ARCH_BIT]);
	if (es_error(al_err(arch_list)))
		perror_msg_and_die("%s", es_get_serror(al_err(arch_list)));
	/* abi_dispatcher turn */
	abi_dispatcher(arch_list, reqs, in_args[ABD_REQ_SET_ABI_BIT]);
	if (es_error(al_err(arch_list)))
		perror_msg_and_die("%s", es_get_serror(al_err(arch_list)));
	/* syscall_dispatcher turn */
	sc_list = syscall_dispatcher(arch_list, reqs, seek_sc_arg(in_args));
	if (es_error(ss_err(sc_list)))
		perror_msg_and_die("%s", es_get_serror(ss_err(sc_list)));
	/* If we want to get info about only architectures thus we print out
	   architectures, otherwise system calls */
	if (!(reqs & SD_REQ_MASK))
		al_dump(arch_list, reqs & SERV_REQ_RAW);
	else
		ss_dump(sc_list, reqs & SERV_REQ_RAW);
	return 0;
}
