#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef _WIN32

#include "includes.prl"
#include <exec/types.h>
#include <exec/memory.h>
#include <devices/audio.h>

#endif // !_WIN32


#include "reciter.h"
#include "sam.h"
#include "debug.h"
#include "easysound.h"

extern struct DosLibrary *DOSBase;

#ifndef _WIN32
static int min(int a, int b) { return a<b?a:b; }
#endif

extern char *buffer;

void WriteWav(char* filename, char* buffer, int bufferlength)
{
	unsigned int filesize;
	unsigned int fmtlength = 16;
	unsigned short int format=1; //PCM
	unsigned short int channels=1;
	unsigned int samplerate = 22050;
	unsigned short int blockalign = 1;
	unsigned short int bitspersample=8;

	FILE *file = fopen(filename, "wb");
	if (file == NULL) return;
	//RIFF header
	fwrite("RIFF", 4, 1,file);
	filesize=bufferlength + 12 + 16 + 8 - 8;
 	fwrite(&filesize, 4, 1, file);
	fwrite("WAVE", 4, 1, file);

	//format chunk
	fwrite("fmt ", 4, 1, file);
	fwrite(&fmtlength, 4, 1, file);
	fwrite(&format, 2, 1, file);
	fwrite(&channels, 2, 1, file);
	fwrite(&samplerate, 4, 1, file);
	fwrite(&samplerate, 4, 1, file); // bytes/second
	fwrite(&blockalign, 2, 1, file);
	fwrite(&bitspersample, 2, 1, file);

	//data chunk
	fwrite("data", 4, 1, file);
	fwrite(&bufferlength, 4, 1, file);
	fwrite(buffer, bufferlength, 1, file);

	fclose(file);
}

void PrintUsage(void)
{
	printf("usage: sam [options] Word1 Word2 ....\n");
	printf("options\n");
	printf("	-phonetic 		enters phonetic mode. (see below)\n");
	printf("	-pitch number		set pitch value (default=64)\n");
	printf("	-speed number		set speed value (default=72)\n");
	printf("	-throat number		set throat value (default=128)\n");
	printf("	-mouth number		set mouth value (default=128)\n");
	printf("	-wav filename		output to wav instead of libsdl\n");
	printf("	-sing			special treatment of pitch\n");
	printf("	-debug			print additional debug messages\n");
	printf("\n");

	
	printf("     VOWELS                            VOICED CONSONANTS	\n");
	printf("IY           f(ee)t                    R        red		\n");
	printf("IH           p(i)n                     L        allow		\n");
	printf("EH           beg                       W        away		\n");
	printf("AE           Sam                       W        whale		\n");
	printf("AA           pot                       Y        you		\n");
	printf("AH           b(u)dget                  M        Sam		\n");
	printf("AO           t(al)k                    N        man		\n");
	printf("OH           cone                      NX       so(ng)		\n");
	printf("UH           book                      B        bad		\n");
	printf("UX           l(oo)t                    D        dog		\n");
	printf("ER           bird                      G        again		\n");
	printf("AX           gall(o)n                  J        judge		\n");
	printf("IX           dig(i)t                   Z        zoo		\n");
	printf("				       ZH       plea(s)ure	\n");
	printf("   DIPHTHONGS                          V        seven		\n");
	printf("EY           m(a)de                    DH       (th)en		\n");
	printf("AY           h(igh)						\n");
	printf("OY           boy						\n");
	printf("AW           h(ow)                     UNVOICED CONSONANTS	\n");
	printf("OW           slow                      S         Sam		\n");
	printf("UW           crew                      Sh        fish		\n");
	printf("                                       F         fish		\n");
	printf("                                       TH        thin		\n");
	printf(" SPECIAL PHONEMES                      P         poke		\n");
	printf("UL           sett(le) (=AXL)           T         talk		\n");
	printf("UM           astron(omy) (=AXM)        K         cake		\n");
	printf("UN           functi(on) (=AXN)         CH        speech		\n");
	printf("Q            kitt-en (glottal stop)    /H        a(h)ead	\n");	
}

int pos = 0;

#ifndef _WIN32
void OutputSound(void) {
	struct SoundInfo *speech_buffer = PrepareSoundFromBuffer(GetBuffer(), GetBufferLength()/50, 22050);
	PlaySound(speech_buffer, MAXVOLUME, LEFT0, NORMALRATE, ONCE);
	Delay(GetBufferLength()/22050);
	StopSound(LEFT0);
}
#else
void OutputSound(void) {
	printf("Direct output not implemented.\n");
}
#endif

int debug = 0;

int main(int argc, char **argv)
{
	int i;
	int phonetic = 0;

	char* wavfilename = NULL;
	unsigned char input[256];
	
	memset(input, 0, 256);

	if (argc <= 1)
	{
		PrintUsage();
		return 1;
	}

	i = 1;
	while(i < argc)
	{
		if (argv[i][0] != '-')
		{
			strcat((char*)input, argv[i]);
			strcat((char*)input, " ");
		} else
		{
			if (strcmp(&argv[i][1], "wav")==0)
			{
				wavfilename = argv[i+1];
				i++;
			} else
			if (strcmp(&argv[i][1], "sing")==0)
			{
				EnableSingmode();
			} else
			if (strcmp(&argv[i][1], "phonetic")==0)
			{
				phonetic = 1;
			} else
			if (strcmp(&argv[i][1], "debug")==0)
			{
				debug = 1;
			} else
			if (strcmp(&argv[i][1], "pitch")==0)
			{
				SetPitch((unsigned char)min(atoi(argv[i+1]),255));
				i++;
			} else
			if (strcmp(&argv[i][1], "speed")==0)
			{
				SetSpeed((unsigned char)min(atoi(argv[i+1]),255));
				i++;
			} else
			if (strcmp(&argv[i][1], "mouth")==0)
			{
				SetMouth((unsigned char)min(atoi(argv[i+1]),255));
				i++;
			} else
			if (strcmp(&argv[i][1], "throat")==0)
			{
				SetThroat((unsigned char)min(atoi(argv[i+1]),255));
				i++;
			} else
			{
				PrintUsage();
				return 1;
			}
		}
		
		i++;
	} //while

	for(i=0; input[i] != 0; i++)
		input[i] = (unsigned char)toupper((int)input[i]);

	if (debug)
	{
		if (phonetic) printf("phonetic input: %s\n", input);
		else printf("text input: %s\n", input); 
	}
	
	if (!phonetic)
	{
		strcat((char*)input, "[");
		if (!TextToPhonemes(input)) return 1;
		if (debug)
			printf("phonetic input: %s\n", input);
	} else strcat((char*)input, "\x9b");

	SetInput(input);
	if (!SAMMain())
	{
		PrintUsage();
		return 1;
	}

	if (wavfilename != NULL) 
		WriteWav(wavfilename, GetBuffer(), GetBufferLength()/50);
	else
		OutputSound();

	#ifdef _WIN32
		if (buffer != NULL)
			free(buffer);
	#else
		if (buffer != NULL)
		    FreeMem(buffer, 22050*10);
	#endif	

	return 0;
}
