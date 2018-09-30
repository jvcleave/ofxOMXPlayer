#pragma once

//this is just a scratch file i use for development - probably broke

#include "ofMain.h"
#include "ofxOMXPlayer.h"
#include "TerminalListener.h"

class ofApp : public ofBaseApp, public KeyListener, public ofxOMXPlayerListener
{
public:
    
    
    vector<ofxOMXPlayer*> omxPlayers; 

    TerminalListener terminalListener;
    vector<string> videoFiles;
    int currentFileIndex;
    bool doCreatePlayer;
    bool doSeek = false;
    bool doReopen = false;
    bool doLoadNext = false;
    // required for ofxOMXPlayerListener
    void onVideoEnd(ofxOMXPlayer* player)
    {
        ofLog() << "ofApp::onVideoEnd: LOOPING ENABLED" << player->isLoopingEnabled();
    };
    
    void setup()
    {
        videoFiles.push_back("/home/pi/videos/current/Timecoded_Big_bunny_1.mov");
        
        videoFiles.push_back("/home/pi/videos/AirBallonTimecode720pAAC.mov");
        
        videoFiles.push_back("https://devimages.apple.com.edgekey.net/samplecode/avfoundationMedia/AVFoundationQueuePlayer_HLS2/master.m3u8");
        
        videoFiles.push_back("http://devimages.apple.com/iphone/samples/bipbop/gear1/prog_index.m3u8");
        
        videoFiles.push_back("/home/pi/videos/KALI UCHIS - NEVER BE YOURS.mp3");
        videoFiles.push_back("/opt/vc/src/hello_pi/hello_video/test.h264");
        
        currentFileIndex = 0;
        terminalListener.setup(this);
        ofDirectory currentVideoDirectory(ofToDataPath("../../../video", true));
        if (currentVideoDirectory.exists()) 
        {
            currentVideoDirectory.listDir();
            vector<ofFile> files = currentVideoDirectory.getFiles();
            
            
            for (int i=0; i<files.size(); i++) 
            {
                ofxOMXPlayerSettings settings;
                settings.videoPath = files[i].path();
                settings.useHDMIForAudio = true;    //default true
                settings.enableLooping = true;        //default true
                settings.enableAudio = true;        //default true, save resources by disabling
                settings.enableTexture = false;        //default true
                settings.listener = this;

                
                ofxOMXPlayer* player = new ofxOMXPlayer();
                player->engine.m_config_audio.device = "omx:alsa";
                player->engine.m_config_audio.subdevice = "hw:1,0";

                player->setup(settings);
                player->setAlpha(ofRandom(90, 255));
                //player->setLayer(i);
                omxPlayers.emplace_back(player);
            }
        }else{
            ofLogError() << "currentVideoDirectory: " << currentVideoDirectory.path() << " MISSING";
            
            
        }
#if 0
        doCreatePlayer = true;
        t
        videoFiles.push_back("/home/pi/videos/current/Timecoded_Big_bunny_1.mov");

        videoFiles.push_back("/home/pi/videos/AirBallonTimecode720pAAC.mov");
        
        videoFiles.push_back("https://devimages.apple.com.edgekey.net/samplecode/avfoundationMedia/AVFoundationQueuePlayer_HLS2/master.m3u8");

        videoFiles.push_back("http://devimages.apple.com/iphone/samples/bipbop/gear1/prog_index.m3u8");
        
        videoFiles.push_back("/home/pi/videos/KALI UCHIS - NEVER BE YOURS.mp3");
        videoFiles.push_back("/opt/vc/src/hello_pi/hello_video/test.h264");
        ofxOMXPlayerSettings settings;
        settings.videoPath = videoFiles[currentFileIndex];
        settings.initialVolume = 0.6;
        settings.enableAudio = true;
        settings.enableTexture = true;

        //settings.loopPoint = "0:0:10";
        settings.loopPoint = "10";
        settings.debugLevel = -1;
        settings.listener = this;
        omxPlayer.engine.m_config_audio.device = "omx:alsa";
        //omxPlayer.engine.m_config_audio.subdevice = "default";
        omxPlayer.engine.m_config_audio.subdevice = "hw:1,0";

        
        
        omxPlayer.setup(settings);
#endif       
    }
    
