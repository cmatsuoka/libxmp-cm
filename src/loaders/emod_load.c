/* Extended Module Player
 * Copyright (C) 1996-2012 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU General Public License. See doc/COPYING
 * for more information.
 */

#include <stdio.h>
#include "load.h"
#include "iff.h"

#define MAGIC_FORM	MAGIC4('F','O','R','M')
#define MAGIC_EMOD	MAGIC4('E','M','O','D')


static int emod_test (FILE *, char *, const int);
static int emod_load (struct xmp_context *, FILE *, const int);

struct xmp_loader_info emod_loader = {
    "EMOD",
    "Quadra Composer",
    emod_test,
    emod_load
};

static int emod_test(FILE *f, char *t, const int start)
{
    if (read32b(f) != MAGIC_FORM)
	return -1;

    read32b(f);

    if (read32b(f) != MAGIC_EMOD)
	return -1;

    read_title(f, t, 0);

    return 0;
}


static uint8 *reorder;


static void get_emic(struct xmp_context *ctx, int size, FILE *f)
{
    struct xmp_mod_context *m = &ctx->m;
    struct xmp_module *mod = &m->mod;
    int i, ver;

    ver = read16b(f);
    fread(mod->name, 1, 20, f);
    fseek(f, 20, SEEK_CUR);
    mod->bpm = read8(f);
    mod->ins = read8(f);
    mod->smp = mod->ins;

    mod->flg |= XXM_FLG_MODRNG;

    snprintf(mod->type, XMP_NAMESIZE, "EMOD v%d (Quadra Composer)", ver);
    MODULE_INFO();

    INSTRUMENT_INIT();

    for (i = 0; i < mod->ins; i++) {
	mod->xxi[i].sub = calloc(sizeof (struct xmp_subinstrument), 1);

	read8(f);		/* num */
	mod->xxi[i].sub[0].vol = read8(f);
	mod->xxs[i].len = 2 * read16b(f);
	fread(mod->xxi[i].name, 1, 20, f);
	mod->xxs[i].flg = read8(f) & 1 ? XMP_SAMPLE_LOOP : 0;
	mod->xxi[i].sub[0].fin = read8(f);
	mod->xxs[i].lps = 2 * read16b(f);
	mod->xxs[i].lpe = mod->xxs[i].lps + 2 * read16b(f);
	read32b(f);		/* ptr */

	mod->xxi[i].nsm = 1;
	mod->xxi[i].sub[0].pan = 0x80;
	mod->xxi[i].sub[0].sid = i;

	_D(_D_INFO "[%2X] %-20.20s %05x %05x %05x %c V%02x %+d",
		i, mod->xxi[i].name, mod->xxs[i].len, mod->xxs[i].lps,
		mod->xxs[i].lpe, mod->xxs[i].flg & XMP_SAMPLE_LOOP ? 'L' : ' ',
		mod->xxi[i].sub[0].vol, mod->xxi[i].sub[0].fin >> 4);
    }

    read8(f);			/* pad */
    mod->pat = read8(f);

    mod->trk = mod->pat * mod->chn;

    PATTERN_INIT();

    reorder = calloc(1, 256);

    for (i = 0; i < mod->pat; i++) {
	reorder[read8(f)] = i;
	PATTERN_ALLOC(i);
	mod->xxp[i]->rows = read8(f) + 1;
	TRACK_ALLOC(i);
	fseek(f, 20, SEEK_CUR);		/* skip name */
	read32b(f);			/* ptr */
    }

    mod->len = read8(f);

    _D(_D_INFO "Module length: %d", mod->len);

    for (i = 0; i < mod->len; i++)
	mod->xxo[i] = reorder[read8(f)];
}


static void get_patt(struct xmp_context *ctx, int size, FILE *f)
{
    struct xmp_mod_context *m = &ctx->m;
    struct xmp_module *mod = &m->mod;
    int i, j, k;
    struct xmp_event *event;
    uint8 x;

    _D(_D_INFO "Stored patterns: %d", mod->pat);

    for (i = 0; i < mod->pat; i++) {
	for (j = 0; j < mod->xxp[i]->rows; j++) {
	    for (k = 0; k < mod->chn; k++) {
		event = &EVENT(i, k, j);
		event->ins = read8(f);
		event->note = read8(f) + 1;
		if (event->note != 0)
		    event->note += 36;
		event->fxt = read8(f) & 0x0f;
		event->fxp = read8(f);

		/* Fix effects */
		switch (event->fxt) {
		case 0x04:
		    x = event->fxp;
		    event->fxp = (x & 0xf0) | ((x << 1) & 0x0f);
		    break;
		case 0x09:
		    event->fxt <<= 1;
		    break;
		case 0x0b:
		    x = event->fxt;
		    event->fxt = 16 * (x / 10) + x % 10;
		    break;
		}
	    }
	}
    }
}


static void get_8smp(struct xmp_context *ctx, int size, FILE *f)
{
    struct xmp_mod_context *m = &ctx->m;
    struct xmp_module *mod = &m->mod;
    int i;

    _D(_D_INFO, "Stored samples : %d ", mod->smp);

    for (i = 0; i < mod->smp; i++) {
	load_sample(ctx, f, i, 0, &mod->xxs[i], NULL);
    }
}


static int emod_load(struct xmp_context *ctx, FILE *f, const int start)
{
    struct xmp_mod_context *m = &ctx->m;

    LOAD_INIT();

    read32b(f);		/* FORM */
    read32b(f);
    read32b(f);		/* EMOD */

    /* IFF chunk IDs */
    iff_register("EMIC", get_emic);
    iff_register("PATT", get_patt);
    iff_register("8SMP", get_8smp);

    /* Load IFF chunks */
    while (!feof(f))
	iff_chunk(ctx, f);

    iff_release();
    free(reorder);

    return 0;
}
