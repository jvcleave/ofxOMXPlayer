#pragma once

#include "ofMain.h"
#include "Component.h"
#include "StreamInfo.h"


class OMXDisplay
{
public:
    OMXDisplay()
    {
        doMirror = false;
        isReady = false;
       
    };
    
    
    OMX_ERRORTYPE setup(Component& renderComponent_, StreamInfo& streamInfo_, ofRectangle& displayRect_)
    {
        
        float display_aspect = 1.0; 
        renderComponent = renderComponent_;
        displayRect = displayRect_;
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
            float fAspect = (float)streamInfo.aspect / (float)streamInfo.width * (float)streamInfo.height;
            
            float par = 0.0f;
            if(streamInfo.aspect)
            {
                par = fAspect/display_aspect;
            }
            //float par = streamInfo.aspect ? fAspect/display_aspect : 0.0f;
            // only set aspect when we have a aspect and display doesn't match the aspect
            if(par != 0.0f && fabs(par - 1.0f) > 0.01f)
            {
                
                
                AVRational aspect;
                aspect = av_d2q(par, 100);
                error = setPixel(aspect.num, aspect.den);
                OMX_TRACE(error)
                ofLog(OF_LOG_VERBOSE, "Aspect : num %d den %d aspect %f pixel aspect %f\n", aspect.num, aspect.den, streamInfo.aspect, par);
                
            }
        }
        OMX_TRACE(error);
        
        return error;
    }
    OMX_ERRORTYPE setPixel(int x, int y)
    {
        configDisplay.set      = OMX_DISPLAY_SET_PIXEL;
        configDisplay.pixel_x  = x;
        configDisplay.pixel_y  = y;
        return applyConfig();
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
        return setCrop(cropRectangle.x, cropRectangle.y, cropRectangle.width, cropRectangle.height);
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
    StreamInfo streamInfo;
    bool doMirror;
    bool isReady;
               
};