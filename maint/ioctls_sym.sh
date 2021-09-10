#!/bin/sh
# Copyright (c) 2015 Dmitry V. Levin <ldv@strace.io>
# Copyright (c) 2015-2021 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

set -efu

export LC_ALL=C

# This script processes header files containing ioctl command definitions in
# symbolic form, assuming that these definitions match the following regular
# expressions:

r_define='^[[:space:]]*#[[:space:]]*define[[:space:]]\+'
r_cmd_name='[A-Z][A-Z0-9_]*'
r_io='\([A-Z]\+\)\?_S\?\(IO\|IOW\|IOR\|IOWR\|IOC\)'
r_value='[[:space:]]\+'"$r_io"'[[:space:]]*([^)]'
regexp="${r_define}${r_cmd_name}${r_value}"

: "${uname_m:=$(uname -m)}"
me="${0##*/}"
msg()
{
	printf >&2 '%s\n' "$me: $*"
}

prefix=
case $# in
	1)	inc_dir="$1"; shift
		;;
	2)	inc_dir="$1"; shift
		prefix="$1"; shift
		;;
	*)	echo >&2 "usage: $me include-directory [prefix]"
		exit 1
		;;
esac

[ -z "$prefix" ] ||
	prefix="${prefix%%/}/"

tmpdir=
cleanup()
{
	trap - EXIT
	[ -z "$tmpdir" ] ||
		rm -rf -- "$tmpdir"
	exit "$@"
}

trap 'cleanup $?' EXIT
trap 'cleanup 1' HUP PIPE INT QUIT TERM
tmpdir="$(mktemp -dt "$me.XXXXXX")"

# list interesting files in $inc_dir.
cd "$inc_dir"
inc_dir="$(pwd -P)"
find . -type f -name '*.h' -exec grep -l "$r_value" -- '{}' + \
	> "$tmpdir"/headers1.list ||:
[ -s "$tmpdir"/headers1.list ] || exit 0

cd - > /dev/null
sed 's|^\./\(uapi/\)\?||' < "$tmpdir"/headers1.list > "$tmpdir"/headers.list
LC_COLLATE=C sort -u -o "$tmpdir"/headers.list "$tmpdir"/headers.list

msg "processing $(wc -l < "$tmpdir"/headers.list) header files from $inc_dir"
failed=0

READELF="${READELF:-readelf}"
CC="${CC:-gcc}"
CPP="${CPP:-cpp}"
CPPFLAGS="${CPPFLAGS-} -D__EXPORTED_HEADERS__"
CFLAGS="${CFLAGS:--Wall -O2} -gdwarf-2 -D__EXPORTED_HEADERS__"
INCLUDES="-I$inc_dir/uapi -I$inc_dir ${INCLUDES-}"

# Hook onto <asm-generic/ioctl.h> and <asm/ioctl.h>
for d in asm-generic asm; do
	mkdir "$tmpdir/$d"
	cat > "$tmpdir/$d"/ioctl.h <<__EOF__
#include_next <$d/ioctl.h>
#undef _IOC_NONE
#define _IOC_NONE (2<<7)
#undef _IOC_READ
#define _IOC_READ (2<<8)
#undef _IOC_WRITE
#define _IOC_WRITE (2<<9)
#undef _IOC
#define _IOC(dir, type, nr, size) \
	char \
		d[1 + (dir)], \
		n[1 + (((unsigned) (type) << _IOC_TYPESHIFT) | \
			((unsigned) (nr) << _IOC_NRSHIFT))], \
		s[1 + (size)]
__EOF__
done

INCLUDES="-I$tmpdir $INCLUDES"

