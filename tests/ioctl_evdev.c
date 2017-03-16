/*
 * This file is part of ioctl_evdev strace test.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tests.h"

#ifdef HAVE_LINUX_INPUT_H

# include <errno.h>
# include <inttypes.h>
# include <stdio.h>
# include <string.h>
# include <sys/ioctl.h>
# include <linux/input.h>

static const unsigned int magic = 0xdeadbeef;
static const unsigned long lmagic = (unsigned long) 0xdeadbeefbadc0dedULL;

# if VERBOSE
static void
print_envelope(const struct ff_envelope *const e)
{
	printf(", envelope={attack_length=%hu, attack_level=%hu"
	       ", fade_length=%hu, fade_level=%#hx}",
	       e->attack_length, e->attack_level,
	       e->fade_length, e->fade_level);
}
# endif /* VERBOSE */

static void
print_ffe_common(const struct ff_effect *const ffe, const char *const type_str)
{
	printf("ioctl(-1, EVIOCSFF, {type=%s, id=%" PRIu16
	       ", direction=%" PRIu16 ", ",
	       type_str, ffe->id, ffe->direction);
# if VERBOSE
	printf("trigger={button=%hu, interval=%hu}"
	       ", replay={length=%hu, delay=%hu}",
	       ffe->trigger.button, ffe->trigger.interval,
	       ffe->replay.length, ffe->replay.delay);
# endif /* VERBOSE */
}

