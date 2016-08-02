/* main.c - this file is part of DeSmuME
*
* Copyright (C) 2006,2007 DeSmuME Team
* Copyright (C) 2007 Pascal Giard (evilynux)
* Copyright (C) 2009 Yoshihiro (DsonPSP)
* This file is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This file is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; see the file COPYING.  If not, write to
* the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*/

#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>

#include <stdio.h>
#include <malloc.h>

#include "../MMU.h"
#include "../NDSSystem.h"
#include "../debug.h"
#include "../render3D.h"
#include "../rasterize.h"
#include "../saves.h"
#include "../mic.h"
#include "../SPU.h"

#include "draw.h"

#define FRAMESKIP 1

volatile bool execute = FALSE;

GPU3DInterface *core3DList[] = {
	&gpu3DNull,
	&gpu3DRasterize,
	NULL
};

SoundInterface_struct *SNDCoreList[] = {
  &SNDDummy,
  &SNDDummy,
  &SNDDummy,
  NULL
};

const char * save_type_names[] = {
	"Autodetect",
	"EEPROM 4kbit",
	"EEPROM 64kbit",
	"EEPROM 512kbit",
	"FRAM 256kbit",
	"FLASH 2mbit",
	"FLASH 4mbit",
	NULL
};

inline unsigned int ABGR1555toRGBA8(unsigned short c)
{
    const unsigned int b = (c&0x7C00) >> 10, g = (c&0x03E0) >> 5, r = c&0x1F;
    return RGBA8(r * 255/31, g * 255/31, b * 255/31, 255);
}

#define ADD_KEY(keypad,key) ( (keypad) |= (key) )
#define KEYMASK(k)	(1 << (k))

#define KEY_A			0
#define KEY_B			1
#define KEY_SELECT		2
#define KEY_START		3
#define KEY_RIGHT		4
#define KEY_LEFT		5
#define KEY_UP			6
#define KEY_DOWN		7
#define KEY_R			8
#define KEY_L			9
#define KEY_X			10
#define KEY_Y			11
#define KEY_DEBUG		12
#define KEY_BOOST		13
#define KEY_LID			14

 /* Update NDS keypad */
void update_keypad(u16 keys)
{
    // Set raw inputs
  {
    buttonstruct<bool> input = {};
    input.G = (keys>>12)&1;
    input.E = (keys>>8)&1;
    input.W = (keys>>9)&1;
    input.X = (keys>>10)&1;
    input.Y = (keys>>11)&1;
    input.A = (keys>>0)&1;
    input.B = (keys>>1)&1;
    input.S = (keys>>3)&1;
    input.T = (keys>>2)&1;
    input.U = (keys>>6)&1;
    input.D = (keys>>7)&1;
    input.L = (keys>>5)&1;
    input.R = (keys>>4)&1;
    input.F = (keys>>14)&1;
    //RunAntipodalRestriction(input);
    NDS_setPad(
      input.R, input.L, input.D, input.U,
      input.T, input.S, input.B, input.A,
      input.Y, input.X, input.W, input.E,
      input.G, input.F);
  }
  
  // Set real input
  NDS_beginProcessingInput();
  {
    UserButtons& input = NDS_getProcessingUserInput().buttons;
    //ApplyAntipodalRestriction(input);
  }
  NDS_endProcessingInput();
}

/* Manage input events */
int process_ctrls_events( uint16_t *keypad ){
	*keypad = 0;
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(0, &pad, 1);

	if(pad.buttons & SCE_CTRL_LEFT)
		ADD_KEY( *keypad, KEYMASK(KEY_LEFT));

	if(pad.buttons & SCE_CTRL_RIGHT)
		ADD_KEY( *keypad, KEYMASK(KEY_RIGHT));

	if(pad.buttons & SCE_CTRL_UP)
		ADD_KEY( *keypad, KEYMASK(KEY_UP));

	if(pad.buttons & SCE_CTRL_DOWN)
		ADD_KEY( *keypad, KEYMASK(KEY_DOWN));

	if(pad.buttons & SCE_CTRL_CROSS)
		ADD_KEY( *keypad, KEYMASK(KEY_B));

	if(pad.buttons & SCE_CTRL_CIRCLE)
		ADD_KEY( *keypad, KEYMASK(KEY_A));

	if(pad.buttons & SCE_CTRL_TRIANGLE)
		ADD_KEY( *keypad, KEYMASK(KEY_X));

	if(pad.buttons & SCE_CTRL_SQUARE)
		ADD_KEY( *keypad, KEYMASK(KEY_Y));

	if(pad.buttons & SCE_CTRL_START)
		ADD_KEY( *keypad, KEYMASK(KEY_START));

	if(pad.buttons & SCE_CTRL_SELECT)
		ADD_KEY( *keypad, KEYMASK(KEY_SELECT));

	if(pad.buttons & SCE_CTRL_RTRIGGER)
		ADD_KEY( *keypad, KEYMASK(KEY_R));

	if(pad.buttons & SCE_CTRL_LTRIGGER)
		ADD_KEY( *keypad, KEYMASK(KEY_L));

	return 0;
}

SceTouchData touch;
bool touchPressed = 0;
uint16_t keypad;
static void desmume_cycle()
{
	sceTouchPeek(0, &touch, 1);
	process_ctrls_events(&keypad);
	update_keypad(keypad);

	if (touch.reportNum > 0) {
		unsigned tx = touch.report[0].x / 2;
		unsigned ty = (touch.report[0].y / 2) - 192;
		if(tx < 256 && ty < 192){
			touchPressed = true;
			NDS_setTouchPos(tx,ty);
		}
	}
	else if( (touch.reportNum == 0) && touchPressed) {
		touchPressed = false;
		NDS_releaseTouch();
	}

	update_keypad(keypad);

    NDS_exec<false>();

    //SPU_Emulate_user();
}

extern "C" {
	int scePowerSetArmClockFrequency(int freq);
}

int main(int argc, char **argv)
{
	scePowerSetArmClockFrequency(444);
	
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, 1);

	init_video();
	
	struct NDS_fw_config_data fw_config;

	NDS_FillDefaultFirmwareConfigData(&fw_config);

  	NDS_Init();

  	NDS_3D_ChangeCore(1);

	backup_setManualBackupType(0);

	#ifdef HAVE_JIT

	CommonSettings.use_jit = true;
	CommonSettings.jit_max_block_size = 35;

	#endif

	CommonSettings.ConsoleType = NDS_CONSOLE_TYPE_FAT;

	if (NDS_LoadROM( "ux0:/data/DeSmuME/game.nds", NULL) < 0) {
		sceKernelExitProcess(0);
		return 0;
	}

	execute = TRUE;

	while(1){

		for(int i=0; i < FRAMESKIP; i++){
			NDS_SkipNextFrame();
			NDS_exec<false>();
		}

		desmume_cycle();

		uint16_t * src = (uint16_t *)GPU->GetDisplayInfo().masterNativeBuffer;

		for(int x=0; x<256; x++){
    		for(int y=0; y<192;y++){
        		draw_pixel(x,y, ABGR1555toRGBA8(src[( y * 256 ) + x]));
        		draw_pixel(x,y+192, ABGR1555toRGBA8(src[( (y + 192) * 256 ) + x]));
    		}
		}
	}

	sceKernelExitProcess(0);
	
	return 0;
}