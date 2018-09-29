#pragma once



#include "ofMain.h"

#include "OMXReader.h"
#include "OMXClock.h"
#include "OMXAudio.h"
#include "OMXPlayerVideo.h"
#include "OMXPlayerAudio.h"
#include "KeyConfig.h"
#include "utils/Strprintf.h"
#include "ofAppEGLWindow.h"
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <EGL/eglext.h>


class EngineListener
{
public:
    virtual void onVideoEnd() = 0;
    virtual void onVideoLoop(bool needsRestart)= 0;
};

class ofxOMXPlayerListener;
class ofxOMXPlayerSettings
{
public:
    ofxOMXPlayerSettings()
    {
        videoPath = "";
        
        useHDMIForAudio = true;
        enableTexture = true;
        doFlipTexture = false; //true for older firmware
        enableLooping = true;
        enableAudio   = true;
        initialVolume = 0.5;
        videoWidth  = 0;
        videoHeight = 0;
        enableFilters = false;
        filter = OMX_ImageFilterNone;
        listener = NULL;
        
    }
    bool enableFilters;
    OMX_IMAGEFILTERTYPE filter;
    string videoPath;
    int videoWidth;
    int videoHeight;
    bool enableTexture;
    bool doFlipTexture;
    bool enableAudio;
    float initialVolume;
    bool useHDMIForAudio;
    bool enableLooping;
    ofRectangle drawRectangle;
    ofxOMXPlayerListener* listener;

    
    
    //PlayerDirectDisplayOptions directDisplayOptions;
    /*
     To use HDMI Audio you may need to add the below line to /boot/config.txt and reboot
     
     hdmi_drive=2
     
     see http://elinux.org/RPiconfig for more details
     */
    
};



