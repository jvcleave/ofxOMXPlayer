#pragma once

#include "ofMain.h"

#include "OMXDecoderBase.h"


class COMXVideo : public OMXDecoderBase
{
public:
	COMXVideo();

	bool Open(COMXStreamInfo &hints, OMXClock *clock, float display_aspect = 0.0f, bool deinterlace = false, bool hdmi_clock_sync = false);
	void Close();
	
	int  Decode(uint8_t *pData, int iSize, double dts, double pts);

	
	COMXCoreComponent m_omx_image_fx;
	COMXCoreTunel     m_omx_tunnel_image_fx;



	bool				m_deinterlace;
	bool				m_hdmi_clock_sync;
	bool				m_syncclock;
	
	~COMXVideo()
	{
		if (m_is_open)
		{
			Close();
			ofLogVerbose() <<  "~OMXDecoderBase called without Close!";
		}
	}
};