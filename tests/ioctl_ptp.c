/*
 * Check decoding of PTP_* commands of ioctl syscall.
 *
 * Copyright (c) 2018 Harsha Sharma <harshasharmaiitr@gmail.com>
 * Copyright (c) 2018-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ptp_clock.h>

#include "xlat.h"
#include "xlat/ptp_extts_flags.h"
#include "xlat/ptp_perout_flags.h"
#include "xlat/ptp_pin_funcs.h"


#ifdef INJECT_RETVAL
# define INJ_STR " (INJECTED)"
#else
# define INJ_STR ""
#endif

#define ARRAY_END(a_) ((a_) + ARRAY_SIZE(a_))
#define ARR_ITEM(arr_, idx_) ((arr_)[(idx_) % ARRAY_SIZE(arr_)])

#if STRACE_SIZEOF_KERNEL_LONG_T == SIZEOF_KERNEL_LONG_T
# define SAFE_TIME_T(t_) t_
#else
# define SAFE_TIME_T(t_) ((time_t) (t_))
#endif

static const char *errstr;

static long
sys_ioctl(kernel_long_t fd, kernel_ulong_t cmd, kernel_ulong_t arg)
{
#ifdef INJECT_RETVAL
	static char buf[256];
#endif
	const long rc = syscall(__NR_ioctl, fd, cmd, arg);
#ifdef INJECT_RETVAL
	snprintf(buf, sizeof(buf), "%s (INJECTED)", sprintrc(rc));
	errstr = buf;
#else
	errstr = sprintrc(rc);
#endif
	return rc;
}

static void
print_lltime(const long long sec, const unsigned long long nsec)
{
#if !XLAT_RAW
	if ((time_t) sec != sec)
		return;

	print_time_t_nsec(sec, nsec, 1);
#endif
}

static void
check_bad_ptr(const uint32_t ioc_val, const char *const ioc_str,
	      const void *const p, const size_t sz)
{
	sys_ioctl(-1, ioc_val, (uintptr_t) NULL);
	printf("ioctl(-1, " XLAT_FMT ", NULL) = %s\n",
	       XLAT_SEL(ioc_val, ioc_str), errstr);

	sys_ioctl(-1, ioc_val, (uintptr_t) p + sz);
	printf("ioctl(-1, " XLAT_FMT ", %p) = %s\n",
	       XLAT_SEL(ioc_val, ioc_str), p + sz, errstr);

	sys_ioctl(-1, ioc_val, (uintptr_t) p + 1);
	printf("ioctl(-1, " XLAT_FMT ", %p) = %s\n",
	       XLAT_SEL(ioc_val, ioc_str), p + 1, errstr);
}

static void
test_no_device(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_clock_caps, caps);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_sys_offset, sysoff);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_sys_offset_extended, soext);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_sys_offset_precise, soprec);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_extts_request, extts);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_perout_request, perout);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_pin_desc, pindesc);
	long rc;

	/* unrecognized */
	static const uint8_t unk_nums[] = { 0, 21, 22, 255 };

	for (const uint8_t *p = unk_nums; p < ARRAY_END(unk_nums); p++) {
		for (uint16_t sz = 0; sz < 1280; sz += 8) {
			static const struct strval32 dirs[] = {
				{ ARG_STR(_IOC_NONE) },
				{ ARG_STR(_IOC_READ) },
				{ ARG_STR(_IOC_WRITE) },
				{ ARG_STR(_IOC_READ|_IOC_WRITE) },
			};
			for (const struct strval32 *d = dirs;
			     d < ARRAY_END(dirs); d++) {
				uint32_t ioc =
					_IOC(d->val, PTP_CLK_MAGIC, *p, sz);
				sys_ioctl(-1, ioc, 0);
				printf("ioctl(-1, "
				       XLAT_KNOWN_FMT("%#x",
						      "_IOC(%s, %#x, %#x, %#x)")
				       ", 0) = %s\n",
				       NABBR(ioc,)
				       NRAW(d->str, PTP_CLK_MAGIC, *p, sz,)
				       errstr);

				/* soprec is the biggest var, at 1216 bytes */
				sys_ioctl(-1, ioc, (uintptr_t) soext);
				printf("ioctl(-1, "
				       XLAT_KNOWN_FMT("%#x",
						      "_IOC(%s, %#x, %#x, %#x)")
				       ", %p) = %s\n",
				       NABBR(ioc,)
				       NRAW(d->str, PTP_CLK_MAGIC, *p, sz,)
				       soext, errstr);
			}
		}
	}

	/* PTP_CLOCK_GETCAPS{,2} */
	static const struct strval32 ioc_caps[] = {
		{ ARG_STR(PTP_CLOCK_GETCAPS) },
		{ ARG_STR(PTP_CLOCK_GETCAPS2) },
	};
	for (const struct strval32 *c = ioc_caps; c < ARRAY_END(ioc_caps); c++)
	{
		check_bad_ptr(c->val, c->str, caps, sizeof(*caps));

		memset(caps, 0, sizeof(*caps));
		rc = sys_ioctl(-1, c->val, (uintptr_t) caps);
		printf("ioctl(-1, " XLAT_FMT ", ", XLAT_SEL(c->val, c->str));
		if (rc >= 0) {
			printf("{max_adj=0, n_alarm=0, n_ext_ts=0, n_per_out=0"
			       ", pps=0, n_pins=0, cross_timestamping=0"
			       ", adjust_phase=0, max_phase_adj=0}");
		} else {
			printf("%p", caps);
		}
		printf(") = %s\n", errstr);

		fill_memory32_ex(caps, sizeof(*caps), 1000000, 1);
		rc = sys_ioctl(-1, c->val, (uintptr_t) caps);
		printf("ioctl(-1, " XLAT_FMT ", ", XLAT_SEL(c->val, c->str));
		if (rc >= 0) {
			printf("{max_adj=1000000, n_alarm=1000000"
			       ", n_ext_ts=1000000, n_per_out=1000000"
			       ", pps=1000000, n_pins=1000000"
			       ", cross_timestamping=1000000"
			       ", adjust_phase=1000000, max_phase_adj=1000000"
			       NRAW(" /* 0.001000000 s */")
			       ", rsv=[0xf4240, 0xf4240, 0xf4240"
			       ", 0xf4240, 0xf4240, 0xf4240, 0xf4240"
			       ", 0xf4240, 0xf4240, 0xf4240, 0xf4240]}");
		} else {
			printf("%p", caps);
		}
		printf(") = %s\n", errstr);

		fill_memory32(caps, sizeof(*caps));
		rc = sys_ioctl(-1, c->val, (uintptr_t) caps);
		printf("ioctl(-1, " XLAT_FMT ", ", XLAT_SEL(c->val, c->str));
		if (rc >= 0) {
			printf("{max_adj=-2136948512, n_alarm=-2136948511"
			       ", n_ext_ts=-2136948510, n_per_out=-2136948509"
			       ", pps=-2136948508, n_pins=-2136948507"
			       ", cross_timestamping=-2136948506"
			       ", adjust_phase=-2136948505"
			       ", max_phase_adj=-2136948504"
			       NRAW(" /* -2.136948504 s */")
			       ", rsv=[0x80a0c0e9, 0x80a0c0ea"
			       ", 0x80a0c0eb, 0x80a0c0ec, 0x80a0c0ed"
			       ", 0x80a0c0ee, 0x80a0c0ef, 0x80a0c0f0"
			       ", 0x80a0c0f1, 0x80a0c0f2, 0x80a0c0f3]}");
		} else {
			printf("%p", caps);
		}
		printf(") = %s\n", errstr);
	}

	/* PTP_EXTTS_REQUEST{,2} */
	static const struct strval32 ioc_extts[] = {
		{ ARG_STR(PTP_EXTTS_REQUEST) },
		{ ARG_STR(PTP_EXTTS_REQUEST2) },
	};
	static const struct strval32 extts_flags[] = {
		{ ARG_XLAT_KNOWN(0x1, "PTP_ENABLE_FEATURE") },
		{ ARG_XLAT_KNOWN(0xdeadbabe, "PTP_RISING_EDGE|PTP_FALLING_EDGE"
					     "|PTP_STRICT_FLAGS|0xdeadbab0") },
		{ ARG_XLAT_UNKNOWN(0xbadbeef0, "PTP_???") },
		{ ARG_STR(0) },
	};
	for (const struct strval32 *c = ioc_extts; c < ARRAY_END(ioc_extts);
	     c++) {
		check_bad_ptr(c->val, c->str, extts, sizeof(*extts));

		memset(extts, 0, sizeof(*extts));
		sys_ioctl(-1, c->val, (uintptr_t) extts);
		printf("ioctl(-1, " XLAT_FMT ", {index=0, flags=0}) = %s\n",
		       XLAT_SEL(c->val, c->str), errstr);

		extts->index = 3141592653;
		for (size_t i = 0; i < ARRAY_SIZE(extts_flags); i++) {
			extts->flags = extts_flags[i].val;
			extts->rsv[0] = i & 1 ? 0xdeadc0de : 0;
			extts->rsv[1] = i & 2 ? 0xcafebeef : 0;

			sys_ioctl(-1, c->val, (uintptr_t) extts);
			printf("ioctl(-1, " XLAT_FMT ", {index=3141592653"
			       ", flags=%s",
			       XLAT_SEL(c->val, c->str), extts_flags[i].str);
			if (c->val == PTP_EXTTS_REQUEST2 && (i & 3)) {
				printf(", rsv=[%#x, %#x]",
				       i & 1 ? 0xdeadc0de : 0,
				       i & 2 ? 0xcafebeef : 0);
			}
			printf("}) = %s\n", errstr);

		}
	}

	/* PTP_PEROUT_REQUEST{,2} */
	static const struct strval32 ioc_perout[] = {
		{ ARG_STR(PTP_PEROUT_REQUEST) },
		{ ARG_STR(PTP_PEROUT_REQUEST2) },
	};
	static const struct perout_flags {
		uint32_t is_phase	:1,
			 is_duty_cycle	:1;
		uint32_t flags;
		const char *str;
	} perout_flags[] = {
		{ false, false, ARG_STR(0) },
		{ false, false, ARG_XLAT_KNOWN(0x1, "PTP_PEROUT_ONE_SHOT") },
		{ false, true,  ARG_XLAT_KNOWN(0x3, "PTP_PEROUT_ONE_SHOT"
						    "|PTP_PEROUT_DUTY_CYCLE") },
		{ true,  false,
		  ARG_XLAT_KNOWN(0xc0dedbad, "PTP_PEROUT_ONE_SHOT"
					     "|PTP_PEROUT_PHASE|0xc0dedba8") },
		{ true,  true,
		  ARG_XLAT_KNOWN(0xdeadbeef,
				 "PTP_PEROUT_ONE_SHOT|PTP_PEROUT_DUTY_CYCLE"
				 "|PTP_PEROUT_PHASE|0xdeadbee8") },
		{ false, false, ARG_XLAT_UNKNOWN(0xdeadbea8, "PTP_PEROUT_???") }
	};
	for (const struct strval32 *c = ioc_perout; c < ARRAY_END(ioc_perout);
	     c++) {
		check_bad_ptr(c->val, c->str, perout, sizeof(*perout));

		memset(perout, 0, sizeof(*perout));
		sys_ioctl(-1, c->val, (uintptr_t) perout);
		printf("ioctl(-1, " XLAT_FMT ", {start={sec=0, nsec=0}"
		       ", period={sec=0, nsec=0}, index=0, flags=0}) = %s\n",
		       XLAT_SEL(c->val, c->str), errstr);

		for (size_t i = 0; i < ARRAY_SIZE(perout_flags); i++) {
			perout->start.sec = SAFE_TIME_T(0x123456789ULL);
			perout->start.nsec = i & 1 ? 1234567890 : 123456789;
			perout->start.reserved = i & 2 ? 2718281828 : 0;
			perout->period.sec = 0xabcdef;
			perout->period.nsec = i & 1 ? 123456789 : 0;
			perout->period.reserved = i & 2 ? 0 : 2345678901;
			perout->index = 3141592653U;
			perout->flags= perout_flags[i].flags;
			perout->on.sec = i & 3 ? 0xabcdef0123456789ULL : 0;
			perout->on.nsec = i & 2 ? 123456789 : 0;
			perout->on.reserved = i & 4 ? 2345678901U : 0;

			sys_ioctl(-1, c->val, (uintptr_t) perout);
			printf("ioctl(-1, " XLAT_FMT ", {%s={sec=%lld"
			       ", nsec=%u%s}",
			       XLAT_SEL(c->val, c->str),
			       perout_flags[i].is_phase ? "phase" : "start",
			       (long long) perout->start.sec,
			       i & 1 ? 1234567890 : 123456789,
			       i & 2 ? ", reserved=0xa205b064" : "");
			if (!perout_flags[i].is_phase) {
				print_lltime(perout->start.sec,
					     perout->start.nsec);
			}
			printf(", period={sec=11259375, nsec=%u%s}"
			       ", index=3141592653, flags=%s",
			       i & 1 ? 123456789 : 0,
			       i & 2 ? "" : ", reserved=0x8bd03835",
			       perout_flags[i].str);
			if (perout_flags[i].is_duty_cycle) {
				printf(", on={sec=%lld, nsec=%u%s}",
				       i & 3 ? 0xabcdef0123456789ULL : 0,
				       i & 2 ? 123456789 : 0,
				       i & 4 ? ", reserved=0x8bd03835" : "");
			} else if (i && c->val == PTP_PEROUT_REQUEST2) {
				printf(", rsv=[%#x, %#x, %#x, %#x]",
				       i & 3 ? BE_LE(0xabcdef01, 0x23456789) : 0,
				       i & 3 ? BE_LE(0x23456789, 0xabcdef01) : 0,
				       i & 2 ? 123456789 : 0,
				       i & 4 ? 2345678901U : 0);
			}
			printf("}) = %s\n", errstr);
		}
	}

	/* PTP_ENABLE_PPS */
	sys_ioctl(-1, PTP_ENABLE_PPS, 0);
	printf("ioctl(-1, %s, 0) = %s\n",
	       XLAT_STR(PTP_ENABLE_PPS), errstr);
	sys_ioctl(-1, PTP_ENABLE_PPS, -1);
	printf("ioctl(-1, %s, %#lx) = %s\n",
	       XLAT_STR(PTP_ENABLE_PPS), (long int) -1, errstr);

	/* PTP_ENABLE_PPS2 */
	sys_ioctl(-1, PTP_ENABLE_PPS2, 0);
	printf("ioctl(-1, %s, 0) = %s\n",
	       XLAT_STR(PTP_ENABLE_PPS2), errstr);
	sys_ioctl(-1, PTP_ENABLE_PPS2, -123456789);
	printf("ioctl(-1, %s, %#lx) = %s\n",
	       XLAT_STR(PTP_ENABLE_PPS2), (long int) -123456789, errstr);

	/* PTP_SYS_OFFSET{,2} */
	static const struct strval32 ioc_sysoff[] = {
		{ ARG_STR(PTP_SYS_OFFSET) },
		{ ARG_STR(PTP_SYS_OFFSET2) },
	};
	for (const struct strval32 *c = ioc_sysoff; c < ARRAY_END(ioc_sysoff);
	     c++) {
		check_bad_ptr(c->val, c->str, sysoff, sizeof(*sysoff));

		memset(sysoff, 0, sizeof(*sysoff));
		rc = sys_ioctl(-1, c->val, (uintptr_t) sysoff);
		printf("ioctl(-1, " XLAT_FMT ", {n_samples=0%s}) = %s\n",
		       XLAT_SEL(c->val, c->str),
		       rc >= 0 ? ", ts=[{sec=0, nsec=0}]" : "", errstr);

		for (size_t i = 0; i < 4; i++) {
			sysoff->n_samples = i > 2 ? 0xdeadface : i * 12 + 1;
			sysoff->rsv[0] = i & 1 ? 0xbadfaced : 0;
			sysoff->rsv[2] = i & 2 ? 0xcafeface : 0;
			for (size_t j = 0; j < 2 * PTP_MAX_SAMPLES + 1; j++) {
				sysoff->ts[j].sec = SAFE_TIME_T(2345678901U + j);
				sysoff->ts[j].nsec = 999999999 - i * 12 + j;
				sysoff->ts[j].reserved = j & 1 ? 0xdeadface : 0;
			}

			rc = sys_ioctl(-1, c->val, (uintptr_t) sysoff);
			printf("ioctl(-1, " XLAT_FMT ", {n_samples=%zu%s%s%s%s",
			       XLAT_SEL(c->val, c->str),
			       i > 2 ? 0xdeadface : i * 12 + 1,
			       i & 3 ? ", rsv=[" : "",
			       i & 3 ? i & 1 ? "0xbadfaced" : "0" : "",
			       i & 3 ? ", 0, " : "",
			       i & 3 ? i & 2 ? "0xcafeface]" : "0]" : "");
			if (rc >= 0) {
				for (size_t j = 0;
				     j < MIN(i * 24 + 3,
					     XLAT_RAW || XLAT_VERBOSE ? 51
								      : 32);
				     j++) {
					printf("%s{sec=%lld, nsec=%u%s}",
					       j ? ", " : ", ts=[",
					       (long long) sysoff->ts[j].sec,
					       sysoff->ts[j].nsec,
					       j & 1 ? ", reserved=0xdeadface"
						     : "");
					print_lltime(sysoff->ts[j].sec,
						     sysoff->ts[j].nsec);
				}
				printf("%s]",
				       XLAT_RAW || XLAT_VERBOSE || i < 2
					? "" : ", ...");
			}
			printf("}) = %s\n", errstr);
		}
	}

	/* PTP_PIN_[GS]ETFUNC{,2} */
	static const struct ioc_pin {
		uint32_t is_get :1,
			 is_v2  :1;
		uint32_t val;
		const char *str;
	} ioc_pin[] = {
		{ true,  false, ARG_STR(PTP_PIN_GETFUNC) },
		{ true,  true,  ARG_STR(PTP_PIN_GETFUNC2) },
		{ false, false, ARG_STR(PTP_PIN_SETFUNC) },
		{ false, true,  ARG_STR(PTP_PIN_SETFUNC2) },
	};
	static const struct strval32 pin_funcs[] = {
		{ ENUM_KNOWN(0x1, PTP_PF_EXTTS) },
		{ ENUM_KNOWN(0x3, PTP_PF_PHYSYNC) },
		{ ARG_XLAT_UNKNOWN(0x4, "PTP_PF_???") },
		{ ARG_XLAT_UNKNOWN(0xdeadcafe, "PTP_PF_???") },
	};
	for (const struct ioc_pin *c = ioc_pin; c < ARRAY_END(ioc_pin); c++) {
		check_bad_ptr(c->val, c->str, pindesc, sizeof(*pindesc));

		memset(pindesc, 0, sizeof(*pindesc));
		rc = sys_ioctl(-1, c->val, (uintptr_t) pindesc);
		printf("ioctl(-1, " XLAT_FMT ", {index=0",
		       XLAT_SEL(c->val, c->str));
		if (rc >= 0 || !c->is_get) {
			printf("%s, func=" XLAT_FMT ", chan=0",
			       c->is_get ? ", name=\"\"" : "",
			       XLAT_ARGS(PTP_PF_NONE));
		}
		printf("}) = %s\n", errstr);

		for (size_t i = 0; i < ARRAY_SIZE(pin_funcs); i++) {
			memcpy(pindesc->name,
			       i & 1 ? "\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17"
				       "OH HAI THAR\176\177\377\0\0\0\0\0\0\0"
				       "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				       "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				     : "abcdefghijklmnopqrstuvwxyz0123456789"
				       "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
			       sizeof(pindesc->name));
			pindesc->index = 0xcafebabeU;
			pindesc->func = pin_funcs[i].val;
			pindesc->chan = 0xfeedbeefU;
			pindesc->rsv[0] = i & 1 ? 0xbadc0ded : 0;
			pindesc->rsv[4] = i & 2 ? 0 : 0xdadfaced;

			rc = sys_ioctl(-1, c->val, (uintptr_t) pindesc);
			printf("ioctl(-1, " XLAT_FMT ", {index=3405691582",
			       XLAT_SEL(c->val, c->str));
			if (c->is_get && c->is_v2 && (i & 3) != 2) {
				printf(", rsv=[%#x, 0, 0, 0, %#x]",
				       i & 1 ? 0xbadc0ded : 0,
				       i & 2 ? 0 : 0xdadfaced);
			}
			if (rc >= 0 || !c->is_get) {
				if (c->is_get) {
					printf(", name=\"%s",
					       i & 1 ? "\\1\\2\\3\\4\\5\\6\\7"
						       "\\10\\t\\n\\v\\f\\r\\16"
						       "\\17OH HAI THAR~\\177"
						       "\\377\""
						     : "abcdefghijklmnopqrstuvw"
						       "xyz0123456789ABCDEFGHIJ"
						       "KLMNOPQRSTUVWXYZ0\""
						       "...");
				}
				printf(", func=%s, chan=4276993775",
				       pin_funcs[i].str);
				if (!c->is_get && c->is_v2
				    && ((i & 1) || !(i & 2))) {
					printf(", rsv=[%#x, 0, 0, 0, %#x]",
					       i & 1 ? 0xbadc0ded : 0,
					       i & 2 ? 0 : 0xdadfaced);
				}
			}
			printf("}) = %s\n", errstr);
		}
	}

	/* PTP_SYS_OFFSET_PRECISE{,2} */
	static const struct strval32 ioc_soprec[] = {
		{ ARG_STR(PTP_SYS_OFFSET_PRECISE) },
		{ ARG_STR(PTP_SYS_OFFSET_PRECISE2) },
	};
	static const struct ptp_clock_time ts_vecs[] = {
		{ 0, 123456789 },
		{ 0x23456789, 0, 0xdeadface },
		{ SAFE_TIME_T(0x123456789ab), 1234567890 },
		{ SAFE_TIME_T(0x123456789abcd), 987654321, 0x1 },
	};
	for (const struct strval32 *c = ioc_soprec; c < ARRAY_END(ioc_soprec);
	     c++) {
		check_bad_ptr(c->val, c->str, soprec, sizeof(*soprec));

		memset(soprec, 0, sizeof(*soprec));
		rc = sys_ioctl(-1, c->val, (uintptr_t) soprec);
		printf("ioctl(-1, " XLAT_FMT ", ", XLAT_SEL(c->val, c->str));
		if (rc >= 0) {
			printf("{device={sec=0, nsec=0}"
			       ", sys_realtime={sec=0, nsec=0}"
			       ", sys_monoraw={sec=0, nsec=0}}");
		} else {
			printf("%p", soprec);
		}
		printf(") = %s\n", errstr);

		for (size_t i = 0; i < ARRAY_SIZE(ts_vecs); i++) {
			soprec->device = ts_vecs[i];
			soprec->sys_realtime = ARR_ITEM(ts_vecs, i + 1);
			soprec->sys_monoraw = ARR_ITEM(ts_vecs, i + 2);
			soprec->rsv[0] = i & 1 ? 0 : 0xbadfaced;
			soprec->rsv[3] = i & 2 ? 0 : 0xdeadbeef;

			rc = sys_ioctl(-1, c->val, (uintptr_t) soprec);
			printf("ioctl(-1, " XLAT_FMT ", ",
			       XLAT_SEL(c->val, c->str));
			if (rc >= 0) {
				printf("{device={sec=%lld, nsec=%u",
				       (long long) ts_vecs[i].sec,
				       ts_vecs[i].nsec);
				if (i & 1) {
					printf(", reserved=%#x",
					       ts_vecs[i].reserved);
				}
				printf("}");
				print_lltime(ts_vecs[i].sec, ts_vecs[i].nsec);
				printf(", sys_realtime={sec=%lld, nsec=%u",
				       (long long) ARR_ITEM(ts_vecs, i + 1).sec,
				       ARR_ITEM(ts_vecs, i + 1).nsec);
				if (!(i & 1)) {
					printf(", reserved=%#x",
					       ARR_ITEM(ts_vecs, i + 1).reserved
					       );
				}
				printf("}");
				print_lltime(ARR_ITEM(ts_vecs, i + 1).sec,
					     ARR_ITEM(ts_vecs, i + 1).nsec);
				printf(", sys_monoraw={sec=%lld, nsec=%u",
				       (long long) ARR_ITEM(ts_vecs, i + 2).sec,
				       ARR_ITEM(ts_vecs, i + 2).nsec);
				if (i & 1) {
					printf(", reserved=%#x",
					       ARR_ITEM(ts_vecs, i + 2).reserved
					       );
				}
				printf("}");
				if ((i & 3) != 3) {
					printf(", rsv=[%#x, 0, 0, %#x]",
					       i & 1 ? 0 : 0xbadfaced,
					       i & 2 ? 0 : 0xdeadbeef);
				}
				printf("}");
			} else {
				printf("%p", soprec);
			}
			printf(") = %s\n", errstr);
		}
	}

	/* PTP_SYS_OFFSET_EXTENDED{,2} */
	static const struct strval32 ioc_soext[] = {
		{ ARG_STR(PTP_SYS_OFFSET_EXTENDED) },
		{ ARG_STR(PTP_SYS_OFFSET_EXTENDED2) },
	};
	for (const struct strval32 *c = ioc_soext; c < ARRAY_END(ioc_soext);
	     c++) {
		check_bad_ptr(c->val, c->str, soext, sizeof(*soext));

		memset(soext, 0, sizeof(*soext));
		rc = sys_ioctl(-1, c->val, (uintptr_t) soext);
		printf("ioctl(-1, " XLAT_FMT ", {n_samples=0%s}) = %s\n",
		       XLAT_SEL(c->val, c->str),
		       rc >= 0 ? ", ts=[]" : "", errstr);

		for (size_t i = 0; i < 4; i++) {
			soext->n_samples = i > 2 ? 0xdeadface : i * 12 + 1;
			soext->rsv[0] = i & 1 ? 0xbadfaced : 0;
			soext->rsv[2] = i & 2 ? 0xcafeface : 0;
			for (size_t j = 0; j < PTP_MAX_SAMPLES; j++) {
				soext->ts[j][0].sec =
						SAFE_TIME_T(2345678901U + j);
				soext->ts[j][0].nsec = 999999999 - i * 12 + j;
				soext->ts[j][0].reserved = j & 1 ? 0xbee : 0;

				soext->ts[j][1].sec =
						SAFE_TIME_T(-123456780123L + j);
				soext->ts[j][1].nsec = -(i * 12) + j;
				soext->ts[j][1].reserved = j & 2 ? 0xface : 0;

				soext->ts[j][2].sec =
						SAFE_TIME_T(j * (1 << 30));
				soext->ts[j][2].nsec = j * (1 << 29);
				soext->ts[j][2].reserved = j & 4 ? 0xbabe : 0;
			}

			rc = sys_ioctl(-1, c->val, (uintptr_t) soext);
			printf("ioctl(-1, " XLAT_FMT ", {n_samples=%zu",
			       XLAT_SEL(c->val, c->str),
			       i > 2 ? 0xdeadface : i * 12 + 1);
			if (i & 3) {
				printf(", rsv=[%#x, 0, %#x]",
				       i & 1 ? 0xbadfaced : 0,
				       i & 2 ? 0xcafeface : 0);
			}
			if (rc >= 0) {
				for (size_t j = 0; j < MIN(i * 12 + 1, 25); j++)
				{
					printf("%s", j ? "], " : ", ts=[");
					for (size_t k = 0; k < 3; k++) {
						printf("%s{sec=%lld, nsec=%u",
						       k ? ", " : "[",
						       (long long) soext->ts[j][k].sec,
						       soext->ts[j][k].nsec);
						if (soext->ts[j][k].reserved)
							printf(", reserved=%#x",
							       soext->ts[j][k]
								     .reserved);
						printf("}");
						print_lltime(
							soext->ts[j][k].sec,
							soext->ts[j][k].nsec);
					}
				}
				printf("]]");
			}
			printf("}) = %s\n", errstr);
		}
	}
}

int
main(int argc, char *argv[])
{
#ifdef INJECT_RETVAL
	unsigned long num_skip;
	bool locked = false;

	if (argc < 2)
		error_msg_and_fail("Usage: %s NUM_SKIP", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);
	for (unsigned int i = 0; i < num_skip; i++) {
		long rc = sys_ioctl(-1, PTP_CLOCK_GETCAPS, 0);
		printf("ioctl(-1, %s, NULL) = %s%s\n",
		       XLAT_STR(PTP_CLOCK_GETCAPS), sprintrc(rc),
		       rc == 42 ? " (INJECTED)" : "");

		if (rc != 42)
			continue;

		locked = true;
		break;
	}

	if (!locked) {
		error_msg_and_fail("Have not locked on ioctl(-1"
				   ", PTP_CLOCK_GETCAPS, NULL) returning 42");
	}
#endif /* INJECT_RETVAL */

	test_no_device();

	puts("+++ exited with 0 +++");
	return 0;
}
