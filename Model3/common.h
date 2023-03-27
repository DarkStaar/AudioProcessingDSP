#ifndef _COMMON_H
#define _COMMON_H

#include <stdfix.h>
#include "stdfix_emu.h"

// potrebno prekopirati sa pocetka stdfix_emu.h ili ukljuciti ceo stdfix_emu.h!
#if defined(__CCC)

#include <stdfix.h>

#define FRACT_NUM(x) (x##r)
#define LONG_FRACT_NUM(x) (x##lr)
#define ACCUM_NUM(x) (x##lk)

#define FRACT_NUM_HEX(x) (x##r)

#define FRACT_TO_INT_BIT_CONV(x) (bitsr(x))
#define INT_TO_FRACT_BIT_CONV(x) (rbits(x))

#define long_accum long accum
#define long_fract long fract

#endif

/////////////////////////////////////////////////////////////////////////////////
// Constant definitions
/////////////////////////////////////////////////////////////////////////////////
#define BLOCK_SIZE 16
#define MAX_NUM_CHANNEL 3
#define MINUS_6DB FRACT_NUM(0.2511886432);
#define MINUS_3DB FRACT_NUM(0.5011872336);
#define MINUS_2DB FRACT_NUM(0.6309573445);

#define DELAY 1

#define MAX_NUM_OUT_CH		3
#define MAX_NUM_IN_CH		2
#define MAX_NUM_OUT_MODE	3

#define L		0
#define R		1
#define C		2

enum OUTPUT_MODES
{
	OM_2_0_0 = 0,
	OM_3_0_0 = 1,
	OM_1_0_0 = 2
};

/////////////////////////////////////////////////////////////////////////////////

/* DSP type definitions */
typedef short DSPshort;					/* DSP integer */
typedef unsigned short DSPushort;		/* DSP unsigned integer */
typedef int DSPint;						/* native integer */
typedef fract DSPfract;					/* DSP fixed-point fractional, data reg, memory word format s.31 */
typedef long_accum DSPaccum;					/* DSP fixed-point fractional, accum reg format s8.63 */

typedef struct
{
	__memY DSPfract* delayBuff;
	DSPint bufferLength;
	DSPint writeIndex;
	DSPint readIndex;
	DSPint delay;
} DelayBuffer;

#define PROCESSING_ASM

#endif //_COMMON_H
