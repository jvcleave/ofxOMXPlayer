#pragma once

#include "ofMain.h"


class FPSLogger
{
public:
	
	int numSamples;
	float frameSampleTotal;
	int fpsSampleRate;
	void update(ofEventArgs & a)
	{
		if (ofGetFrameNum() % fpsSampleRate == 0) 
		{
			frameSampleTotal+=ofGetFrameRate();
			numSamples++;
		}
	}
	
	void exit(ofEventArgs & a)
	{
		if (numSamples>0 && frameSampleTotal>0) 
		{
			float averageFPS = frameSampleTotal/numSamples;
			ofBuffer buffer = ofToString(averageFPS);
			string fileName = ofToDataPath(ofGetTimestampString() + "_" + ofGetCurrentRenderer()->getType() + "_FPS.log", true);
			bool didWriteFile = ofBufferToFile(fileName, buffer);
			
			if(didWriteFile)
			{
				ofLogVerbose()	<< fileName << " write PASS";
			}else 
			{
				ofLogError()	<< fileName << " write FAIL";
			}			  
		}
		ofRemoveListener(ofEvents().update, this,	&FPSLogger::update);
		ofRemoveListener(ofEvents().exit, this, &FPSLogger::exit);
	}
	
	FPSLogger()
	{
		numSamples = 0;
		frameSampleTotal = 0.0f;
		fpsSampleRate = 5;
		ofAddListener(ofEvents().update, this,	&FPSLogger::update);
		ofAddListener(ofEvents().exit, this, &FPSLogger::exit);
	}
	
};
