#include "testApp.h"

#if 0
void testApp::DecoderEventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData)
{
	COMXCoreComponent *ctx = static_cast<COMXCoreComponent*>(pAppData);
	string name = ctx->GetName();
	ofLogVerbose() << "testApp DecoderEventHandler " << "name: " << name;
	
	/*if (name == "OMX.broadcom.egl_render") 
	{
		
		OMX_STATETYPE  checkState = ctx->GetState();
		switch (checkState) 
		{
			case OMX_StateExecuting:
				ofLogVerbose () << "testApp checkState OMX_StateExecuting";
				break;
			case OMX_StateIdle:
				ofLogVerbose () << "testApp checkState OMX_StateIdle";
				break;
			case OMX_StateLoaded:
				ofLogVerbose () << "testApp checkState OMX_StateLoaded";
				break;
			case OMX_StateInvalid:
				ofLogVerbose () << "testApp checkState OMX_StateInvalid";
				break;
			case OMX_StateWaitForResources:
				ofLogVerbose () << "testApp checkState OMX_StateWaitForResources";
				break;
			default:
				ofLogVerbose () <<"testApp checkState STATE: " << checkState;
				break;
		}
		ofLogVerbose() << "testApp DecoderEventHandler " << "InputPort: " << ctx->GetInputPort();
		ofLogVerbose() << "testApp DecoderEventHandler " << "OutputPort: " << ctx->GetOutputPort() << endl;
	}
	unsigned int      GetInputPort()   { return m_input_port;    };
	unsigned int      GetOutputPort()  { return m_output_port;   };
	std::string       GetName()        { return m_componentName; };*/
}

void testApp::DecoderEmptyBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
	//ofLogVerbose() << "DecoderEmptyBufferDone<-------------------------";
}

void testApp::DecoderFillBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
	ofLogVerbose() << "DecoderFillBufferDone<-------------------------";
}
#endif
//--------------------------------------------------------------
void testApp::setup(){
	isReady = false;
	m_bMpeg               = false;
	m_has_video           = false;
	m_thread_player       = true;
	m_omx_pkt = NULL;
	m_buffer_empty = true;
	
	
	
	
	ofSetLogLevel(OF_LOG_VERBOSE);
	g_RBP.Initialize();
	g_OMX.Initialize();
	
	m_av_clock = new OMXClock(); 
	bool m_dump_format = false;
	string m_filename = ofToDataPath("test.h264", true);
	//
	//m_filename = "/home/pi/P1020079.MOV";
	//m_filename = "/home/pi/sorted_1280x720.mp4";
	//m_filename = "/home/pi/lily_allen_-the_fear-_mk_ii_HD.mp4";
	m_filename = "/home/pi/super8_vimeo_480x270.mp4";
	if(m_omx_reader.Open(m_filename.c_str(), m_dump_format))
	{
		
		ofLogVerbose() << "m_omx_reader successfully opened: " << m_filename;
		m_has_video     = m_omx_reader.VideoStreamCount();
		ofLogVerbose() << "m_bMpeg " << m_bMpeg;
		ofLogVerbose() << "m_has_video " << m_has_video;
		
		if(m_av_clock->OMXInitialize(m_has_video, false))
		{
			ofLogVerbose() << "Clock Init success";
			
			m_omx_reader.GetHints(OMXSTREAM_VIDEO, m_hints_video);
			
			// get display aspect
			/*memset(&current_tv_state, 0, sizeof(TV_GET_STATE_RESP_T));
			m_BcmHost.vc_tv_get_state(&current_tv_state);
			
			if(current_tv_state.width && current_tv_state.height)
			{
				m_display_aspect = (float)current_tv_state.width / (float)current_tv_state.height;
				ofLogVerbose() << "(float)current_tv_state.width " << (float)current_tv_state.width;
				ofLogVerbose() << "(float)current_tv_state.height " << (float)current_tv_state.height;
				ofLogVerbose() << "m_display_aspect: " << m_display_aspect;
			}*/
			if(m_has_video)
			{
				bool didOpenVideo = m_player_video.Open(m_hints_video, m_av_clock);
				
				if (didOpenVideo) 
				{
					ofLogVerbose() << "Opened video!";
					m_av_clock->SetSpeed(DVD_PLAYSPEED_NORMAL);
					m_av_clock->OMXStateExecute();
					m_av_clock->OMXStart();
					isReady  = true;

				}else 
				{
					ofLogError() << "could not open video";
				}
			}else 
			{
				ofLogError() << "file has no video";
			}
		}else 
		{
			ofLogError() << "m_av_clock could not init";
		}
	}else 
	{
		ofLogError() << "m_omx_reader could not open" << m_filename;
	}


}

//--------------------------------------------------------------
void testApp::update()
{
	if(!isReady)
	{
		return;
	}

	
	
	/*if(m_omx_reader.IsEof() && !m_omx_pkt)
    {
		if (!m_player_video.GetCached())
		{
			break;
		}
		// Abort audio buffering, now we're on our own
		if (m_buffer_empty)
		{
			m_av_clock->OMXResume();
		}
		OMXClock::OMXSleep(10);
		continue;
    }*/
	if(!m_omx_pkt)
	{
		m_omx_pkt = m_omx_reader.Read();
	}
	if(m_has_video && m_omx_pkt && m_omx_reader.IsActive(OMXSTREAM_VIDEO, m_omx_pkt->stream_index))
    {
		if(m_player_video.AddPacket(m_omx_pkt))
		{
			m_omx_pkt = NULL;
		}else 
		{
			OMXClock::OMXSleep(10);
		}

	}else 
	{
		if(m_omx_pkt)
		{
			m_omx_reader.FreePacket(m_omx_pkt);
			m_omx_pkt = NULL;
		}
	}

	
	
}

//--------------------------------------------------------------
void testApp::draw(){
	if(!isReady)
	{
		return;
	}
	ofDrawBitmapStringHighlight(ofToString(m_av_clock->OMXMediaTime()), 200, 200, ofColor::black, ofColor::yellow);
	/*ofLog(OF_LOG_VERBOSE, "OMXMediaTime: %8.02f", m_av_clock->OMXMediaTime());
	ofLog(OF_LOG_VERBOSE, "GetDecoderBufferSize: %8d", m_player_video.GetDecoderBufferSize());
	ofLog(OF_LOG_VERBOSE, "GetDecoderFreeSpace: %8d", m_player_video.GetDecoderFreeSpace());
	ofLog(OF_LOG_VERBOSE, "GetCached: %8d", m_player_video.GetCached());*/
}


void testApp::exit(){
	if (isReady) 
	{
		m_av_clock->OMXStop();
		m_av_clock->OMXStateIdle();
		m_player_video.Close();
		if(m_omx_pkt)
		{
			m_omx_reader.FreePacket(m_omx_pkt);
			m_omx_pkt = NULL;
		}
		m_omx_reader.Close();
	}
	
	
	g_OMX.Deinitialize();
	g_RBP.Deinitialize();
}
//--------------------------------------------------------------
void testApp::keyPressed  (int key){

	
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){


}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}


//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}

