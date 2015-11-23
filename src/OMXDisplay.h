#pragma once

#include "ofMain.h"
#include "Component.h"
#include "StreamInfo.h"


class OMXDisplay
{
public:
    
    void onUpdate(ofEventArgs& args)
    {
        if(!isReady)
        {
            return;
        }
        
        stringstream info;
        info << "fullscreen: " << configDisplay.fullscreen << endl; 
        info << "noaspect: " << configDisplay.noaspect << endl;
        info << "src_rect x: " << configDisplay.src_rect.x_offset << endl;  
        info << "src_rect y: " << configDisplay.src_rect.y_offset << endl;  
        info << "src_rect width: " << configDisplay.src_rect.width << endl;    
        info << "src_rect height: " << configDisplay.src_rect.height << endl;    
        
        info << "dest_rect x: " << configDisplay.dest_rect.x_offset << endl; 
        info << "dest_rect y: " << configDisplay.dest_rect.y_offset << endl; 
        info << "dest_rect width: " << configDisplay.dest_rect.width << endl;    
        info << "dest_rect height: " << configDisplay.dest_rect.height << endl;
        
        info << "pixel_x: " << configDisplay.pixel_x << endl;
        info << "pixel_y: " << configDisplay.pixel_y << endl;
        info << "transform: " << configDisplay.transform << endl;
        
        ofLogVerbose() << info.str();
        
    }
    
    
    OMXDisplay()
    {
        doMirror = false;
        isReady = false;
       
        ofAddListener(ofEvents().update, this, &OMXDisplay::onUpdate);
    };
    
   
    OMX_ERRORTYPE setup(Component& renderComponent_, StreamInfo& streamInfo_)
    {
        
        
        renderComponent = renderComponent_;
        displayRect.set(0, 0, streamInfo.width, streamInfo.height);
        cropRect = displayRect;
        streamInfo = streamInfo_;
        
        
        OMX_INIT_STRUCTURE(configDisplay);
        configDisplay.nPortIndex = renderComponent.getInputPort();
        isReady = true;
        
        OMX_ERRORTYPE error = OMX_ErrorNone;
        if (displayRect.getWidth()>0)
        {
            
            error = setFullScreen(false);
            OMX_TRACE(error);
                        
            error = setDestinationRect(displayRect);
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
    
    OMX_ERRORTYPE setPixelAspectRatio(float pixelAspectRatio)
    {
        OMX_ERRORTYPE error = OMX_ErrorNone;
        
        // only set aspect when we have a aspect and display doesn't match the aspect
        if(pixelAspectRatio != 0.0f && fabs(pixelAspectRatio - 1.0f) > 0.01f)
        {
            
            
            AVRational aspectRatio;
            aspectRatio = av_d2q(pixelAspectRatio, 100);
            
            configDisplay.set      = OMX_DISPLAY_SET_PIXEL;
            configDisplay.pixel_x  = aspectRatio.num;
            configDisplay.pixel_y  = aspectRatio.den;
            error = applyConfig();
            OMX_TRACE(error)
            ofLog(OF_LOG_VERBOSE, "Aspect : num %d den %d aspect %f pixel aspect %f\n", aspectRatio.num, aspectRatio.den, streamInfo.aspect, pixelAspectRatio);
            
        }
        
        
        return error;
    }
    
    OMX_ERRORTYPE setDestinationRect(ofRectangle& rectangle)
    {
        configDisplay.set     = OMX_DISPLAY_SET_DEST_RECT;
        configDisplay.dest_rect.x_offset  = displayRect.x;
        configDisplay.dest_rect.y_offset  = displayRect.y;
        configDisplay.dest_rect.width     = displayRect.getWidth();
        configDisplay.dest_rect.height    = displayRect.getHeight();
        
        return applyConfig();
    }
    
    OMX_ERRORTYPE setFullScreen(bool doFullScreen)
    {
        configDisplay.set     = OMX_DISPLAY_SET_FULLSCREEN;
        configDisplay.fullscreen = (OMX_BOOL)doFullScreen;
        return applyConfig();
    }
    
    OMX_ERRORTYPE applyConfig()
    {
        if(!isReady) return OMX_ErrorNone;
        
        return renderComponent.setConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
    }
    
    OMX_ERRORTYPE setCrop(ofRectangle& cropRectangle)
    {
        if(cropRect == cropRectangle)
        {
            return OMX_ErrorNone;
        }
        cropRect = cropRectangle;
        return setCrop(cropRect.x, cropRect.y, cropRect.width, cropRect.height);
    }
    
    OMX_ERRORTYPE setCrop(int x, int y, int w, int h)
    {
        configDisplay.set = OMX_DISPLAY_SET_SRC_RECT;
        configDisplay.src_rect.x_offset  = x;
        configDisplay.src_rect.y_offset  = y;
        configDisplay.src_rect.width     = w;
        configDisplay.src_rect.height    = h;
        return applyConfig();
    }
    
    OMX_ERRORTYPE enableAspectRatio(bool doEnable)
    {
        configDisplay.set = OMX_DISPLAY_SET_NOASPECT;
        configDisplay.noaspect = (OMX_BOOL)doEnable;
        return applyConfig();
    }
    
    /*
     OMX_ERRORTYPE demo(OMX_DISPLAYTRANSFORMTYPE type)
    {
        OMX_ERRORTYPE error = OMX_ErrorNone;
        if (!isReady) 
        {
            return error;
        }
        error = setDestinationRect(0, 0, streamInfo.width, streamInfo.height);
        error = setFullScreen(false);
        error = enableAspectRatio(false);
        
       return error();
        
    }*/
    
    
    OMX_ERRORTYPE rotateDisplay(OMX_DISPLAYTRANSFORMTYPE type)
    {
        OMX_ERRORTYPE error = OMX_ErrorNone;
        if (!isReady) 
        {
            return error;
        }
        configDisplay.set = OMX_DISPLAY_SET_TRANSFORM;
        configDisplay.transform  = type;
        
        error = applyConfig();
        OMX_TRACE(error);
        return error;
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
        
        if(doMirror)
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
        
        OMX_ERRORTYPE error = rotateDisplay(type);
        OMX_TRACE(error);
        return error;
    }
    
    OMX_ERRORTYPE rotateRandom()
    {
        int randomRotation = ofRandom(0, 7);
        ofLogVerbose() << "randomRotation: " << randomRotation;
        OMX_ERRORTYPE error = rotateDisplay((OMX_DISPLAYTRANSFORMTYPE)randomRotation);
        OMX_TRACE(error);
        return error;
    }

    
    OMX_ERRORTYPE cropRandom()
    {

        OMX_ERRORTYPE error = setCrop(0, 0, ofRandom(streamInfo.width/4, streamInfo.width), ofRandom(streamInfo.height/4, streamInfo.height));
        OMX_TRACE(error);
        return error;
    }
    
    
    OMX_ERRORTYPE setDisplayRect(ofRectangle& rectangle)
    {
        bool hasChanged = (displayRect != rectangle);
        OMX_ERRORTYPE error = OMX_ErrorNone;
        if (hasChanged) 
        {
            displayRect = rectangle;
        }else
        {
            return error;
        }
        if (hasChanged) 
        {
            
            error = setFullScreen(false);
            OMX_TRACE(error);
            error = setDestinationRect(rectangle);
            OMX_TRACE(error);
        }
        return error;
    }
    
    
    OMX_CONFIG_DISPLAYREGIONTYPE configDisplay;
    Component renderComponent;
    ofRectangle displayRect;
    ofRectangle cropRect;
    StreamInfo streamInfo;
    bool doMirror;
    bool isReady;
               
};