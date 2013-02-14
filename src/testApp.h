#pragma once

#include "ofMain.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
};
#include "RBP.h"
#include "OMXClock.h"
#include "OMXPlayerVideo.h"
class testApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		void exit();
	
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);		

		CRBP                  g_RBP;
		COMXCore              g_OMX;
		OMXClock * m_av_clock;
		 
		OMXPlayerVideo    m_player_video;
		OMXReader         m_omx_reader;
	
	COMXStreamInfo    m_hints_video;
	
	bool m_bMpeg;
	bool m_has_video;
	bool m_Deinterlace;
	bool m_thread_player;
	
	DllBcmHost        m_BcmHost;
	TV_GET_STATE_RESP_T current_tv_state;
	float             m_display_aspect;
	
	struct timespec starttime;
	struct timespec endtime;
	bool isReady;
	
	OMXPacket* m_omx_pkt;
	bool m_buffer_empty;
	void DecoderEventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData);
	void DecoderEmptyBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer);
	void DecoderFillBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer);

};

