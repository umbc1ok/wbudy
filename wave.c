#include "wave.h"
#include "pff2/src/pff.h"
#include "startup/printf_P.h"
#include "startup/lpc2xxx.h"
#include "timer.h"
#include "led.h"
#include "lcd.h"


static uint8 wave_stream_buf[ 512 * 1 ];//1 sectors

WORD bytesReaded;
FRESULT rc;


WAVEHEADER wh;
FmtChunk ws1;
DataChunk ws2;


tBool playerInit(void){
	bytesReaded = 0;

	uint32 i = 0;

//	printf("read data\n");
	rc = pf_read(wave_stream_buf, sizeof(wave_stream_buf), &bytesReaded);
	
	if( rc || !bytesReaded )
		return FALSE;

//	printf("fill structs\n");
	//wave header
	wh.chunkId = wave_stream_buf[i++] << 24;
	wh.chunkId += wave_stream_buf[i++] << 16;
	wh.chunkId += wave_stream_buf[i++] << 8;
	wh.chunkId += wave_stream_buf[i++];

	wh.chunkSize = wave_stream_buf[i++];
	wh.chunkSize += wave_stream_buf[i++] << 8;
	wh.chunkSize += wave_stream_buf[i++] << 16;
	wh.chunkSize += wave_stream_buf[i++] << 24;

	wh.format = wave_stream_buf[i++] << 24;
	wh.format += wave_stream_buf[i++] << 16;
	wh.format += wave_stream_buf[i++] << 8;
	wh.format += wave_stream_buf[i++];

//	printf("id=%x size=%x format=%x\n",wh.chunkId,wh.chunkSize,wh.format);

	ws1.subChunkId = wave_stream_buf[i++] << 24;
	ws1.subChunkId += wave_stream_buf[i++] << 16;
	ws1.subChunkId += wave_stream_buf[i++] << 8;
	ws1.subChunkId += wave_stream_buf[i++];

	ws1.subChunkSize = wave_stream_buf[i++];
	ws1.subChunkSize += wave_stream_buf[i++] << 8;
	ws1.subChunkSize += wave_stream_buf[i++] << 16;
	ws1.subChunkSize += wave_stream_buf[i++] << 24;

	ws1.audioFormat = wave_stream_buf[i++];
	ws1.audioFormat += wave_stream_buf[i++] << 8;

	ws1.numChannels = wave_stream_buf[i++];
	ws1.numChannels += wave_stream_buf[i++] << 8;

	ws1.sampleRate = wave_stream_buf[i++];
	ws1.sampleRate += wave_stream_buf[i++] << 8;
	ws1.sampleRate += wave_stream_buf[i++] << 16;
	ws1.sampleRate += wave_stream_buf[i++] << 24;

	ws1.byteRate = wave_stream_buf[i++];
	ws1.byteRate += wave_stream_buf[i++] << 8;
	ws1.byteRate += wave_stream_buf[i++] << 16;
	ws1.byteRate += wave_stream_buf[i++] << 24;

	ws1.blockAlign = wave_stream_buf[i++];
	ws1.blockAlign += wave_stream_buf[i++] << 8;

	ws1.bitsPerSample = wave_stream_buf[i++];
	ws1.bitsPerSample += wave_stream_buf[i++] << 8;

//printf("sid=%x ssize=%x audioformat=%x\n",ws1.subChunkId,ws1.subChunkSize,ws1.audioFormat);
//printf("ch=%x sr=%x br=%x\n",ws1.numChannels,ws1.sampleRate,ws1.byteRate);
//printf("block=%x bps=%x\n",ws1.blockAlign,ws1.bitsPerSample);

	ws2.subChunkId = wave_stream_buf[i++] << 24;
	ws2.subChunkId += wave_stream_buf[i++] << 16;
	ws2.subChunkId += wave_stream_buf[i++] << 8;
	ws2.subChunkId += wave_stream_buf[i++];

	ws2.subChunkSize = wave_stream_buf[i++];
	ws2.subChunkSize += wave_stream_buf[i++] << 8;
	ws2.subChunkSize += wave_stream_buf[i++] << 16;
	ws2.subChunkSize += wave_stream_buf[i++] << 24;

	ws2.byteToOmmit = i;

//	printf("end init\n");
//	printf("sid=%x ssize=%x i=%x\n",ws2.subChunkId,ws2.subChunkSize,ws2.byteToOmmit);

	return TRUE;
}

