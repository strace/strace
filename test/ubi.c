#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <mtd/ubi-user.h>

#define zero(x) memset(&x, 0, sizeof(x))

int main() {
	int fd = open("/dev/null", 0);
	struct ubi_mkvol_req mkvol = {
		.vol_id = 3,
		.alignment = 124,
		.bytes = 1125899906842624ULL,
		.vol_type = 3,
		.name_len = 7,
		.name = "foobar",
	};
	struct ubi_rsvol_req rsvol = {
		.bytes = 1125899906842624ULL,
		.vol_id = -3,
	};
	struct ubi_rnvol_req rnvol = {
		.count = 300,
	};
	struct ubi_attach_req attach;
	struct ubi_map_req map;
	struct ubi_set_vol_prop_req prop = {
		.property = 1,
		.value = 1125899906842624ULL,
	};
	uint64_t bytes = ((uint64_t)1 << 50) | 0x123;

	ioctl(fd, UBI_IOCMKVOL, &mkvol);
	ioctl(fd, UBI_IOCRSVOL, &rsvol);
	ioctl(fd, UBI_IOCRNVOL, &rnvol);
	ioctl(fd, UBI_IOCATT, &attach);
	ioctl(fd, UBI_IOCVOLUP, &bytes);
	ioctl(fd, UBI_IOCEBMAP, &map);
	ioctl(fd, UBI_IOCSETVOLPROP, &prop);
	zero(prop);
	ioctl(fd, UBI_IOCSETVOLPROP, &prop);
	ioctl(fd, UBI_IOCRMVOL, 1);
	ioctl(fd, UBI_IOCDET, 2);
	ioctl(fd, UBI_IOCEBER, 3);
	ioctl(fd, UBI_IOCEBCH, 4);
	ioctl(fd, UBI_IOCEBUNMAP, 5);
	ioctl(fd, UBI_IOCEBISMAP, 6);

	return 0;
}
