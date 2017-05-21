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

/*
 * Tue, 30 Jun 1998 20:23:11 +0200
 * Reported by John v/d Kamp <blade_@dds.nl>:
 * I have this song from Purple Motion called wcharts.s3m, the global
 * volume was set to 0, creating a devide by 0 error in xmp. There should
 * be an extra test if it's 0 or not.
 *
 * Claudio's fix: global volume ignored
 */

/*
 * Sat, 29 Aug 1998 18:50:43 -0500 (CDT)
 * Reported by Joel Jordan <scriber@usa.net>:
 * S3M files support tempos outside the ranges defined by xmp (that is,
 * the MOD/XM tempo ranges).  S3M's can have tempos from 0 to 255 and speeds
 * from 0 to 255 as well, since these are handled with separate effects
 * unlike the MOD format.  This becomes an issue in such songs as Skaven's
 * "Catch that Goblin", which uses speeds above 0x1f.
 *
 * Claudio's fix: FX_S3M_SPEED added. S3M supports speeds from 0 to 255 and
 * tempos from 32 to 255 (S3M speed == xmp tempo, S3M tempo == xmp BPM).
 */

/* Wed, 21 Oct 1998 15:03:44 -0500  Geoff Reedy <vader21@imsa.edu>
 * It appears that xmp has issues loading/playing a specific instrument
 * used in LUCCA.S3M.
 * (Fixed by Hipolito in xmp-2.0.0dev34)
 */

/*
 * From http://code.pui.ch/2007/02/18/turn-demoscene-modules-into-mp3s/
 * The only flaw I noticed [in xmp] is a problem with portamento in Purple
 * Motion's second reality soundtrack (1:06-1:17)
 *
 * Claudio's note: that's a dissonant beating between channels 6 and 7
 * starting at pos12, caused by pitchbending effect F25.
 */

/*
 * From: Ralf Hoffmann <ralf@boomerangsworld.de>
 * Date: Wed, 26 Sep 2007 17:12:41 +0200
 * ftp://ftp.scenesp.org/pub/compilations/modplanet/normal/bonuscd/artists/
 * Iq/return%20of%20litmus.s3m doesn't start playing, just uses 100% cpu,
 * the number of patterns is unusually high
 *
 * Claudio's fix: this module seems to be a bad conversion, bad rip or
 * simply corrupted since it has many instances of 0x87 instead of 0x00
 * in the module and instrument headers. I'm adding a simple workaround
 * to be able to load/play the module as is, see the fix87() macro below.
 */

#include "loader.h"
#include "s3m.h"
#include "period.h"

#define MAGIC_SCRM	MAGIC4('S','C','R','M')
#define MAGIC_SCRI	MAGIC4('S','C','R','I')
#define MAGIC_SCRS	MAGIC4('S','C','R','S')

static int s3m_test(LIBXMP_MEM , LIBXMP_BUFFER , char *, const int);
static int s3m_load(LIBXMP_MEM , LIBXMP_BUFFER , struct module_data *, const int);

const struct format_loader libxmp_loader_s3m = {
	"Scream Tracker 3",
	s3m_test,
	s3m_load
};

static int s3m_test(LIBXMP_MEM mem, LIBXMP_BUFFER buf, char *t, const int start)
{
	libxmp_buffer_seek(buf, start + 44, SEEK_SET);
	if (libxmp_buffer_read32b(buf) != MAGIC_SCRM) {
		return -1;
	}

	libxmp_buffer_seek(buf, start + 29, SEEK_SET);
	if (libxmp_buffer_read8(buf) != 0x10) {
		return -1;
	}

	libxmp_buffer_seek(buf, start + 0, SEEK_SET);
	libxmp_read_title(buf, t, 28);

	return 0;
}

#define NONE		0xff
#define FX_S3M_EXTENDED	0xfe

