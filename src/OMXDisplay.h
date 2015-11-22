#pragma once

#include "ofMain.h"
#include "Component.h"
#include "OMXStreamInfo.h"


class OMXDisplay
{
public:
    OMXDisplay()
    {
    
    };
    
    
    OMX_ERRORTYPE setup(Component& renderComponent_, OMXStreamInfo& streamInfo_, ofRectangle& displayRect_)
    {
        float display_aspect = 1.0; 
        renderComponent = renderComponent_;
        displayRect = displayRect_;
        streamInfo = streamInfo_;
        OMX_ERRORTYPE error = OMX_ErrorNone;
        OMX_CONFIG_DISPLAYREGIONTYPE configDisplay;
        OMX_INIT_STRUCTURE(configDisplay);
        configDisplay.nPortIndex = renderComponent.getInputPort();
        
        if (displayRect.getWidth()>0)
        {
            configDisplay.set     = OMX_DISPLAY_SET_FULLSCREEN;
            configDisplay.fullscreen = OMX_FALSE;
            
            error = renderComponent.setConfig(OMX_IndexConfigDisplayRegion, &configDisplay); 
            OMX_TRACE(error);
            
            configDisplay.set     = OMX_DISPLAY_SET_DEST_RECT;
            configDisplay.dest_rect.x_offset  = displayRect.x;
            configDisplay.dest_rect.y_offset  = displayRect.y;
            configDisplay.dest_rect.width     = displayRect.getWidth();
            configDisplay.dest_rect.height    = displayRect.getHeight();
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
            bool doDisplayChange = true;
            if(doDisplayChange)
            {
                if(par != 0.0f && fabs(par - 1.0f) > 0.01f)
                {
                    
                    
                    AVRational aspect;
                    aspect = av_d2q(par, 100);
                    configDisplay.set      = OMX_DISPLAY_SET_PIXEL;
                    configDisplay.pixel_x  = aspect.num;
                    configDisplay.pixel_y  = aspect.den;
                    ofLog(OF_LOG_VERBOSE, "Aspect : num %d den %d aspect %f pixel aspect %f\n", aspect.num, aspect.den, streamInfo.aspect, par);
                    error = renderComponent.setConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
                    OMX_TRACE(error);
                }
                
            }
        }
            
        
        
        error = renderComponent.setConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
        OMX_TRACE(error);
        return error;
    }
    
    void setDisplayRect(ofRectangle& rectangle)
    {
        bool hasChanged = (displayRect != rectangle);
        
        if (hasChanged) 
        {
            displayRect = rectangle;
        }
        if (hasChanged) 
        {
            
            OMX_CONFIG_DISPLAYREGIONTYPE configDisplay;
            OMX_INIT_STRUCTURE(configDisplay);
            configDisplay.nPortIndex = renderComponent.getInputPort();
            
            configDisplay.set     = OMX_DISPLAY_SET_FULLSCREEN;
            configDisplay.fullscreen = OMX_FALSE;
            
            renderComponent.setConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
            
            configDisplay.set     = OMX_DISPLAY_SET_DEST_RECT;
            configDisplay.dest_rect.x_offset  = displayRect.x;
            configDisplay.dest_rect.y_offset  = displayRect.y;
            configDisplay.dest_rect.width     = displayRect.getWidth();
            configDisplay.dest_rect.height    = displayRect.getHeight();
            
            renderComponent.setConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
        }	
    }
    
    Component renderComponent;
    ofRectangle displayRect;
    OMXStreamInfo streamInfo;

               
};