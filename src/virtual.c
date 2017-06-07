/* Extended Module Player
 * Copyright (C) 1996-2016 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "common.h"
#include "virtual.h"
#include "mixer.h"

#ifdef LIBXMP_PAULA_SIMULATOR
#include "paula.h"
#endif

#define	FREE	-1

/* For virt_pastnote() */
void libxmp_player_set_release(struct context_data *, int);
void libxmp_player_set_fadeout(struct context_data *, int);


/* Get parent channel */
int libxmp_virt_getroot(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	int voc;

	voc = p->virt.virt_channel[chn].map;
	if (voc < 0) {
		return -1;
	}

	return libxmp_mixer_getroot(ctx->s, voc);
}

void libxmp_virt_resetvoice(struct context_data *ctx, int voc, int mute)
{
	struct player_data *p = &ctx->p;
	int root, chn;

	if ((uint32)voc >= p->virt.maxvoc) {
		return;
	}

	if (mute) {
		libxmp_mixer_setvol(ctx->s, voc, 0);
	}

	root = libxmp_mixer_getroot(ctx->s, voc);
	chn = libxmp_mixer_getchn(ctx->s, voc);

	p->virt.virt_used--;
	p->virt.virt_channel[root].count--;
	p->virt.virt_channel[chn].map = FREE;

	libxmp_mixer_resetvoice(ctx->s, voc);
}

/* virt_on (number of tracks) */
int libxmp_virt_on(struct context_data *ctx, int num, int has_virt, int has_paula)
{
	struct player_data *p = &ctx->p;
	struct mixer_data *s = ctx->s;
	LIBXMP_MM mem = p->mem;
	int i;

	p->virt.num_tracks = num;
	num = libxmp_mixer_numvoices(ctx->s, -1);

	p->virt.virt_channels = p->virt.num_tracks;

	if (has_virt) {
		p->virt.virt_channels += num;
	} else if (num > p->virt.virt_channels) {
		num = p->virt.virt_channels;
	}

	p->virt.maxvoc = libxmp_mixer_numvoices(ctx->s, num);
	s->voice = libxmp_mm_calloc(mem, p->virt.maxvoc * sizeof(struct mixer_voice));
	p->virt.nna = libxmp_mm_calloc(mem, p->virt.maxvoc);

	for (i = 0; i < p->virt.maxvoc; i++) {
		s->voice[i].chn = FREE;
		s->voice[i].root = FREE;
	}

#ifdef LIBXMP_PAULA_SIMULATOR
	/* Initialize Paula simulator */
	if (has_paula) {
		for (i = 0; i < p->virt.maxvoc; i++) {
			s->voice[i].paula = libxmp_mm_calloc(mem, sizeof (struct paula_state));
			libxmp_paula_init(ctx, s->voice[i].paula, s->freq);
		}
	}
#endif

	p->virt.virt_channel = libxmp_mm_alloc(mem, p->virt.virt_channels * sizeof(struct virt_channel));

	for (i = 0; i < p->virt.virt_channels; i++) {
		p->virt.virt_channel[i].map = FREE;
		p->virt.virt_channel[i].count = 0;
	}

	p->virt.virt_used = 0;

	return 0;
}

void libxmp_virt_off(struct context_data *ctx)
{
	struct player_data *p = &ctx->p;

	p->virt.virt_used = p->virt.maxvoc = 0;
	p->virt.virt_channels = 0;
	p->virt.num_tracks = 0;
}

void libxmp_virt_reset(struct context_data *ctx)
{
	struct player_data *p = &ctx->p;
	int i;

	if (p->virt.virt_channels < 1) {
		return;
	}

	/* CID 129203 (#1 of 1): Useless call (USELESS_CALL)
	 * Call is only useful for its return value, which is ignored. 
	 *
	 * libxmp_mixer_numvoices(ctx, p->virt.maxvoc);
	 */

	for (i = 0; i < p->virt.maxvoc; i++) {
		libxmp_mixer_resetvoice(ctx->s, i);
	}

	for (i = 0; i < p->virt.virt_channels; i++) {
		p->virt.virt_channel[i].map = FREE;
		p->virt.virt_channel[i].count = 0;
	}

	p->virt.virt_used = 0;
}

