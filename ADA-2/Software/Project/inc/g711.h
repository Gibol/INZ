#ifndef G711_H
#define G711_H

#define	SIGN_BIT	(0x80)		/* Sign bit for a A-law byte. */
#define	QUANT_MASK	(0xf)		/* Quantization field mask. */
#define	NSEGS		(8)		/* Number of A-law segments. */
#define	SEG_SHIFT	(4)		/* Left shift for segment number. */
#define	SEG_MASK	(0x70)		/* Segment field mask. */
#define	BIAS		(0x84)		/* Bias for linear code. */

static int search(int val, short *table, int size);
unsigned char linear2alaw(int pcm_val);
int alaw2linear(unsigned char	a_val);
unsigned char linear2ulaw(int	pcm_val);
int ulaw2linear(unsigned char	u_val);
unsigned char alaw2ulaw(unsigned char	aval);
unsigned char ulaw2alaw(unsigned char uval);

#endif //G711_H
