#pragma once

#include "ofMain.h"


extern "C"
{
	#include <fcntl.h>
	#include <stdio.h>
	#include <sys/stat.h>
	#include <unistd.h>
};

#define PIPE_BUFFER_SIZE 32

class ofxPipeListenerEventData
{
public:
	ofxPipeListenerEventData(char character_)
	{
		character = character_;
	}
	char character;
};

class ofxPipeListener
{
public:
	virtual void onCharacterReceived(ofxPipeListenerEventData& e) = 0;
};

class PipeReader : public ofThread{

public:
	
	string namedPipe;
	int fd;	
	ofxPipeListener* listener;
	PipeReader()
	{
		listener = NULL;
	}

	void start(ofxPipeListener *listener_)
	{
		listener = listener_;
		namedPipe = ofToDataPath("ofpipe", true);
		ofFile file(namedPipe, ofFile::ReadWrite, false);
		if (!file.exists()) 
		{
			bool didCreate = file.create();
			if (didCreate) 
			{
				ofLogVerbose() << "file creation PASS " << namedPipe;
			}else 
			{
				ofLogError() << "file creation FAIL " << namedPipe;
			}

		}else 
		{
			ofLogVerbose() << namedPipe << " EXISTS";
		}
		ofLogVerbose() << "use command echo SOMEKEY > " << namedPipe << " in a second terminal to send key commands";
		ofLogVerbose() << "EXAMPLE:  " << endl << "$echo p > " << namedPipe << endl << " will pause the player";

		mkfifo(namedPipe.c_str(), 0777);
		
		startThread(true, false);
	}

	void stop()
	{
		
		stopThread();
	}

	void threadedFunction()
	{
		while( isThreadRunning())
		{
			ofBuffer pipeContents = ofBufferFromFile(namedPipe, false);
			char * content = pipeContents.getBinaryBuffer();
			if (content[0] != 0) 
			{
				ofxPipeListenerEventData eventData(content[0]);
				listener->onCharacterReceived(eventData);
				pipeContents.clear();
				ofBufferToFile(namedPipe, pipeContents, false);
			}
			
		}
	}
};

