#pragma once

#include "ofMain.h"

#include "OMXDecoderBase.h"


class COMXVideo : public OMXDecoderBase
{
	public:
		COMXVideo();
		~COMXVideo();
		bool Open(COMXStreamInfo& hints, OMXClock *clock, float display_aspect = 0.0f, bool deinterlace = false, bool hdmi_clock_sync = false);
		

		bool  Decode(uint8_t *pData, int iSize, double pts);

		COMXCoreComponent m_omx_image_fx;
		COMXCoreTunnel     m_omx_tunnel_image_fx;



		bool				m_deinterlace;
		bool				m_hdmi_clock_sync;
		bool				m_syncclock;
		ofRectangle displayRect;
		void setDisplayRect(ofRectangle& rectangle);
		void configureDisplay();
		void updateFrameCount();
		void onUpdate(ofEventArgs& args);
	
	int getCurrentFrame();
	void resetFrameCounter();
private:
	int frameCounter;
	int frameOffset;
	
	
};