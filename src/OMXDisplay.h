#pragma once

#include "ofMain.h"
#include "Component.h"
#include "StreamInfo.h"
#include "ofxOMXPlayerSettings.h"


class OMXDisplay
{
public:
    
    OMX_CONFIG_DISPLAYREGIONTYPE omxConfig;
    OMX_CONFIG_DISPLAYREGIONTYPE omxConfigDefaults;
    Component* renderComponent;
    StreamInfo streamInfo;
    DirectDisplayOptions options;
    ofxOMXPlayerSettings playerSettings;
    bool isReady;
    bool previousMirror;
    int previousRotationDegrees;
    OMXDisplay()
    {

        isReady = false;
        previousRotationDegrees = 0;
        OMX_INIT_STRUCTURE(omxConfig);
        omxConfigDefaults = omxConfig;
        ofAddListener(ofEvents().update, this, &OMXDisplay::onUpdate);
    };
    
    ~OMXDisplay()
    {
        ofRemoveListener(ofEvents().update, this, &OMXDisplay::onUpdate);
    }
    
    
    DirectDisplayOptions& getConfig()
    {
        return options;
    }
    
    OMX_ERRORTYPE setup(Component* renderComponent_, StreamInfo& streamInfo_, ofxOMXPlayerSettings& playerSettings_)
    {
        
        
        renderComponent = renderComponent_;
        streamInfo = streamInfo_;
        playerSettings = playerSettings_;
        options = playerSettings.directDisplayOptions;
        
        previousMirror = options.doMirror;        
        if(options.drawRectangle.getArea()==0)
        {
           options.drawRectangle.set(0, 0, streamInfo.width, streamInfo.height); 
        }

        
        omxConfig.nPortIndex = renderComponent->getInputPort();
        
        
    
        
        isReady = true;
        
        OMX_ERRORTYPE error = OMX_ErrorNone;
        if (options.drawRectangle.getWidth()>0)
        {
            
            error = applyConfig(); 
            OMX_TRACE(error);
            
        }else
        {
            float videoAspectRatio = (float)streamInfo.aspect / (float)streamInfo.width * (float)streamInfo.height;
            float displayAspectRatio = 1.0; 
            
            float pixelAspectRatio = 0.0f;
            if(streamInfo.aspect)
            {
                pixelAspectRatio = videoAspectRatio/displayAspectRatio;
            }
            error = setPixelAspectRatio(pixelAspectRatio);
        }
        
        OMX_TRACE(error);
        return error;
        
    }
    
    void onUpdate(ofEventArgs& args)
    {
        if(!isReady)
        {
            return;
        }
        
        if(previousMirror != options.doMirror)
        {
            rotateDisplay(previousRotationDegrees);
            previousMirror = options.doMirror;
        }
        
        OMX_ERRORTYPE error =applyConfig();
        OMX_TRACE(error); 

#ifdef DEBUG_VIDEO_DISPLAY
        stringstream info;
        info << "fullscreen: " << omxConfig.fullscreen << endl; 
        info << "noaspect: " << omxConfig.noaspect << endl;
        info << "src_rect x: " << omxConfig.src_rect.x_offset << endl;  
        info << "src_rect y: " << omxConfig.src_rect.y_offset << endl;  
        info << "src_rect width: " << omxConfig.src_rect.width << endl;    
        info << "src_rect height: " << omxConfig.src_rect.height << endl;    
        
        info << "dest_rect x: " << omxConfig.dest_rect.x_offset << endl; 
        info << "dest_rect y: " << omxConfig.dest_rect.y_offset << endl; 
        info << "dest_rect width: " << omxConfig.dest_rect.width << endl;    
        info << "dest_rect height: " << omxConfig.dest_rect.height << endl;
        
        info << "pixel_x: " << omxConfig.pixel_x << endl;
        info << "pixel_y: " << omxConfig.pixel_y << endl; 
        info << "transform: " << omxConfig.transform << endl;
        
        
        info << "pixel_x: " << omxConfig.pixel_x << endl;
        info << "pixel_y: " << omxConfig.pixel_y << endl; 
        info << "transform: " << omxConfig.transform << endl;
        
        info << "mode: " << omxConfig.mode << endl;
        
        info << "layer: " << omxConfig.layer << endl;
        info << "alpha: " << omxConfig.alpha << endl;
        
        info << "copyprotect_required: " << omxConfig.copyprotect_required << endl;
        info << "wfc_context_width: " << omxConfig.wfc_context_width << endl;
        info << "wfc_context_height: " << omxConfig.wfc_context_height << endl;
        
    
        
        ofLogVerbose() << info.str();
        ofLogVerbose() << "options.drawRectangle: " << options.drawRectangle;
        ofLogVerbose() << "options.drawRectangle.getArea(): " << options.drawRectangle.getArea();
#endif
        
    }
    