process_file()
{
	local f="$1"; shift

	# Common code for every processed file.
	cat > "$tmpdir"/printents.c <<-__EOF__
		#include <linux/compiler_attributes.h>
		#include <asm/termbits.h>
		#include <asm/ioctl.h>
		#include <linux/types.h>
		#include <linux/limits.h>
		#include <linux/major.h>

		#include <sys/types.h>
		#include <sys/socket.h>
		#include <stdint.h>
		#include <stdbool.h>

		#ifndef NULL
		# define NULL ((void*)0)
		#endif
		#ifndef __user
		# define __user
		#endif
		#ifndef __iomem
		# define __iomem
		#endif
		#ifndef __noreturn
		# define __noreturn __attribute__((noreturn))
		#endif
		#ifndef __packed
		# define __packed __attribute__((packed))
		#endif
		#ifndef __no_sanitize_or_inline
		# define __no_sanitize_or_inline
		#endif
		#ifndef __no_kasan_or_inline
		# define __no_kasan_or_inline
		#endif

		typedef signed char s8;
		typedef unsigned char u8;
		typedef signed short s16;
		typedef unsigned short u16;
		typedef signed int s32;
		typedef unsigned int u32;
		typedef signed long long s64;
		typedef unsigned long long u64;

		#include "fixes.h"

		#ifndef UL
		# define UL(x) (_UL(x))
		#endif

		#ifndef ULL
		# define ULL(x) (_ULL(x))
		#endif

		#include <asm/bitsperlong.h>
		#ifndef BITS_PER_LONG
		# define BITS_PER_LONG __BITS_PER_LONG
		#endif

		#include "$f"

		#include "defs.h"

		__EOF__

	# Soft pre-include workarounds for some processed files.  Fragile.
	case "$f" in
		*asm/amigayle.h)
			return 0 # false positive
			;;
		*asm/cmb.h)
			echo '#include <asm/dasd.h>'
			;;
		*asm/core_*.h)
			return 0 # false positives
			;;
		*asm*/ioctls.h)
			cat <<-'__EOF__'
				#include <asm/termios.h>
				#include <linux/serial.h>
				__EOF__
			;;
		drm/sis_drm.h)
			echo '#include <drm/drm.h>'
			;;
		*drm/*_drm.h)
			echo '#include <drm/drm.h>' > "$tmpdir/drm.h"
			;;
		fbio.h|*/fbio.h)
			cat <<-'__EOF__'
				#include <linux/fb.h>
				#undef FBIOGETCMAP
				#undef FBIOPUTCMAP
				__EOF__
			;;
		*linux/atm_zatm.h)
			cat <<-'__EOF__'
				#include <linux/atm.h>
				#ifndef _LINUX_TIME_H
				# define _LINUX_TIME_H
				#endif
				#ifndef _UAPI_LINUX_TIME_H
				# define _UAPI_LINUX_TIME_H
				#endif
				__EOF__
			;;
		*linux/atm?*.h)
			echo '#include <linux/atm.h>'
			;;
		*linux/auto_fs*.h)
			echo 'typedef u32 compat_ulong_t;'
			;;
		*linux/coda.h|*android_alarm.h)
			cat <<-'__EOF__'
				#ifndef _LINUX_TIME_H
				# define _LINUX_TIME_H
				#endif
				#ifndef _UAPI_LINUX_TIME_H
				# define _UAPI_LINUX_TIME_H
				#endif
				__EOF__
			;;
		*linux/fs.h|*linux/ncp_fs.h)
			cat <<-'__EOF__'
				#include <linux/blktrace_api.h>
				#include <linux/fiemap.h>
				__EOF__
			;;
		*linux/ndctl.h)
			echo '#define PAGE_SIZE 0'
			;;
		*linux/if_pppox.h)
			echo '#include <netinet/in.h>'
			;;
		*linux/if_tun.h|*linux/ppp-ioctl.h)
			echo '#include <linux/filter.h>'
			;;
		*linux/isdn_ppp.h|*linux/gsmmux.h)
			echo '#include <linux/if.h>'
			;;
		*media*/saa6588.h)
			echo 'typedef struct poll_table_struct poll_table;'
			;;
		*linux/ivtvfb.h|*linux/meye.h|*media/*.h)
			echo '#include <linux/videodev2.h>'
			;;
		*linux/kvm.h)
			case "$uname_m" in
				i?86|x86_64|aarch64|mips*|ppc*|s390*) ;;
				*) return 0 ;; # not applicable
			esac
			;;
		*linux/omap3isp.h)
			echo 'struct omap3isp_stat_data_time32 {uint32_t dummy32[4]; uint16_t dummy16[3]; };'
			;;
		*linux/platform_data/cros_ec_chardev.h)
			echo 'struct cros_ec_command {uint32_t dummy32[5]; uint8_t dummy8[0]; };'
			;;
		*linux/sonet.h)
			echo '#include <linux/atmioc.h>'
			;;
		*linux/usbdevice_fs.h)
			cat <<-'__EOF__'
				struct usbdevfs_ctrltransfer32 { __u32 unused[4]; };
				struct usbdevfs_bulktransfer32 { __u32 unused[4]; };
				struct usbdevfs_disconnectsignal32 { __u32 unused[2]; };
				struct usbdevfs_urb32 { __u8 unused[42]; };
				struct usbdevfs_ioctl32 { __u32 unused[3]; };
				__EOF__
			;;
		logger.h|*/logger.h)
			echo 'typedef __u32 kuid_t;'
			;;
		*sound/asequencer.h)
			cat <<-'__EOF__'
				#include <sound/asound.h>
				struct snd_seq_queue_owner { __u32 unused[0]; };
				__EOF__
			;;
		*sound/emu10k1.h)
			cat <<-'__EOF__'
				#include <sound/asound.h>
				#ifndef DECLARE_BITMAP
				# define DIV_ROUND_UP(x,y) (((x) + ((y) - 1)) / (y))
				# define BITS_TO_LONGS(nr) DIV_ROUND_UP(nr, 8 * sizeof(long))
				# define DECLARE_BITMAP(name,bits) unsigned long name[BITS_TO_LONGS(bits)]
				#endif
				__EOF__
			;;
		*video/sstfb.h)
			echo 'struct fb_info;'
			;;
		*xen/evtchn.h|*xen/gntdev.h)
			cat <<-'__EOF__'
				typedef uint32_t grant_ref_t;
				typedef uint16_t domid_t;
				__EOF__
			;;
		*xen/interface/*.h)
			return 0 # false positives
			;;
		*xen/privcmd.h)
			return 0 # too much work to make it compileable
			;;
	esac > "$tmpdir"/fixes.h

	cat > "$tmpdir"/header.in <<-__EOF__
		#include <asm/bitsperlong.h>
		#ifndef BITS_PER_LONG
		# define BITS_PER_LONG __BITS_PER_LONG
		#endif
		#include "$f"
		__EOF__

	if [ -f "$inc_dir/uapi/$f" ]; then
		s="$inc_dir/uapi/$f"
	elif [ -f "$inc_dir/$f" ]; then
		s="$inc_dir/$f"
	else
		msg "$f: file not found"
		return 1
	fi

	[ -n "${f##*/*}" ] ||
		mkdir -p "$tmpdir/${f%/*}"
	# Hard workarounds for some processed files.  Very fragile.
	case "$f" in
		*asm-generic/ioctls.h)
			# Filter out macros defined using unavailable types.
			case "$uname_m" in
				alpha*|ppc*)
					grep -Fv 'struct termios2' < "$s" > "$tmpdir/$f"
					;;
			esac
			;;
		*acpi/*|*linux/i2o.h|*media*/exynos-fimc.h|*media/v4l2-subdev.h|*net/bluetooth/*|net/nfc/nci_core.h)
			# Fetch macros only.
			grep "${r_define}${r_cmd_name}" < "$s" > "$tmpdir/$f"
			;;
		binder.h|*/binder.h)
			# Convert enums to macros.
			sed '/^enum binder/,/^};/d' < "$s" > "$tmpdir/$f"
			sed -n '/^enum binder/,/^};/ s/^[[:space:]].*/&/p' < "$s" |
			sed -e '
