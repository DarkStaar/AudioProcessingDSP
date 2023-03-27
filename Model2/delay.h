#ifndef DELAY_H
#define DELAY_H

#include "common.h"

#if __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////
// Control state structure
/////////////////////////////////////////////////////////////////////////////////
	typedef struct
	{
		DSPfract* delayBuff;
		DSPint bufferLength;
		DSPint writeIndex;
		DSPint readIndex;
		DSPint delay;
	} DelayBuffer;


	/////////////////////////////////////////////////////////////////////////////////


	void delay_init(DelayBuffer* delBuff, DSPfract* buffer, const DSPint bufLen, const DSPint delay);

	void delay(DSPfract* inputBuffer, DSPfract* outputBuffer, DSPint inputLength, DelayBuffer* delBuff);


#if __cplusplus
}
#endif

#endif