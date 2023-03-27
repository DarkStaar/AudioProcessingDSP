#ifndef PROCESSING_H
#define PROCESSING_H

#include "common.h"
#include "delay.h"

#ifdef __cplusplus
extern "C" {
#endif



void processing(double input[][BLOCK_SIZE], double output[][BLOCK_SIZE], int mode, int enable, DelayBuffer* delBuff);

#ifdef __cplusplus
}
#endif

#endif