#define BUFFSIZE 512 * 5 //wielko�� bufora
#define BUFFCOUNT 2 //ilo�� bufor�w

uint8 buffer[BUFFCOUNT][BUFFSIZE]; //bufor na pr�bki

uint8 bufferPlayed = 0; //numer bufora z kt�rego s� aktualnie odczytywane dane
uint8 bufferToDownload = 0; //numer bufora kt�ry nale�y uzupe�ni�
uint16 sample = 0; //dana pr�bka z bufora
uint64 totalPlayed = 0; //ilo�� oftworzonych pr�bek
tBool refil = FALSE; //czy bufor pusty

tBool playWave(PlayerStatus *status, TimerStatus *timer){
	bufferPlayed = 0;
	bufferToDownload = 0;
	sample = 0;
	totalPlayed = 0;
	refil = FALSE;

	bytesReaded = 0;

	uint32 i = 0;
	pf_lseek(ws2.byteToOmmit);

	//wype�niamy na sam pocz�tek czym�
	do{
		rc = pf_read(buffer[i], sizeof(buffer[i]), &bytesReaded);
		i++;
	}while( i < BUFFCOUNT && !rc && bytesReaded );

	initLed(3);

	uint32 uplynelo = 0;
	uint32 czasCalkowity = ws2.subChunkSize/ws1.byteRate ;
	//ilo�� pr�bek przez pr�dkos� odtwarzania
	uint32 pozostalo = 0;

	while( totalPlayed < ws2.subChunkSize ){
		if( *status == Player_Playing ){
			if( sample >= (BUFFSIZE) ){
				sample = 0;
				bufferToDownload = bufferPlayed++;
				refil = TRUE;
			}
			if( bufferPlayed >= BUFFCOUNT )
				bufferPlayed = 0;


			if( ws1.bitsPerSample == 8 )
				DACR = buffer[bufferPlayed][sample++] << 5;
			else if( ws1.bitsPerSample == 16 )
				DACR = (( buffer[bufferPlayed][sample++] << 8) +
						( buffer[bufferPlayed][sample++]) )<< 5;
			//lcdGotoxy(0, 64);
			//lcdPuts();

			if( totalPlayed % ws1.byteRate == 0 ){
				if( *timer == Timer_UP ){
					uplynelo = totalPlayed/ws1.byteRate;
					printf("uplynelo: %d - ", uplynelo);
					printf("%d\n", czasCalkowity );
					lcdGotoxy(0,32);
					lcdPutsNumber( uplynelo );
					lcdPuts( " - " );
					lcdPutsNumber( czasCalkowity );
				}else{
					pozostalo = (ws2.subChunkSize - totalPlayed)/ws1.byteRate;

					lcdGotoxy(0,32);
					if( pozostalo >= 100 ){
						lcdPuts( "-" );
					}else if( pozostalo >= 10 ){
						lcdPuts( " -" );
					}else{
						lcdPuts( "  -" );
					}
					lcdPutsNumber( pozostalo );
				}

			}
		}

		//sprawd� przyciski
		updateStatus( status, timer, FALSE );

		//wyjd� z p�tli je�li trzeba przerywaj�c odtwarzanie
		if( *status != Player_Playing && *status != Player_Paused )
			return FALSE;


		if( refil ){ //uzupe�nij bufor nawet jak zapauzowane
			turnOnLed(3);
			rc = pf_read(buffer[bufferToDownload],
						 sizeof(buffer[bufferToDownload]), &bytesReaded);
			refil = FALSE;
			turnOffLed(3);
		}else if( refil == FALSE && *status == Player_Playing ){
			if( totalPlayed % ws1.byteRate != 0 )
				zegarek(1000000/(ws1.byteRate + 2500));
			//trzeba przy�pieszy� mimo wszystko o jakie� 2500 pr�bek
			//ekran bowiem daje za du�e op�nienia :/ i tak to nie jest
			//doskona�e
			//gdy odczyt z karty, to on s�u�y za op�xnienie, wi�c nie czekaj

		}


		if( *status == Player_Playing ){
			totalPlayed++;
		}
	}
	return TRUE;
}
