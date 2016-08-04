#include "video.h"

#include <stdio.h>
#include <string.h>

#include "../GPU.h"

#define PI 3.14159265

vita2d_texture *fb = NULL;
int video_layout = 0;
void *data;

void video_Init(){
	vita2d_init();
	vita2d_set_vblank_wait(0);
	vita2d_set_clear_color(RGBA8(0, 0, 0, 0xFF));

	fb = vita2d_create_empty_texture_format(256, 192*2,
		SCE_GXM_TEXTURE_FORMAT_U1U5U5U5_ABGR);

	data = vita2d_texture_get_datap(fb);
}

void video_Exit(){
	vita2d_fini();
	if (fb) {
		vita2d_free_texture(fb);
	}
}

void video_DrawFrame(){

	uint16_t *src = (uint16_t *)GPU->GetDisplayInfo().masterNativeBuffer;

	vita2d_start_drawing();
	vita2d_clear_screen();

	memcpy(data, src, sizeof(uint16_t) * 256 * 192 * 2);

	switch(video_layout){
		case LAYOUT_PORTRAIT:
			vita2d_draw_texture_scale_rotate(fb, PORTRAIT_X, PORTRAIT_Y, 2, 2, PI/2);
			break;
		case LAYOUT_SBS:
			vita2d_draw_texture_part_scale(fb, 0, SBS_Y, 0, 0, 256, 192, SBS_SCALE, SBS_SCALE);
			vita2d_draw_texture_part_scale(fb, SBS_X, SBS_Y, 0, 192, 256, 192, SBS_SCALE, SBS_SCALE);
			break;
		case LAYOUT_LANDSCAPE:
		default:
			vita2d_draw_texture_scale(fb, LANDSCAPE_X, LANDSCAPE_Y, LANDSCAPE_SCALE, LANDSCAPE_SCALE);
			break;
	}

	vita2d_draw_rectangle(930,514,30,30,RGBA8(30,30,30,255));

	vita2d_end_drawing();
	vita2d_swap_buffers();

}