# define TEST_NULL_ARG(cmd) \
	ioctl(-1, cmd, 0); \
	printf("ioctl(-1, %s, NULL) = -1 EBADF (%m)\n", #cmd)

int
main(void)
{
	TEST_NULL_ARG(EVIOCGVERSION);
	TEST_NULL_ARG(EVIOCGEFFECTS);
	TEST_NULL_ARG(EVIOCGID);
	TEST_NULL_ARG(EVIOCGKEYCODE);
	TEST_NULL_ARG(EVIOCSKEYCODE);
	TEST_NULL_ARG(EVIOCSFF);
# ifdef EVIOCGKEYCODE_V2
	TEST_NULL_ARG(EVIOCGKEYCODE_V2);
# endif
# ifdef EVIOCSKEYCODE_V2
	TEST_NULL_ARG(EVIOCSKEYCODE_V2);
# endif
# ifdef EVIOCGREP
	TEST_NULL_ARG(EVIOCGREP);
# endif
# ifdef EVIOCSREP
	TEST_NULL_ARG(EVIOCSREP);
# endif
# ifdef EVIOCSCLOCKID
	TEST_NULL_ARG(EVIOCSCLOCKID);
# endif

	TEST_NULL_ARG(EVIOCGNAME(0));
	TEST_NULL_ARG(EVIOCGPHYS(0));
	TEST_NULL_ARG(EVIOCGUNIQ(0));
	TEST_NULL_ARG(EVIOCGKEY(0));
	TEST_NULL_ARG(EVIOCGLED(0));
# ifdef EVIOCGMTSLOTS
	TEST_NULL_ARG(EVIOCGMTSLOTS(0));
# endif
# ifdef EVIOCGPROP
	TEST_NULL_ARG(EVIOCGPROP(0));
# endif
	TEST_NULL_ARG(EVIOCGSND(0));
# ifdef EVIOCGSW
	TEST_NULL_ARG(EVIOCGSW(0));
# endif

	TEST_NULL_ARG(EVIOCGABS(ABS_X));
	TEST_NULL_ARG(EVIOCSABS(ABS_X));

	TEST_NULL_ARG(EVIOCGBIT(EV_SYN, 0));
	TEST_NULL_ARG(EVIOCGBIT(EV_KEY, 1));
	TEST_NULL_ARG(EVIOCGBIT(EV_REL, 2));
	TEST_NULL_ARG(EVIOCGBIT(EV_ABS, 3));
	TEST_NULL_ARG(EVIOCGBIT(EV_MSC, 4));
# ifdef EV_SW
	TEST_NULL_ARG(EVIOCGBIT(EV_SW, 5));
# endif
	TEST_NULL_ARG(EVIOCGBIT(EV_LED, 6));
	TEST_NULL_ARG(EVIOCGBIT(EV_SND, 7));
	TEST_NULL_ARG(EVIOCGBIT(EV_REP, 8));
	TEST_NULL_ARG(EVIOCGBIT(EV_FF, 9));
	TEST_NULL_ARG(EVIOCGBIT(EV_PWR, 10));
	TEST_NULL_ARG(EVIOCGBIT(EV_FF_STATUS, 11));

	ioctl(-1, EVIOCGBIT(EV_MAX, 42), 0);
	printf("ioctl(-1, EVIOCGBIT(%#x /* EV_??? */, 42), NULL)"
	       " = -1 EBADF (%m)\n", EV_MAX);

	ioctl(-1, EVIOCRMFF, lmagic);
	printf("ioctl(-1, EVIOCRMFF, %d) = -1 EBADF (%m)\n", (int) lmagic);

	ioctl(-1, EVIOCGRAB, lmagic);
	printf("ioctl(-1, EVIOCGRAB, %lu) = -1 EBADF (%m)\n", lmagic);

# ifdef EVIOCREVOKE
	ioctl(-1, EVIOCREVOKE, lmagic);
	printf("ioctl(-1, EVIOCREVOKE, %lu) = -1 EBADF (%m)\n", lmagic);
# endif

	const unsigned int size = get_page_size();
	void *const page = tail_alloc(size);
	fill_memory(page, size);

	TAIL_ALLOC_OBJECT_CONST_PTR(int, val_int);
	*val_int = magic;

# ifdef EVIOCSCLOCKID
	ioctl(-1, EVIOCSCLOCKID, val_int);
	printf("ioctl(-1, EVIOCSCLOCKID, [%u]) = -1 EBADF (%m)\n", *val_int);
# endif

	int *pair_int = tail_alloc(sizeof(*pair_int) * 2);
	pair_int[0] = 0xdeadbeef;
	pair_int[1] = 0xbadc0ded;

# ifdef EVIOSGREP
	ioctl(-1, EVIOCSREP, pair_int);
	printf("ioctl(-1, EVIOCSREP, [%u, %u]) = -1 EBADF (%m)\n",
	       pair_int[0], pair_int[1]);
# endif

	pair_int[1] = 1;
	ioctl(-1, EVIOCSKEYCODE, pair_int);
	printf("ioctl(-1, EVIOCSKEYCODE, [%u, %s]) = -1 EBADF (%m)\n",
	       pair_int[0], "KEY_ESC");

# ifdef EVIOCSKEYCODE_V2
	TAIL_ALLOC_OBJECT_CONST_PTR(struct input_keymap_entry, ike);
	fill_memory(ike, sizeof(*ike));
	ike->keycode = 2;

	ioctl(-1, EVIOCSKEYCODE_V2, ike);
	printf("ioctl(-1, EVIOCSKEYCODE_V2, {flags=%" PRIu8
	       ", len=%" PRIu8 ", ", ike->flags, ike->len);
#  if VERBOSE
	printf("index=%" PRIu16 ", keycode=%s, scancode=[",
	       ike->index, "KEY_1");
	unsigned int i;
	for (i = 0; i < ARRAY_SIZE(ike->scancode); ++i) {
		if (i > 0)
			printf(", ");
		printf("%" PRIx8, ike->scancode[i]);
	}
	printf("]");
#  else
	printf("...");
#  endif
	errno = EBADF;
	printf("}) = -1 EBADF (%m)\n");
# endif

	TAIL_ALLOC_OBJECT_CONST_PTR(struct ff_effect, ffe);
	fill_memory(ffe, sizeof(*ffe));

	ffe->type = FF_CONSTANT;
	ioctl(-1, EVIOCSFF, ffe);
	print_ffe_common(ffe, "FF_CONSTANT");

#  if VERBOSE
	printf(", constant={level=%hd", ffe->u.constant.level);
	print_envelope(&ffe->u.constant.envelope);
	printf("}");
#  else
	printf("...");
#  endif
	errno = EBADF;
	printf("}) = -1 EBADF (%m)\n");

#  if VERBOSE
	ffe->type = FF_RAMP;
	ioctl(-1, EVIOCSFF, ffe);
	print_ffe_common(ffe, "FF_RAMP");
	printf(", ramp={start_level=%hd, end_level=%hd",
	       ffe->u.ramp.start_level, ffe->u.ramp.end_level);
	print_envelope(&ffe->u.ramp.envelope);
	errno = EBADF;
	printf("}}) = -1 EBADF (%m)\n");

	ffe->type = FF_PERIODIC;
	ioctl(-1, EVIOCSFF, ffe);
	print_ffe_common(ffe, "FF_PERIODIC");
	printf(", periodic={waveform=%hu, period=%hu, magnitude=%hd"
	       ", offset=%hd, phase=%hu",
	       ffe->u.periodic.waveform, ffe->u.periodic.period,
	       ffe->u.periodic.magnitude, ffe->u.periodic.offset,
	       ffe->u.periodic.phase);
	print_envelope(&ffe->u.periodic.envelope);
	printf(", custom_len=%u, custom_data=%p}",
	       ffe->u.periodic.custom_len, ffe->u.periodic.custom_data);
	errno = EBADF;
	printf("}) = -1 EBADF (%m)\n");

	ffe->type = FF_RUMBLE;
	ioctl(-1, EVIOCSFF, ffe);
	print_ffe_common(ffe, "FF_RUMBLE");
	printf(", rumble={strong_magnitude=%hu, weak_magnitude=%hu}",
	       ffe->u.rumble.strong_magnitude, ffe->u.rumble.weak_magnitude);
	errno = EBADF;
	printf("}) = -1 EBADF (%m)\n");

	ffe->type = 0xff;
	ioctl(-1, EVIOCSFF, ffe);
	print_ffe_common(ffe, "0xff /* FF_??? */");
	errno = EBADF;
	printf("}) = -1 EBADF (%m)\n");
#  endif

	ioctl(-1, _IOC(_IOC_READ, 0x45, 0x1, 0xff), lmagic);
	printf("ioctl(-1, %s, %#lx) = -1 EBADF (%m)\n",
	       "_IOC(_IOC_READ, 0x45, 0x1, 0xff)", lmagic);

	ioctl(-1, _IOC(_IOC_WRITE, 0x45, 0x1, 0xff), lmagic);
	printf("ioctl(-1, %s, %#lx) = -1 EBADF (%m)\n",
	       "_IOC(_IOC_WRITE, 0x45, 0x1, 0xff)", lmagic);

	ioctl(-1, _IOC(_IOC_READ|_IOC_WRITE, 0x45, 0xfe, 0xff), lmagic);
	printf("ioctl(-1, %s, %#lx) = -1 EBADF (%m)\n",
	       "_IOC(_IOC_READ|_IOC_WRITE, 0x45, 0xfe, 0xff)", lmagic);

	ioctl(-1, _IOC(_IOC_READ|_IOC_WRITE, 0x45, 0, 0), lmagic);
	printf("ioctl(-1, %s, %#lx) = -1 EBADF (%m)\n",
	       "_IOC(_IOC_READ|_IOC_WRITE, 0x45, 0, 0)", lmagic);

	puts("+++ exited with 0 +++");
	return 0;
}
#else

SKIP_MAIN_UNDEFINED("HAVE_LINUX_INPUT_H")

#endif
