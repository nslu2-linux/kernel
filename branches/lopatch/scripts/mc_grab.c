/*
 * mc_grab.c  - grabs IXP4XX microcode from a binary datastream
 * e.g. The redboot bootloader....
 *
 * usage: mc_grab 1010200 2010200 < /dev/mtd/0 > /dev/misc/npe
 *
 * Copyright (C) 2006 Christian Hohnstaedt <chohnstaedt@innominate.com>
 *
 * This file is released under the GPLv2
 */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <byteswap.h>

#define MAX_IMG 6

#define DL_MAGIC 0xfeedf00d
#define DL_MAGIC_SWAP 0x0df0edfe

int need_swap = 0;

#define mswap(x) ((need_swap) ? bswap_32(x):(x))

static void print_mc_info(unsigned id, int siz)
{
	unsigned idx;
	unsigned char buf[sizeof(unsigned)];
	const char *names[] = { "IXP425", "IXP465", "unknown" };
	char *endian =
#if __BYTE_ORDER == __LITTLE_ENDIAN
		(need_swap)
#else
		(!need_swap)
#endif
		? "BE" : "LE";

	*(unsigned*)buf = ntohl(mswap(id));

	idx = (buf[0] >> 4) < 2 ? (buf[0] >> 4) : 2;
	fprintf(stderr, "Device: %s:NPE_%c Func: %2x Rev: %02x.%02x "
		"Size: %5d bytes ID:%08x (%s)\n", names[idx], (buf[0] & 0xf)+'A',
		buf[1], buf[2], buf[3], mswap(siz)*4, mswap(id), endian);
}

int main(int argc, char *argv[])
{
	int i, idx, j;
	unsigned magic;
	unsigned id, my_ids[MAX_IMG+1], siz, mysize;
	int ret=1, verbose=0;

	for (i=0, j=0; i<argc-1 && j<MAX_IMG; i++) {
		if (!strcmp(argv[i+1], "-v"))
			verbose = 1;
		else
			my_ids[j++] = htonl(strtoul(argv[i+1], NULL, 16));
	}
	my_ids[j] = 0;
	if (my_ids[0] == 0 && !verbose) {
		fprintf(stderr, "Usage: %s <-v> [ID1] [ID2] [IDn]\n", argv[0]);
		return 1;
	}

	while ((ret = read(0, &magic, sizeof(magic))) == sizeof(magic)) {
		need_swap = 0;
		if (magic == DL_MAGIC_SWAP)
			need_swap = 1;
		else if (magic != DL_MAGIC)
			continue;

		if ((ret = read(0, &id, sizeof(id))) != sizeof(id) )
			break;

		if ((ret = read(0, &siz, sizeof(siz))) != sizeof(siz) )
			break;

		if (verbose)
			print_mc_info(id, siz);

		for(idx=0; my_ids[idx]; idx++)
			if (mswap(id) == my_ids[idx])
				break;
		if (!my_ids[idx])
			continue;

		if (!verbose)
			print_mc_info(id, siz);

		write(1, &magic, sizeof(magic));
		write(1, &id, sizeof(id));
		write(1, &siz, sizeof(siz));
		mysize = mswap(siz) * 4;
		while (mysize > 0) {
			char b[1024];
			ret = read(0, b, mysize > 1024 ? 1024:mysize);
			if (ret <= 0)
				break;
			write(1, b, ret);
			mysize -= ret;
		}
		if (mysize) {
			fprintf(stderr, "Image %d bytes too short\n", mysize);
			ret = -1;
			break;
		}
		my_ids[idx] = 1; /* invalid id */
	}
	if (ret<0)
		fprintf(stderr, "Error reading  Microcode\n");
	return ret;
}
