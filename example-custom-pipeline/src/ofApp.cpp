#include "ofApp.h"


int currentFilterIndex = 0;
//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
    consoleListener.setup(this);
    
    imageFilterNames.push_back("None");
    imageFilterNames.push_back("Cartoon");
    imageFilterNames.push_back("Negative");
    imageFilterNames.push_back("Watercolor");
    //imageFilterNames.push_back("Sketch");
    imageFilterNames.push_back("OilPaint");
    imageFilterNames.push_back("Pastel");
    imageFilterNames.push_back("Sharpen");
    imageFilterNames.push_back("Blur");
    //imageFilterNames.push_back("Saturation");
    
    imageFilterNames.push_back("ColourSwap");
    imageFilterNames.push_back("WashedOut");
    imageFilterNames.push_back("ColourPoint");
    imageFilterNames.push_back("Posterise");
    //imageFilterNames.push_back("ColourBalance");
    
#if 0    
    imageFilterNames.push_back("Gpen");
    imageFilterNames.push_back("Hatch");
    imageFilterNames.push_back("Noise");

    imageFilterNames.push_back("Emboss");
    imageFilterNames.push_back("Solarize");
    imageFilterNames.push_back("Film");
    imageFilterNames.push_back("Antialias");
    imageFilterNames.push_back("DeInterlaceLineDouble");
    imageFilterNames.push_back("DeInterlaceAdvanced");
    imageFilterNames.push_back("DeRing");
#endif
	string videoPath = ofToDataPath("../../../video/Timecoded_Big_bunny_1.mov", true);

	//Somewhat like ofFboSettings we may have a lot of options so this is the current model
	ofxOMXPlayerSettings settings;
	settings.videoPath = videoPath;
	settings.useHDMIForAudio = true;	//default true
    settings.enableTexture = true;		//default true
	settings.enableLooping = true;		//default true
	settings.enableAudio = true;		//default true, save resources by disabling
    settings.enableFilters = true;
    settings.filter  = GetImageFilter(imageFilterNames[currentFilterIndex]);
	
	//so either pass in the settings
	omxPlayer.setup(settings);
	omxPlayerRecorder.setup(&omxPlayer);
}


bool doFilterChange = false;
//--------------------------------------------------------------
void ofApp::update()
{
    if(doFilterChange)
    {
        if(ofGetFrameNum() % 120 == 0)
        {
            if(currentFilterIndex+1 < imageFilterNames.size())
            {
                currentFilterIndex++;
            }else
            {
                currentFilterIndex = 0;
            }
            omxPlayer.setFilter(GetImageFilter(imageFilterNames[currentFilterIndex]));
        }
        
    }
}


//--------------------------------------------------------------
void ofApp::draw(){
	if(!omxPlayer.isTextureEnabled())
	{
		return;
	}
	
	omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
	
	//draw a smaller version in the lower right
	int scaledHeight	= omxPlayer.getHeight()/4;
	int scaledWidth		= omxPlayer.getWidth()/4;
	omxPlayer.draw(ofGetWidth()-scaledWidth, ofGetHeight()-scaledHeight, scaledWidth, scaledHeight);
    stringstream info;
   

    info << omxPlayer.getInfo() << endl;
    info << endl;
    
    info << "PRESS 1 TO START RECORDING" << endl;
    info << "PRESS 2 TO STOP RECORDING" << endl;
    info << "CURRENTLY RECORDING: " << omxPlayerRecorder.isRecording << endl;
    info << "RECORDED FRAMES: " << omxPlayerRecorder.recordedFrameCounter << endl;
    
    
    ofDrawBitmapStringHighlight(info.str(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
}


void ofApp::keyPressed  (int key)
{
    ofLogVerbose(__func__) << "key: " << key;
    
    switch (key) 
    {
        case '1':
        {
            omxPlayerRecorder.startRecording(2.0);
            break;
        }
        case '2':
        {
            omxPlayerRecorder.stopRecording();
            break;
        }
        case '3' :
        {
            doFilterChange = !doFilterChange;


        }
        default:
            break;
    }
}
