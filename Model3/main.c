#include <stdio.h>
#include <dsplib\wavefile.h>
#include <stdfix.h>
#include <string.h>
#include "common.h"

__memY DelayBuffer delayBuff;
__memY DSPfract delBuff[BLOCK_SIZE];

DSPfract limiterThreshold = FRACT_NUM(0.999);

DSPfract saturation(DSPfract in)
{
	DSPaccum inValue = in;

	if (in > (DSPaccum)limiterThreshold)
	{
		return (DSPaccum)limiterThreshold;
	}
	else if (in < (DSPaccum)-limiterThreshold)
	{
		return (DSPaccum)-limiterThreshold;
	}

	return inValue;
};

//#ifdef PROCESSING_ASM
//extern void delay_init(__memY DelayBuffer* delBuff,__memY DSPfract* buffer);
//#else
void delay_init(__memY DelayBuffer* delBuff,__memY DSPfract* buffer)
{
	DSPint i;
	for (i = 0; i < BLOCK_SIZE; i++)
	{
		*buffer = FRACT_NUM(0.0);
		buffer++;
	}
	buffer -= BLOCK_SIZE;
	delBuff->delayBuff = buffer;
	delBuff->bufferLength = BLOCK_SIZE;
	delBuff->writeIndex = BLOCK_SIZE - 1;
	delBuff->delay = DELAY;
	delBuff->readIndex = BLOCK_SIZE - 1 - DELAY;
};
//#endif

//#ifdef PROCESSING_ASM
//extern void delay(__memY DSPfract* inputBuffer,__memY DSPfract* outputBuffer, DSPint inputLength,__memY DelayBuffer* delBuff);
//#else
void delay(__memY DSPfract* inputBuffer,__memY DSPfract* outputBuffer, DSPint inputLength,__memY DelayBuffer* delBuff)
{
	DSPint i;
	DSPint k;

	__memY DSPfract* DB;
	__memY DSPfract* RI;

	for (i = 0; i < inputLength; i++)
	{
		DB = delBuff->delayBuff;
		for (k = 0; k < delBuff->writeIndex; k++)
		{
			DB++;
		}
		*DB = *inputBuffer;

		delBuff->writeIndex = saturation((delBuff->writeIndex + 1) & (delBuff->bufferLength - 1));

		*outputBuffer = *inputBuffer;

		RI = delBuff->delayBuff;

		for (k = 0; k < delBuff->readIndex; k++)
		{
			RI++;
		}

		*outputBuffer = *outputBuffer + *RI;

		delBuff->readIndex = saturation((delBuff->readIndex + 1) & (delBuff->bufferLength - 1));

		outputBuffer++;
		inputBuffer++;
	}
};
//#endif
DSPfract inputGain;
DSPfract headroomGain;
DSPfract afterDelayGain;

#ifdef PROCESSING_ASM
extern void processing(__memY DSPfract input[][BLOCK_SIZE],__memY  DSPfract output[][BLOCK_SIZE],__memY  DelayBuffer* delBuff);
#else
void processing(__memY DSPfract input[][BLOCK_SIZE],__memY DSPfract output[][BLOCK_SIZE],__memY DelayBuffer* delBuff)
{
	__memY DSPfract* inputL = *(input + L);
	__memY DSPfract* inputR = *(input + R);

	__memY DSPfract* outputL = *(output + L);
	__memY DSPfract* outputR = *(output + R);
	__memY DSPfract* outputC = *(output + C);

	DSPaccum accumL;
	DSPaccum accumR;
	DSPaccum accumC;

	DSPint i;

	for (i = 0; i < BLOCK_SIZE; i++)
	{
		accumL = (DSPfract)*inputL * inputGain;

		*outputL = saturation(accumL);

		accumR = (DSPfract)*inputR * inputGain;
		*outputR = saturation(accumR);

		outputL++;
		outputR++;
		inputL++;
		inputR++;
	}

	outputL = *(output + L);
	outputR = *(output + R);

	for (i = 0; i < BLOCK_SIZE; i++)
	{
		*outputC = (DSPfract)*outputL + (DSPfract)*outputR;

		accumC = (DSPfract)*outputC * headroomGain;

		*outputC = saturation(accumC);

		outputC++;
		outputL++;
		outputR++;
	}

	outputC = *(output + C);
	outputL = *(output + L);
	outputR = *(output + R);

	delay(outputL, outputL, BLOCK_SIZE, delBuff);
	delay(outputR, outputR, BLOCK_SIZE, delBuff);

	for (i = 0; i < BLOCK_SIZE; i++)
	{
		accumL = (DSPfract)*outputL * afterDelayGain;

		*outputL = saturation(accumL);

		outputL++;
	}
};
#endif

