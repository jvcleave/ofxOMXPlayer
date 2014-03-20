/*
 *  FrameCounter.h
 *  openFrameworksRPi
 *
 *  Created by jason van cleave on 3/5/14.
 *  Copyright 2014 jasonvancleave.com. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"



class FrameCounter
{
public:
	static FrameCounter& getInstance()
	{
		static FrameCounter    instance;
		return instance;
	}
	int reset()
	{
		frame = 0;
	}
	int frame;
private:
	FrameCounter()
	{
		frame =0;
	}
	~FrameCounter() {};
	FrameCounter(FrameCounter const&);
	void operator=(FrameCounter const&);
	
};