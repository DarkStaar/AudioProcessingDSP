#include "delay.h"

static const double threshold = 0.9999;

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

void delay_init(DelayBuffer* delBuff, double buffer[])
{
	int i;
	for (i = 0; i < BLOCK_SIZE; i++)
	{
		buffer[i] = 0.00;
	}

	delBuff->delayBuff = buffer;
	delBuff->bufferLength = BLOCK_SIZE;
	delBuff->writeIndex = BLOCK_SIZE - 1;
	delBuff->delay = DELAY;
	delBuff->readIndex = BLOCK_SIZE - 1 - DELAY;
}

void delay(double inputBuffer[], double outputBuffer[], DelayBuffer* delBuff)
{
	for (int i = 0; i < BLOCK_SIZE; i++)
	{
		delBuff->delayBuff[delBuff->writeIndex] = inputBuffer[i];
		delBuff->writeIndex = saturation((delBuff->writeIndex + 1) % delBuff->bufferLength, threshold);

		outputBuffer[i] = inputBuffer[i];

		outputBuffer[i] += delBuff->delayBuff[delBuff->readIndex];
		delBuff->readIndex = saturation((delBuff->readIndex + 1) % delBuff->bufferLength, threshold);
	}
}