s/^[[:space:]]*\([A-Z][A-Z_0-9]*\)[[:space:]]*=[[:space:]]*_\(IO\|IOW\|IOR\|IOWR\|IOC\)[[:space:]]*(/#define \1 _\2(/
s/^\(#define .*)\),$/\1/
s/^\(#define .*,\)$/\1 \\/
s/^\([[:space:]]\+[^),]\+)\),$/\1/' >> "$tmpdir/$f"
			;;
		*drm/r128_drm.h)
			# Filter out the code that references unknown types.
			sed '/drm_r128_clear2_t/d' < "$s" > "$tmpdir/$f"
			;;
		*drm/sis_drm.h)
			# Filter out the code that references unknown types.
			sed '/^struct sis_file_private/,/^}/d' < "$s" > "$tmpdir/$f"
			;;
		*drm/via_drm.h)
			# Create the file it attempts to include.
			touch "$tmpdir/via_drmclient.h"
			# Filter out the code that references unknown types.
			sed '/^struct via_file_private/,/^}/d' < "$s" > "$tmpdir/$f"
			;;
		*linux/dma-buf.h)
			# Filter out duplicates.
			sed '/\<DMA_BUF_SET_NAME\>/d' < "$s" > "$tmpdir/$f"
			;;
		*linux/nilfs2_fs.h)
			# Create the file it attempts to include.
			touch "$tmpdir/asm/bug.h"
			;;
		*linux/videodev2.h)
			# Force time64 based definitions.
			sed -e 's/__KERNEL__/__linux__/' \
			    -e 's/linux\/types\.h/linux\/time_types.h/' \
			    < "$s" > "$tmpdir/$f"
			;;
		*linux/vmw_vmci_defs.h)
			# Fetch ioctl macros only.
			grep "${r_define}I" < "$s" > "$tmpdir/$f"
			;;
		*media/v4l2-common.h)
			# Fetch one piece of code containing ioctls definitions.
			sed -n '/\* s_config \*/,/ ---/p' < "$s" >> "$tmpdir/$f"
			;;
		*media/v4l2-ioctl.h)
			echo 'struct old_timespec32 {int32_t dummy[2];};' >> "$tmpdir/$f"
			echo 'struct old_timeval32 {int32_t dummy[2];};' >> "$tmpdir/$f"
			# Fetch one piece of code containing ioctls definitions.
			awk '/^struct v4l2_event_time32/{p=1}/#endif/{p=0}p{print}' < "$s" >> "$tmpdir/$f"
			;;
		*sound/pcm.h)
			echo '#include <uapi/sound/asound.h>' > "$tmpdir/$f"
			# Fetch one piece of code containing ioctls definitions.
			awk '/^struct snd_pcm_status64 {/{p=1}/#endif/{p=0}p{print}' < "$s" >> "$tmpdir/$f"
			;;
		openpromio.h|*/openpromio.h|fbio.h|*/fbio.h)
			# Create the file it attempts to include.
			mkdir -p "$tmpdir/linux"
			touch "$tmpdir/linux/compiler.h"
	esac
	if [ -f "$tmpdir/$f" ]; then
		s="$tmpdir/$f"
	fi

	# This may fail if the file includes unavailable headers.
	# In case of success it outputs both the #define directives
	# and the result of preprocessing.
	$CPP $CPPFLAGS -dD $INCLUDES < "$tmpdir"/header.in > "$tmpdir"/header.out

	# Soft post-preprocess workarounds.  Fragile.
	case "$f" in
		*linux/btrfs.h)
			sed -i '/[[:space:]]BTRFS_IOC_[GS]ET_FSLABEL[[:space:]]/d' \
				"$tmpdir"/header.out
			;;
		*linux/kvm.h)
			arm_list='KVM_ARM_[A-Z_]+'
			ppc_list='KVM_ALLOCATE_RMA|KVM_CREATE_SPAPR_TCE|KVM_CREATE_SPAPR_TCE_64|KVM_PPC_[A-Z1-9_]+'
			s390_list='KVM_S390_[A-Z_]+'
			x86_list='KVM_GET_CPUID2|KVM_GET_DEBUGREGS|KVM_GET_EMULATED_CPUID|KVM_GET_LAPIC|KVM_GET_MSRS|KVM_GET_MSR_FEATURE_INDEX_LIST|KVM_GET_MSR_INDEX_LIST|KVM_GET_NESTED_STATE|KVM_GET_PIT|KVM_GET_PIT2|KVM_GET_SREGS2|KVM_GET_SUPPORTED_CPUID|KVM_GET_SUPPORTED_HV_CPUID|KVM_GET_VCPU_EVENTS|KVM_GET_XCRS|KVM_GET_XSAVE|KVM_HYPERV_EVENTFD|KVM_SET_CPUID|KVM_SET_CPUID2|KVM_SET_DEBUGREGS|KVM_SET_LAPIC|KVM_SET_MEMORY_ALIAS|KVM_SET_MSRS|KVM_SET_NESTED_STATE|KVM_SET_PIT|KVM_SET_PIT2|KVM_SET_PMU_EVENT_FILTER|KVM_SET_SREGS2|KVM_SET_VCPU_EVENTS|KVM_SET_XCRS|KVM_SET_XSAVE|KVM_XEN_HVM_CONFIG|KVM_X86_[A-Z_]+'
			case "$uname_m" in
				aarch64|arm*) list="$ppc_list|$s390_list|$x86_list" ;;
				ppc*) list="$arm_list|$s390_list|$x86_list" ;;
				s390*) list="$arm_list|$ppc_list|$x86_list" ;;
				i?86|x86_64*) list="$arm_list|$ppc_list|$s390_list" ;;
				*) list="$arm_list|$ppc_list|$s390_list|$x86_list" ;;
			esac
			sed -r -i "/[[:space:]]($list)[[:space:]]/d" "$tmpdir"/header.out
			;;
		*linux/v4l2-subdev.h)
			sed -r -i '/[[:space:]]VIDIOC_SUBDEV_(DV_TIMINGS_CAP|ENUM_DV_TIMINGS|ENUMSTD|G_DV_TIMINGS|G_EDID|G_STD|QUERY_DV_TIMINGS|QUERYSTD|S_DV_TIMINGS|S_EDID|S_STD)[[:space:]]/d' \
				"$tmpdir"/header.out
			;;
	esac

	# Need to exclude ioctl commands defined elsewhere.
	local_defines='^[[:space:]]*#[[:space:]]*define[[:space:]]\+\('"$r_cmd_name"'\)[[:space:]]'
	sed -n 's/'"$local_defines"'.*/\1\\/p' "$s" > "$tmpdir"/local_names
	r_local_names="$(tr '\n' '|' < "$tmpdir"/local_names)"
	r_local_names="${r_local_names%%|}"
	r_local_names="${r_local_names%%\\}"

	# Keep this in sync with $regexp by replacing $r_cmd_name with $r_local_names.
	defs_regexp="${r_define}\($r_local_names\)${r_value}"

	# This outputs lines in the following format:
	# struct {IOCTL_CMD_NAME;} ioc_IOCTL_CMD_NAME;
	sed -n 's/'"$defs_regexp"'.*/struct {\1;} ioc_\1;/p' \
		< "$tmpdir"/header.out > "$tmpdir"/defs.h

	# If something is wrong with the file, this will fail.
	$CC $INCLUDES $CFLAGS -c -o "$tmpdir"/printents.o "$tmpdir"/printents.c

	$READELF --wide --debug-dump=info "$tmpdir"/printents.o \
		> "$tmpdir"/debug-dump

	sed -r -n '
		/^[[:space:]]*<1>/,/^[[:space:]]*<1><[^>]+>: Abbrev Number: 0/!d
		/^[[:space:]]*<[^>]*><[^>]*>: Abbrev Number: 0/d
		s/^[[:space:]]*<[[:xdigit:]]+>[[:space:]]+//
		s/^[[:space:]]*((<[[:xdigit:]]+>){2}):[[:space:]]+/\1\n/
		s/[[:space:]]+$//
		p' "$tmpdir"/debug-dump > "$tmpdir"/debug-info
	gawk -v HEADER_NAME="$prefix$f" -f "${0%/*}"/ioctls_sym.awk \
		"$tmpdir"/debug-info > "$tmpdir"/ioctlents

	cat "$tmpdir"/ioctlents
	msg "$f: fetched $(grep -c '^{' "$tmpdir"/ioctlents) ioctl entries"
}

while read f; do
	(process_file "$f" < /dev/null)
	[ $? -eq 0 ] || {
		msg "$f: failed to process"
		failed=$((1 + $failed))
	}
done < "$tmpdir"/headers.list

[ $failed -eq 0 ] ||
	msg "failed to process $failed file(s)"
