#include "processing.h"

#define MAX_NUM_OUT_CH		3
#define MAX_NUM_IN_CH		2
#define MAX_NUM_OUT_MODE	3

#define L		0
#define R		1
#define C		2

double inputGain = (0.2511886432);
double headroomGain = (0.5011872336);
double afterDelayLeft = (0.6309573445);

static const int outputChMask[MAX_NUM_OUT_MODE][MAX_NUM_OUT_CH] =
{
	//L  R  C
	{ 1, 1, 0 },	// 2.0.0b
	{ 1, 1, 1 },	// 3.0.0
	{ 0, 0, 1 }		// 1.0.0
};

int get_out_num_ch(int mode)
{
	int retVal = 0;

	for (int i = 0; i < MAX_NUM_OUT_CH; i++)
	{
		retVal += outputChMask[mode][i];
	}

	return retVal;
};

int is_channel_active(int mode, int channel)
{
	return outputChMask[mode][channel];
};


void processing(double input[][BLOCK_SIZE], double output[][BLOCK_SIZE], DelayBuffer* delBuff)
{
	double* inputL = *(input + L);
	double* inputR = *(input + R);

	double* outputL = *(output + L);
	double* outputR = *(output + R);
	double* outputC = *(output + C);

	for (int i = 0; i < BLOCK_SIZE; i++)
	{
		*outputL = (*inputL) * inputGain;
		*outputR = (*inputR) * inputGain;

		inputL++;
		inputR++;
		outputL++;
		outputR++;
	}

	outputL -= BLOCK_SIZE;
	outputR -= BLOCK_SIZE;

	for (int i = 0; i < BLOCK_SIZE; i++)
	{
		*outputC = (*outputL + *outputR) * headroomGain;

		outputC++;
		outputL++;
		outputR++;
	}

	outputC -= BLOCK_SIZE;
	outputL -= BLOCK_SIZE;
	outputR -= BLOCK_SIZE;

	delay(outputL, outputL, delBuff);
	delay(outputR, outputR, delBuff);

	for (int i = 0; i < BLOCK_SIZE; i++)
	{
		*outputL = (*outputL) * afterDelayLeft;

		outputL++;
	}
	
};
