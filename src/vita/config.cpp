#include "config.h"

TUserConfiguration UserConfiguration;

TUserConfiguration::TUserConfiguration() {
	soundEnabled    = true;
	jitEnabled 		= true;
	frameSkip 		= 1;
	threadedRendering = true;
}