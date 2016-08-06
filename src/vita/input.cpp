#include <psp2/ctrl.h>
#include <psp2/touch.h>

#include "../NDSSystem.h"
#include "input.h"
#include "video.h"

static bool _setTouchLandscape(unsigned x, unsigned y){

  if(x > 301 && x < 659 && y > 272 && y < 541){
    NDS_setTouchPos((x - 301)/1.4, (y - 272)/1.4);
    return true;
  }

  return false;
}

static bool _setTouchPortrait(unsigned x, unsigned y){

  if(x > 96 && x < 480 && y > 16 && y < 528){
    NDS_setTouchPos((y - 16)/2, 192 - (x - 96)/2);
    return true;
  }

  return false;
}

static bool _setTouchSBS(unsigned x, unsigned y){

  if(x > 480 && y > 92 && y < 476){
    NDS_setTouchPos((x - 480)/1.875,(y - 92)/1.875);
    return true;
  }

  return false;
}

void input_UpdateTouch(){

  static bool touchPressed = 0;
  SceTouchData touch;
  sceTouchPeek(0, &touch, 1);

  if (touch.reportNum > 0) {

    unsigned x = touch.report[0].x/2;
    unsigned y = touch.report[0].y/2;

    switch(video_layout){
      case LAYOUT_PORTRAIT:
        touchPressed = _setTouchPortrait(x, y);
        break;
      case LAYOUT_SBS:
        touchPressed = _setTouchSBS(x, y);
        break;
      case LAYOUT_LANDSCAPE:
      default:
        touchPressed = _setTouchLandscape(x, y);
        break;
    }

    if(x > 930 && y > 514){
      if(++video_layout > 2)
        video_layout = 0;
    }

  }
  else if( (touch.reportNum == 0) && touchPressed) {
    touchPressed = false;
    NDS_releaseTouch();
  }

}

void input_UpdateKeypad(){

  SceCtrlData pad;
  sceCtrlPeekBufferPositive(0, &pad, 1);
  if((pad.buttons & 833) == 833)
    execute = false;

  {
    buttonstruct<bool> input = {};

    input.E = pad.buttons & SCE_CTRL_RTRIGGER;
    input.W = pad.buttons & SCE_CTRL_LTRIGGER;
    input.X = pad.buttons & SCE_CTRL_SQUARE;
    input.Y = pad.buttons & SCE_CTRL_TRIANGLE;
    input.A = pad.buttons & SCE_CTRL_CIRCLE;
    input.B = pad.buttons & SCE_CTRL_CROSS;
    input.S = pad.buttons & SCE_CTRL_START;
    input.T = pad.buttons & SCE_CTRL_SELECT;
    input.U = pad.buttons & SCE_CTRL_UP;
    input.D = pad.buttons & SCE_CTRL_DOWN;
    input.L = pad.buttons & SCE_CTRL_LEFT;
    input.R = pad.buttons & SCE_CTRL_RIGHT;

    NDS_setPad(
      input.R, input.L, input.D, input.U,
      input.T, input.S, input.B, input.A,
      input.Y, input.X, input.W, input.E,
      input.G, input.F);
  }

  NDS_beginProcessingInput();
  {
    UserButtons& input = NDS_getProcessingUserInput().buttons;
  }
  NDS_endProcessingInput();

}