#pragma once


#include "OMXVideoPlayer.h"
#include "OMXVideo.h"




class OMXPlayerVideo : public OMXVideoPlayer
{
public:

	bool                      m_Deinterlace;
	float                     m_display_aspect;
	bool                      m_bMpeg;
	bool                      m_use_thread;


	bool                      m_hdmi_clock_sync;
	COMXVideo* nonTextureDecoder;
	OMXPlayerVideo();
	//~OMXPlayerVideo();
	bool Open(COMXStreamInfo &hints, OMXClock *av_clock, bool deinterlace, bool mpeg, bool hdmi_clock_sync, bool use_thread, float display_aspect);
	bool OpenDecoder();
	//void SetDelay(double delay) { m_iVideoDelay = delay; }
	//double GetDelay() { return m_iVideoDelay; }
	//void SetSpeed(int iSpeed);
	
};
