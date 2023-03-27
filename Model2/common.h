#ifndef COMMON_H
#define COMMON_H

#include "fixed_point_math.h"
#include "stdfix_emu.h"

#define MINUS_6DB FRACT_NUM(0.2511886432);
#define MINUS_3DB FRACT_NUM(0.5011872336);
#define MINUS_2DB FRACT_NUM(0.6309573445);

#define MAX_NUM_OUT_CH		3
#define MAX_NUM_IN_CH		2
#define MAX_NUM_OUT_MODE	3

#define L		0
#define R		1
#define C		2

#define DELAY 1

enum OUTPUT_MODES
{
	OM_2_0_0 = 0,
	OM_3_0_0 = 1,
	OM_1_0_0 = 2
};

typedef short DSPshort;
typedef unsigned short DSPushort;
typedef int DSPint;
typedef fract DSPfract;
typedef long_accum DSPaccum;
typedef unsigned int DSPuint;
typedef float DSPfloat;

#endif