#define fix87(x) do { \
	int i; for (i = 0; i < sizeof(x); i++) { \
		if (*((uint8 *)&x + i) == 0x87) *((uint8 *)&x + i) = 0; } \
	} while (0)

/* Effect conversion table */
static const uint8 fx[] = {
	NONE,
	FX_S3M_SPEED,		/* Axx  Set speed to xx (the default is 06) */
	FX_JUMP,		/* Bxx  Jump to order xx (hexadecimal) */
	FX_BREAK,		/* Cxx  Break pattern to row xx (decimal) */
	FX_VOLSLIDE,		/* Dxy  Volume slide down by y/up by x */
	FX_PORTA_DN,		/* Exx  Slide down by xx */
	FX_PORTA_UP,		/* Fxx  Slide up by xx */
	FX_TONEPORTA,		/* Gxx  Tone portamento with speed xx */
	FX_VIBRATO,		/* Hxy  Vibrato with speed x and depth y */
	FX_TREMOR,		/* Ixy  Tremor with ontime x and offtime y */
	FX_S3M_ARPEGGIO,	/* Jxy  Arpeggio with halfnote additions */
	FX_VIBRA_VSLIDE,	/* Kxy  Dual command: H00 and Dxy */
	FX_TONE_VSLIDE,		/* Lxy  Dual command: G00 and Dxy */
	NONE,
	NONE,
	FX_OFFSET,		/* Oxy  Set sample offset */
	NONE,
	FX_MULTI_RETRIG,	/* Qxy  Retrig (+volumeslide) note */
	FX_TREMOLO,		/* Rxy  Tremolo with speed x and depth y */
	FX_S3M_EXTENDED,	/* Sxx  (misc effects) */
	FX_S3M_BPM,		/* Txx  Tempo = xx (hex) */
	FX_FINE_VIBRATO,	/* Uxx  Fine vibrato */
	FX_GLOBALVOL,		/* Vxx  Set global volume */
	NONE,
	FX_SETPAN,		/* Xxx  Set pan */
	NONE,
	NONE
};

/* Effect translation */
static void xlat_fx(int c, struct xmp_event *e)
{
	uint8 h = MSN(e->fxp), l = LSN(e->fxp);

	if (e->fxt > 26) {
		D_(D_WARN "invalid effect %02x", e->fxt);
		e->fxt = e->fxp = 0;
		return;
	}

	switch (e->fxt = fx[e->fxt]) {
	case FX_S3M_BPM:
		if (e->fxp < 0x20) {
			e->fxp = e->fxt = 0;
		}
		break;
	case FX_S3M_EXTENDED:	/* Extended effects */
		e->fxt = FX_EXTENDED;
		switch (h) {
		case 0x1:	/* Glissando */
			e->fxp = LSN(e->fxp) | (EX_GLISS << 4);
			break;
		case 0x2:	/* Finetune */
			e->fxp =
			    ((LSN(e->fxp) - 8) & 0x0f) | (EX_FINETUNE << 4);
			break;
		case 0x3:	/* Vibrato wave */
			e->fxp = LSN(e->fxp) | (EX_VIBRATO_WF << 4);
			break;
		case 0x4:	/* Tremolo wave */
			e->fxp = LSN(e->fxp) | (EX_TREMOLO_WF << 4);
			break;
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:	/* Ignore */
			e->fxt = e->fxp = 0;
			break;
		case 0x8:	/* Set pan */
			e->fxt = FX_SETPAN;
			e->fxp = l << 4;
			break;
		case 0xb:	/* Pattern loop */
			e->fxp = LSN(e->fxp) | (EX_PATTERN_LOOP << 4);
			break;
		case 0xc:
			if (!l)
				e->fxt = e->fxp = 0;
		}
		break;
	case FX_SETPAN:
		/* Saga Musix says: "The X effect in S3M files is not
		 * exclusive to IT and clones. You will find tons of S3Ms made
		 * with ST3 itself using this effect (and relying on an
		 * external player being used). X in S3M also behaves
		 * differently than in IT, which your code does not seem to
		 * handle: X00 - X80 is left... right, XA4 is surround (like
		 * S91 in IT), other values are not supposed to do anything.
		 */
		if (e->fxp == 0xa4) {
			// surround
			e->fxt = FX_SURROUND;
			e->fxp = 1;
		} else {
			int pan = ((int)e->fxp) << 1;
			if (pan > 0xff) {
				pan = 0xff;
			}
			e->fxp = pan;
		}
		break;
	case NONE:		/* No effect */
		e->fxt = e->fxp = 0;
		break;
	}
}

