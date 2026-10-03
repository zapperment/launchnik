#include <cstring> 
#include "Jukebox.h"
#include "Launchnik.h" 

void* JBox_Export_CreateNativeObject(const char iOperation[], const TJBox_Value iParams[], TJBox_UInt32 iCount) {
  if(std::strcmp(iOperation, "Instance") == 0){ 
		// iParams[0] is the sample rate, passed by init_instance in realtime_controller.lua
		TJBox_Float32 sampleRate = static_cast<TJBox_Float32>(JBox_GetNumber(iParams[0]));
		return new CLaunchnik(sampleRate);
	}
  return nullptr;
}

void JBox_Export_RenderRealtime(void* privateState, const TJBox_PropertyDiff iPropertyDiffs[], TJBox_UInt32 iDiffCount) {

	if(privateState == nullptr){ 
		return;
	}

	CLaunchnik * pi = reinterpret_cast<CLaunchnik*>(privateState); //(2)
	pi->RenderBatch(iPropertyDiffs, iDiffCount); //(3)
}


void JBox_Export_Draw(const TJBox_DisplayArgs* /*iArgs*/) { }

void JBox_Export_Gesture(TJBox_GestureArgs* /*iArgs*/) { }

void JBox_Export_DisplaySetup(const TJBox_DisplayArgs* /*iArgs*/) { }

void JBox_Export_Notify(TJBox_NotifyArgs* /*iArgs*/) { }
