#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <vita2d.h>
#include <psp2/io/dirent.h>

#include <vector>
#include <malloc.h>
#include <stdio.h>

#include "video.h"
#include "config.h"

#include "../NDSSystem.h"

//Very rough implementation of a rom selector... Yeah I know its ugly
char* menu_FileBrowser() {
	std::vector<SceIoDirent> entries;
	SceUID directory;
	SceIoDirent dirent;
	directory = sceIoDopen("ux0:/data/desmume");

	while(sceIoDread(directory, &dirent) > 0){
		char *extension = strrchr(dirent.d_name,'.');
		if(extension != NULL){
			if(strcmp(extension, ".nds") == 0)
				entries.push_back(dirent);
		}
	}

	if(entries.empty())
		return NULL;

	int cursor = 0;
	bool buttonPressed = false;
	int count;
	SceCtrlData pad;
	while(true) {
		sceCtrlPeekBufferPositive(0, &pad, 1);

		if(!pad.buttons)
			buttonPressed = 0;

		if(pad.buttons & SCE_CTRL_CROSS && !buttonPressed){
			break;
		}

		if(pad.buttons & SCE_CTRL_SQUARE && !buttonPressed){
			UserConfiguration.jitEnabled = !UserConfiguration.jitEnabled;
		}

		if(pad.buttons & SCE_CTRL_CIRCLE && !buttonPressed){
			UserConfiguration.soundEnabled = !UserConfiguration.soundEnabled;
		}

		if(pad.buttons & SCE_CTRL_DOWN && !buttonPressed){
			cursor++;
		}

		if(pad.buttons & SCE_CTRL_UP && !buttonPressed){
			cursor--;
		}

		if(pad.buttons & SCE_CTRL_LEFT && !buttonPressed){
			if(UserConfiguration.frameSkip > 0)
			UserConfiguration.frameSkip--;
		}

		if(pad.buttons & SCE_CTRL_RIGHT && !buttonPressed){
			UserConfiguration.frameSkip++;
		}

		if(pad.buttons)
			buttonPressed = 1;

		if(cursor < 0)
			cursor = 0;

		if(cursor > entries.size() - 1)
			cursor = entries.size() - 1;

		video_BeginDrawing();
		count = 0;
		for( std::vector<SceIoDirent>::iterator it = entries.begin(); it != entries.end(); it++){
			vita2d_pgf_draw_text(video_font,0,15 + count*15,cursor == count ? RGBA8(0,255,0,255) : RGBA8(255,255,255,255),1.0f,it->d_name);
			count++;
		}
		vita2d_pgf_draw_textf(video_font,500,20, RGBA8(0,0,255,255) ,1.0f,"Press ([ ]) to %s JIT", UserConfiguration.jitEnabled ? "disable" : "enable");
		vita2d_pgf_draw_textf(video_font,500,40, RGBA8(0,0,255,255) ,1.0f,"Press (O) to %s Sound", UserConfiguration.soundEnabled ? "disable" : "enable");
		vita2d_pgf_draw_textf(video_font,500,60, RGBA8(0,0,255,255) ,1.0f,"Press Left or Right DPAD to change frameSkip value");
		vita2d_pgf_draw_textf(video_font,500,80, RGBA8(0,0,255,255) ,1.0f,"Current frameSkip value: %u", UserConfiguration.frameSkip);
		video_EndDrawing();
	}

	char *filename = (char*)malloc(4096);
	sprintf(filename, "ux0:/data/desmume/%s", entries[cursor].d_name);
	return filename;
}

//Uninplemented
int menu_Init(){
	return 0;
}

//Uninplemented
//This is where all the tabs(rom selection, save states, settings, etc) will be handled
int menu_Display(){
	return 0;
}