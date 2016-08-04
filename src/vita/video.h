#ifndef VITA_VIDEO_H
#define VITA_VIDEO_H

#include <vita2d.h>

#define SCREEN_W 960
#define SCREEN_H 544

#define LANDSCAPE_X		301
#define LANDSCAPE_Y		1
#define LANDSCAPE_SCALE 1.4

#define PORTRAIT_X	480
#define PORTRAIT_Y	272

#define SBS_X		480
#define SBS_Y		92
#define SBS_SCALE 	1.875

enum {
	LAYOUT_LANDSCAPE = 0,
	LAYOUT_PORTRAIT  = 1,
	LAYOUT_SBS		 = 2,
};

void video_Init();
void video_Exit();
void video_DrawFrame();

extern vita2d_texture *fb;
extern int video_layout;

#endif