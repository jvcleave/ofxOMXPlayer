#pragma once


#include "OMXPlayerVideoBase.h"
#include "OMXVideo.h"




class OMXPlayerVideo : public OMXPlayerVideoBase
{
	public:

		bool                      m_Deinterlace;
		float                     m_display_aspect;

		bool                      m_hdmi_clock_sync;
		OMXVideo* nonTextureDecoder;
		OMXPlayerVideo();
		~OMXPlayerVideo();

		bool Open(OMXStreamInfo& hints, OMXClock *av_clock, bool deinterlace, bool hdmi_clock_sync, float display_aspect);
		bool openDecoder();
		bool close();
		ofRectangle displayRect;
		void setDisplayRect(ofRectangle& rectangle);
		bool validateDisplayRect(ofRectangle& rectangle);
};
