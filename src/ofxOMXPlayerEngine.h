#pragma once

#include "ofMain.h"
#include "ofxOMXPlayerSettings.h"
#include "OMXReader.h"
#include "OMXClock.h"
#include "OMXAudio.h"
#include "OMXPlayerVideo.h"
#include "OMXPlayerAudio.h"
#include "utils/Strprintf.h"
#include "ofAppEGLWindow.h"
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <EGL/eglext.h>

class EngineListener
{
public:
    EngineListener(){};
    virtual ~EngineListener(){};
    virtual void onVideoEnd() = 0;
    virtual void onVideoLoop(bool needsRestart)= 0;
};



class ofxOMXPlayerEngine : public ofThread
{
    
    
public:
    
    
    OMXReader m_omx_reader;
    OMXClock omxClock;
    
    OMXAudioConfig    m_config_audio;
    OMXVideoConfig    m_config_video;
    OMXPlayerVideo    m_player_video;
    OMXPlayerAudio    m_player_audio;
    
    //int count;
    float m_threshold;
    float m_last_check_time;
    bool isFirstFrame;
    
    bool m_has_video;
    bool m_has_audio;
    double m_incr;
    double last_seek_pos;
    OMXPacket *m_omx_pkt;
    bool m_send_eos;
    double m_loop_from;
    bool m_seek_flush;
    bool m_chapter_seek;
    bool sentStarted;
    bool m_packet_after_seek;
    bool m_stats;
    bool m_tv_show_info;
    bool m_Pause;
    float m_latency;
    bool m_loop;
    bool m_stop;
    bool m_NativeDeinterlace;
    bool m_refresh;
    TV_DISPLAY_STATE_T   tv_state;
    long m_Volume;
    double startpts;
    int m_timeout;
    string m_cookie;
    string m_user_agent;
    string m_lavfdopts;
    
    vector<int>speeds;
    string m_filename;
    
    EGLImageKHR eglImage;
    bool useTexture;
    int videoWidth;
    int videoHeight;
    ofFbo           fbo;
    ofTexture       texture;
    
    unsigned char*  pixels;
    GLuint          textureID;
    EGLDisplay      display;
    EGLContext      context;
    ofAppEGLWindow* appEGLWindow;
    int updateCounter;
    
    int totalNumFrames;
    int videoFrameRate;
    int duration;
    bool isOpen;
    
    EngineListener* listener;
    bool hasNewFrame;
    //float currentPlaybackSpeed;
    CRect cropRect;
    CRect drawRect;
    
    int currentSpeed; 
    int normalSpeedIndex;
    
    
    int createSpeed(float x);
    
    bool TRICKPLAY(int speed);
    
    ofxOMXPlayerEngine();
    void clear();
    bool setup(ofxOMXPlayerSettings settings);
    void threadedFunction();

    void updatePixels();
    bool generateEGLImage();
    void destroyEGLImage();
    void draw(float x, float y, float width, float height);
    void drawCropped(float cropX, float cropY, float cropWidth, float cropHeight,
                     float drawX, float drawY, float drawWidth, float drawHeight);
    
    void setLayer(int layer);
    void setAlpha(int alpha);
    void setFilter(OMX_IMAGEFILTERTYPE filterType);
    void rotateVideo(int degrees, bool doMirror = false);
    
    void onUpdate(ofEventArgs& eventArgs);
  
    void seekToFrame(int frameTarget);
    void seekToTimeInSeconds(double timeInSeconds);
    void increaseSpeed();
    void decreaseSpeed();
    void setNormalSpeed();
    void stepFrameForward();
    void stepNumFrames(int step);
    
    
    void decreaseVolume();
    void increaseVolume();
    void applyVolume();
    
    void SetSpeed();
    void FlushStreams(double pts);
    void SetVideoMode(int width, int height, int fpsrate, int fpsscale);
    
    static void CallbackTvServiceCallback(void *userdata, uint32_t reason, uint32_t param1, uint32_t param2);
    float get_display_aspect_ratio(HDMI_ASPECT_T aspect);
    float get_display_aspect_ratio(SDTV_ASPECT_T aspect);
    
    
    void close(bool clearTextures = false);
    void doExit();
    ~ofxOMXPlayerEngine();
    
};

