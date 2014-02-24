#pragma once

#include "ofMain.h"
#include "ofAppEGLWindow.h"
#include "OMXDecoderBase.h"

class OMXEGLImage : public OMXDecoderBase
{
public:
	OMXEGLImage();
	~OMXEGLImage();
	
	bool Open(COMXStreamInfo &hints, OMXClock *clock, ofxOMXPlayerSettings& settings);
	bool PortSettingsChanged() {return true;};
	int  Decode(uint8_t *pData, int iSize, double dts, double pts);
	int  Decode(uint8_t *pData, int iSize, double pts);
	
	ofFbo fbo;
	ofTexture texture;
	EGLImageKHR eglImage;
	GLuint textureID;
	ofAppEGLWindow*		appEGLWindow;
	EGLDisplay			display;
	EGLContext			context;
	bool hasGenerated;
	int videoWidth;
	int videoHeight;
	unsigned char * pixels;
	bool isExiting;
	ofxOMXPlayerSettings settings;
	
	void updatePixels()
	{
		fbo.begin(false);
		ofClear(0, 0, 0, 0);
		texture.draw(0, 0);
		glReadPixels(0,0,videoWidth, videoHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		fbo.end();
	}
	
	
	void generateEGLImage(int videoWidth_, int videoHeight_)
	{	
		//Lock();
		ofLogVerbose(__func__) << "START";
		bool needsRegeneration = false;
		if (videoWidth != videoWidth_) 
		{
			needsRegeneration = true;
			videoWidth = videoWidth_;
		}
		if (videoHeight != videoHeight_) 
		{
			needsRegeneration = true;
			videoHeight = videoHeight_;
		}
		
		if (hasGenerated) 
		{
			
			if (!needsRegeneration) 
			{
				return;
			}else 
			{
				destroyEGLImage();
			}
		}
		
		if (appEGLWindow == NULL) 
		{
			appEGLWindow = (ofAppEGLWindow *) ofGetWindowPtr();
		}
		
		if (!appEGLWindow) 
		{
			ofLogError(__func__) << "appEGLWindow is NULL - RETURNING";
			return;
		}
		if (display == NULL) 
		{
			display = appEGLWindow->getEglDisplay();
		}
		if (context == NULL) 
		{
			context = appEGLWindow->getEglContext();
		}
		
		
		if (texture.isAllocated()) 
		{
			texture.clear();
		}
		
		ofFbo::Settings fboSettings;
		fboSettings.width = videoWidth;
		fboSettings.height = videoHeight;
		fboSettings.wrapModeVertical = GL_REPEAT;	// GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER etc.
		fboSettings.wrapModeHorizontal = GL_REPEAT; // GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER etc.
		//int		wrapModeHorizontal;		// GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER etc.
		//int		wrapModeVertical;		// GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER etc.
		
		fbo.allocate(fboSettings);
		//fbo.allocate(videoWidth, videoHeight, GL_RGBA);
		
		
		texture.allocate(videoWidth, videoHeight, GL_RGBA);
		//Video renders upside down and backwards when Broadcom proprietary tunnels are enabled
		//may be resolved in future firmare
		//https://github.com/raspberrypi/firmware/issues/176
		
		if (settings.doFlipTexture) 
		{
			texture.getTextureData().bFlipTexture = true;
		}
		texture.setTextureWrap(GL_REPEAT, GL_REPEAT);
		textureID = texture.getTextureData().textureID;
		
		ofLogVerbose(__func__) << "textureID: " << textureID;
		ofLogVerbose(__func__) << "tex.isAllocated(): " << texture.isAllocated();
		
		glEnable(GL_TEXTURE_2D);
		
		// setup first texture
		int dataSize = videoWidth * videoHeight * 4;
		
		GLubyte* pixelData = new GLubyte [dataSize];
		
		
		memset(pixelData, 0xff, dataSize);  // white texture, opaque
		
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, videoWidth, videoHeight, 0,
					 GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
		
		delete[] pixelData;
		
		
		// Create EGL Image
		eglImage = eglCreateImageKHR(
									 display,
									 context,
									 EGL_GL_TEXTURE_2D_KHR,
									 (EGLClientBuffer)textureID,
									 0);
		glDisable(GL_TEXTURE_2D);
		if (eglImage == EGL_NO_IMAGE_KHR) 
		{
			ofLogError()	<< "Create EGLImage FAIL";
		}
		else
		{
			ofLogVerbose()	<< "Create EGLImage PASS";
			pixels = new unsigned char[dataSize];
			hasGenerated = true;
		}
		//UnLock();
	}
	
	void destroyEGLImage()
	{
		//Lock();
		if (eglImage) 
		{
			if (eglDestroyImageKHR(display, eglImage)) 
			{
				ofLogVerbose(__func__) << "eglDestroyImageKHR PASS";
			}
			else
			{
				ofLogError(__func__) << "eglDestroyImageKHR FAIL";
			}
			eglImage = NULL;
		}
		//UnLock();
		/*if (texture.isAllocated()) 
		 {
		 texture.clear();
		 }*/
	}
	
};