static int free_voice(struct context_data *ctx)
{
	struct player_data *p = &ctx->p;
	struct mixer_data *s =ctx->s;
	int i, num, vol;

	/* Find background voice with lowest volume*/
	num = FREE;
	vol = INT_MAX;
	for (i = 0; i < p->virt.maxvoc; i++) {
		struct mixer_voice *vi = &s->voice[i];

		if (vi->chn >= p->virt.num_tracks && vi->vol < vol) {
			num = i;
			vol = vi->vol;
		}
	}

	/* Free voice */
	if (num >= 0) {
		p->virt.virt_channel[s->voice[num].chn].map = FREE;
		p->virt.virt_channel[s->voice[num].root].count--;
		p->virt.virt_used--;
	}

	return num;
}

static int alloc_voice(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	struct mixer_data *s = ctx->s;
	int i;

	/* Find free voice */
	for (i = 0; i < p->virt.maxvoc; i++) {
		if (s->voice[i].chn == FREE)
			break;
	}

	/* not found */
	if (i == p->virt.maxvoc) {
		i = free_voice(ctx);
	}

	if (i >= 0) {
		p->virt.virt_channel[chn].count++;
		p->virt.virt_used++;

		s->voice[i].chn = chn;
		s->voice[i].root = chn;
		p->virt.virt_channel[chn].map = i;
	}

	return i;
}

static int map_virt_channel(struct player_data *p, int chn)
{
	int voc;

	if ((uint32)chn >= p->virt.virt_channels)
		return -1;

	voc = p->virt.virt_channel[chn].map;

	if ((uint32)voc >= p->virt.maxvoc)
		return -1;

	return voc;
}

int libxmp_virt_mapchannel(struct context_data *ctx, int chn)
{
	return map_virt_channel(&ctx->p, chn);
}

void libxmp_virt_resetchannel(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	int root;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0)
		return;

	root = libxmp_mixer_getroot(ctx->s, voc);

	p->virt.virt_used--;
	p->virt.virt_channel[root].count--;
	p->virt.virt_channel[chn].map = FREE;

	libxmp_mixer_setvol(ctx->s, voc, 0);
	libxmp_mixer_resetvoice(ctx->s, voc);
}

void libxmp_virt_setvol(struct context_data *ctx, int chn, int vol)
{
	struct player_data *p = &ctx->p;
	int voc, root;

	if ((voc = map_virt_channel(p, chn)) < 0) {
		return;
	}

	root = libxmp_mixer_getroot(ctx->s, voc);
	if (root < XMP_MAX_CHANNELS && p->channel_mute[root]) {
		vol = 0;
	}

	libxmp_mixer_setvol(ctx->s, voc, vol);

	if (vol == 0 && chn >= p->virt.num_tracks) {
		libxmp_virt_resetvoice(ctx, voc, 1);
	}
}

void libxmp_virt_release(struct context_data *ctx, int chn, int rel)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0) {
		return;
	}

	libxmp_mixer_voicerelease(ctx->s, voc, rel);
}

void libxmp_virt_setpan(struct context_data *ctx, int chn, int pan)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0) {
		return;
	}

	libxmp_mixer_setpan(ctx->s, voc, pan);
}

void libxmp_virt_seteffect(struct context_data *ctx, int chn, int type, int val)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0) {
		return;
	}

	libxmp_mixer_seteffect(ctx->s, voc, type, val);
}

double libxmp_virt_getvoicepos(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0) {
		return -1;
	}

	return libxmp_mixer_getvoicepos(ctx->s, voc, ctx->m.mod.xxs);
}

#ifndef LIBXMP_CORE_PLAYER

void libxmp_virt_setsmp(struct context_data *ctx, int chn, int smp)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct mixer_data *s = ctx->s;
	struct mixer_voice *vi;
	double pos;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0) {
		return;
	}

	vi = &s->voice[voc];
	if (vi->smp == smp) {
		return;
	}

	pos = libxmp_mixer_getvoicepos(s, voc, ctx->m.mod.xxs);
	libxmp_mixer_setpatch(ctx, voc, smp, 0);
	libxmp_mixer_voicepos(s, voc, pos, 0, IS_PLAYER_MODE_IT(), ctx->m.mod.xxs);	/* Restore old position */
}

#endif

#ifndef LIBXMP_CORE_DISABLE_IT

void libxmp_virt_setnna(struct context_data *ctx, int chn, int nna)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0) {
		return;
	}

	p->virt.nna[voc] = nna;
}