    void update()
    {
        if(doLoadNext)
        {
            doLoadNext = false;
            if(currentFileIndex+1 < videoFiles.size())
            {
                currentFileIndex++;
                
            }else
            {
                currentFileIndex = 0;
            }
            for (int i=0; i<omxPlayers.size(); i++) 
            {
                ofxOMXPlayer* player = omxPlayers[i];
                player->loadMovie(videoFiles[currentFileIndex]);

            }
        }
   

        
    }
    
    void onCharacterReceived(KeyListenerEventData& e)
    {
        
        keyPressed(e.character);
  
    }
    
    int layerCounter = 0;
    void draw()
    {
        
        
        ofBackgroundGradient(ofColor::red, ofColor::black);
        stringstream info;
        ofRectangle drawRect;
        for (int i=0; i<omxPlayers.size(); i++) 
        {
            ofxOMXPlayer* player = omxPlayers[i];
            if(player->isPlaying())
            {
                drawRect.set((player->getWidth()/4)*i, 20, player->getWidth()/4, player->getHeight()/4);
                
                if(player->isTextureEnabled())
                {
                    //player->draw(20, 20, player->getWidth()/4, player->getHeight()/4);
                    
                    player->draw(drawRect.x, drawRect.y, drawRect.getWidth(), drawRect.getHeight());
                    
                }else
                {
                    
                    //player->setAlpha(ofRandom(90, 255));
                    //player->setLayer(i);
                    if(layerCounter +1 < 10)
                    {
                        layerCounter++;
                    }else
                    {
                        layerCounter = 0;
                    }
                    ofRectangle cropRect;
                    //ofRectangle drawRect(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), player->getWidth()/2, player->getHeight()/2);
                    
                    player->drawCropped(cropRect.x, cropRect.y, cropRect.getWidth(), cropRect.getHeight(),
                                        drawRect.x, drawRect.y, drawRect.getWidth(), drawRect.getHeight());
                    
                    
                }
                info << player->getInfo() << endl;
                info << drawRect << endl;
            }
            
        }
        
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
        
        ofDrawBitmapStringHighlight(info.str(), 60, 200, ofColor(ofColor::black, 90), ofColor::yellow);
    }
    
    
    void keyPressed  (int key){
        
        ofLog(OF_LOG_VERBOSE, "%c keyPressed", key);
        for (int i=0; i<omxPlayers.size(); i++) 
        {
            ofxOMXPlayer* omxPlayer = omxPlayers[i];
            switch (key) 
            {
                case ' ':
                case 'p':
                {
                    omxPlayer->togglePause();
                    break;
                }
                case '1':
                {
                    omxPlayer->decreaseSpeed();
                    break;
                }
                case '2':
                {
                    omxPlayer->increaseSpeed();
                    break;
                }
                case '3':
                {
                    omxPlayer->setNormalSpeed();
                    break;
                }
                case '-':
                {
                    
                    ofLogVerbose() << "decreaseVolume";
                    omxPlayer->decreaseVolume();
                    break;
                }
                case '=':
                case '+':
                {
                    ofLogVerbose() << "increaseVolume";
                    omxPlayer->increaseVolume();
                    break;
                }
                case 'v':
                {
                    ofLogVerbose() << "stepFrameForward";
                    omxPlayer->stepFrameForward();
                    break;
                }
                case 'V':
                {
                    ofLogVerbose() << "stepNumFrames 5";
                    omxPlayer->stepNumFrames(5);
                    break;
                }
                case 's':
                {
                    doSeek = true;
                    break;
                }
                case 'r':
                {
                    omxPlayer->restartMovie();
                    break;
                }
                case '>':
                case '.':
                {
                    omxPlayer->fastForward();
                    break;
                }    
                case '<':
                case ',':
                {
                    omxPlayer->rewind();
                    break;
                }     
                case 'q':
                {
                    omxPlayer->close();
                    break;
                }
                case 'o':
                {
                    doReopen = true;
                    break;
                }
                case 'n':
                {
                    doLoadNext = true;
                    break;
                }
                default:
                {
                    break;
                }    
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

