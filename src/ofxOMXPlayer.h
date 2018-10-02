#pragma once

#include "ofMain.h"
#include "ofxOMXPlayerEngine.h"


class ofxOMXPlayer;

class ofxOMXPlayerListener
{
public:
    virtual void onVideoEnd(ofxOMXPlayer*) = 0;
};

class ofxOMXPlayer : public EngineListener
{
public:
     
    
    COMXCore omxCore;
    ofxOMXPlayerEngine engine;

    
    int speedMultiplier;

    ofxOMXPlayerSettings settings;
    ofxOMXPlayerListener* listener;
    bool engineNeedsRestart;
    
#pragma mark SETUP

    ofxOMXPlayer()
    {
        listener = NULL;
        engineNeedsRestart = false;
        OMX_Init();
        av_register_all();
        avformat_network_init();
        omxCore.Initialize();
        ofAddListener(ofEvents().update, this, &ofxOMXPlayer::onUpdate);

        
    }
    
    bool setup(ofxOMXPlayerSettings settings_)
    {
        
        settings = settings_;
        if(settings.listener)
        {
            listener = settings.listener;  
        }
        bool result = engine.setup(settings);
        if(result)
        {
            engine.listener = this; 
        }
        return result;
    }
    
    
    void start()
    {
        if(!engine.isThreadRunning())
        {
            engine.startThread();
        }
    }
    void loadMovie(string videoPath)
    {
        settings.videoPath = videoPath;
        engineNeedsRestart = true;
    }
    
    void reopen()
    {
        engineNeedsRestart = true;
    }
    
#pragma mark GETTERS

    int getWidth()
    {
        return engine.videoWidth; 
    }
    
    int getHeight()
    {
        return engine.videoHeight; 
    }
    
    float getFPS()
    {
        return engine.videoFrameRate;
    }
    
    int getTotalNumFrames()
    {
        return engine.totalNumFrames;
    }
    
    float getDurationInSeconds()
    {
        return engine.duration;
    }
    
    ofTexture&  getTextureReference()
    {
        return engine.fbo.getTextureReference();
    }
    
    GLuint getTextureID()
    {
        return engine.texture.getTextureData().textureID;
    }
    
    unsigned char* getPixels()
    {
        return engine.pixels;
    }
    
    int getClockSpeed()
    {
        return engine.omxClock.OMXPlaySpeed();
    }
    
    bool isOpen()
    {
        return engine.isOpen;
    }
    
    bool getIsOpen()
    {
        return isOpen();
    }
    
    bool isPlaying()
    {
        bool result = false;
        if(isOpen() && !isPaused() && engine.isThreadRunning())
        {
            result = true;
        }
        return result;
    }
    
    float getPlaybackSpeed()
    {
        return engine.speeds[engine.currentSpeed];
    }
    
    
   
    
    float getMediaTime()
    {
        float t = (float)(engine.omxClock.OMXMediaTime()*1e-6);
        return t;
    }
    
    int getCurrentFrame()
    {
        int result =0;
        int fps = getFPS();
        if(fps)
        {            
            result = getMediaTime()*fps;
        }
        return result;
    }
    
    float getVolume()
    {
        return getVolumeNormalized();
    }
    
    float getVolumeDB()
    {
        return engine.m_Volume;
    }
    
    bool isLoopingEnabled()
    {
        return engine.m_loop; 
    }
    
    
    bool isTextureEnabled()
    {
        return settings.enableTexture;
    }
    
    bool isFrameNew()
    {
        return engine.hasNewFrame;
    }
    
    COMXStreamInfo&  getVideoStreamInfo()
    {
        return engine.m_config_video.hints;
    }
    COMXStreamInfo&  getAudioStreamInfo()
    {
        return engine.m_config_audio.hints;
    }
    
    static string getRandomVideo(string path)
    {
        string videoPath = "";
        
        ofDirectory currentVideoDirectory(ofToDataPath(path, true));
        if (currentVideoDirectory.exists())
        {
            currentVideoDirectory.listDir();
            currentVideoDirectory.sort();
            vector<ofFile> files = currentVideoDirectory.getFiles();
            if (files.size()>0)
            {
                if (files.size()==1)
                {
                    videoPath = files[0].path();
                }else
                {
                    int randomIndex = ofRandom(files.size());
                    videoPath = files[randomIndex].path(); 
                }
                
            }
            
        }else
        {
            ofLogError(__func__) << "NO FILES FOUND AT" << currentVideoDirectory.path();
        }
        
        if(videoPath.empty())
        {
            ofLogWarning(__func__) << "returning empty string";
        }
        return videoPath;
    }  
    
    string getInfo()
    {
        stringstream info;
        info << "APP FPS: "+ ofToString(ofGetFrameRate()) << endl;
        if(isOpen())
        {
            int t = getMediaTime();
            info << "MEDIA TIME: " << (t/3600)<<"h:"<< (t/60)%60 <<"m:"<< t%60 <<":s"<<  " raw: " << getMediaTime() <<endl;
            
            info << "OMX CLOCK SPEED: " << getClockSpeed() << endl;
            info << "DIMENSIONS: " << getWidth()<<"x"<<getHeight();
            
            info << "FPS: " << getFPS() << endl;
            info << "DURATION IN SECS: " << getDurationInSeconds() << endl;
            info << "TOTAL FRAMES: " << getTotalNumFrames() << endl;
            info << "CURRENT FRAME: " << getCurrentFrame() << endl;
            if (getTotalNumFrames() > 0) 
            {
                info << "REMAINING FRAMES: " << getTotalNumFrames() - getCurrentFrame() << endl;
            }else 
            {
                info << "REMAINING FRAMES: N/A, NO TOTAL FRAMES" << endl;
            }        
            info << "LOOPING ENABLED: " << isLoopingEnabled() << endl;
            info << "CURRENT VOLUME: " << getVolume() << endl;
            info << "CURRENT VOLUME NORMALIZED: " << getVolumeNormalized() << endl; 
            info << "FILE: " << settings.videoPath << endl; 
            info << "TEXTURE ENABLED: " << isTextureEnabled() << endl; 
        }else
        {
            info << "CLOSED" << endl;
            
        }
        
        
        return info.str();
    }
    
#pragma mark LISTENERS
    
