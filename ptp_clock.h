#ifndef STRACE_PTP_CLOCK_H
#define STRACE_PTP_CLOCK_H

#include "stdint.h"

#ifndef PTP_CLK_MAGIC
# define PTP_CLK_MAGIC '='
#else
# if PTP_CLK_MAGIC != '='
#  error  "Unexpected value of PTP_CLK_MAGIC"
# endif
#endif

struct strace_ptp_clock_time {
        int64_t  sec;
        uint32_t nsec;
        uint32_t reserved;
};

#define strace_ptp_clock_time_size \
	sizeof(struct strace_ptp_clock_time)
#define expected_strace_ptp_clock_time_size 16

struct strace_ptp_clock_caps {
        int max_adj;
        int n_alarm;
        int n_ext_ts;
        int n_per_out;
        int pps;
        int n_pins;
        int cross_timestamping;
        int rsv[13];
};

#define strace_ptp_clock_caps_size \
	sizeof(struct strace_ptp_clock_caps)
#define expected_strace_ptp_clock_caps_size 80

struct strace_ptp_extts_request {
        unsigned int index;
        unsigned int flags;
        unsigned int rsv[2];
};

#define strace_ptp_extts_request_size \
	sizeof(struct strace_ptp_extts_request)
#define expected_strace_ptp_extts_request_size 16

struct strace_ptp_perout_request {
        struct strace_ptp_clock_time start;
        struct strace_ptp_clock_time period;
        unsigned int index;
        unsigned int flags;
        unsigned int rsv[4];
};

#define strace_ptp_perout_request_size \
	sizeof(struct strace_ptp_perout_request)
#define expected_strace_ptp_perout_request_size 56

#ifndef PTP_MAX_SAMPLES
# define PTP_MAX_SAMPLES 25
#else
# if PTP_MAX_SAMPLES != 25
#  error "Unexpected value of PTP_MAX_SAMPLES"
# endif
#endif

struct strace_ptp_sys_offset {
        unsigned int n_samples;
        unsigned int rsv[3];
        struct strace_ptp_clock_time ts[2 * PTP_MAX_SAMPLES + 1];
};

#define strace_ptp_sys_offset_size \
	sizeof(struct strace_ptp_sys_offset)
#define expected_strace_ptp_sys_offset_size 832

struct strace_ptp_sys_offset_precise {
        struct strace_ptp_clock_time device;
        struct strace_ptp_clock_time sys_realtime;
        struct strace_ptp_clock_time sys_monoraw;
        unsigned int rsv[4];
};

#define strace_ptp_sys_offset_precise_size \
	sizeof(struct strace_ptp_sys_offset_precise)
#define expected_strace_ptp_sys_offset_precise_size 64

struct strace_ptp_pin_desc {
        char name[64];
        unsigned int index;
        unsigned int func;
        unsigned int chan;
        unsigned int rsv[5];
};

#define strace_ptp_pin_desc_size \
	sizeof(struct strace_ptp_pin_desc)
#define expected_strace_ptp_pin_desc_size 96

struct strace_ptp_extts_event {
        struct strace_ptp_clock_time t;
        unsigned int index;
        unsigned int flags;
        unsigned int rsv[2];
};

#define strace_ptp_extts_event_size \
	sizeof(struct strace_ptp_extts_event)
#define expected_strace_ptp_extts_event_size 32

#endif /* STRACE_PTP_CLOCK_H */
