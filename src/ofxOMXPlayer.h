#pragma once
#include "ofMain.h"
#include "ofxOMXPlayerEngine.h"
class ofxOMXPlayer;
class ofxOMXPlayerListener
{
public:
    virtual void onVideoEnd(ofxOMXPlayer*) = 0;
    virtual void onVideoLoop(ofxOMXPlayer*) = 0;
    
};
class ImageFilter
{
public:
    string name;
    OMX_IMAGEFILTERTYPE filterType;
    ImageFilter(string name_, OMX_IMAGEFILTERTYPE filterType_ )
    {
        name = name_;
        filterType = filterType_;
    };
};
class ofxOMXPlayer : public EngineListener
{
public:
    
    ~ofxOMXPlayer();
    COMXCore omxCore;
    ofxOMXPlayerEngine engine;
    ofxOMXPlayerSettings settings;
    ofxOMXPlayerListener* listener;
    bool engineNeedsRestart;
    bool pendingLoopMessage;
    vector<ImageFilter>imageFilters;
    string currentFilterName;
    int playerID;
    
#pragma mark SETUP
    ofxOMXPlayer();
    bool setup(ofxOMXPlayerSettings settings_);
    void start();
    void loadMovie(string videoPath);
    void reopen();
    void close();

#pragma mark GETTERS
    int getWidth();
    int getHeight();
    float getFPS();
    int getTotalNumFrames();
    float getDurationInSeconds();
    ofTexture&  getTextureReference();
    ofFbo& getFboReference();
    GLuint getTextureID();
    unsigned char* getPixels();
    int getClockSpeed();
    bool isOpen();
    bool getIsOpen();
    bool isPlaying();
    float getPlaybackSpeed();
    float getMediaTime();
    int getCurrentFrame();
    float getVolume();
    float getVolumeDB();
    bool isLoopingEnabled();
    bool isTextureEnabled();
    bool isFrameNew();
    COMXStreamInfo&  getVideoStreamInfo();
    COMXStreamInfo&  getAudioStreamInfo();
    static string getRandomVideo(string path);
    string getInfo();
    
#pragma mark LISTENERS
    void onVideoEnd();
    void onVideoLoop(bool needsRestart);
    void onUpdate(ofEventArgs& eventArgs);

#pragma mark DRAWING
    void draw(float x, float y, float w, float h);
    void draw(ofRectangle rectangle);
    void drawCropped(float cropX, float cropY, float cropWidth, float cropHeight,
                     float drawX, float drawY, float drawWidth, float drawHeight);
    void drawCropped(ofRectangle cropRectangle, ofRectangle drawRectangle);
    void setAlpha(int alpha);
    void setLayer(int layer);
    void rotateVideo(int degrees, bool doMirror = false);
    
    void setFilter(OMX_IMAGEFILTERTYPE filterType);
    string findFilterName(OMX_IMAGEFILTERTYPE filterType);
#pragma mark PLAYBACK CONTROLS
    bool isPaused();
    void setPaused(bool doPause);
    void togglePause();
    void setNormalSpeed();
    void increaseSpeed();
    void decreaseSpeed();
    void stepFrameForward();
    void stepNumFrames(int step);
    void seekToTimeInSeconds(int timeInSeconds);
    void seekToFrame(int frameTarget);
    void restartMovie();
    void enableLooping();
    void disableLooping();
#pragma mark PLAYBACK AUDIO
    
    void increaseVolume();
    void decreaseVolume();
    void setVolumeNormalized(float volume);
    void setVolume(float volume);
    float getVolumeNormalized();
    
#pragma mark PIXELS
    
    void updatePixels();
    void saveImage(string imagePath="");

#pragma mark OLD/TODO
    void scrubForward(int step=1);

#if 0     
    void applyFilter(OMX_IMAGEFILTERTYPE filter);
    //direct only
    void        setDisplayRect(float x, float y, float width, float height);
    void        setDisplayRect(ofRectangle&);
    void        cropVideo(ofRectangle&);
    void        cropVideo(float x, float y, float width, float height);
    void        setMirror(bool);
    void        setAlpha(int alpha);
    void        setFullScreen(bool);
    void        setForceFill(bool);
    ofRectangle* cropRectangle;
    ofRectangle* drawRectangle;
    
#endif
    
};

