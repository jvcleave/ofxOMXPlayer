#pragma once

#include "ofMain.h"

#include "OMXCore.h"
#include "OMXStreamInfo.h"

#include <IL/OMX_Video.h>

#include "OMXClock.h"
#include "OMXReader.h"



#define CLASSNAME "COMXVideo"

class DllAvUtil;
class DllAvFormat;
class COMXVideo
{
public:
	COMXVideo();
	~COMXVideo();

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
protected:
	// Video format
	bool              m_drop_state;
	unsigned int      m_decoded_width;
	unsigned int      m_decoded_height;

	OMX_VIDEO_CODINGTYPE m_codingType;

	COMXCoreComponent m_omx_decoder;
	COMXCoreComponent m_omx_render;
	COMXCoreComponent m_omx_sched;
	COMXCoreComponent *m_omx_clock;
	OMXClock           *m_av_clock;

	COMXCoreTunel     m_omx_tunnel_decoder;
	COMXCoreTunel     m_omx_tunnel_clock;
	COMXCoreTunel     m_omx_tunnel_sched;
	bool              m_is_open;

	bool              m_Pause;
	bool              m_setStartTime;
	bool              m_setStartTimeText;

	uint8_t           *m_extradata;
	int               m_extrasize;

	std::string       m_video_codec_name;

	bool              m_first_frame;

};
