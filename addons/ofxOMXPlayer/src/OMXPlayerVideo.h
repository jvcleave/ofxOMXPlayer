#pragma once


#include "OMXPlayerVideoBase.h"
#include "OMXVideo.h"




class OMXPlayerVideo : public OMXPlayerVideoBase
{
public:

	bool                      m_Deinterlace;
	float                     m_display_aspect;

	bool                      m_hdmi_clock_sync;
	COMXVideo* nonTextureDecoder;
	OMXPlayerVideo();

	bool Open(COMXStreamInfo &hints, OMXClock *av_clock, bool deinterlace, bool hdmi_clock_sync, float display_aspect);
	bool OpenDecoder();

	
};
