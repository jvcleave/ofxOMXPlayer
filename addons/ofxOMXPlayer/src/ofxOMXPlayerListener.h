#pragma once

#include "ofMain.h"


class ofxOMXPlayerListenerEventData
{
public:
	ofxOMXPlayerListenerEventData(void* listener_)
	{
		listener = listener_;
	}
	void* listener;
};


class ofxOMXPlayerListener
{
public:
	virtual void onVideoEnd(ofxOMXPlayerListenerEventData& e) = 0;
	virtual void onVideoLoop(ofxOMXPlayerListenerEventData& e) = 0;
	
};



