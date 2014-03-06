#pragma once

#include "ofMain.h"
#include "ofxOMXPlayerListener.h"

class ofxOMXPlayerSettings
{
	public:
		ofxOMXPlayerSettings()
		{
			videoPath = "";

			useHDMIForAudio = true;
			enableTexture = true;
			doFlipTexture = false; //true for older firmware
			enableLooping = true;
			listener	  = NULL;
			enableAudio	  = true;
			videoWidth	= 0;
			videoHeight = 0;

		}
		string videoPath;
		int videoWidth;
		int videoHeight;
		bool enableTexture;
		bool doFlipTexture;
		bool enableAudio;
		bool useHDMIForAudio;
		bool enableLooping;
		ofxOMXPlayerListener* listener;
		ofRectangle displayRect;
		/*
		 To use HDMI Audio you may need to add the below line to /boot/config.txt and reboot

		 hdmi_drive=2

		 see http://elinux.org/RPiconfig for more details
		 */

};