#include <stdio.h>
#include "ref_asinfo_output.h"

#define TRY_HELP "Try \'../../asinfo -h\' for more information."

int
main(int argc, char *argv[])
{
	puts("../../asinfo: unrecognized option \'--set-ar\'\n"
	     TRY_HELP "\n"
	     "../../asinfo: parameter \'--get-arch\' has been used more than "
	     "once\n"
	     TRY_HELP "\n"
	     "../../asinfo: parameter \'--set-arch\' requires argument\n"
	     TRY_HELP "\n"
	     "../../asinfo: argument \'aarch64,\' of \'--set-arch\' parameter "
	     "has a wrong format\n"
	     TRY_HELP "\n"
	     "../../asinfo: exclusive parameters\n"
	     TRY_HELP "\n"
	     "../../asinfo: wrong parameters\n"
	     TRY_HELP "\n"
	     "../../asinfo: \'--list-arch\' cannot be used with any ABI "
	     "parameters\n"
	     TRY_HELP "\n"
	     "../../asinfo: ABI parameters could be used only with "
	     "architecture parameter\n"
	     TRY_HELP "\n"
	     "../../asinfo: ABI modes cannot be automatically detected for "
	     "multiple architectures\n"
	     TRY_HELP "\n"
	     "../../asinfo: each architecture needs respective ABI mode, "
	     "and vice versa\n"
	     TRY_HELP "\n"
	     "../../asinfo: first set main output syscall characteristics\n"
	     TRY_HELP "\n"
	     "../../asinfo: raw data implies existing data\n"
	     TRY_HELP "\n"
	     "../../asinfo: must have OPTIONS\n"
	     TRY_HELP);
	return 0;
}
