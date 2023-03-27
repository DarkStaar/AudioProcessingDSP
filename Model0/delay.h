#ifndef DELAY_H
#define DELAY_H

#include "common.h"
#include <math.h>

#if __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////
// Control state structure
/////////////////////////////////////////////////////////////////////////////////
	typedef struct
	{
		double* delayBuff;
		int bufferLength;
		int writeIndex;
		int readIndex;
		int delay;
	} DelayBuffer;


	/////////////////////////////////////////////////////////////////////////////////

	double saturation(double in, double threshold);

	void delay_init(DelayBuffer* delBuff, double buffer[]);

	void delay(double inputBuffer[], double outputBuffer[], DelayBuffer* delBuff);


#if __cplusplus
}
#endif

#endif