static void check_dct(struct context_data *ctx, int i, int chn, int ins,
			int smp, int note, int nna, int dct, int dca)
{
	struct player_data *p = &ctx->p;
	struct mixer_data *s = ctx->s;
	struct mixer_voice *vi = &s->voice[i];
	int voc;

	voc = p->virt.virt_channel[chn].map;

	if (vi->root == chn && vi->ins == ins) {

		if (nna == XMP_INST_NNA_CUT) {
		    libxmp_virt_resetvoice(ctx, i, 1);
		    return;
		}

		p->virt.nna[i] = nna;

		if ((dct == XMP_INST_DCT_INST) ||
		    (dct == XMP_INST_DCT_SMP && vi->smp == smp) ||
		    (dct == XMP_INST_DCT_NOTE && vi->note == note)) {

			if (nna == XMP_INST_NNA_OFF && dca == XMP_INST_DCA_FADE) {
				p->virt.nna[i] = VIRT_ACTION_OFF;
			} else if (dca) {
				if (i != voc || p->virt.nna[i]) {
					p->virt.nna[i] = dca;
				}
			} else {
				libxmp_virt_resetvoice(ctx, i, 1);
			}
		}
	}
}

#endif

/* For note slides */
void libxmp_virt_setnote(struct context_data *ctx, int chn, int note)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0) {
		return;
	}

	libxmp_mixer_setnote(ctx->s, voc, note);
}

int libxmp_virt_setpatch(struct context_data *ctx, int chn, int ins, int smp,
		    			int note, int nna, int dct, int dca)
{
	struct player_data *p = &ctx->p;
	struct mixer_data *s = ctx->s;
	int voc, vfree;

	if ((uint32)chn >= p->virt.virt_channels) {
		return -1;
	}

	if (ins < 0) {
		smp = -1;
	}

#ifndef LIBXMP_CORE_DISABLE_IT
	if (dct) {
		int i;

		for (i = 0; i < p->virt.maxvoc; i++) {
			check_dct(ctx, i, chn, ins, smp, note, nna, dct, dca);
		}
	}
#endif

	voc = p->virt.virt_channel[chn].map;

	if (voc > FREE) {
		if (p->virt.nna[voc]) {
			vfree = alloc_voice(ctx, chn);

			if (vfree < 0) {
				return -1;
			}

			for (chn = p->virt.num_tracks;
			     p->virt.virt_channel[chn++].map > FREE;) ;

			s->voice[voc].chn = --chn;
			p->virt.virt_channel[chn].map = voc;
			voc = vfree;
		}
	} else {
		voc = alloc_voice(ctx, chn);
		if (voc < 0) {
			return -1;
		}
	}

	if (smp < 0) {
		libxmp_virt_resetvoice(ctx, voc, 1);
		return chn;	/* was -1 */
	}

	libxmp_mixer_setpatch(ctx, voc, smp, 1);
	libxmp_mixer_setnote(ctx->s, voc, note);
	s->voice[voc].ins = ins;
	p->virt.nna[voc] = nna;

	return chn;
}

void libxmp_virt_setperiod(struct context_data *ctx, int chn, double period)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0) {
		return;
	}

	libxmp_mixer_setperiod(ctx->s, voc, period);
}

void libxmp_virt_voicepos(struct context_data *ctx, int chn, double pos)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0) {
		return;
	}

	libxmp_mixer_voicepos(ctx->s, voc, pos, 1, IS_PLAYER_MODE_IT(), ctx->m.mod.xxs);
}

#ifndef LIBXMP_CORE_DISABLE_IT

void libxmp_virt_pastnote(struct context_data *ctx, int chn, int act)
{
	struct player_data *p = &ctx->p;
	struct mixer_data *s = ctx->s;
	int c, voc;

	for (c = p->virt.num_tracks; c < p->virt.virt_channels; c++) {
		if ((voc = map_virt_channel(p, c)) < 0)
			continue;

		if (s->voice[voc].root == chn) {
			switch (act) {
			case VIRT_ACTION_CUT:
				libxmp_virt_resetvoice(ctx, voc, 1);
				break;
			case VIRT_ACTION_OFF:
				libxmp_player_set_release(ctx, c);
				break;
			case VIRT_ACTION_FADE:
				libxmp_player_set_fadeout(ctx, c);
				break;
			}
		}
	}
}

#endif

int libxmp_virt_cstat(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0) {
		return VIRT_INVALID;
	}

	if (chn < p->virt.num_tracks) {
		return VIRT_ACTIVE;
	}

	return p->virt.nna[voc];
}
