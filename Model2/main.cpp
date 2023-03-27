
#include <stdlib.h>
#include <string.h> 
#include "WAVheader.h"
#include "common.h"

#define BLOCK_SIZE 16
#define MAX_NUM_CHANNEL 3

typedef struct
{
	DSPfract* delayBuff;
	DSPint bufferLength;
	DSPint writeIndex;
	DSPint readIndex;
	DSPint delay;
} DelayBuffer;

static DSPfract limiterThreshold = FRACT_NUM(0.999);

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
}

void delay_init(DelayBuffer* delBuff, DSPfract* buffer)
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
}

void delay(DSPfract* inputBuffer, DSPfract* outputBuffer, DelayBuffer* delBuff)
{
	DSPint i;
	DSPint k;

	DSPfract* DB;
	DSPfract* RI;

	for (i = 0; i < BLOCK_SIZE; i++)
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
}

static const DSPint outputChMask[MAX_NUM_OUT_MODE][MAX_NUM_OUT_CH] =
{
	//L  R  C
	{ 1, 1, 0 },	// 2.0.0
	{ 1, 1, 1 },	// 3.0.0
	{ 0, 0, 1 }		// 1.0.0
};

DSPint get_out_num_ch(DSPint mode)
{
	DSPint retVal = 0;

	DSPint i;
	for (i = 0; i < MAX_NUM_OUT_CH; i++)
	{
		retVal += outputChMask[mode][i];
	}

	return retVal;
};

DSPint is_channel_active(DSPint mode, DSPint channel)
{
	return outputChMask[mode][channel];
};

static DSPfract sampleBuffer[MAX_NUM_CHANNEL][BLOCK_SIZE];
static const DSPfract inputGain = MINUS_6DB;
static const DSPfract headroomGain = MINUS_3DB;
static const DSPfract afterDelayGain = MINUS_2DB;

void processing(DSPfract input[][BLOCK_SIZE], DSPfract output[][BLOCK_SIZE], DelayBuffer* delBuff)
{
	DSPfract* inputL = *(input + L);
	DSPfract* inputR = *(input + R);

	DSPfract* outputL = *(output + L);
	DSPfract* outputR = *(output + R);
	DSPfract* outputC = *(output + C);

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

	delay(outputL, outputL, delBuff);
	delay(outputR, outputR, delBuff);

	for (i = 0; i < BLOCK_SIZE; i++)
	{
		accumL = (DSPfract)*outputL * afterDelayGain;

		*outputL = saturation(accumL);

		outputL++;
	}
};

int main(int argc, char* argv[])
{
	FILE* wav_in = NULL;
	FILE* wav_out = NULL;
	char WavInputName[256];
	char WavOutputName[256];
	WAV_HEADER inputWAVhdr, outputWAVhdr;


	DelayBuffer delayBuff;
	DSPfract delBuff[BLOCK_SIZE];

	delay_init(&delayBuff, delBuff);

	// Init channel buffers
	DSPint i;
	DSPint j;
	for (i = 0; i < MAX_NUM_CHANNEL; i++)
	{
		for (j = 0; j < BLOCK_SIZE; j++)
		{
			sampleBuffer[i][j] = FRACT_NUM(0.0);
		}
	}

	DSPint mode = atoi(argv[3]);

	// Open input and output wav files
	//-------------------------------------------------
	strcpy(WavInputName, argv[1]);
	wav_in = OpenWavFileForRead(WavInputName, "rb");
	strcpy(WavOutputName, argv[2]);
	wav_out = OpenWavFileForRead(WavOutputName, "wb");
	//-------------------------------------------------

	// Read input wav header
	//-------------------------------------------------
	ReadWavHeader(wav_in, inputWAVhdr);
	//-------------------------------------------------

	// Set up output WAV header
	//-------------------------------------------------	
	outputWAVhdr = inputWAVhdr;
	outputWAVhdr.fmt.NumChannels = get_out_num_ch(mode); // change number of channels

	DSPint oneChannelSubChunk2Size = inputWAVhdr.data.SubChunk2Size / inputWAVhdr.fmt.NumChannels;
	DSPint oneChannelByteRate = inputWAVhdr.fmt.ByteRate / inputWAVhdr.fmt.NumChannels;
	DSPint oneChannelBlockAlign = inputWAVhdr.fmt.BlockAlign / inputWAVhdr.fmt.NumChannels;

	outputWAVhdr.data.SubChunk2Size = oneChannelSubChunk2Size * outputWAVhdr.fmt.NumChannels;
	outputWAVhdr.fmt.ByteRate = oneChannelByteRate * outputWAVhdr.fmt.NumChannels;
	outputWAVhdr.fmt.BlockAlign = oneChannelBlockAlign * outputWAVhdr.fmt.NumChannels;


	// Write output WAV header to file
	//-------------------------------------------------
	WriteWavHeader(wav_out, outputWAVhdr);


	// Processing loop
	//-------------------------------------------------	
	{
		DSPint sample;
		DSPint BytesPerSample = inputWAVhdr.fmt.BitsPerSample / 8;
		const double SAMPLE_SCALE = -(double)(1 << 31);		//2^31
		DSPint iNumSamples = inputWAVhdr.data.SubChunk2Size / (inputWAVhdr.fmt.NumChannels * inputWAVhdr.fmt.BitsPerSample / 8);

		// exact file length should be handled correctly...
		DSPint k;
		for (i = 0; i < iNumSamples / BLOCK_SIZE; i++)
		{
			for (j = 0; j < BLOCK_SIZE; j++)
			{
				for (k = 0; k < inputWAVhdr.fmt.NumChannels; k++)
				{
					sample = 0; //debug
					fread(&sample, BytesPerSample, 1, wav_in);
					sample = sample << (32 - inputWAVhdr.fmt.BitsPerSample); // force signextend
					sampleBuffer[k][j] = sample / SAMPLE_SCALE;				// scale sample to 1.0/-1.0 range		
				}
			}

			if (atoi(argv[4]))
			{
				processing(sampleBuffer, sampleBuffer, &delayBuff);
			}

			for (j = 0; j < BLOCK_SIZE; j++)
			{
				for (k = 0; k < outputWAVhdr.fmt.NumChannels; k++)
				{
					if (is_channel_active(mode, k))
					{
						sample = sampleBuffer[k][j].toLong();	// crude, non-rounding 			
						sample = sample >> (32 - inputWAVhdr.fmt.BitsPerSample);
						fwrite(&sample, outputWAVhdr.fmt.BitsPerSample / 8, 1, wav_out);
					}
				}
			}
			fflush(wav_out);
		}
	}

	// Close files
	//-------------------------------------------------	
	fclose(wav_in);
	fclose(wav_out);
	//-------------------------------------------------	

	return 0;
}