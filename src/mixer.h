#ifndef LIBXMP_MIXER_H
#define LIBXMP_MIXER_H

#define C4_PERIOD	428.0

#define SMIX_NUMVOC	128	/* default number of softmixer voices */
#define SMIX_SHIFT	16
#define SMIX_MASK	0xffff

#define FILTER_SHIFT	16
#define ANTICLICK_SHIFT	3

#ifdef LIBXMP_PAULA_SIMULATOR
#include "paula.h"
#endif

#include "mm.h"

#define MIXER(f) void libxmp_mix_##f(struct mixer_voice *vi, int *buffer, \
	int count, int vl, int vr, int step, int ramp, int delta_l, int delta_r)

struct mixer_voice {
	int chn;		/* channel number */
	int root;		/* */
	int note;		/* */
#define PAN_SURROUND 0x8000
	int pan;		/* */
	int vol;		/* */
	double period;		/* current period */
	double pos;		/* position in sample */
	int pos0;		/* position in sample before mixing */
	int fidx;		/* mixer function index */
	int ins;		/* instrument number */
	int smp;		/* sample number */
	int end;		/* loop end */
	//int act;		/* nna info & status of voice */
	int old_vl;		/* previous volume, left channel */
	int old_vr;		/* previous volume, right channel */
	int sleft;		/* last left sample output, in 32bit */
	int sright;		/* last right sample output, in 32bit */
#define VOICE_RELEASE	(1 << 0)
#define ANTICLICK	(1 << 1)	
#define SAMPLE_LOOP	(1 << 2)
	int flags;		/* flags */
	void *sptr;		/* sample pointer */
#ifdef LIBXMP_PAULA_SIMULATOR
	struct paula_state *paula; /* paula simulation state */
#endif

#ifndef LIBXMP_CORE_DISABLE_IT
	struct {
		int r1;		/* filter variables */
		int r2;
		int l1;
		int l2;
		int a0;
		int b0;
		int b1;
		int cutoff;
		int resonance;
	} filter;
#endif
};

struct mixer_data {
	int freq;		/* sampling rate */
	int format;		/* sample format */
	int amplify;		/* amplification multiplier */
	int mix;		/* percentage of channel separation */
	int interp;		/* interpolation type */
	int dsp;		/* dsp effect flags */
	char* buffer;		/* output buffer */
	int32* buf32;		/* temporary buffer for 32 bit samples */
	int numvoc;		/* default softmixer voices number */
	int ticksize;
	int dtright;		/* anticlick control, right channel */
	int dtleft;		/* anticlick control, left channel */
	double pbase;		/* period base */

	struct mixer_voice *voice;
};

struct mixer_data *libxmp_mixer_new		(LIBXMP_MM);
void	libxmp_mixer_on		(struct mixer_data *, int, int, int);
void	libxmp_mixer_off	(struct mixer_data *);
void    libxmp_mixer_setvol	(struct mixer_data *, int, int);
void    libxmp_mixer_seteffect	(struct mixer_data *, int, int, int);
void    libxmp_mixer_setpan	(struct mixer_data *, int, int);
int	libxmp_mixer_numvoices	(struct mixer_data *, int);
void	libxmp_mixer_softmixer	(struct context_data *);
void	libxmp_mixer_reset	(struct context_data *);
void	libxmp_mixer_setpatch	(struct context_data *, int, int, int);
void	libxmp_mixer_voicepos	(struct context_data *, int, double, int);
double	libxmp_mixer_getvoicepos(struct context_data *, int);
void	libxmp_mixer_setnote	(struct mixer_data *, int, int);
void	libxmp_mixer_setperiod	(struct mixer_data *, int, double);
void	libxmp_mixer_voicerelease(struct mixer_data *, int, int);

int	libxmp_mixer_getroot	(struct mixer_data *, int);
int	libxmp_mixer_getchn	(struct mixer_data *, int);
void	libxmp_mixer_resetvoice	(struct mixer_data *, int);

#endif /* LIBXMP_MIXER_H */
