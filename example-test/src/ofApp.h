#pragma once

#include "ofMain.h"
#include "TexturedLoopTest.h"
#include "TexturedStreamTest.h"
#include "DirectLoopTest.h"

#include "TerminalListener.h"
#include "PlaybackTestRunner.h"

class ofApp : public ofBaseApp, public KeyListener, public TestListener
{
    
    
public:
    
    
    vector<BaseTest*> tests;
    BaseTest* currentTest;
    TerminalListener consoleListener;
    bool doDeleteCurrent;
    bool doNextTest;
    int currentTestID;
    PlaybackTestRunner playbackTestRunner;
    
    void setup()
    {
        
        doDeleteCurrent = false;
        doNextTest = false;
        
        consoleListener.setup(this);
        
        TexturedLoopTest* texturedLoopTest = new TexturedLoopTest();
        texturedLoopTest->setup("TexturedLoopTest");
        
        TexturedStreamTest* texturedStreamTest = new TexturedStreamTest();
        texturedStreamTest->setup("TexturedStreamTest");
        
        DirectLoopTest* directLoopTest = new DirectLoopTest();
        directLoopTest->setup("DirectLoopTest");
        

        
        tests.push_back(texturedLoopTest);
        tests.push_back(directLoopTest);
        
        tests.push_back(texturedStreamTest);


        
        
        currentTestID = 0;
        currentTest = tests[currentTestID];
        currentTest->listener = this;
        currentTest->start();
        playbackTestRunner.test = currentTest;
        
    }
    
    
    void update()
    {
        if(doDeleteCurrent)
        {
            if(currentTest->isOpen)
            {
                currentTest->close(); 
            }
            doDeleteCurrent = false;
            doNextTest = true;
            playbackTestRunner.stopAll();
        }
        
        if(doNextTest)
        {
            doNextTest = false;
            if(currentTestID+1 < tests.size())
            {
                currentTestID++;
            }else
            {
                currentTestID=0;
            }
            currentTest = tests[currentTestID];
            currentTest->listener = this;
            ofLogVerbose(__func__) << "STARTING NEW TEST: " << currentTest->name;
            
            currentTest->start();
            
        }
        
        if(currentTest)
        {
            currentTest->update();
        }
        
        playbackTestRunner.update();
    }
    
    
    
    
    void draw()
    {
        if(currentTest)
        {
            currentTest->draw();
        }
        stringstream info;
        info << "PRESS 1 for PAUSE TEST" << endl;
        info << "PRESS 2 for RESTART TEST" << endl;
        info << "PRESS 3 for STEP TEST" << endl;
        info << "PRESS 4 for SCRUB TEST" << endl;
        info << "PRESS 5 for SEEK TEST" << endl;
        info << "PRESS 6 for SEEK TO FRAME TEST" << endl;
        info << "PRESS 7 for VOLUME TEST" << endl;
        info << "PRESS 8 for INCREASE SPEED TEST" << endl;
        info << "PRESS 9 for DECREASE SPEED TEST" << endl;
        info << "PRESS f for FILTER TEST" << endl;

        ofDrawBitmapStringHighlight(info.str(), 600, 60, ofColor(ofColor::black, 90), ofColor::yellow);


    }
    
    void onTestComplete(BaseTest* test)
    {
        ofLogVerbose(__func__) << test->name << " COMPLETE";
        ofLogVerbose(__func__) << "CLOSING TEST: " << test->name;

        test->close();

        doDeleteCurrent = true;
        
    }
    void onCharacterReceived(KeyListenerEventData& e)
    {
        keyPressed((int)e.character);
    }
    
    void keyPressed  (int key)
    {
        
        ofLogVerbose(__func__) << "key: " << key;
        switch (key) 
        {
            case '1':
            {
                playbackTestRunner.startPauseTest(currentTest);
                break;
            }
            case '2':
            {
                playbackTestRunner.startRestartTest(currentTest);
                break;
            }
            case '3':
            {
                playbackTestRunner.startStepTest(currentTest);
                break;
            } 
            case '4':
            {
                playbackTestRunner.startScrubTest(currentTest);
                break;
            }
            case '5':
            {
                playbackTestRunner.startSeekTest(currentTest);
                break;
            }    
              
                
            case '6':
            {
                playbackTestRunner.startSeekToFrameTest(currentTest);
                break;
            }
            
            case '7':
            {
                playbackTestRunner.startVolumeTest(currentTest);
                break;
            }
            case '8':
            {
                playbackTestRunner.startIncreaseSpeedTest(currentTest);
                break;
            } 
            case '9':
            {
                playbackTestRunner.startDecreaseSpeedTest(currentTest);
                break;
            }
            case 'f':
            {
                playbackTestRunner.startFilterTest(currentTest);
                break;
            }
            case 'n':
            {
                doDeleteCurrent = true;
                
                break;
            }
            case 'a' :
            {
                //currentTest->omxPlayer->engine->enableAdjustments();
                break;
            }
                
        }
        if(currentTest)
        {
            currentTest->onKeyPressed(key);
        }
        
    }
};

