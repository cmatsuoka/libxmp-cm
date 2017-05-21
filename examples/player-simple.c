/* A simple frontend for libxmp */
/* This file is in public domain */

#include <stdio.h>
#include <stdlib.h>
#include <xmp.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sound.h"

static void display_data(struct xmp_module_info *mi, struct xmp_frame_info *fi)
{
	printf("%3d/%3d %3d/%3d\r",
	       fi->pos, mi->mod->len, fi->row, fi->num_rows);

	fflush(stdout);
}

int main(int argc, char **argv)
{
	xmp_context ctx;
	struct xmp_module_info mi;
	struct xmp_frame_info fi;
	int row, i;

	if (sound_init(44100, 2) < 0) {
		fprintf(stderr, "%s: can't initialize sound\n", argv[0]);
		exit(1);
	}

	ctx = xmp_create_context();

	for (i = 1; i < argc; i++) {
		int fd;
		void *addr;
		struct stat st;

		/* mmap mod file */
		if ((fd = open(argv[i], O_RDONLY)) < 0) {
			fprintf(stderr, "%s: can't open file\n", argv[0]);
			exit(1);
		}

		if (fstat(fd, &st) < 0) {
			fprintf(stderr, "%s: can't stat file\n", argv[0]);
			exit(1);
		}

		addr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
		if (addr == MAP_FAILED) {
			fprintf(stderr, "%s: can't mmap file\n", argv[0]);
			exit(1);
		}

		if (xmp_load_module(ctx, addr, st.st_size) < 0) {
			fprintf(stderr, "%s: error loading %s\n", argv[0],
				argv[i]);
			continue;
		}

		if (xmp_start_player(ctx, 44100, 0) == 0) {

			/* Show module data */

			xmp_get_module_info(ctx, &mi);
			printf("%s (%s)\n", mi.mod->name, mi.mod->type);

			/* Play module */

			row = -1;
			while (xmp_play_frame(ctx) == 0) {
				xmp_get_frame_info(ctx, &fi);
				if (fi.loop_count > 0)
					break;

				sound_play(fi.buffer, fi.buffer_size);
			
				if (fi.row != row) {
					display_data(&mi, &fi);
					row = fi.row;
				}
			}
			xmp_end_player(ctx);
		}

		xmp_release_module(ctx);
		printf("\n");
	}

	xmp_free_context(ctx);

	sound_deinit();

	return 0;
}