#define TRICKPLAY(speed) (speed < 0 || speed > 4 * DVD_PLAYSPEED_NORMAL)
#define S(x) (int)(DVD_PLAYSPEED_NORMAL*(x))





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
    bool didSeek;
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
    int playspeed_current;
    map<int,int> keymap;
    int playspeeds[20];
    string m_filename;
    const int playspeed_slow_min = 0, playspeed_slow_max = 7, playspeed_rew_max = 8, playspeed_rew_min = 13, playspeed_normal = 14, playspeed_ff_min = 15, playspeed_ff_max = 19; 

    int keyCommand;
    
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
    int frameCounter;
    
    int totalNumFrames;
    int videoFrameRate;
    int duration;
    bool isOpen;

    EngineListener* listener;
    bool hasNewFrame;
    float currentPlaybackSpeed;
    ofxOMXPlayerEngine()
    {
        eglImage = NULL;

        clear();
        playspeeds[0] = S(0);
        playspeeds[1] = S(1/16.0);
        playspeeds[2] = S(1/8.0);
        playspeeds[3] = S(1/4.0);
        playspeeds[4] = S(1/2.0);
        playspeeds[5] = S(0.975);
        playspeeds[6] = S(1.0);
        playspeeds[7] = S(1.125);
        playspeeds[8] = S(-32.0);
        playspeeds[9] = S(-16.0);
        playspeeds[10] = S(-8.0);
        playspeeds[11] = S(-4);
        playspeeds[12] = S(-2);
        playspeeds[13] = S(-1);
        playspeeds[14] = S(1);
        playspeeds[15] = S(2.0);
        playspeeds[16] = S(4.0);
        playspeeds[17] = S(8.0);
        playspeeds[18] = S(16.0);
        playspeeds[19] = S(32.0);
        keymap = KeyConfig::buildDefaultKeymap();
        
        CLog::SetLogLevel(LOG_LEVEL_NONE);
        string logDirectory = ofToDataPath("", true);
        bool logToOF = true;
        CLog::Init(logDirectory.c_str(), logToOF);
        
    }
    
    void clear()
    {
        currentPlaybackSpeed = 0.0;
        hasNewFrame = false;
        listener = NULL;
        isOpen = false;
        duration = 0;
        totalNumFrames = 0;
        videoFrameRate = 25;
        keyCommand = -1;
        m_last_check_time = 0.0;
        isFirstFrame = true;
        m_seek_flush = false;
        m_chapter_seek = false;
        sentStarted = false;
        m_packet_after_seek = false;
        m_stats = false;
        m_tv_show_info = false;
        m_Pause = false;
        m_latency = 0.0f;
        m_loop = true;
        m_stop = false;
        m_NativeDeinterlace = false;
        m_refresh = false;
        m_Volume = 0;
        startpts = 0;
        updateCounter = 0;
        frameCounter = 0;
        
        pixels = NULL;
        display = NULL;
        context = NULL;
        appEGLWindow = NULL;
        
        m_omx_pkt = NULL;
        
        m_threshold      = -1.0f;
        m_incr = 0;
        last_seek_pos = 0;
        m_omx_pkt = NULL;
        m_send_eos = false;
        int h = 0;
        int m = 0;
        int s = 1;
        m_incr = 0;
        m_loop_from = m_incr;
        didSeek = false;
    }
    ~ofxOMXPlayerEngine()
    {
        close();
        destroyEGLImage();
        if(pixels)
        {
            delete[] pixels;
            pixels = NULL;
        }
    }
    
   void updatePixels()
    {    
        lock();
        if (!texture.isAllocated() && !fbo.isAllocated())
        {
            unlock();
            return;
        }
        fbo.begin();
        ofClear(0, 0, 0, 0);
        texture.draw(0, 0);
        //ofLogVerbose() << "updatePixels";
        glReadPixels(0,0,videoWidth, videoHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        fbo.end();
        unlock();
    }
    
    void destroyEGLImage()
    {
        
        
        if (eglImage)
        {
            if (appEGLWindow == NULL)
            {
                appEGLWindow = (ofAppEGLWindow *) ofGetWindowPtr();
            }
            
            if (display == NULL)
            {
                display = appEGLWindow->getEglDisplay();
            }
            
            if (!eglDestroyImageKHR(display, eglImage))
            {
                ofLog() << __func__ << " FAIL <---------------- :(";
            }else
            {
                ofLog() << __func__ << " PASS <---------------- :)";
            }
            eglImage = NULL;
        }
        
    }
    
    void draw(float x, float y, float width, float height)
    {
        if (!texture.isAllocated() && !fbo.isAllocated()) return;
        fbo.draw(x, y, width, height);
    }
    
    
    
    void onUpdate(ofEventArgs& eventArgs)
    {
        
        if(!m_has_video) return;
        if (!texture.isAllocated() && !fbo.isAllocated()) return;
        if(omxClock.OMXMediaTime()<0) return; 
        
        int frameNumber = m_player_video.getFrameNumber();
        if(updateCounter != frameNumber)
        {
            hasNewFrame = true;
            fbo.begin();
            ofClear(0, 0, 0, 0);
            texture.draw(0, 0, texture.getWidth(), texture.getHeight()); 
            fbo.end();
            updateCounter = frameNumber;
            //ofLog() << "frameCounter: " << frameNumber;
        }else
        {
            hasNewFrame = false;
        }
       
    }
    
    
    bool generateEGLImage(int videoWidth_, int videoHeight_)
    {
        bool success = false;
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
        
        if (!texture.isAllocated())
        {
            needsRegeneration = true;
        }
        else
        {
            if (texture.getWidth() != videoWidth && texture.getHeight() != videoHeight)
            {
                needsRegeneration = true;
            }
        }
        
        if (!fbo.isAllocated())
        {
            needsRegeneration = true;
        }
        else
        {
            if (fbo.getWidth() != videoWidth && fbo.getHeight() != videoHeight)
            {
                needsRegeneration = true;
            }
        }
        
        if(!needsRegeneration)
        {
            //ofLogVerbose(__func__) << "NO CHANGES NEEDED - RETURNING EARLY";
            return true;
        }
        
        if (appEGLWindow == NULL)
        {
            appEGLWindow = (ofAppEGLWindow *) ofGetWindowPtr();
        }
        
        if (appEGLWindow == NULL)
        {
            ofLogError(__func__) << "appEGLWindow is NULL - RETURNING";
            return false;
        }
        if (display == NULL)
        {
            display = appEGLWindow->getEglDisplay();
        }
        if (context == NULL)
        {
            context = appEGLWindow->getEglContext();
        }
        
        if (display == NULL)
        {
            ofLogError(__func__) << "display is NULL - RETURNING";
            return false;
        }
        if (context == NULL)
        {
            ofLogError(__func__) << "context is NULL - RETURNING";
            return false;
        }
        
        if (needsRegeneration)
        {
            
            fbo.allocate(videoWidth, videoHeight, GL_RGBA);
            texture.allocate(videoWidth, videoHeight, GL_RGBA);
            texture.setTextureWrap(GL_REPEAT, GL_REPEAT);
            textureID = texture.getTextureData().textureID;
        }
        
        ofLog() << "textureID: " << textureID;
        ofLog() << "tex.isAllocated(): " << texture.isAllocated();
        ofLog() << "videoWidth: " << videoWidth;
        ofLog() << "videoHeight: " << videoHeight;
        ofLog() << "pixels: " << videoHeight;

        // setup first texture
        int dataSize = videoWidth * videoHeight * 4;
        
        if (pixels && needsRegeneration)
        {
            delete[] pixels;
            pixels = NULL;
        }
        
        if (pixels == NULL)
        {
            pixels = new unsigned char[dataSize];
        }
        ofLog() << "dataSize: " << dataSize;

        //memset(pixels, 0xff, dataSize);  // white texture, opaque
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, videoWidth, videoHeight, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        
        
        if (eglImage && needsRegeneration)
        {
            destroyEGLImage();
        }
        
        // Create EGL Image
        eglImage = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR, (EGLClientBuffer)textureID, NULL);
        
        if (eglImage == EGL_NO_IMAGE_KHR)
        {
            ofLog()    << "Create EGLImage FAIL <---------------- :(";
            
        }
        else
        {
            success = true;
            ofLog()  << "Create EGLImage PASS <---------------- :)";
        }
        return success;
        
    }
    
   
    
    void close(bool clearTextures = false)
    {
        lock();
        //ofRemoveListener(ofEvents().update, this, &ofxOMXPlayerEngine::onUpdate);
        stopThread();
        //
      
        if(clearTextures)
        {
            fbo.clear();
            texture.clear();
        }
    
        unlock();
        doExit(); 
    }

    bool setup(ofxOMXPlayerSettings* settings)
    {
        
        m_filename = settings->videoPath;
        useTexture = settings->enableTexture;
        
        bool didOpen = true;
        
        m_config_video.useTexture = useTexture;
        bool m_dump_format = true;
        bool m_config_audio_is_live = false;
        int m_timeout = 1000;
        string m_cookie = "";
        string m_user_agent = "";
        string m_lavfdopts = "";
        
        bool didOpenReader = m_omx_reader.Open(m_filename.c_str(),
                                   m_dump_format,
                                   m_config_audio_is_live,
                                   m_timeout,
                                   m_cookie.c_str(),
                                   m_user_agent.c_str(),
                                   m_lavfdopts.c_str());
        ofLog() << "didOpenReader: " << didOpenReader;
        
        
        ofLog() << "VideoStreamCount(): " << m_omx_reader.VideoStreamCount();
        ofLog() << "AudioStreamCount(): " << m_omx_reader.AudioStreamCount();
        ofLog() << "CanSeek(): " << m_omx_reader.CanSeek();
        ofLog() << "useTexture: " << useTexture;

        if(!didOpenReader)
        {
            didOpen = false;
            ofLogError() << "READER OPEN FAILED";

            return didOpen;
        }
        omxClock.OMXInitialize();
        omxClock.OMXStateIdle();
        omxClock.OMXStop();
        omxClock.OMXPause();
        
        m_omx_reader.GetHints(OMXSTREAM_AUDIO, m_config_audio.hints);
        m_omx_reader.GetHints(OMXSTREAM_VIDEO, m_config_video.hints);
        
        
        m_has_video     = m_omx_reader.VideoStreamCount();
        m_has_audio     = m_omx_reader.AudioStreamCount();
        
        
        omxClock.OMXReset(m_has_video, m_has_audio);
        omxClock.OMXStateExecute();

        if(m_has_video)
        {
            //calculate numFrames/fps
            totalNumFrames = m_config_video.hints.nb_frames;
            if(!totalNumFrames)
            {
                ofLog() << "PROBABLY A STREAM";
            }
            if (m_config_video.hints.fpsrate && m_config_video.hints.fpsscale)
            {
                videoFrameRate = DVD_TIME_BASE / OMXReader::NormalizeFrameduration((double)DVD_TIME_BASE * m_config_video.hints.fpsscale / m_config_video.hints.fpsrate);
            }
            
            
            if( videoFrameRate > 100 || videoFrameRate < 5 )
            {
                printf("Invalid framerate %d, using forced 25fps and just trust timestamps\n", (int)videoFrameRate);
                videoFrameRate = 25;
            }
            
            duration = m_config_video.hints.nb_frames / videoFrameRate;
            if(useTexture)
            {
                bool didCreateEGLImage = generateEGLImage(m_config_video.hints.width, m_config_video.hints.height);
                if(!didCreateEGLImage)
                {
                    didOpen = false;
                    ofLogError() << "generateEGLImage FAILED";
                    
                    return didOpen;
                }
                m_config_video.eglImage = eglImage;
                
            }else
            {
                bool setDisplayResolution = false;
                
                if(setDisplayResolution)
                {
                    SetVideoMode(m_config_video.hints.width,
                                 m_config_video.hints.height,
                                 m_config_video.hints.fpsrate,
                                 m_config_video.hints.fpsscale);
                    
                    
                    TV_DISPLAY_STATE_T current_tv_state;
                    memset(&current_tv_state, 0, sizeof(TV_DISPLAY_STATE_T));
                    vc_tv_get_display_state(&current_tv_state);
                    if(current_tv_state.state & ( VC_HDMI_HDMI | VC_HDMI_DVI ))
                    {
                        //HDMI or DVI on
                        m_config_video.display_aspect = get_display_aspect_ratio((HDMI_ASPECT_T)current_tv_state.display.hdmi.aspect_ratio);
                    } else 
                    {
                        //composite on
                        m_config_video.display_aspect = get_display_aspect_ratio((SDTV_ASPECT_T)current_tv_state.display.sdtv.display_options.aspect);
                    }
                    m_config_video.display_aspect *= (float)current_tv_state.display.hdmi.height/(float)current_tv_state.display.hdmi.width;
                    
                    ofLog() << "height: " << current_tv_state.display.hdmi.height;
                    ofLog() << "width: " << current_tv_state.display.hdmi.width;
                    
                    
                    ofLog() << "m_config_video.display_aspect: " << m_config_video.display_aspect;
                    if(m_config_video.hdmi_clock_sync)
                    {
                        omxClock.HDMIClockSync();
                    }
                }
               
                
            }
            
            
            bool didVideoOpen =  m_player_video.Open(&omxClock, m_config_video);
            if(didVideoOpen && useTexture)
            {
                ofAddListener(ofEvents().update, this, &ofxOMXPlayerEngine::onUpdate);
                
            }
            if(!didVideoOpen)
            {
                didOpen = false;
                ofLogError() << "VIDEO OPEN FAILED";
                return didOpen;
            }
        }
        
       
       
        if(m_has_audio)
        {
            if (m_config_audio.device == "")
            {
                if (vc_tv_hdmi_audio_supported(EDID_AudioFormat_ePCM, 2, EDID_AudioSampleRate_e44KHz, EDID_AudioSampleSize_16bit ) == 0)
                    m_config_audio.device = "omx:hdmi";
                else
                    m_config_audio.device = "omx:local";
            }
            
            if ((m_config_audio.hints.codec == AV_CODEC_ID_AC3 || m_config_audio.hints.codec == AV_CODEC_ID_EAC3) &&
                vc_tv_hdmi_audio_supported(EDID_AudioFormat_eAC3, 2, EDID_AudioSampleRate_e44KHz, EDID_AudioSampleSize_16bit ) != 0)
            {
                m_config_audio.passthrough = false;
            }
            if (m_config_audio.hints.codec == AV_CODEC_ID_DTS &&
                vc_tv_hdmi_audio_supported(EDID_AudioFormat_eDTS, 2, EDID_AudioSampleRate_e44KHz, EDID_AudioSampleSize_16bit ) != 0)
            {
                m_config_audio.passthrough = false;
            }
            bool didAudioOpen = m_player_audio.Open(&omxClock, m_config_audio, &m_omx_reader);
            
            if(!didAudioOpen)
            {
                didOpen = false;
                ofLogError() << "AUDIO OPEN FAILED";
                return didOpen;
            }else
            {
                if (m_threshold < 0.0f)
                {
                    m_threshold = m_config_audio.is_live ? 0.7f : 0.2f;
                }
                omxClock.OMXSetSpeed(DVD_PLAYSPEED_NORMAL);
                
                omxClock.OMXReset(m_has_video, m_has_audio);
                omxClock.OMXStateExecute();
                m_player_audio.SetVolume(0.1);
            }
        }

        if(didOpen)
        {
           startThread(); 
        }
        isOpen = didOpen;
        return didOpen;
    }
    
    
    
    
    
    void FlushStreams(double pts)
    {
        omxClock.OMXStop();
        omxClock.OMXPause();
        
        if(m_has_video)
            m_player_video.Flush();
        
        if(m_has_audio)
            m_player_audio.Flush();
        
        if(pts != DVD_NOPTS_VALUE)
            omxClock.OMXMediaTime(pts);
        
        
        if(m_omx_pkt)
        {
            m_omx_reader.FreePacket(m_omx_pkt);
            m_omx_pkt = NULL;
        }
    }
    
    
    void change_file()
    {
        
        
    }

    void threadedFunction()
    {
        
        
        while(isThreadRunning())
        {
            if(isFirstFrame)
            {
                if(omxClock.OMXIsPaused())
                {
                    ofLog() << "omxClock.OMXIsPaused(): " << omxClock.OMXIsPaused();
                    omxClock.OMXResume();
                    ofLog() << "RESUMED";
                }
                isFirstFrame = false;
            }
            {
          
                
                double now = omxClock.GetAbsoluteClock();
                bool update = false;
                if (m_last_check_time == 0.0 || m_last_check_time + DVD_MSEC_TO_TIME(20) <= now) 
                {
                    update = true;
                    m_last_check_time = now;
                }
                
                if (update) 
                {                    
                    //processCommand();
                }
                
               
                
                if(m_seek_flush || m_incr != 0)
                {
                    double seek_pos     = 0;
                    double pts          = 0;
                    
                   
                    if (!m_chapter_seek)
                    {
                        pts = omxClock.OMXMediaTime();
                        
                        seek_pos = (pts ? pts / DVD_TIME_BASE : last_seek_pos) + m_incr;
                        last_seek_pos = seek_pos;
                        
                        seek_pos *= 1000.0;
                        
                        if(m_omx_reader.SeekTime((int)seek_pos, m_incr < 0.0f, &startpts))
                        {
                            unsigned t = (unsigned)(startpts*1e-6);
                            auto dur = m_omx_reader.GetStreamLength() / 1000;
                            ofLog(OF_LOG_NOTICE, "m_omx_reader Seek\n%02d:%02d:%02d / %02d:%02d:%02d",
                                                        (t/3600), (t/60)%60, t%60, (dur/3600), (dur/60)%60, dur%60);
                            FlushStreams(startpts);
                        }
                    }
                    
                    sentStarted = false;
                    
                    if (m_omx_reader.IsEof())
                        {
                            doExit();
                        }
                    
                    // Quick reset to reduce delay during loop & seek.
                    if (m_has_video && !m_player_video.Reset())
                        {
                            doExit();
                        }
                    
                    ofLog(OF_LOG_NOTICE, "Seeked %.0f %.0f %.0f\n", DVD_MSEC_TO_TIME(seek_pos), startpts, omxClock.OMXMediaTime());
                   
                    omxClock.OMXPause();
                                           
                    m_packet_after_seek = false;
                    m_seek_flush = false;
                    m_incr = 0;
                }
                else if(m_packet_after_seek && TRICKPLAY(omxClock.OMXPlaySpeed()))
                {
                    double seek_pos     = 0;
                    double pts          = 0;
                    
                    pts = omxClock.OMXMediaTime();
                    seek_pos = (pts / DVD_TIME_BASE);
                    
                    seek_pos *= 1000.0;
                    if(m_omx_reader.SeekTime((int)seek_pos, omxClock.OMXPlaySpeed() < 0, &startpts))
                    {
                        //FlushStreams(DVD_NOPTS_VALUE);
                    }

                    ofLog(OF_LOG_NOTICE, "m_omx_reader Seeked %.0f %.0f %.0f\n", DVD_MSEC_TO_TIME(seek_pos), startpts, omxClock.OMXMediaTime());
                    
                    //unsigned t = (unsigned)(startpts*1e-6);
                    unsigned t = (unsigned)(pts*1e-6);
                    m_packet_after_seek = false;
                }
                
                /* player got in an error state */
                if(m_player_audio.Error())
                {
                    ofLog(OF_LOG_ERROR, "audio player error. emergency exit!!!\n");
                    doExit();
                }
                
                if (update)
                {
                    /* when the video/audio fifos are low, we pause clock, when high we resume */
                    double stamp = omxClock.OMXMediaTime();
                    double audio_pts = m_player_audio.GetCurrentPTS();
                    double video_pts = m_player_video.GetCurrentPTS();
                    
                    if (0 && omxClock.OMXIsPaused())
                    {
                        double old_stamp = stamp;
                        if (audio_pts != DVD_NOPTS_VALUE && (stamp == 0 || audio_pts < stamp))
                            stamp = audio_pts;
                        if (video_pts != DVD_NOPTS_VALUE && (stamp == 0 || video_pts < stamp))
                            stamp = video_pts;
                        if (old_stamp != stamp)
                        {
                            omxClock.OMXMediaTime(stamp);
                            stamp = omxClock.OMXMediaTime();
                        }
                    }
                    
                    float audio_fifo = audio_pts == DVD_NOPTS_VALUE ? 0.0f : audio_pts / DVD_TIME_BASE - stamp * 1e-6;
                    float video_fifo = video_pts == DVD_NOPTS_VALUE ? 0.0f : video_pts / DVD_TIME_BASE - stamp * 1e-6;
                    float threshold = std::min(0.1f, (float)m_player_audio.GetCacheTotal() * 0.1f);
                    bool audio_fifo_low = false, video_fifo_low = false, audio_fifo_high = false, video_fifo_high = false;
                    
                    if(m_stats)
                    {
                        static int count;
                        if ((count++ & 7) == 0)
                            ofLog(OF_LOG_NOTICE, "M:%8.0f V:%6.2fs %6dk/%6dk A:%6.2f %6.02fs/%6.02fs Cv:%6dk Ca:%6dk                            \r", stamp,
                                   video_fifo, (m_player_video.GetDecoderBufferSize()-m_player_video.GetDecoderFreeSpace())>>10, m_player_video.GetDecoderBufferSize()>>10,
                                   audio_fifo, m_player_audio.GetDelay(), m_player_audio.GetCacheTotal(),
                                   m_player_video.GetCached()>>10, m_player_audio.GetCached()>>10);
                    }
                    
                    if(m_tv_show_info)
                    {
                        static unsigned count;
                        if ((count++ & 7) == 0)
                        {
                            char response[80];
                            if (m_player_video.GetDecoderBufferSize() && m_player_audio.GetCacheTotal())
                                vc_gencmd(response, sizeof response, "render_bar 4 video_fifo %d %d %d %d",
                                          (int)(100.0*m_player_video.GetDecoderBufferSize()-m_player_video.GetDecoderFreeSpace())/m_player_video.GetDecoderBufferSize(),
                                          (int)(100.0*video_fifo/m_player_audio.GetCacheTotal()),
                                          0, 100);
                            if (m_player_audio.GetCacheTotal())
                                vc_gencmd(response, sizeof response, "render_bar 5 audio_fifo %d %d %d %d",
                                          (int)(100.0*audio_fifo/m_player_audio.GetCacheTotal()),
                                          (int)(100.0*m_player_audio.GetDelay()/m_player_audio.GetCacheTotal()),
                                          0, 100);
                            vc_gencmd(response, sizeof response, "render_bar 6 video_queue %d %d %d %d",
                                      m_player_video.GetLevel(), 0, 0, 100);
                            vc_gencmd(response, sizeof response, "render_bar 7 audio_queue %d %d %d %d",
                                      m_player_audio.GetLevel(), 0, 0, 100);
                        }
                    }
                    
                    if (audio_pts != DVD_NOPTS_VALUE)
                    {
                        audio_fifo_low = m_has_audio && audio_fifo < threshold;
                        audio_fifo_high = !m_has_audio || (audio_pts != DVD_NOPTS_VALUE && audio_fifo > m_threshold);
                    }
                    if (video_pts != DVD_NOPTS_VALUE)
                    {
                        video_fifo_low = m_has_video && video_fifo < threshold;
                        video_fifo_high = !m_has_video || (video_pts != DVD_NOPTS_VALUE && video_fifo > m_threshold);
                    }
                    /*
                    ofLog(OF_LOG_NOTICE, "Normal M:%.0f (A:%.0f V:%.0f) P:%d A:%.2f V:%.2f/T:%.2f (%d,%d,%d,%d) A:%d%% V:%d%% (%.2f,%.2f)\n", stamp, audio_pts, video_pts, omxClock.OMXIsPaused(), 
                              audio_pts == DVD_NOPTS_VALUE ? 0.0:audio_fifo, video_pts == DVD_NOPTS_VALUE ? 0.0:video_fifo, m_threshold, audio_fifo_low, video_fifo_low, audio_fifo_high, video_fifo_high,
                              m_player_audio.GetLevel(), m_player_video.GetLevel(), m_player_audio.GetDelay(), (float)m_player_audio.GetCacheTotal());*/
                    
                    // keep latency under control by adjusting clock (and so resampling audio)
                    if (m_config_audio.is_live)
                    {
                        float latency = DVD_NOPTS_VALUE;
                        if (m_has_audio && audio_pts != DVD_NOPTS_VALUE)
                            latency = audio_fifo;
                        else if (!m_has_audio && m_has_video && video_pts != DVD_NOPTS_VALUE)
                            latency = video_fifo;
                        if (!m_Pause && latency != DVD_NOPTS_VALUE)
                        {
                            if (omxClock.OMXIsPaused())
                            {
                                if (latency > m_threshold)
                                {
                                    ofLog(OF_LOG_NOTICE,  "Resume %.2f,%.2f (%d,%d,%d,%d) EOF:%d PKT:%p\n", audio_fifo, video_fifo, audio_fifo_low, video_fifo_low, audio_fifo_high, video_fifo_high, m_omx_reader.IsEof(), m_omx_pkt);
                                    omxClock.OMXResume();
                                    m_latency = latency;
                                }
                            }
                            else
                            {
                                m_latency = m_latency*0.99f + latency*0.01f;
                                float speed = 1.0f;
                                if (m_latency < 0.5f*m_threshold)
                                    speed = 0.990f;
                                else if (m_latency < 0.9f*m_threshold)
                                    speed = 0.999f;
                                else if (m_latency > 2.0f*m_threshold)
                                    speed = 1.010f;
                                else if (m_latency > 1.1f*m_threshold)
                                    speed = 1.001f;
                                
                                omxClock.OMXSetSpeed(S(speed));
                                omxClock.OMXSetSpeed(S(speed), true, true);
                                ofLog(OF_LOG_NOTICE,  "Live: %.2f (%.2f) S:%.3f T:%.2f\n", m_latency, latency, speed, m_threshold);
                            }
                        }
                    }
                    else if(!m_Pause && (m_omx_reader.IsEof() || m_omx_pkt || TRICKPLAY(omxClock.OMXPlaySpeed()) || (audio_fifo_high && video_fifo_high)))
                    {
                        if (omxClock.OMXIsPaused())
                        {
                            ofLog(OF_LOG_NOTICE, "Resume %.2f,%.2f (%d,%d,%d,%d) EOF:%d PKT:%p\n", audio_fifo, video_fifo, audio_fifo_low, video_fifo_low, audio_fifo_high, video_fifo_high, m_omx_reader.IsEof(), m_omx_pkt);
                            omxClock.OMXResume();
                        }
                    }
                    else if (m_Pause || audio_fifo_low || video_fifo_low)
                    {
                        if (!omxClock.OMXIsPaused())
                        {
                            if (!m_Pause)
                            {
                                m_threshold = std::min(2.0f*m_threshold, 16.0f);
                                ofLog(OF_LOG_NOTICE, "Pause %.2f,%.2f (%d,%d,%d,%d) %.2f\n", audio_fifo, video_fifo, audio_fifo_low, video_fifo_low, audio_fifo_high, video_fifo_high, m_threshold);
                            }
                            omxClock.OMXPause();
                        }
                    }
                }
                if (!sentStarted)
                {
                    ofLog(OF_LOG_NOTICE, "COMXPlayer::HandleMessages - player started RESET");
                    omxClock.OMXReset(m_has_video, m_has_audio);
                    sentStarted = true;
                }
                
                if(!m_omx_pkt)
                    m_omx_pkt = m_omx_reader.Read();
                
                if(m_omx_pkt)
                    m_send_eos = false;
                
                if(m_omx_reader.IsEof() && !m_omx_pkt)
                {
                    // demuxer EOF, but may have not played out data yet
                    if ( (m_has_video && m_player_video.GetCached()) ||
                        (m_has_audio && m_player_audio.GetCached()) )
                    {
                        OMXClock::OMXSleep(10);
                        continue;
                    }
                    if (!m_send_eos && m_has_video)
                        m_player_video.SubmitEOS();
                    if (!m_send_eos && m_has_audio)
                        m_player_audio.SubmitEOS();
                    m_send_eos = true;
                    if ( (m_has_video && !m_player_video.IsEOS()) ||
                        (m_has_audio && !m_player_audio.IsEOS()) )
                    {
                        OMXClock::OMXSleep(10);
                        continue;
                    }
                    ofLog() << "REACHED END OF STREAM";
                    if(listener)
                    {
                        listener->onVideoEnd();
                    }
                    if (m_loop)
                    {
                        if(totalNumFrames)
                        {
                            m_incr = m_loop_from - (omxClock.OMXMediaTime() ? omxClock.OMXMediaTime() / DVD_TIME_BASE : last_seek_pos); 
                        }else
                        {
                            ofLog() << "WILL LOOP VIA RESTART";

                            if(listener)
                            {
                                listener->onVideoLoop(true);
                            }
                        }    
                        
                        
                        continue;
                    }
                    
                    break;
                }
                
                if(m_has_video && m_omx_pkt && m_omx_reader.IsActive(OMXSTREAM_VIDEO, m_omx_pkt->stream_index))
                {
                    if (TRICKPLAY(omxClock.OMXPlaySpeed()))
                    {
                        m_packet_after_seek = true;
                    }
                    if(m_player_video.AddPacket(m_omx_pkt))
                        m_omx_pkt = NULL;
                    else
                        OMXClock::OMXSleep(10);
                }
                else if(m_has_audio && m_omx_pkt && !TRICKPLAY(omxClock.OMXPlaySpeed()) && m_omx_pkt->codec_type == AVMEDIA_TYPE_AUDIO)
                {
                    if(m_player_audio.AddPacket(m_omx_pkt))
                        m_omx_pkt = NULL;
                    else
                        OMXClock::OMXSleep(10);
                }
                else
                {
                    if(m_omx_pkt)
                    {
                        m_omx_reader.FreePacket(m_omx_pkt);
                        m_omx_pkt = NULL;
                    }
                    else
                        OMXClock::OMXSleep(10);
                }
            }
        }
        
    }
    void seekToFrame(int frameTarget)
    {
        lock();
        int fps =  videoFrameRate;
        
        double seekTime = frameTarget/fps;
        printf("seekTime %lf\n", seekTime);
        double oldPos = omxClock.OMXMediaTime()*1e-6;
        m_incr = seekTime-oldPos;
        
        ofLog() << "frameTarget: " << frameTarget << " seekTime: " << seekTime << " m_incr: " << m_incr;
        unlock();
        
    }

    void seekToTimeInSeconds(int timeInSeconds)
    {
        lock();
        int seekTime = timeInSeconds%60;
        double oldPos = omxClock.OMXMediaTime()*1e-6;
        m_incr = seekTime-oldPos;
        ofLog() << "seekToTimeInSeconds: " << m_incr;
        unlock();
    }

    void increaseSpeed()
    {
        lock();
        if (playspeed_current < playspeed_slow_min || playspeed_current > playspeed_slow_max)
        {
            playspeed_current = playspeed_slow_max-1;
        }
        playspeed_current = std::min(playspeed_current+1, playspeed_slow_max);
        SetSpeed(playspeeds[playspeed_current]);
        m_Pause = false;
        unlock();
    }
    
    void decreaseSpeed()
    {
        lock();
        if (playspeed_current < playspeed_slow_min || playspeed_current > playspeed_slow_max)
        {
             playspeed_current = playspeed_slow_max-1;
        }
        playspeed_current = std::max(playspeed_current-1, playspeed_slow_min);
        SetSpeed(playspeeds[playspeed_current]);
        m_Pause = false;
        unlock();
        
    }
    
    void setNormalSpeed()
    {
        SetSpeed(playspeeds[playspeed_normal]);
    }
    
    void stepFrameForward()
    {
        stepNumFrames(1);
    }
    
    void stepNumFrames(int step)
    {
        if (!m_Pause)
        {
            m_Pause=true;
        }
        lock();
        if (step > 1) 
        {
            int count = step;
            while (count > 0) 
            {
                omxClock.OMXStep();
                count--;
            }
        }else
        {
            omxClock.OMXStep();
        }
        unlock();
    }
    
    void rewind()
    {
        lock();
        if (playspeed_current >= playspeed_ff_min && playspeed_current <= playspeed_ff_max)
        {
            playspeed_current = playspeed_normal;
            m_seek_flush = true;
        }
        else if (playspeed_current < playspeed_rew_max || playspeed_current > playspeed_rew_min)
        {
            playspeed_current = playspeed_rew_min;
        }
        else
        {
            playspeed_current = std::max(playspeed_current-1, playspeed_rew_max);
        }
        SetSpeed(playspeeds[playspeed_current]);
        m_Pause = false;
        unlock();
    }
    
    void fastForward()
    {
        lock();
        if (playspeed_current >= playspeed_rew_max && playspeed_current <= playspeed_rew_min)
        {
            playspeed_current = playspeed_normal;
            m_seek_flush = true;
        } 
        else if (playspeed_current < playspeed_ff_min || playspeed_current > playspeed_ff_max)
        {
            playspeed_current = playspeed_ff_min;
        } 
        else
        {
            playspeed_current = std::min(playspeed_current+1, playspeed_ff_max);
        }
        SetSpeed(playspeeds[playspeed_current]);

        m_Pause = false;
        unlock();
    }
    void decreaseVolume()
    {
        m_Volume -= 300;
        m_player_audio.SetVolume(pow(10, m_Volume / 2000.0));
        ofLog(OF_LOG_NOTICE, "Current Volume: %.2fdB\n", m_Volume / 100.0f);
    }
    
    void increaseVolume()
    {
        m_Volume += 300;
        m_player_audio.SetVolume(pow(10, m_Volume / 2000.0));
        ofLog(OF_LOG_NOTICE, "Current Volume: %.2fdB\n", m_Volume / 100.0f);
    }
    
    static float get_display_aspect_ratio(SDTV_ASPECT_T aspect)
    {
        float display_aspect;
        switch (aspect) {
            case SDTV_ASPECT_4_3:  display_aspect = 4.0/3.0;  break;
            case SDTV_ASPECT_14_9: display_aspect = 14.0/9.0; break;
            case SDTV_ASPECT_16_9: display_aspect = 16.0/9.0; break;
            default:               display_aspect = 4.0/3.0;  break;
        }
        return display_aspect;
    }
    
    static float get_display_aspect_ratio(HDMI_ASPECT_T aspect)
    {
        float display_aspect;
        switch (aspect) {
            case HDMI_ASPECT_4_3:   display_aspect = 4.0/3.0;   break;
            case HDMI_ASPECT_14_9:  display_aspect = 14.0/9.0;  break;
            case HDMI_ASPECT_16_9:  display_aspect = 16.0/9.0;  break;
            case HDMI_ASPECT_5_4:   display_aspect = 5.0/4.0;   break;
            case HDMI_ASPECT_16_10: display_aspect = 16.0/10.0; break;
            case HDMI_ASPECT_15_9:  display_aspect = 15.0/9.0;  break;
            case HDMI_ASPECT_64_27: display_aspect = 64.0/27.0; break;
            default:                display_aspect = 16.0/9.0;  break;
        }
        return display_aspect;
    }
    
    static void CallbackTvServiceCallback(void *userdata, uint32_t reason, uint32_t param1, uint32_t param2)
    {
        sem_t *tv_synced = (sem_t *)userdata;
        switch(reason)
        {
            case VC_HDMI_UNPLUGGED:
                break;
            case VC_HDMI_STANDBY:
                break;
            case VC_SDTV_NTSC:
            case VC_SDTV_PAL:
            case VC_HDMI_HDMI:
            case VC_HDMI_DVI:
                // Signal we are ready now
                sem_post(tv_synced);
                break;
            default:
                break;
        }
    }
    
    void SetSpeed(int iSpeed)
    {
        currentPlaybackSpeed = playspeeds[playspeed_current]/1000.0f;
        ofLog(OF_LOG_NOTICE, "Playspeed: %.3f", currentPlaybackSpeed);
        m_omx_reader.SetSpeed(iSpeed);
        
        // flush when in trickplay mode
        if (TRICKPLAY(iSpeed) || TRICKPLAY(omxClock.OMXPlaySpeed()))
        {
            FlushStreams(DVD_NOPTS_VALUE);
        }
        
        omxClock.OMXSetSpeed(iSpeed);
        omxClock.OMXSetSpeed(iSpeed, true, true);
    }
    
    void SetVideoMode(int width, int height, int fpsrate, int fpsscale)
    {
        bool m_gen_log  = false;
        bool m_NativeDeinterlace=false;
        
        int32_t num_modes = 0;
        int i;
        HDMI_RES_GROUP_T prefer_group;
        HDMI_RES_GROUP_T group = HDMI_RES_GROUP_CEA;
        float fps = 60.0f; // better to force to higher rate if no information is known
        uint32_t prefer_mode;
        
        if (fpsrate && fpsscale)
            fps = DVD_TIME_BASE / OMXReader::NormalizeFrameduration((double)DVD_TIME_BASE * fpsscale / fpsrate);
        
        //Supported HDMI CEA/DMT resolutions, preferred resolution will be returned
        TV_SUPPORTED_MODE_NEW_T *supported_modes = NULL;
        // query the number of modes first
        int max_supported_modes = vc_tv_hdmi_get_supported_modes_new(group, NULL, 0, &prefer_group, &prefer_mode);
        
        if (max_supported_modes > 0)
            supported_modes = new TV_SUPPORTED_MODE_NEW_T[max_supported_modes];
        
        if (supported_modes)
        {
            num_modes = vc_tv_hdmi_get_supported_modes_new(group,
                                                                     supported_modes, max_supported_modes, &prefer_group, &prefer_mode);
            
            if(m_gen_log) {
                CLog::Log(LOGDEBUG, "EGL get supported modes (%d) = %d, prefer_group=%x, prefer_mode=%x\n",
                          group, num_modes, prefer_group, prefer_mode);
            }
        }
        
        TV_SUPPORTED_MODE_NEW_T *tv_found = NULL;
        
        if (num_modes > 0 && prefer_group != HDMI_RES_GROUP_INVALID)
        {
            uint32_t best_score = 1<<30;
            uint32_t scan_mode = m_NativeDeinterlace;
            
            for (i=0; i<num_modes; i++)
            {
                TV_SUPPORTED_MODE_NEW_T *tv = supported_modes + i;
                uint32_t score = 0;
                uint32_t w = tv->width;
                uint32_t h = tv->height;
                uint32_t r = tv->frame_rate;
                
                /* Check if frame rate match (equal or exact multiple) */
                if(fabs(r - 1.0f*fps) / fps < 0.002f)
                    score += 0;
                else if(fabs(r - 2.0f*fps) / fps < 0.002f)
                    score += 1<<8;
                else 
                    score += (1<<16) + (1<<20)/r; // bad - but prefer higher framerate
                
                /* Check size too, only choose, bigger resolutions */
                if(width && height) 
                {
                    /* cost of too small a resolution is high */
                    score += max((int)(width -w), 0) * (1<<16);
                    score += max((int)(height-h), 0) * (1<<16);
                    /* cost of too high a resolution is lower */
                    score += max((int)(w-width ), 0) * (1<<4);
                    score += max((int)(h-height), 0) * (1<<4);
                } 
                
                // native is good
                if (!tv->native) 
                    score += 1<<16;
                
                // interlace is bad
                if (scan_mode != tv->scan_mode) 
                    score += (1<<16);
                
                // wanting 3D but not getting it is a negative
                /*
                if (is3d == CONF_FLAGS_FORMAT_SBS && !(tv->struct_3d_mask & HDMI_3D_STRUCT_SIDE_BY_SIDE_HALF_HORIZONTAL))
                    score += 1<<18;
                if (is3d == CONF_FLAGS_FORMAT_TB  && !(tv->struct_3d_mask & HDMI_3D_STRUCT_TOP_AND_BOTTOM))
                    score += 1<<18;
                if (is3d == CONF_FLAGS_FORMAT_FP  && !(tv->struct_3d_mask & HDMI_3D_STRUCT_FRAME_PACKING))
                    score += 1<<18;
                */
                // prefer square pixels modes
                float par = get_display_aspect_ratio((HDMI_ASPECT_T)tv->aspect_ratio)*(float)tv->height/(float)tv->width;
                score += fabs(par - 1.0f) * (1<<12);
                
                /*printf("mode %dx%d@%d %s%s:%x par=%.2f score=%d\n", tv->width, tv->height, 
                 tv->frame_rate, tv->native?"N":"", tv->scan_mode?"I":"", tv->code, par, score);*/
                
                if (score < best_score) 
                {
                    tv_found = tv;
                    best_score = score;
                }
            }
        }
        
        if(tv_found)
        {
            char response[80];
            ofLog(OF_LOG_NOTICE, "Output mode %d: %dx%d@%d %s%s:%x\n", tv_found->code, tv_found->width, tv_found->height, 
                   tv_found->frame_rate, tv_found->native?"N":"", tv_found->scan_mode?"I":"", tv_found->code);
            if (m_NativeDeinterlace && tv_found->scan_mode)
                vc_gencmd(response, sizeof response, "hvs_update_fields %d", 1);
            
            // if we are closer to ntsc version of framerate, let gpu know
            int ifps = (int)(fps+0.5f);
            bool ntsc_freq = fabs(fps*1001.0f/1000.0f - ifps) < fabs(fps-ifps);
            
            /* inform TV of ntsc setting */
            HDMI_PROPERTY_PARAM_T property;
            property.property = HDMI_PROPERTY_PIXEL_CLOCK_TYPE;
            property.param1 = ntsc_freq ? HDMI_PIXEL_CLOCK_TYPE_NTSC : HDMI_PIXEL_CLOCK_TYPE_PAL;
            property.param2 = 0;
            
            /* inform TV of any 3D settings. Note this property just applies to next hdmi mode change, so no need to call for 2D modes */
            property.property = HDMI_PROPERTY_3D_STRUCTURE;
            property.param1 = HDMI_3D_FORMAT_NONE;
            property.param2 = 0;
            /*
            if (is3d != CONF_FLAGS_FORMAT_NONE)
            {
                if (is3d == CONF_FLAGS_FORMAT_SBS && tv_found->struct_3d_mask & HDMI_3D_STRUCT_SIDE_BY_SIDE_HALF_HORIZONTAL)
                    property.param1 = HDMI_3D_FORMAT_SBS_HALF;
                else if (is3d == CONF_FLAGS_FORMAT_TB && tv_found->struct_3d_mask & HDMI_3D_STRUCT_TOP_AND_BOTTOM)
                    property.param1 = HDMI_3D_FORMAT_TB_HALF;
                else if (is3d == CONF_FLAGS_FORMAT_FP && tv_found->struct_3d_mask & HDMI_3D_STRUCT_FRAME_PACKING)
                    property.param1 = HDMI_3D_FORMAT_FRAME_PACKING;
                vc_tv_hdmi_set_property(&property);
            }*/
            
            ofLog(OF_LOG_NOTICE, "ntsc_freq:%d %s\n", ntsc_freq, property.param1 == HDMI_3D_FORMAT_SBS_HALF ? "3DSBS" :
                   property.param1 == HDMI_3D_FORMAT_TB_HALF ? "3DTB" : property.param1 == HDMI_3D_FORMAT_FRAME_PACKING ? "3DFP":"");
            sem_t tv_synced;
            sem_init(&tv_synced, 0, 0);
            vc_tv_register_callback(CallbackTvServiceCallback, &tv_synced);
            int success = vc_tv_hdmi_power_on_explicit_new(HDMI_MODE_HDMI, (HDMI_RES_GROUP_T)group, tv_found->code);
            if (success == 0)
                sem_wait(&tv_synced);
            vc_tv_unregister_callback(CallbackTvServiceCallback);
            sem_destroy(&tv_synced);
        }
        if (supported_modes)
            delete[] supported_modes;
    }
    
    int doExit()
    {
        ofLog() << "EXITING";
        
        if (m_stats)
            ofLog(OF_LOG_NOTICE, "\n");
        
        if (m_stop)
        {
            unsigned t = (unsigned)(omxClock.OMXMediaTime()*1e-6);
            ofLog(OF_LOG_NOTICE, "Stopped at: %02d:%02d:%02d\n", (t/3600), (t/60)%60, t%60);
        }
        
        if (m_NativeDeinterlace)
        {
            char response[80];
            vc_gencmd(response, sizeof response, "hvs_update_fields %d", 0);
        }
        if(m_has_video && m_refresh && tv_state.display.hdmi.group && tv_state.display.hdmi.mode)
        {
            vc_tv_hdmi_power_on_explicit_new(HDMI_MODE_HDMI, (HDMI_RES_GROUP_T)tv_state.display.hdmi.group, tv_state.display.hdmi.mode);
        }
        
        omxClock.OMXStop();
        omxClock.OMXStateIdle();
        
        m_player_video.Close();
        m_player_audio.Close();
       
        if(m_omx_pkt)
        {
            m_omx_reader.FreePacket(m_omx_pkt);
            m_omx_pkt = NULL;
        }
        
        m_omx_reader.Close();
        
        omxClock.OMXDeinitialize();

        
        vc_tv_show_info(0);
        
        //g_OMX.Deinitialize();
        // g_RBP.Deinitialize();
        ofLog(OF_LOG_NOTICE, "have a nice day ;)\n");
        clear();
        // exit status success on playback end
        if (m_send_eos)
            return EXIT_SUCCESS;
        // exit status OMXPlayer defined value on user quit
        if (m_stop)
            return 3;
        // exit status failure on other cases
        return EXIT_FAILURE;
    }
    
