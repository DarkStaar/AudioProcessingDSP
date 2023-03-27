#include "delay.h"

double saturation(double in, double threshold)
{
	if (in > threshold)
	{
		return fmin(in, threshold);
	}
	else if (in < -threshold)
	{
		return fmax(in, -threshold);
	}

	return in;
};

void delay_init(DelayBuffer* delBuff, double* buffer)
{
	int i;
	for (i = 0; i < BLOCK_SIZE; i++)
	{
		*buffer = 0.0;
		buffer++;
	}
	buffer -= BLOCK_SIZE;
	delBuff->delayBuff = buffer;
	delBuff->bufferLength = BLOCK_SIZE;
	delBuff->writeIndex = BLOCK_SIZE - 1;
	delBuff->delay = DELAY;
	delBuff->readIndex = BLOCK_SIZE - 1 - DELAY;
}

void delay(double* inputBuffer, double* outputBuffer, DelayBuffer* delBuff)
{
	int i;
	int k;

	double* DB;
	double* RI;

	double limiterThreshold = 0.9999;

	for (i = 0; i < BLOCK_SIZE; i++)
	{
		DB = delBuff->delayBuff;
		for (k = 0; k < delBuff->writeIndex; k++)
		{
			DB++;
		}
		*DB = *inputBuffer;

		delBuff->writeIndex = saturation((delBuff->writeIndex + 1) & (delBuff->bufferLength - 1), limiterThreshold);

		*outputBuffer = *inputBuffer;

		RI = delBuff->delayBuff;

		for (k = 0; k < delBuff->readIndex; k++)
		{
			RI++;
		}

		*outputBuffer = *outputBuffer + *RI;

		delBuff->readIndex = saturation((delBuff->readIndex + 1) & (delBuff->bufferLength - 1), limiterThreshold);

		outputBuffer++;
		inputBuffer++;
	}
}