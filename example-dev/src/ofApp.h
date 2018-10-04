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
    
    vector<ImageFilter>imageFilters;

    
    void onVideoEnd(ofxOMXPlayer* player)
    {
        ofLog() << "ofApp::onVideoEnd";
    };
    
    void onVideoLoop(ofxOMXPlayer* player)
    {
        ofLog() << "ofApp::onVideoLoop";

    };

    void setup()
    {
        ofDirectory currentVideoDirectory("/home/pi/videos/current");
        vector<ofFile> files = currentVideoDirectory.getFiles();
        
        for (int i=0; i<files.size(); i++) 
        {
            videoFiles.push_back(files[i].getAbsolutePath());
            
        }

        
        currentFileIndex = 0;
        terminalListener.setup(this);
        ofDirectory videoDirectory(ofToDataPath("../../../video", true));
        if (videoDirectory.exists()) 
        {
            videoDirectory.listDir();
            vector<ofFile> files = videoDirectory.getFiles();
            
            
            for (int i=0; i<files.size(); i++) 
            {
                ofxOMXPlayerSettings settings;
                settings.videoPath = files[i].path();
                settings.useHDMIForAudio = true;    //default true
                settings.enableLooping = true;        //default true
                settings.enableAudio = true;        //default true, save resources by disabling
                settings.enableTexture = i==0;        //default true
                //settings.loopPoint = "0:0:10";
                //settings.loopPoint = "10";
                settings.listener = this;
                
                settings.autoStart = true;

                
                
                
                ofxOMXPlayer* player = new ofxOMXPlayer();
                //player->engine.m_config_video.aspectMode = 3;
                /*player->engine.m_config_video.dst_rect.SetRect(settings.drawRectangle.x,
                                                               settings.drawRectangle.y,
                                                               settings.drawRectangle.x+settings.drawRectangle.width,*/
                    
                imageFilters = player->imageFilters;
                if(!settings.enableTexture)
                {
                    player->engine.m_config_audio.device = "omx:alsa";
                    //omxPlayer.engine.m_config_audio.subdevice = "default";
                    player->engine.m_config_audio.subdevice = "hw:1,0";
                }
                
                player->setup(settings); 
                omxPlayers.push_back(player);
                
                
                //player->engine.m_config_audio.device = "omx:alsa";
                //player->engine.m_config_audio.subdevice = "hw:1,0";

                //
                //
                //player->setLayer(i);
               
            }
        }else{
            ofLogError() << "currentVideoDirectory: " << currentVideoDirectory.path() << " MISSING";
            
            
        }
     
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
                omxPlayers[i]->loadMovie(videoFiles[currentFileIndex]);

            }
        }
        
        

        
    }
    
    int index = 0;
    void onCharacterReceived(KeyListenerEventData& e)
    {
        if(e.character == 'F')
        {
            for (int i=0; i<omxPlayers.size(); i++) 
            {
                omxPlayers[i]->setFilter(OMX_ImageFilterNone); 
            }
        }
        
        if(e.character == 'f')
        {
            ofLog() << "APPLYING FILTER: " << imageFilters[index].name;
            for (int i=0; i<omxPlayers.size(); i++) 
            {
                omxPlayers[i]->setFilter(imageFilters[index].filterType); 
            }
            if(index+1 < imageFilters.size())
            {
                index++;
            }else
            {
                index = 0;
            }
        }else
        {
            keyPressed(e.character);

        }

        
  
    }
    
    int layerCounter = 0;
    void draw()
    {
        
        
        ofBackgroundGradient(ofColor::red, ofColor::black);
        //ofLog() << "omxPlayers.size(): " << omxPlayers.size();
        for (int i=0; i<omxPlayers.size(); i++) 
        {
            
            if(ofGetFrameNum()%60 == 0)
            {
                int degrees = (ofGetFrameNum()%360);
                omxPlayers[i]->rotateVideo(degrees, ofRandom(0, 1)); 
            }
            
            stringstream info;
            float halfWidth = omxPlayers[i]->getWidth()*.5;
            
                ofRectangle drawRect(halfWidth*i,
                                     0,
                                     halfWidth,
                                     omxPlayers[i]->getHeight()*.5);
            
                omxPlayers[i]->draw(drawRect);
                
                
             
            
                info << "drawRect: " << drawRect << endl;
                info << omxPlayers[i]->getInfo() << endl;
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
                info << "PRESS f for RANDOM FILTER" << endl;

                ofDrawBitmapStringHighlight(info.str(), drawRect.x, drawRect.getHeight(), ofColor(ofColor::black, 90), ofColor::yellow);
        }

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
                    omxPlayer->increaseSpeed();
                    break;
                }    
                case '<':
                case ',':
                {
                    omxPlayer->decreaseSpeed();
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

