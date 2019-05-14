#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"
#include "ofBaseTypes.h"


class ofRPIVideoPlayer: public ofBaseVideoPlayer, public ofxOMXPlayerListener
{

public:
    ofRPIVideoPlayer();
    bool load(string name);
    bool loadWithSettings(ofxOMXPlayerSettings newSettings);
    void loadAsync(string name);
    void play();
    void stop();
    ofTexture* getTexturePtr();
    float getWidth() const;
    float getHeight() const;
    bool isPaused() const;
    bool isLoaded() const;
    bool isPlaying() const;
    bool isInitialized() const;
    bool getIsMovieDone() const;
    const ofPixels& getPixels() const;
    ofPixels& getPixels();
    void update();
    bool isFrameNew() const;
    void close(); 
    bool setPixelFormat(ofPixelFormat pixelFormat);
    ofPixelFormat getPixelFormat() const;
    void setLoopState(ofLoopType state);
    
    ofxOMXPlayer omxPlayer;
    void draw(float x, float y, float w, float h);
    void draw(float x, float y);
    void enablePixels();
    void disablePixels();
    bool pixelsEnabled() { return doPixels; };
    void setPaused(bool doPause);
    void setVolume(float);
    float getVolume();
    int getCurrentFrame();
    int getTotalNumFrames();
protected:
    ofPixels pixels;
    ofPixelFormat pixelFormat;
    float videoWidth;
    float videoHeight;
    bool pauseState;
    bool openState;
    bool isPlayingState;
    bool hasNewFrame;
    bool doPixels;
    bool videoHasEnded;
    void onVideoEnd(ofxOMXPlayer*);
    void onVideoLoop(ofxOMXPlayer*);

    ofxOMXPlayerSettings settings;
    bool openOMXPlayer(ofxOMXPlayerSettings);
};