static int s3m_load(LIBXMP_MEM mem, LIBXMP_BUFFER buf, struct module_data *m, const int start)
{
	struct xmp_module *mod = &m->mod;
	int c, r, i;
	struct xmp_event *event = 0, dummy;
	struct s3m_file_header sfh;
	struct s3m_instrument_header sih;
#ifndef LIBXMP_CORE_PLAYER
	struct s3m_adlib_header sah;
	char tracker_name[40];
	int quirk87 = 0;
#endif
	int pat_len;
	uint8 n, b;
	uint16 *pp_ins;			/* Parapointers to instruments */
	uint16 *pp_pat;			/* Parapointers to patterns */
	int ret;

	LOAD_INIT();

	libxmp_buffer_scan(buf, "s28;b8;b8;w16l;w16l;w16l;w16l;w16l;w16l;w16l;d32b;b8;b8;b8;b8;b8;b8;d32l;d32l;w16l;s32",
		&sfh.name,	/* Song name */
		NULL,		/* 0x1a */
		&sfh.type,	/* File type */
		NULL,		/* -- */
		&sfh.ordnum,	/* Number of orders (must be even) */
		&sfh.insnum,	/* Number of instruments */
		&sfh.patnum,	/* Number of patterns */
		&sfh.flags,	/* Flags */
		&sfh.version,	/* Tracker ID and version */
		&sfh.ffi,	/* File format information */
		&sfh.magic,	/* 'SCRM' */
		&sfh.gv,	/* Global volume */
		&sfh.is,	/* Initial speed */
		&sfh.it,	/* Initial tempo */
		&sfh.mv,	/* Master volume */
		&sfh.uc,	/* Ultra click removal */
		&sfh.dp,	/* Default pan positions if 0xfc */
		NULL, NULL,	/* 54-61 reserved */
		&sfh.special,	/* Ptr to special custom data */
		&sfh.chset);	/* Channel settings */

	/* Sanity check */
	if (sfh.ffi != 1 && sfh.ffi != 2) {
		D_(D_CRIT "sanity check: ffi=%d", sfh.ffi);
		return -1;
	}
	if (sfh.ordnum > 255 || sfh.insnum > 255 || sfh.patnum > 255) {
		D_(D_CRIT "sanity check: ordnum=%d insnum=%d patnum=%d", sfh.ordnum, sfh.insnum, sfh.patnum);
		return -1;
	}
	if (sfh.magic != MAGIC_SCRM) {
		D_(D_CRIT "sanity check: magic=%08x", sfh.magic);
		return -1;
	}

#ifndef LIBXMP_CORE_PLAYER
	/* S3M anomaly in return_of_litmus.s3m */
	if (sfh.version == 0x1301 && sfh.name[27] == 0x87)
		quirk87 = 1;

	if (quirk87) {
		fix87(sfh.name);
		fix87(sfh.patnum);
		fix87(sfh.flags);
	}
#endif

	libxmp_copy_adjust(mod->name, sfh.name, 28);

	pp_ins = libxmp_mem_calloc(mem, 2 * sfh.insnum);
	pp_pat = libxmp_mem_calloc(mem, 2 * sfh.patnum);

	if (sfh.flags & S3M_AMIGA_RANGE) {
		m->period_type = PERIOD_MODRNG;
	}
	if (sfh.flags & S3M_ST300_VOLS) {
		m->quirk |= QUIRK_VSALL;
	}
	/* m->volbase = 4096 / sfh.gv; */
	mod->spd = sfh.is;
	mod->bpm = sfh.it;
	mod->chn = 0;

	for (i = 0; i < 32; i++) {
		if (sfh.chset[i] == S3M_CH_OFF) {
			continue;
		}

		mod->chn = i + 1;

		if (sfh.mv & 0x80) {	/* stereo */
			int x = sfh.chset[i] & S3M_CH_PAN;
			mod->xxc[i].pan = (x & 0x0f) < 8 ? 0x30 : 0xc0;
		} else {
			mod->xxc[i].pan = 0x80;
		}
	}

	if (sfh.ordnum <= XMP_MAX_MOD_LENGTH) {
		mod->len = sfh.ordnum;
		libxmp_buffer_read(buf, mod->xxo, mod->len);
	} else {
		mod->len = XMP_MAX_MOD_LENGTH;
		libxmp_buffer_read(buf, mod->xxo, mod->len);
		libxmp_buffer_seek(buf, sfh.ordnum - XMP_MAX_MOD_LENGTH, SEEK_CUR);
	}

	/* Don't trust sfh.patnum */
	mod->pat = -1;
	for (i = 0; i < mod->len; ++i) {
		if (mod->xxo[i] < 0xfe && mod->xxo[i] > mod->pat) {
			mod->pat = mod->xxo[i];
		}
	}
	mod->pat++;
	if (mod->pat > sfh.patnum) {
		mod->pat = sfh.patnum;
	}
	if (mod->pat == 0) {
		return -1;
	}

	mod->trk = mod->pat * mod->chn;
	/* Load and convert header */
	mod->ins = sfh.insnum;
	mod->smp = mod->ins;

	for (i = 0; i < sfh.insnum; i++) {
		pp_ins[i] = libxmp_buffer_read16l(buf);
	}

	for (i = 0; i < sfh.patnum; i++) {
		pp_pat[i] = libxmp_buffer_read16l(buf);
	}

	/* Default pan positions */

	for (i = 0, sfh.dp -= 0xfc; !sfh.dp /* && n */  && (i < 32); i++) {
		uint8 x = libxmp_buffer_read8(buf);
		if (x & S3M_PAN_SET) {
			mod->xxc[i].pan = (x << 4) & 0xff;
		} else {
			mod->xxc[i].pan = sfh.mv % 0x80 ? 0x30 + 0xa0 * (i & 1) : 0x80;
		}
	}

	m->c4rate = C4_NTSC_RATE;

	if (sfh.version == 0x1300) {
		m->quirk |= QUIRK_VSALL;
	}

#ifndef LIBXMP_CORE_PLAYER
	switch (sfh.version >> 12) {
	case 1:
		snprintf(tracker_name, 40, "Scream Tracker %d.%02x",
			 (sfh.version & 0x0f00) >> 8, sfh.version & 0xff);
		m->quirk |= QUIRK_ST3BUGS;
		break;
	case 2:
		snprintf(tracker_name, 40, "Imago Orpheus %d.%02x",
			 (sfh.version & 0x0f00) >> 8, sfh.version & 0xff);
		break;
	case 3:
		if (sfh.version == 0x3216) {
			strcpy(tracker_name, "Impulse Tracker 2.14v3");
		} else if (sfh.version == 0x3217) {
			strcpy(tracker_name, "Impulse Tracker 2.14v5");
		} else {
			snprintf(tracker_name, 40, "Impulse Tracker %d.%02x",
				 (sfh.version & 0x0f00) >> 8,
				 sfh.version & 0xff);
		}
		break;
	case 5:
		snprintf(tracker_name, 40, "OpenMPT %d.%02x",
			 (sfh.version & 0x0f00) >> 8, sfh.version & 0xff);
		m->quirk |= QUIRK_ST3BUGS;
		break;
	case 4:
		if (sfh.version != 0x4100) {
			snprintf(tracker_name, 40, "Schism Tracker %d.%02x",
				 (sfh.version & 0x0f00) >> 8,
				 sfh.version & 0xff);
			break;
		}
		/* fall through */
	case 6:
		snprintf(tracker_name, 40, "BeRoTracker %d.%02x",
			 (sfh.version & 0x0f00) >> 8, sfh.version & 0xff);
		break;
	default:
		snprintf(tracker_name, 40, "unknown (%04x)", sfh.version);
	}

	libxmp_set_type(m, "%s S3M", tracker_name);
#else
	libxmp_set_type(m, "Scream Tracker 3");
	m->quirk |= QUIRK_ST3BUGS;
#endif

	MODULE_INFO();

	libxmp_init_pattern(mem, mod);

	/* Read patterns */

	D_(D_INFO "Stored patterns: %d", mod->pat);

	for (i = 0; i < mod->pat; i++) {
		libxmp_alloc_pattern_tracks(mem, mod, i, 64);

		if (pp_pat[i] == 0) {
			continue;
		}

		libxmp_buffer_seek(buf, start + pp_pat[i] * 16, SEEK_SET);
		r = 0;
		pat_len = libxmp_buffer_read16l(buf) - 2;

		while (pat_len >= 0 && r < mod->xxp[i]->rows) {
			b = libxmp_buffer_read8(buf);

			if (b == S3M_EOR) {
				r++;
				continue;
			}

			c = b & S3M_CH_MASK;
			event = c >= mod->chn ? &dummy : &EVENT(i, c, r);

			if (b & S3M_NI_FOLLOW) {
				switch (n = libxmp_buffer_read8(buf)) {
				case 255:
					n = 0;
					break;	/* Empty note */
				case 254:
					n = XMP_KEY_OFF;
					break;	/* Key off */
				default:
					n = 13 + 12 * MSN(n) + LSN(n);
				}
				event->note = n;
				event->ins = libxmp_buffer_read8(buf);
				pat_len -= 2;
			}

			if (b & S3M_VOL_FOLLOWS) {
				event->vol = libxmp_buffer_read8(buf) + 1;
				pat_len--;
			}

			if (b & S3M_FX_FOLLOWS) {
				event->fxt = libxmp_buffer_read8(buf);
				event->fxp = libxmp_buffer_read8(buf);
				xlat_fx(c, event);

				pat_len -= 2;
			}
		}
	}

	D_(D_INFO "Stereo enabled: %s", sfh.mv & 0x80 ? "yes" : "no");
	D_(D_INFO "Pan settings: %s", sfh.dp ? "no" : "yes");

	libxmp_init_instrument(mem, m);

	/* Read and convert instruments and samples */

	D_(D_INFO "Instruments: %d", mod->ins);

	for (i = 0; i < mod->ins; i++) {
		struct xmp_instrument *xxi = &mod->xxi[i];
		struct xmp_sample *xxs = &mod->xxs[i];
		struct xmp_subinstrument *sub;
		uint8 x;

		xxi->sub = libxmp_mem_calloc(mem, sizeof(struct xmp_subinstrument));

		sub = &xxi->sub[0];

		libxmp_buffer_seek(buf, start + pp_ins[i] * 16, SEEK_SET);
		sub->pan = 0x80;
		sub->sid = i;

		x = libxmp_buffer_read8(buf);

		if (x >= 2) {
#ifndef LIBXMP_CORE_PLAYER
			/* OPL2 FM instrument */

			libxmp_buffer_scan(buf, "s12;b8;s12;b8;b8;w16l;s28;d32b",
				&sah.dosname,		/* DOS file name */
				NULL,
				&sah.reg,		/* Adlib registers */
				&sah.vol,
				&sah.dsk,
				&sah.c2spd,		/* C4 speed */
				&sah.name,		/* Instrument name */
				&sah.magic);		/* 'SCRI' */

			if (sah.magic != MAGIC_SCRI) {
				D_(D_CRIT "error: FM instrument magic");
				return -1;
			}
			sah.magic = 0;

			libxmp_instrument_name(mod, i, sah.name, 28);

			xxi->nsm = 1;
			sub->vol = sah.vol;
			libxmp_c2spd_to_note(sah.c2spd, &sub->xpo, &sub->fin);
			sub->xpo += 12;
			ret = libxmp_load_sample(mem, buf, m, SAMPLE_FLAG_ADLIB, xxs, (char *)&sah.reg);
			if (ret < 0) {
				return -1;
			}

			D_(D_INFO "[%2X] %-28.28s", i, xxi->name);

			continue;
#else
			return -1;
#endif
		}

		libxmp_buffer_scan(buf, "s12;b8;w16l;d32l;d32l;d32l;b8;b8;b8;b8;w16l;w16l;d32l;w16l;w16l;d32l;s28;d32b",
			&sih.dosname,		/* DOS file name */
			NULL,			/* 0x1a */
			&sih.memseg,		/* Pointer to sample data */
			&sih.length,		/* Length */
			&sih.loopbeg,		/* Loop begin */
			&sih.loopend,		/* Loop end */
			&sih.vol,		/* Volume */
			NULL,
			&sih.pack,		/* Packing type (not used) */
			&sih.flags,		/* Loop/stereo/16bit flags */
			&sih.c2spd, NULL,	/* C4 speed */
			NULL, NULL, NULL, NULL,
			&sih.name,		/* Instrument name */
			&sih.magic);		/* 'SCRS' */

#if 0
		/* ST3 limit */
		if ((sfh.version >> 12) == 1 && sih.length > 64000)
			sih.length = 64000;
#endif

		if (sih.length > MAX_SAMPLE_SIZE) {
			D_(D_CRIT "error: length: %d", sih.length);
			return -1;
		}

		if (x == 1 && sih.magic != MAGIC_SCRS) {
			D_(D_CRIT "error: instrument magic: %08x", sih.magic);
			return -1;
		}
#ifndef LIBXMP_CORE_PLAYER
		if (quirk87) {
			fix87(sih.length);
			fix87(sih.loopbeg);
			fix87(sih.loopend);
			fix87(sih.flags);
		}
#endif

		xxs->len = sih.length;
		xxi->nsm = sih.length > 0 ? 1 : 0;
		xxs->lps = sih.loopbeg;
		xxs->lpe = sih.loopend;

		xxs->flg = sih.flags & 1 ? XMP_SAMPLE_LOOP : 0;

		if (sih.flags & 4) {
			xxs->flg |= XMP_SAMPLE_16BIT;
		}

		sub->vol = sih.vol;
		sih.magic = 0;

		libxmp_instrument_name(mod, i, sih.name, 28);

		D_(D_INFO "[%2X] %-28.28s %04x%c%04x %04x %c V%02x %5d",
		   i, mod->xxi[i].name, mod->xxs[i].len,
		   xxs->flg & XMP_SAMPLE_16BIT ? '+' : ' ',
		   xxs->lps, mod->xxs[i].lpe,
		   xxs->flg & XMP_SAMPLE_LOOP ? 'L' : ' ', sub->vol, sih.c2spd);

		libxmp_c2spd_to_note(sih.c2spd, &sub->xpo, &sub->fin);

		libxmp_buffer_seek(buf, start + 16L * sih.memseg, SEEK_SET);

		ret = libxmp_load_sample(mem, buf, m, sfh.ffi == 1 ? 0 : SAMPLE_FLAG_UNS, xxs, NULL);
		if (ret < 0) {
			return -1;
		}
	}

	m->quirk |= QUIRKS_ST3 | QUIRK_ARPMEM;
	m->read_event_type = READ_EVENT_ST3;

	return 0;
}