__memY DSPfract sampleBuffer[MAX_NUM_CHANNEL][BLOCK_SIZE];
int main(int argc, char *argv[])
 {
    WAVREAD_HANDLE *wav_in;
    WAVWRITE_HANDLE *wav_out;

	char WavInputName[256];
	char WavOutputName[256];

    DSPint inChannels;
    DSPint outChannels;
    DSPint bitsPerSample;
    DSPint sampleRate;

    DSPint mode;
    DSPint iNumSamples;
	DSPint OUTPUT_NUM_CHANNELS;


	DSPint delay;

    DSPint i;
    DSPint j;

	mode = OM_2_0_0;
	inputGain = MINUS_6DB;
	headroomGain = MINUS_3DB;
	afterDelayGain = MINUS_2DB;
	OUTPUT_NUM_CHANNELS = 2;

	delay_init(&delayBuff, delBuff);

	// Init channel buffers
	for(i=0; i<MAX_NUM_CHANNEL; i++)
		for(j=0; j<BLOCK_SIZE; j++)
			sampleBuffer[i][j] = FRACT_NUM(0.0);

	// Open input wav file
	//-------------------------------------------------
	strcpy(WavInputName,"C:\\Users\\Milan\\Desktop\\4_1\\AADSP2\\VE-1\\VE-1\\TestStreams\\speech_2ch.wav");
	wav_in = cl_wavread_open(WavInputName);
	 if(wav_in == NULL)
    {
        printf("Error: Could not open wavefile.\n");
        return -1;
    }
	//-------------------------------------------------

	// Read input wav header
	//-------------------------------------------------
	inChannels = cl_wavread_getnchannels(wav_in);
    bitsPerSample = cl_wavread_bits_per_sample(wav_in);
    sampleRate = cl_wavread_frame_rate(wav_in);
    iNumSamples =  cl_wavread_number_of_frames(wav_in);
	//-------------------------------------------------

	// Open output wav file
	//-------------------------------------------------
	strcpy(WavOutputName,"C:\\Users\\Milan\\Desktop\\4_1\\AADSP2\\VE-1\\VE-1\\TestStreams\\model3.wav");
	outChannels = OUTPUT_NUM_CHANNELS;
	wav_out = cl_wavwrite_open(WavOutputName, bitsPerSample, outChannels, sampleRate);
	if(!wav_out)
    {
        printf("Error: Could not open wavefile.\n");
        return -1;
    }
	//-------------------------------------------------
	switch(mode)
	{
	case OM_2_0_0:
		OUTPUT_NUM_CHANNELS = 2;
		break;
	case OM_3_0_0:
		OUTPUT_NUM_CHANNELS = 3;
		break;
	case OM_1_0_0:
			OUTPUT_NUM_CHANNELS = 1;
			break;
	default:
		OUTPUT_NUM_CHANNELS = 2;
		break;
	}
	// Processing loop
	//-------------------------------------------------
    {
		int i;
		int j;
		int k;
		int sample;

		// exact file length should be handled correctly...
		for(i=0; i<iNumSamples/BLOCK_SIZE; i++)
		{
			for(j=0; j<BLOCK_SIZE; j++)
			{
				for(k=0; k<inChannels; k++)
				{
					sample = cl_wavread_recvsample(wav_in);
        			sampleBuffer[k][j] = rbits(sample);
				}
			}

			processing(sampleBuffer, sampleBuffer, &delayBuff);

			for(j=0; j<BLOCK_SIZE; j++)
			{
				for(k=0; k<outChannels; k++)
				{
					int channel = 0;
					switch(mode)
					{
					case OM_2_0_0:
						if (k == 0) channel = L;
						if (k == 1) channel = R;
						break;
					case OM_1_0_0:
						if (k == 0) channel = C;
						break;
					case OM_3_0_0:
						if (k == 0) channel = L;
						if (k == 1) channel = R;
						if (k == 2) channel = C;
						break;
					default:
						if (k == 0) channel = L;
						if (k == 1) channel = R;
						break;
					}
					sample = bitsr(sampleBuffer[channel][j]);
					cl_wavwrite_sendsample(wav_out, sample);
				}
			}
		}
	}

	// Close files
	//-------------------------------------------------
    cl_wavread_close(wav_in);
    cl_wavwrite_close(wav_out);
	//-------------------------------------------------

    return 0;
 }
