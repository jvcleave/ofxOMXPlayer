#pragma once

#include "ofMain.h"
#include "OMXInitializer.h"
#include "ofxOMXPlayerSettings.h"
#include "ofxOMXPlayerListener.h"
#include "ofxOMXPlayerEngine.h"

class ofxOMXPlayer
{
public:
    ofxOMXPlayer();
    ~ofxOMXPlayer();
    bool setup(ofxOMXPlayerSettings settings);
    ofxOMXPlayerSettings    settings;
    
    void        loadMovie(string videoPath);
    bool        isPaused();
    bool        isPlaying();
    bool        isFrameNew();
    bool        isTextureEnabled();
    
    ofTexture&  getTextureReference();
    GLuint      getTextureID();
    int         getHeight();
    int         getWidth();
    float       getFPS();
    double      getMediaTime();
    bool        getIsOpen();
    float       getDurationInSeconds();
    int         getCurrentFrame();
    int         getTotalNumFrames();
    
    void        draw(float x, float y, float w, float h);
    void        setDisplayRectForNonTexture(float x, float y, float width, float height);
    void        draw(float x=0, float y=0);
    
    void        increaseVolume();
    void        decreaseVolume();
    void        setVolume(float volume); // 0..1
    float       getVolume();
    
    void        setPaused(bool doPause);
    void        togglePause();
    void        stepFrameForward();
    
    void        increaseSpeed();
    int         getSpeedMultiplier();
    void        setNormalSpeed();
    void        rewind();
    void        restartMovie();
    void        seekToTimeInSeconds(int timeInSeconds);
    
    void        saveImage(string imagePath="");
    void        updatePixels();
    unsigned char*   getPixels();
    
    StreamInfo  getVideoStreamInfo();
    StreamInfo  getAudioStreamInfo();
    
    string      getInfo();
    void        close();
    
    ofFbo           fbo;
    ofTexture       texture;
    unsigned char*  pixels;
private:
    vector<int> signals;
    bool openEngine(int startTimeInSeconds = 0);
    void addExitHandler();
    void onUpdateDuringExit(ofEventArgs& args);
    void generateEGLImage(int videoWidth_, int videoHeight_);
    void destroyEGLImage();
    void onUpdate(ofEventArgs& args);
    
    static bool doExit;
    static void signal_handler(int signum);
    ofxOMXPlayerEngine* engine;
    
 
    EGLImageKHR     eglImage;
    GLuint          textureID;
    EGLDisplay      display;
    EGLContext      context;
    ofAppEGLWindow* appEGLWindow;
    int             videoWidth;
    int             videoHeight;
    
    
    bool            hasNewFrame;
    int             prevFrame;
    
    bool            doRestart;
    
    bool            textureEnabled;
    
    bool            didSeek;
    bool            didWarnAboutInaccurateCurrentFrame;
    bool            isOpen;
    int             speedMultiplier;
    
};