    void onVideoEnd()
    {
        if(listener)
        {
            listener->onVideoEnd(this);
        }
    }
    void onVideoLoop(bool needsRestart)
    {
        engineNeedsRestart = needsRestart;
    }

    
    
    
    void onUpdate(ofEventArgs& eventArgs)
    {
        if(engineNeedsRestart)
        {
            engineNeedsRestart = false;
            if(isOpen())
            {
                engine.close();
            }
            
            setup(settings);
        }
    }
    
    
    void close()
    {
        engine.close();
    }
    
    void enableLooping()
    {
        engine.m_loop = true;
    }
    
    void disableLooping()
    {
        engine.m_loop = false; 
    }
    
#pragma mark DRAWING

    void draw(float x, float y, float w, float h)
    {
        //ofLog() << "draw: " << ofRectangle(x, y, w, h);
        
        engine.draw(x, y, w, h);
    }
    
    void draw(ofRectangle& rectangle)
    {
        if(isTextureEnabled())
        {
            draw(rectangle.x, rectangle.y, rectangle.width, rectangle.height);

        }else
        {
            ofRectangle cropRect;
            rectangle.width += rectangle.x;
            drawCropped(cropRect, rectangle);
        }

    }
    
    void drawCropped(float cropX, float cropY, float cropWidth, float cropHeight,
                     float drawX, float drawY, float drawWidth, float drawHeight)
    {
        engine.drawCropped(cropX, cropY, cropWidth, cropHeight,
                           drawX, drawY, drawWidth, drawHeight);
    }
    
    void drawCropped(ofRectangle& cropRectangle, ofRectangle& drawRectangle)
    {
        
        drawCropped(cropRectangle.x, cropRectangle.y, cropRectangle.width, cropRectangle.height,
                    drawRectangle.x, drawRectangle.y, drawRectangle.width, drawRectangle.height);
    }
    
    void setAlpha(int alpha)
    {
        engine.setAlpha(alpha);
    }
    
    void setLayer(int layer)
    {
        engine.setLayer(layer);
    }

#pragma mark PLAYBACK CONTROLS

    bool isPaused()
    {
        return engine.m_Pause;
    }
    
    void setPaused(bool doPause)
    {
        engine.m_Pause = doPause;
    }
    
    void togglePause()
    {
        engine.m_Pause = !engine.m_Pause;
    }
    
    void setNormalSpeed()
    {
        engine.setNormalSpeed();
    }
    
    void increaseSpeed()
    {
        engine.increaseSpeed();
    }
    
    void decreaseSpeed()
    {
        engine.decreaseSpeed();
    }
    
    void stepFrameForward()
    {
        stepNumFrames(1);
    }
    
    void stepNumFrames(int step)
    {
        engine.stepNumFrames(step);
    }
    
    void seekToTimeInSeconds(int timeInSeconds)
    {
        engine.seekToTimeInSeconds(timeInSeconds);
    }
    
    void seekToFrame(int frameTarget)
    {
        engine.seekToFrame(frameTarget);
    }
    
    
    void restartMovie()
    {
        seekToFrame(0);
    }
    
#pragma mark PLAYBACK AUDIO
 
    void increaseVolume()
    {
        engine.increaseVolume();
    }
    
    void decreaseVolume()
    {
        engine.decreaseVolume();

    }
    
    void setVolumeNormalized(float volume)
    {
        float value = ofMap(volume, 0.0, 1.0, -6000.0, 6000.0, true);
        engine.m_Volume = value;
    }
    
    void setVolume(float volume)
    {
        setVolumeNormalized(volume);
    }
    
    float getVolumeNormalized()
    {
        float value = ofMap(engine.m_Volume, -6000.0, 6000.0, 0.0, 1.0, true);
        return value;
    }
    
   
    
#pragma mark PIXELS
    
    void updatePixels()
    {
        engine.updatePixels();
    }
    
    
    void saveImage(string imagePath="")
    {
        if(!isTextureEnabled()) return;
        if(imagePath == "")
        {
            imagePath = ofToDataPath(ofGetTimestampString()+".png", true);
        }
        updatePixels();
        //TODO make smarter, re-allocating every time
        ofImage image;
        image.setFromPixels(getPixels(), getWidth(), getHeight(), OF_IMAGE_COLOR_ALPHA);
        image.saveImage(imagePath);
        
        ofLog() << "SAVED IMAGE TO: " << imagePath;
    }
    

    //void        draw(float x=0, float y=0);
    //void        draw(ofRectangle&);
    
    void scrubForward(int step=1)
    {
        ofLogError() << " scrubForward NOT IMPLEMENTED";

    }
    
    
    
#pragma mark OLD/TODO
    
#if 0     
    void applyFilter(OMX_IMAGEFILTERTYPE filter);

    //direct only
    void        setDisplayRect(float x, float y, float width, float height);
    void        setDisplayRect(ofRectangle&);
    void        cropVideo(ofRectangle&);
    void        cropVideo(float x, float y, float width, float height);
    void        rotateVideo(int degrees);
    void        setMirror(bool);
    void        setAlpha(int alpha);
    void        setFullScreen(bool);
    void        setForceFill(bool);
    ofRectangle* cropRectangle;
    ofRectangle* drawRectangle;
    
#endif
    
};


