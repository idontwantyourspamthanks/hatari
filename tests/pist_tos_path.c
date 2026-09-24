/*
  Confirms the in-process core boots the ROM path it was given.

  Two images differ only in the TOS version word. Whichever path is passed
  is the image whose version comes back. There is no built-in EmuTOS to
  fall back on.
*/

#include "pist_libretro_abi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int pist_hatari_tos_version(void);

static int write_rom(const char *path, unsigned version)
{
	const size_t bytes = 512 * 1024;
	unsigned char *image = calloc(1, bytes);
	FILE *out;
	if (!image)
		return 1;
	image[2] = (unsigned char)(version >> 8);
	image[3] = (unsigned char)version;
	/* TOS address 0x00e00000 */
	image[8] = 0x00;
	image[9] = 0xe0;
	image[10] = 0x00;
	image[11] = 0x00;
	/* 'ETOS' — Hatari treats a 512 KiB image with this mark as EmuTOS and
	 * does not reject it for the machine type. The version word above is
	 * still the one read back. */
	image[0x2c] = 'E';
	image[0x2d] = 'T';
	image[0x2e] = 'O';
	image[0x2f] = 'S';
	out = fopen(path, "wb");
	if (!out) {
		free(image);
		return 1;
	}
	if (fwrite(image, 1, bytes, out) != bytes) {
		fclose(out);
		free(image);
		return 1;
	}
	fclose(out);
	free(image);
	return 0;
}

static int boot(const char *path, int expect)
{
	PistHatariSession session;
	char err[512];
	int version;

	memset(&session, 0, sizeof(session));
	session.abi = PIST_HATARI_ABI;
	session.tosPath = path;
	session.machine = "st";
	session.memSizeMiB = 1;
	err[0] = '\0';

	if (pist_hatari_start(&session, err, (int)sizeof(err)) != 0) {
		fprintf(stderr, "start failed for %s: %s\n", path, err);
		return 1;
	}
	version = pist_hatari_tos_version();
	pist_hatari_stop();
	if (version != expect) {
		fprintf(stderr, "%s loaded version %04x, expected %04x\n",
		        path, version, expect);
		return 1;
	}
	printf("%s -> TOS %04x\n", path, version);
	return 0;
}

int main(void)
{
	PistHatariSession session;
	char err[512];
	const char *a = "pist-tos-a.img";
	const char *b = "pist-tos-b.img";

	memset(&session, 0, sizeof(session));
	session.abi = PIST_HATARI_ABI;
	session.machine = "st";
	session.memSizeMiB = 1;
	err[0] = '\0';
	if (pist_hatari_start(&session, err, (int)sizeof(err)) == 0) {
		fprintf(stderr, "start succeeded with no ROM\n");
		pist_hatari_stop();
		return 1;
	}
	if (!strstr(err, "No TOS ROM")) {
		fprintf(stderr, "empty ROM error was: %s\n", err);
		return 1;
	}

	if (write_rom(a, 0x0104) || write_rom(b, 0x0206)) {
		fprintf(stderr, "could not write ROM images\n");
		return 1;
	}
	if (boot(a, 0x0104) || boot(b, 0x0206))
		return 1;

	remove(a);
	remove(b);
	return 0;
}
