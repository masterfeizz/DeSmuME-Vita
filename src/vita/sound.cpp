/*
   Copyright (C) 2016 Felipe Izzo <MasterFeizz>
   Copyright (C) 2005-2015 DeSmuME Team
   Copyright 2005-2006 Theo Berkau

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 2 of the License, or
   (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/audioout.h>

#include "types.h"
#include "SPU.h"
#include "sound.h"
#include "debug.h"

#define VITA_AUDIO_MAX_VOLUME      0x8000
#define VITA_OUTPUT_BUFFER_SAMPLES 2048
#define VITA_OUTPUT_BUFFER_SIZE    2048 * 4

int  SNDVITAInit(int buffersize);
void SNDVITADeInit();
void SNDVITAUpdateAudio(s16 *buffer, u32 num_samples);
u32  SNDVITAGetAudioSpace();
void SNDVITAMuteAudio();
void SNDVITAUnMuteAudio();
void SNDVITASetVolume(int volume);

SoundInterface_struct SNDVITA = {
SNDCORE_VITA,
"Vita Sound Interface",
SNDVITAInit,
SNDVITADeInit,
SNDVITAUpdateAudio,
SNDVITAGetAudioSpace,
SNDVITAMuteAudio,
SNDVITAUnMuteAudio,
SNDVITASetVolume
};

static u16 *stereodata16;
static u16 *outputbuffer;
static u32 soundoffset;
static volatile u32 soundpos;
static u32 soundlen;
static u32 soundbufsize;
static u32 samplecount;

static int handle;
static int stopAudio;


static inline void Sound_Callback(uint8_t *stream, int len) {
   int i;
   uint8_t *soundbuf=(uint8_t *)stereodata16;

   for (i = 0; i < len; i++)
   {
      if (soundpos >= soundbufsize)
         soundpos = 0;

      stream[i] = soundbuf[soundpos];
      soundbuf[soundpos] = 0;
      soundpos++;
   }
}

static int Sound_Thread(SceSize args, void *argp)
{
   int vols[2]={VITA_AUDIO_MAX_VOLUME,VITA_AUDIO_MAX_VOLUME};

   sceAudioOutSetVolume(handle,SCE_AUDIO_VOLUME_FLAG_L_CH|SCE_AUDIO_VOLUME_FLAG_R_CH,vols);

   sceAudioOutSetConfig(handle, -1, -1, -1);
   
   while (!stopAudio)
   {
         Sound_Callback((uint8_t*)outputbuffer, VITA_OUTPUT_BUFFER_SIZE);
         sceAudioOutOutput(handle, outputbuffer);
      
   }

   sceAudioOutReleasePort(handle);
   sceKernelExitThread(0);
   return 0;
}

//////////////////////////////////////////////////////////////////////////////

int SNDVITAInit(int buffersize)
{
   stopAudio = 0;
   samplecount = 512;

   while (samplecount < ((DESMUME_SAMPLE_RATE / 60) * 2)) 
      samplecount <<= 1;
   
   soundlen = DESMUME_SAMPLE_RATE / 60; // 60 for NTSC
   soundbufsize = buffersize * sizeof(s16) * 2;

   handle = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_VOICE, VITA_OUTPUT_BUFFER_SAMPLES, DESMUME_SAMPLE_RATE, SCE_AUDIO_OUT_MODE_STEREO);

   if(handle < 0){
      sceKernelExitProcess(0);
      return 0;
   }

   outputbuffer = (u16*)malloc(VITA_OUTPUT_BUFFER_SIZE);

   stereodata16 = (u16 *)malloc(soundbufsize);
   if (stereodata16 == NULL)
      return -1;

   memset(stereodata16, 0, soundbufsize);

   SceUID audiothread = sceKernelCreateThread("Audio Thread", Sound_Thread, 0x10000100, 0x10000, 0, 0, NULL);
   
   int res = sceKernelStartThread(audiothread, 0, NULL);
   
   if (res != 0){
      return 0;
   }

   soundpos = 0;

   return 0;
}

//////////////////////////////////////////////////////////////////////////////

void SNDVITADeInit()
{
   stopAudio = 1;
   if (stereodata16)
      free(stereodata16);
}

//////////////////////////////////////////////////////////////////////////////

void SNDVITAUpdateAudio(s16 *buffer, u32 num_samples)
{
   u32 copy1size=0, copy2size=0;

   if ((soundbufsize - soundoffset) < (num_samples * sizeof(s16) * 2))
   {
      copy1size = (soundbufsize - soundoffset);
      copy2size = (num_samples * sizeof(s16) * 2) - copy1size;
   }
   else
   {
      copy1size = (num_samples * sizeof(s16) * 2);
      copy2size = 0;
   }

   memcpy((((u8 *)stereodata16)+soundoffset), buffer, copy1size);
//   ScspConvert32uto16s((s32 *)leftchanbuffer, (s32 *)rightchanbuffer, (s16 *)(((u8 *)stereodata16)+soundoffset), copy1size / sizeof(s16) / 2);

   if (copy2size)
      memcpy(stereodata16, ((u8 *)buffer)+copy1size, copy2size);
//      ScspConvert32uto16s((s32 *)leftchanbuffer, (s32 *)rightchanbuffer, (s16 *)stereodata16, copy2size / sizeof(s16) / 2);

   soundoffset += copy1size + copy2size;
   soundoffset %= soundbufsize;
}

//////////////////////////////////////////////////////////////////////////////

u32 SNDVITAGetAudioSpace()
{
   u32 freespace=0;

   if (soundoffset > soundpos)
      freespace = soundbufsize - soundoffset + soundpos;
   else
      freespace = soundpos - soundoffset;

   return (freespace / sizeof(s16) / 2);
}

//////////////////////////////////////////////////////////////////////////////

void SNDVITAMuteAudio()
{

}

//////////////////////////////////////////////////////////////////////////////

void SNDVITAUnMuteAudio()
{

}

//////////////////////////////////////////////////////////////////////////////

void SNDVITASetVolume(int volume)
{
}

//////////////////////////////////////////////////////////////////////////////
