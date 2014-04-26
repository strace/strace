#include "defs.h"
#include <sys/ioctl.h>
#include <linux/ptp_clock.h>

#include "xlat/ptp_flags_options.h"

int ptp_ioctl(struct tcb *tcp, long code, long arg)
{
	if (!verbose(tcp))
		return 0;

	switch (code) {
		case PTP_CLOCK_GETCAPS: /* decode on exit */
		{
			struct ptp_clock_caps caps;

			if (entering(tcp) || syserror(tcp) ||
			    umove(tcp, arg, &caps) < 0)
				return 0;

			tprintf(", {max_adj=%d, n_alarm=%d, n_ext_ts=%d, n_per_out=%d, pps=%d}",
				caps.max_adj, caps.n_alarm, caps.n_ext_ts,
				caps.n_per_out, caps.pps);
			return 1;
		}

		case PTP_EXTTS_REQUEST: /* decode on enter */
		{
			struct ptp_extts_request extts;

			if (exiting(tcp))
				return 1;
			if (umove(tcp, arg, &extts) < 0) {
				tprintf(", %#lx", arg);
				return 0;
			}
			tprintf(", {index=%d, flags=", extts.index);
			printflags(ptp_flags_options, extts.flags, "PTP_???");
			tprints("}");
			return 1;
		}

		case PTP_PEROUT_REQUEST: /* decode on enter */
		{
			struct ptp_perout_request perout;

			if (exiting(tcp))
				return 1;
			if (umove(tcp, arg, &perout) < 0) {
				tprintf(", %#lx", arg);
				return 0;
			}

			tprintf(", {start={%" PRId64 ", %" PRIu32 "}"
				   ", period={%" PRId64 ", %" PRIu32 "}"
				   ", index=%d, flags=",
				(int64_t)perout.start.sec, perout.start.nsec,
				(int64_t)perout.period.sec, perout.period.nsec,
				perout.index);
			printflags(ptp_flags_options, perout.flags, "PTP_???");
			tprints("}");
			return 1;
		}

		case PTP_ENABLE_PPS: /* decode on enter */
			if (entering(tcp))
				tprintf(", %ld", arg);
			return 1;

		case PTP_SYS_OFFSET: /* decode on exit */
		{
			struct ptp_sys_offset sysoff;
			unsigned int i;

			if (entering(tcp) || umove(tcp, arg, &sysoff) < 0)
				return 0;

			tprintf(", {n_samples=%u, ts={", sysoff.n_samples);
			if (syserror(tcp)) {
				tprints("...}}");
				return 1;
			}
			if (sysoff.n_samples > PTP_MAX_SAMPLES)
				sysoff.n_samples = PTP_MAX_SAMPLES;
			tprintf("{%" PRId64 ", %" PRIu32 "}",
				(int64_t)sysoff.ts[0].sec, sysoff.ts[0].nsec);
			for (i = 1; i < 2*sysoff.n_samples+1; ++i)
				tprintf(", {%" PRId64 ", %" PRIu32 "}",
					(int64_t)sysoff.ts[i].sec, sysoff.ts[i].nsec);
			tprints("}}");
			return 1;
		}

		default: /* decode on exit */
			return 0;
	}
}