    OMX_ERRORTYPE applyConfig()
    {
        omxConfig.set = (OMX_DISPLAYSETTYPE)(OMX_DISPLAY_SET_DEST_RECT| OMX_DISPLAY_SET_SRC_RECT | OMX_DISPLAY_SET_FULLSCREEN | OMX_DISPLAY_SET_NOASPECT | OMX_DISPLAY_SET_TRANSFORM | OMX_DISPLAY_SET_ALPHA | OMX_DISPLAY_SET_PIXEL | OMX_DISPLAY_SET_MODE);
        
        omxConfig.dest_rect.x_offset  = options.drawRectangle.x;
        omxConfig.dest_rect.y_offset  = options.drawRectangle.y;
        omxConfig.dest_rect.width     = options.drawRectangle.getWidth();
        omxConfig.dest_rect.height    = options.drawRectangle.getHeight();
        
        omxConfig.src_rect.x_offset  = options.cropRectangle.x;
        omxConfig.src_rect.y_offset  = options.cropRectangle.y;
        omxConfig.src_rect.width     = options.cropRectangle.getWidth();
        omxConfig.src_rect.height    = options.cropRectangle.getHeight();
        
        omxConfig.fullscreen = (OMX_BOOL)options.doFullScreen;
        omxConfig.noaspect   = (OMX_BOOL)options.noAspectRatio;    
        omxConfig.transform  = (OMX_DISPLAYTRANSFORMTYPE)options.rotationIndex;
        //int alpha = (ofGetFrameNum() % 255); 
        omxConfig.alpha  = options.alpha;
        omxConfig.pixel_x  = options.pixelAspectX;
        omxConfig.pixel_y  = options.pixelAspectY;
        if(options.doForceFill)
        {
            omxConfig.mode  = OMX_DISPLAY_MODE_FILL;  
        }else
        {
            omxConfig.mode = omxConfigDefaults.mode;
        }
        //omxConfig.mode  = OMX_DISPLAY_MODE_FILL;
        //omxConfig.mode  = (OMX_DISPLAYMODETYPE)ofRandom(0, 5);
        //return OMX_ErrorNone;
        return renderComponent->setConfig(OMX_IndexConfigDisplayRegion, &omxConfig);
    }

 
    
   

    
    OMX_ERRORTYPE setPixelAspectRatio(float pixelAspectRatio)
    {
        OMX_ERRORTYPE error = OMX_ErrorNone;
        
        // only set aspect when we have a aspect and display doesn't match the aspect
        if(pixelAspectRatio != 0.0f && fabs(pixelAspectRatio - 1.0f) > 0.01f)
        {
            
            
            AVRational aspectRatio;
            aspectRatio = av_d2q(pixelAspectRatio, 100); 
            
            options.pixelAspectX = aspectRatio.num;
            options.pixelAspectY = aspectRatio.den;

            error = applyConfig();
            OMX_TRACE(error)
            ofLog(OF_LOG_VERBOSE, "Aspect : num %d den %d aspect %f pixel aspect %f\n", aspectRatio.num, aspectRatio.den, streamInfo.aspect, pixelAspectRatio);
            
        }
        
        
        return error;
    }
   
    
    OMX_ERRORTYPE rotateDisplay(OMX_DISPLAYTRANSFORMTYPE type)
    {

        options.rotationIndex = (int)type;
        return applyConfig();
    }
    
    
    OMX_ERRORTYPE rotateDisplay(int degreesClockWise)
    {
        OMX_DISPLAYTRANSFORMTYPE type = OMX_DISPLAY_ROT0;
        
        if(degreesClockWise<0)
        {
            type = OMX_DISPLAY_ROT0;
        }
        if(degreesClockWise >=90 && degreesClockWise < 180)
        {
            type = OMX_DISPLAY_ROT90;
        }
        if(degreesClockWise >=180 && degreesClockWise < 270)
        {
            type = OMX_DISPLAY_ROT270;
        }
        
        if(options.doMirror)
        {
            switch (type) 
            {
                case OMX_DISPLAY_ROT0:
                {
                    type = OMX_DISPLAY_MIRROR_ROT0;
                    break;
                }
                case OMX_DISPLAY_ROT90:
                {
                    type = OMX_DISPLAY_MIRROR_ROT90;
                    break;
                }
                case OMX_DISPLAY_ROT270:
                {
                    type = OMX_DISPLAY_MIRROR_ROT270;
                    break;
                }
                    
                default:
                    break;
            }
        }
        previousRotationDegrees = degreesClockWise;
        return rotateDisplay(type);
    }
    
    OMX_ERRORTYPE rotateRandom()
    {
        int randomRotation = ofRandom(0, 7);
        return rotateDisplay((OMX_DISPLAYTRANSFORMTYPE)randomRotation);
    }

    
    OMX_ERRORTYPE cropRandom()
    {
        options.cropRectangle.set(0, 0, ofRandom(streamInfo.width/4, streamInfo.width), ofRandom(streamInfo.height/4, streamInfo.height));
        return applyConfig();
    }

    
    
    
 
               
};