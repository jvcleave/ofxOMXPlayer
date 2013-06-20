#pragma once

#include "ofMain.h"

#include "OMXDecoder.h"

class OMXEGLImage : public OMXDecoder
{
public:
	OMXEGLImage();
	~OMXEGLImage();

	// Required overrides
	bool SendDecoderConfig();
	bool NaluFormatStartCodes(enum CodecID codec, uint8_t *in_extradata, int in_extrasize);
	bool Open(COMXStreamInfo &hints, OMXClock *clock, EGLImageKHR eglImage_);
	void Close(void);
	unsigned int GetFreeSpace();
	unsigned int GetSize();
	int  Decode(uint8_t *pData, int iSize, double dts, double pts);
	void Reset(void);
	void SetDropState(bool bDrop);
	bool Pause();
	bool Resume();
	std::string GetDecoderName() { return m_video_codec_name; };
	int GetInputBufferSize();
	void WaitCompletion();
	EGLImageKHR eglImage;
	OMX_BUFFERHEADERTYPE* eglBuffer;
	char debugInfoBuffer [128];
	string debugInfo;
	bool doDebugging;





	

	
};
