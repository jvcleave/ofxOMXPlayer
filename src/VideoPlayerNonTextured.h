#pragma once


#include "VideoPlayerBase.h"
#include "VideoDecoderNonTextured.h"




class VideoPlayerNonTextured : public VideoPlayerBase
{
	public:

		bool                      m_Deinterlace;
		float                     m_display_aspect;

		bool                      m_hdmi_clock_sync;
		VideoDecoderNonTextured* nonTextureDecoder;
		VideoPlayerNonTextured();
		~VideoPlayerNonTextured();

		bool open(OMXStreamInfo& hints, OMXClock *av_clock, bool deinterlace, bool hdmi_clock_sync, float display_aspect);
		bool openDecoder();
		bool close();
		ofRectangle displayRect;
		void setDisplayRect(ofRectangle& rectangle);
		bool validateDisplayRect(ofRectangle& rectangle);
};
