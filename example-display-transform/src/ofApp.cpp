#include "ofApp.h"


void ofApp::onCharacterReceived(KeyListenerEventData& e)
{
	keyPressed((int)e.character);
}


bool doCrop = false;
bool doRotation = false;
bool doAlpha = false;
bool doMirror = false;
bool doReset = false;

ofRectangle cropRect;
vector<ofRectangle> grids;
int gridCounter = 0;
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_VERBOSE);
    
	consoleListener.setup(this);
	ofHideCursor();
		
	string videoPath = ofToDataPath("../../../video/Timecoded_Big_bunny_1.mov", true);
	
    settings.videoPath = videoPath;
    settings.useHDMIForAudio = true;	//default true
    settings.enableLooping = true;		//default true
    settings.enableTexture = false;		//default true
    omxPlayer.setup(settings);
    
    int step=200;
    for(int i=0; i<omxPlayer.getWidth(); i+=step)
    {
        for(int j=0; j<omxPlayer.getHeight(); j+=step)
        {
            ofRectangle grid(i, j, step, step);
            grids.push_back(grid);
        }
        
        
    }
	
}

bool forceFill = false;
bool doDisplayInCropArea = true;
void ofApp::update()
{
    if (omxPlayer.isTextureEnabled()) 
    {
        ofLogError() << "this example only works correctly in direct mode";
        return;
    }
    
    if (doReset) 
    {
        doReset = false;
        doCrop = false;
        doAlpha = false;
        doMirror = false;
        forceFill = false;
        doRotation = false;
        doDisplayInCropArea = true;
        omxPlayer.draw(0, 0, omxPlayer.getWidth(), omxPlayer.getHeight());
        omxPlayer.cropVideo(0, 0, omxPlayer.getWidth(), omxPlayer.getHeight()); 
        omxPlayer.setAlpha(255);
        omxPlayer.rotateVideo(0);
        omxPlayer.setMirror(false);
        return;
    }
    
    if(doCrop)
    {

        omxPlayer.cropVideo(grids[gridCounter]);
                
        if (doDisplayInCropArea) 
        {
            //display where the cropping was done
            omxPlayer.draw(grids[gridCounter]);
        }else
        {
            if(!forceFill)
            {
                //display at top left
                omxPlayer.draw(0, 0, grids[gridCounter].width, grids[gridCounter].height); 
            }else
            {
                //fill whole screen
                omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight()); 
            }
            
        }
        
        if(gridCounter+1<grids.size())
        {
            gridCounter++;
        }else
        {
            gridCounter = 0;
        }
    }
    if(doAlpha)
    {
        //range is 0-255
        int alpha = ofGetFrameNum() % 255;
        omxPlayer.setAlpha(alpha);
        
    }
    if(doMirror)
    {
        omxPlayer.setMirror(true);
    }else
    {
        omxPlayer.setMirror(false);
    }
    
    //mirroring works with rotation as well
    //rotation is actually in 45 degree increments but method adjusts from 0-360
    if (doRotation) 
    {
        int rotation = ofGetFrameNum() % 360;
        omxPlayer.rotateVideo(rotation);
    }    
}

bool hasPrintedInfo = false;
void ofApp::draw()
{
    
    
    
    ofPushStyle();
    ofNoFill();
    ofSetColor(ofColor::black);
    for(int i=0; i<grids.size(); i++)
    {   
        ofDrawRectangle(grids[i]);
    }
    ofPopStyle();
    
    stringstream info;
    info << "PRESS x TO RESET" << endl;
    info << "PRESS c TO CROP" << endl;
    info << "PRESS C TO DISPLAY IN CROP AREA" << endl;
    info << "PRESS f TO FORCE FILL" << endl;
    info << "PRESS a FOR ALPHA" << endl;
    info << "PRESS r TO TOGGLE ROTATION" << endl;
    info << "PRESS m TO TOGGLE MIRROR " << endl;
    if(omxPlayer.isTextureEnabled())
    {
        info.str("");
        info << "this example only works correctly in direct mode" << endl;
        info << omxPlayer.getInfo() << endl;
    }else
    {
        if(!hasPrintedInfo)
        {
            info << omxPlayer.getInfo() << endl;
            ofLogVerbose() << info.str();
            hasPrintedInfo = true;
        } 
    }
    ofDrawBitmapStringHighlight(info.str(), 60, 20, ofColor::black, ofColor::yellow);

    
}


void ofApp::keyPressed  (int key)
{
	 
	ofLog(OF_LOG_VERBOSE, "%c keyPressed", key);
	switch (key) 
	{
		
		case 'c':
        {
            doCrop = !doCrop;
            break;
        }
        case 'C':
        {
            doDisplayInCropArea = !doDisplayInCropArea;
            break;
        }
        case 'f':
        {
            forceFill = !forceFill;
            break;
        }
        case 'a':
        {
            doAlpha = !doAlpha;
            break;
        }
        case 'r':
        {
            doRotation = !doRotation;
            break;
        }
        case 'm':
        {
            doMirror = !doMirror;
            break;
        }
        case 'x':
        {
            doReset = true;
            break;
        }
		default:
		{
			break;
		}	
	}
	
}
