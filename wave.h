#ifndef WAVE_H
#define WAVE_H

#include "inttypes.h"
#include "functions.h"



typedef struct { //g³ówny nag³ówek, okreœlamy jaki uk³ad bitów
	uint32 chunkId; //big
	uint32 chunkSize; //little
	uint32 format; //big
} WAVEHEADER;

typedef struct {
	uint32 subChunkId; //big [fmt ]
	uint32 subChunkSize; //little
	uint16 audioFormat; //little
	uint16 numChannels; //little
	uint32 sampleRate; //little
	uint32 byteRate; //little
	uint16 blockAlign; //little
	uint16 bitsPerSample; //little
} FmtChunk; //fmt


typedef struct {
	uint32 subChunkId; //big [data]
	uint32 subChunkSize; //little //data/samples size
	uint32 byteToOmmit; //where is the first sample
} DataChunk; //data


//inicjuj nag³ówki
tBool playerInit(void);

//rozpocznij granie
tBool playWave(PlayerStatus *status, TimerStatus *timer);



#endif // WAVE_H
