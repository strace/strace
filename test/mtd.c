#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>

int main() {
	int fd = open("/dev/null", 0);
	struct mtd_info_user minfo;
	struct erase_info_user einfo;
	struct erase_info_user64 einfo64;
	struct mtd_oob_buf mbuf;
	struct mtd_oob_buf64 mbuf64;
	struct region_info_user rinfo;
	/* struct otp_info oinfo; */
	struct mtd_ecc_stats estat;
	struct mtd_write_req mreq;
	struct nand_oobinfo ninfo;
	struct nand_ecclayout_user nlay;
	off_t f = 333;

	memset(&einfo, 0, sizeof(einfo));
	memset(&einfo64, 0xff, sizeof(einfo64));

	ioctl(fd, MEMGETINFO, &minfo);

	ioctl(fd, MEMERASE, &einfo);
	ioctl(fd, MEMERASE64, &einfo64);

	ioctl(fd, MEMGETBADBLOCK, &f);
	int i = 0;
	ioctl(fd, OTPSELECT, &i);
	ioctl(fd, MEMSETBADBLOCK, &f);

	ioctl(fd, MEMREADOOB, &mbuf);
	ioctl(fd, MEMREADOOB64, &mbuf64);

	ioctl(fd, MEMGETREGIONINFO, &rinfo);

	ioctl(fd, ECCGETSTATS, &estat);
	ioctl(fd, MEMWRITE, &mreq);

	ioctl(fd, MEMGETOOBSEL, &ninfo);
	ioctl(fd, ECCGETLAYOUT, &nlay);

	return 0;
}