#if 0
    void processCommand()
    {
        if(keyCommand == -1) return;
        lock();
        
        double oldPos, newPos;
        
        switch(keyCommand)
        {
            case KeyConfig::ACTION_CHANGE_FILE:
                FlushStreams(DVD_NOPTS_VALUE);
                m_omx_reader.Close();
                m_player_video.Close();
                m_player_audio.Close();
                //m_filename = result.getWinArg();
                change_file();
                break;
            case KeyConfig::ACTION_SHOW_INFO:
                m_tv_show_info = !m_tv_show_info;
                vc_tv_show_info(m_tv_show_info);
                break;
            case KeyConfig::ACTION_DECREASE_SPEED:
                decreaseSpeed();
                
                break;
            case KeyConfig::ACTION_INCREASE_SPEED:
                increaseSpeed();
                
                break;
            case KeyConfig::ACTION_REWIND:
                rewind();
                break;
            case KeyConfig::ACTION_FAST_FORWARD:
                fastForward();
                break;
            case KeyConfig::ACTION_STEP:
                omxClock.OMXStep();
                printf("Step\n");
            {
                auto t = (unsigned) (omxClock.OMXMediaTime()*1e-3);
                auto dur = m_omx_reader.GetStreamLength() / 1000;
                ofLog(OF_LOG_NOTICE, "Step\n%02d:%02d:%02d.%03d / %02d:%02d:%02d",
                      (t/3600000), (t/60000)%60, (t/1000)%60, t%1000,
                      (dur/3600), (dur/60)%60, dur%60);
            }
                break;
            case KeyConfig::ACTION_PREVIOUS_AUDIO:
                if(m_has_audio)
                {
                    int new_index = m_omx_reader.GetAudioIndex() - 1;
                    if(new_index >= 0)
                    {
                        m_omx_reader.SetActiveStream(OMXSTREAM_AUDIO, new_index);
                        ofLog(OF_LOG_NOTICE, "Audio stream: %d", m_omx_reader.GetAudioIndex() + 1);
                    }
                }
                break;
            case KeyConfig::ACTION_NEXT_AUDIO:
                if(m_has_audio)
                {
                    m_omx_reader.SetActiveStream(OMXSTREAM_AUDIO, m_omx_reader.GetAudioIndex() + 1);
                    ofLog(OF_LOG_NOTICE, "Audio stream: %d", m_omx_reader.GetAudioIndex() + 1);
                }
                break;
            case KeyConfig::ACTION_PREVIOUS_CHAPTER:
                if(m_omx_reader.GetChapterCount() > 0)
                {
                    m_omx_reader.SeekChapter(m_omx_reader.GetChapter() - 1, &startpts);
                    ofLog(OF_LOG_NOTICE, "Chapter %d", m_omx_reader.GetChapter());
                    FlushStreams(startpts);
                    m_seek_flush = true;
                    m_chapter_seek = true;
                }
                else
                {
                    m_incr = -600.0;
                }
                break;
            case KeyConfig::ACTION_NEXT_CHAPTER:
                if(m_omx_reader.GetChapterCount() > 0)
                {
                    m_omx_reader.SeekChapter(m_omx_reader.GetChapter() + 1, &startpts);
                    ofLog(OF_LOG_NOTICE, "Chapter %d", m_omx_reader.GetChapter());
                    FlushStreams(startpts);
                    m_seek_flush = true;
                    m_chapter_seek = true;
                }
                else
                {
                    m_incr = 600.0;
                }
                break;
                
            case KeyConfig::ACTION_EXIT:
                m_stop = true;
                doExit();
                break;
            case KeyConfig::ACTION_SEEK_BACK_SMALL:
                if(m_omx_reader.CanSeek()) m_incr = -30.0;
                break;
            case KeyConfig::ACTION_SEEK_FORWARD_SMALL:
                if(m_omx_reader.CanSeek()) m_incr = 30.0;
                break;
            case KeyConfig::ACTION_SEEK_FORWARD_LARGE:
                if(m_omx_reader.CanSeek()) m_incr = 600.0;
                break;
            case KeyConfig::ACTION_SEEK_BACK_LARGE:
                if(m_omx_reader.CanSeek()) m_incr = -600.0;
                break;
            case KeyConfig::ACTION_SEEK_RELATIVE:
                //m_incr = result.getArg() * 1e-6;
                break;
            case KeyConfig::ACTION_SEEK_ABSOLUTE:
                //newPos = result.getArg() * 1e-6;
                oldPos = omxClock.OMXMediaTime()*1e-6;
                m_incr = newPos - oldPos;
                break;
            case KeyConfig::ACTION_SET_ALPHA:
                //m_player_video.SetAlpha(result.getArg());
                break;
            case KeyConfig::ACTION_SET_LAYER:
                //m_player_video.SetLayer(result.getArg());
                break;
            case KeyConfig::ACTION_PLAY:
                m_Pause=false;
                
                break;
            case KeyConfig::ACTION_PAUSE:
                m_Pause=true;
                
                break;
            case KeyConfig::ACTION_PLAYPAUSE:
                m_Pause = !m_Pause;
                if (omxClock.OMXPlaySpeed() != DVD_PLAYSPEED_NORMAL && omxClock.OMXPlaySpeed() != DVD_PLAYSPEED_PAUSE)
                {
                    printf("resume\n");
                    playspeed_current = playspeed_normal;
                    SetSpeed(playspeeds[playspeed_current]);
                    m_seek_flush = true;
                }
                if(m_Pause)
                {
                    
                    
                    auto t = (unsigned) (omxClock.OMXMediaTime()*1e-6);
                    auto dur = m_omx_reader.GetStreamLength() / 1000;
                    ofLog(OF_LOG_NOTICE, "Pause\n%02d:%02d:%02d / %02d:%02d:%02d", (t/3600), (t/60)%60, t%60, (dur/3600), (dur/60)%60, dur%60);
                }
                else
                {
                    
                    
                    auto t = (unsigned) (omxClock.OMXMediaTime()*1e-6);
                    auto dur = m_omx_reader.GetStreamLength() / 1000;
                    ofLog(OF_LOG_NOTICE, "Play\n%02d:%02d:%02d / %02d:%02d:%02d", (t/3600), (t/60)%60, t%60, (dur/3600), (dur/60)%60, dur%60);
                }
                break;
#if 0
            case KeyConfig::ACTION_MOVE_VIDEO:
                sscanf(result.getWinArg(), "%f %f %f %f", &m_config_video.dst_rect.x1, &m_config_video.dst_rect.y1, &m_config_video.dst_rect.x2, &m_config_video.dst_rect.y2);
                m_player_video.SetVideoRect(m_config_video.src_rect, m_config_video.dst_rect);
                
                break;
            case KeyConfig::ACTION_CROP_VIDEO:
                sscanf(result.getWinArg(), "%f %f %f %f", &m_config_video.src_rect.x1, &m_config_video.src_rect.y1, &m_config_video.src_rect.x2, &m_config_video.src_rect.y2);
                m_player_video.SetVideoRect(m_config_video.src_rect, m_config_video.dst_rect);
                break;
            case KeyConfig::ACTION_SET_ASPECT_MODE:
                if (result.getWinArg()) {
                    if (!strcasecmp(result.getWinArg(), "letterbox"))
                        m_config_video.aspectMode = 1;
                    else if (!strcasecmp(result.getWinArg(), "fill"))
                        m_config_video.aspectMode = 2;
                    else if (!strcasecmp(result.getWinArg(), "stretch"))
                        m_config_video.aspectMode = 3;
                    else
                        m_config_video.aspectMode = 0;
                    m_player_video.SetVideoRect(m_config_video.aspectMode);
                }
                break;
#endif
            case KeyConfig::ACTION_HIDE_VIDEO:
                // set alpha to minimum
                m_player_video.SetAlpha(0);
                break;
            case KeyConfig::ACTION_UNHIDE_VIDEO:
                // set alpha to maximum
                m_player_video.SetAlpha(255);
                break;
                
            case KeyConfig::ACTION_DECREASE_VOLUME:
                decreaseVolume();
                break;
            case KeyConfig::ACTION_INCREASE_VOLUME:
                increaseVolume();
                break;
            default:
                break;
        }
        keyCommand = -1;
        unlock();
        
    }
#endif
   
};
