#pragma once

//this is just a scratch file i use for development - probably broke

#include "ofMain.h"
#include "ofxOMXPlayer.h"
#include "TerminalListener.h"

class ofApp : public ofBaseApp, public KeyListener, public ofxOMXPlayerListener
{
public:
    
    
    ofxOMXPlayer omxPlayer;
    TerminalListener terminalListener;
    vector<string> videoFiles;
    int currentFileIndex;
    bool doCreatePlayer;
    bool doSeek = false;
    bool doReopen = false;
    
    // required for ofxOMXPlayerListener
    void onVideoEnd(ofxOMXPlayer* player)
    {
        ofLog() << "ofApp::onVideoEnd: LOOPING ENABLED" << player->isLoopingEnabled();
    };
    
    void setup()
    {
        currentFileIndex = 0;
        doCreatePlayer = true;
        terminalListener.setup(this);
        videoFiles.push_back("/home/pi/videos/current/Timecoded_Big_bunny_1.mov");

        videoFiles.push_back("/home/pi/videos/AirBallonTimecode720pAAC.mov");
        
        videoFiles.push_back("https://devimages.apple.com.edgekey.net/samplecode/avfoundationMedia/AVFoundationQueuePlayer_HLS2/master.m3u8");

        videoFiles.push_back("http://devimages.apple.com/iphone/samples/bipbop/gear1/prog_index.m3u8");
        
        videoFiles.push_back("/home/pi/videos/KALI UCHIS - NEVER BE YOURS.mp3");
        videoFiles.push_back("/opt/vc/src/hello_pi/hello_video/test.h264");
        ofxOMXPlayerSettings settings;
        settings.videoPath = videoFiles[currentFileIndex];
        settings.initialVolume = 0.6;
        settings.enableAudio = false;
        
        //settings.loopPoint = "0:0:10";
        settings.loopPoint = "10";
        settings.debugLevel = 2;
        settings.listener = this;
        omxPlayer.engine.m_config_audio.device = "omx:alsa";
        //omxPlayer.engine.m_config_audio.subdevice = "default";
        omxPlayer.engine.m_config_audio.subdevice = "hw:1,0";

        
        
        omxPlayer.setup(settings);
        
    }
    
    void update()
    {
        /*
        if(doCreatePlayer)
        {
            doCreatePlayer = false;
            if(player)
            {
                player->close();
                delete player;
            }
            player = new ofxOMXMediaPlayer();
            player->setup(videoFiles[currentFileIndex], true);
            if(currentFileIndex+1 < videoFiles.size())
            {
                currentFileIndex++;

            }else
            {
                currentFileIndex = 0;
            }
        }*/
        
    }
    
    void onCharacterReceived(KeyListenerEventData& e)
    {
        
        keyPressed(e.character);
  
    }
    
    void draw()
    {
        
        
        ofBackgroundGradient(ofColor::red, ofColor::black);
        omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
        
        stringstream info;
        info << omxPlayer.getInfo() << endl;
        info << endl;
        info << "COMMANDS" << endl;
        info << "PRESS p TO PAUSE" << endl;
        info << "PRESS v TO STEP FRAME" << endl;
        //info << "PRESS r TO RESTART" << endl;
        info << "PRESS 1 TO DECREASE SPEED" << endl;
        info << "PRESS 2 TO INCREASE SPEED" << endl;
        info << "PRESS 3 FOR NORMAL SPEED" << endl;
        info << "PRESS s FOR RANDOM SEEK" << endl;
        info << "PRESS r TO RESTART MOVIE" << endl;
        info << "PRESS p TO TOGGLE PAUSE"<< endl;
        info << "PRESS v TO SEEK TO STEP FORWARD 1 FRAME" << endl;
        info << "PRESS V TO SEEK TO STEP FORWARD 5 FRAMES" << endl;
        info << "PRESS - TO SEEK TO DECREASE VOLUME" << endl;
        info << "PRESS + or = TO SEEK TO INCREASE VOLUME" << endl;
        
        ofDrawBitmapStringHighlight(info.str(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
    }
    
    
    void keyPressed  (int key){
        
        ofLog(OF_LOG_VERBOSE, "%c keyPressed", key);
        switch (key) 
        {
            case ' ':
            case 'p':
            {
                omxPlayer.togglePause();
                break;
            }
            case '1':
            {
                omxPlayer.decreaseSpeed();
                break;
            }
            case '2':
            {
                omxPlayer.increaseSpeed();
                break;
            }
            case '3':
            {
                omxPlayer.setNormalSpeed();
                break;
            }
            case '-':
            {
                
                ofLogVerbose() << "decreaseVolume";
                omxPlayer.decreaseVolume();
                break;
            }
            case '=':
            case '+':
            {
                ofLogVerbose() << "increaseVolume";
                omxPlayer.increaseVolume();
                break;
            }
            case 'v':
            {
                ofLogVerbose() << "stepFrameForward";
                omxPlayer.stepFrameForward();
                break;
            }
            case 'V':
            {
                ofLogVerbose() << "stepNumFrames 5";
                omxPlayer.stepNumFrames(5);
                break;
            }
            case 's':
            {
                doSeek = true;
                break;
            }
            case 'r':
            {
                omxPlayer.restartMovie();
                break;
            }
            case '>':
            case '.':
            {
                omxPlayer.fastForward();
                break;
            }    
            case '<':
            case ',':
            {
                omxPlayer.rewind();
                break;
            }     
            case 'q':
            {
                omxPlayer.close();
                break;
            }
            case 'o':
            {
                doReopen = true;
                break;
            }
            default:
            {
                break;
            }    
        }
        
#if 0
        switch(e.character)
        {
            case 'n':
            {
                doCreatePlayer = true;
                return;
                
            }    
        }
        
        if(player)
        {
            int charInt = (int)e.character;
            if(player->keymap[charInt])
            {
                ofLog() << "charInt: " << player->keymap[charInt];
                player->keyCommand = player->keymap[charInt];
            }else
            {
                ofLog() << "NOTHING MAPPED FOR " << e.character;
            }
        }else
        {
            switch(e.character)
            {
                case 't':
                {
                    //EGLImageManager::getInstance().useTexture  = !EGLImageManager::getInstance().useTexture;
                    //ofLog() << "EGLImageManager::getInstance().useTexture: " << EGLImageManager::getInstance().useTexture;
                    return;
                    
                }
                    
            }
        }
#endif  
        
    }
};

