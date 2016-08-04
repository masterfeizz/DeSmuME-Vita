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

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/display.h>
#include <psp2/power.h>
#include <psp2/kernel/processmgr.h>

#include "video.h"
#include "input.h"

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

static void desmume_cycle()
{
	input_UpdateKeypad();
	input_UpdateTouch();

    NDS_exec<false>();
}

extern "C" {
	int scePowerSetArmClockFrequency(int freq);
}

int main()
{
	scePowerSetArmClockFrequency(444);
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, 1);

	video_Init();

	struct NDS_fw_config_data fw_config;
	NDS_FillDefaultFirmwareConfigData(&fw_config);
  	NDS_Init();
	NDS_3D_ChangeCore(1);
	backup_setManualBackupType(0);
	CommonSettings.ConsoleType = NDS_CONSOLE_TYPE_FAT;

#ifdef HAVE_JIT
	CommonSettings.use_jit = true;
	CommonSettings.jit_max_block_size = 40;
#endif

	if (NDS_LoadROM("ux0:/data/DeSmuME/game.nds") < 0) {
		goto exit;
	}

	execute = true;

	int i;

	while (1) {

		for (i = 0; i < FRAMESKIP; i++) {
			NDS_SkipNextFrame();
			NDS_exec<false>();
		}

		desmume_cycle();
		video_DrawFrame();

	}

exit:
	video_Exit();

	sceKernelExitProcess(0);
	return 0;
}