#include "testApp.h"

void testApp::generateEGLImage()
{
	ofDisableArbTex();
	
	ofAppEGLWindow *appEGLWindow = (ofAppEGLWindow *) ofGetWindowPtr();
	display = appEGLWindow->getEglDisplay();
	context = appEGLWindow->getEglContext();
	ofLogVerbose() << "videoWidth: " << videoWidth;
	ofLogVerbose() << "videoHeight: " << videoHeight;
	
	textureSource.allocate(videoWidth, videoHeight, GL_RGBA);
	textureSource.getTextureData().bFlipTexture = true;
	textureSource.setTextureWrap(GL_REPEAT, GL_REPEAT);
	//textureSource.setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
	//textureSource.setTextureWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	texture = textureSource.getTextureData().textureID;
	
	
	glEnable(GL_TEXTURE_2D);
	
	
	// setup first texture
	int dataSize = videoWidth * videoHeight * 4;
	
	GLubyte* pixelData = new GLubyte [dataSize];
	
	
    memset(pixelData, 0xff, dataSize);  // white texture, opaque
	
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, videoWidth, videoHeight, 0,
				 GL_RGBA, GL_UNSIGNED_BYTE, pixelData); //I think this is why other videos won't display
	
	delete[] pixelData;
	
	
	// Create EGL Image
	eglImage = eglCreateImageKHR(
								 display,
								 context,
								 EGL_GL_TEXTURE_2D_KHR,
								 (EGLClientBuffer)texture,
								 0);
    glDisable(GL_TEXTURE_2D);
	if (eglImage == EGL_NO_IMAGE_KHR)
	{
		ofLogError()	<< "Create EGLImage FAIL";
		return;
	}
	else
	{
		ofLogVerbose()	<< "Create EGLImage PASS";
	}
	
}
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
	m_filename = "/opt/vc/src/hello_pi/hello_video/test.h264";
	//m_filename = "/home/pi/super8_vimeo_480x270.mp4";
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
			videoWidth = m_hints_video.width;
			videoHeight = m_hints_video.height;
			
			generateEGLImage();
			if(m_has_video)
			{
				bool didOpenVideo = m_player_video.Open(m_hints_video, m_av_clock, eglImage);
				
				if (didOpenVideo) 
				{
					ofLogVerbose() << "Opened video!";
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
	textureSource.draw(0, 0, ofGetWidth(), ofGetHeight());
	ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 200, 200, ofColor::black, ofColor::yellow);
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
	if (eglImage !=NULL) 
	{
		eglDestroyImageKHR(display, eglImage);
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

