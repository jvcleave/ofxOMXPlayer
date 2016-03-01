#include "ofMain.h"
#include "ofxOMXPlayer.h"
#include "ofBaseTypes.h"

#include "ofxOMXPlayerListener.h"

class ofRPIVideoPlayer: public ofBaseVideoPlayer
{
    class _InnerListener : public ofxOMXPlayerListener
    {
        public:
            bool videoHasEnded;

            _InnerListener() : videoHasEnded(false) { }

            virtual void onVideoEnd(ofxOMXPlayerListenerEventData& e)
            {
                videoHasEnded = true;
            }

            virtual void onVideoLoop(ofxOMXPlayerListenerEventData& e)
            {
            }

    };

    _InnerListener listener;

public:
    ofRPIVideoPlayer();
    bool load(string name);
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
    int getCurrentFrame() /*const*/;
    int getTotalNumFrames() /*const*/;
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
};

