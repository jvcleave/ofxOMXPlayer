/*
 *  OMXVideoPlayer.h
 *  openFrameworksLib
 *
 *  Created by jason van cleave on 6/19/13.
 *  Copyright 2013 jasonvancleave.com. All rights reserved.
 *
 */

#pragma once
#include "ofMain.h"

#include "DllAvUtil.h"
#include "DllAvFormat.h"
#include "DllAvCodec.h"

#include "OMXReader.h"
#include "OMXClock.h"
#include "OMXStreamInfo.h"
#include "OMXThread.h"
#include "OMXVideo.h"

#define MAX_DATA_SIZE    40 * 1024 * 1024


class OMXVideoPlayer: public OMXThread
{
public:
	OMXVideoPlayer(){
		ofLogVerbose() << "OMXVideoPlayer created";
	};
	virtual ~OMXVideoPlayer(){
		ofLogVerbose() << "OMXVideoPlayer destroyed";
	};
	
	virtual bool AddPacket(OMXPacket *pkt) =0;
	virtual double GetCurrentPTS() =0;
	virtual double GetFPS() =0;
	virtual void  WaitCompletion() =0;
	virtual bool Close() =0;
	virtual int  GetDecoderBufferSize() =0;
	virtual int  GetDecoderFreeSpace() =0;
	virtual unsigned int GetCached() =0;
	//virtual void Process() = 0;
};