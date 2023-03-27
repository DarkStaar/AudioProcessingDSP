#ifndef PROCESSING_H
#define PROCESSING_H

#include "common.h"
#include "delay.h"

#ifdef __cplusplus
extern "C" {
#endif

enum OUTPUT_MODES
{
	OM_2_0_0 = 0,
	OM_3_0_0 = 1,
	OM_1_0_0 = 2
};

void processing(double input[][BLOCK_SIZE], double output[][BLOCK_SIZE], DelayBuffer* delBuff);
int get_out_num_ch(int mode);
int is_channel_active(int mode, int channel);
#ifdef __cplusplus
}
#endif

#endif