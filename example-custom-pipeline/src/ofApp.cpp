#include "ofApp.h"



unsigned int to_88(double val)
{
    /* no error-checking on range of 'val' */
    unsigned int ival = (unsigned int) val;
    unsigned int fval = (unsigned int) (100*(val-ival) + 0.5);
    return (ival << 8) + fval;
}


int currentFilterIndex = 0;
//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
    consoleListener.setup(this);
    
    imageFilterNames = OMX_Maps::getInstance().imageFilterNames;
    
#if 0
    imageFilterNames.push_back("Cartoon");
    imageFilterNames.push_back("Negative");
    imageFilterNames.push_back("Watercolor");
    imageFilterNames.push_back("Sketch");
    imageFilterNames.push_back("OilPaint");
    imageFilterNames.push_back("Pastel");
    imageFilterNames.push_back("Sharpen");
    imageFilterNames.push_back("Blur");
    imageFilterNames.push_back("Saturation");
    
    imageFilterNames.push_back("ColourSwap");
    imageFilterNames.push_back("WashedOut");
    imageFilterNames.push_back("ColourPoint");
    imageFilterNames.push_back("Posterise");
    //imageFilterNames.push_back("ColourBalance");
    
   
    imageFilterNames.push_back("Gpen");
    imageFilterNames.push_back("Hatch");
    imageFilterNames.push_back("Noise");

    imageFilterNames.push_back("Emboss");
    imageFilterNames.push_back("Solarize");
    imageFilterNames.push_back("Film");
    imageFilterNames.push_back("None");
    
    imageFilterNames.push_back("DeInterlaceLineDouble");
    imageFilterNames.push_back("DeInterlaceAdvanced");

    imageFilterNames.push_back("DeRing");
    imageFilterNames.push_back("Antialias");

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
    //settings.filter  = GetImageFilter(imageFilterNames[currentFilterIndex]);
	
	//so either pass in the settings
	omxPlayer.setup(settings);
	omxPlayerRecorder.setup(&omxPlayer);
}


bool doFilterChange = false;
bool doCycle = false;
int cycleCounter = 0;
//--------------------------------------------------------------
void ofApp::update()
{
    if(doFilterChange)
    {
        doFilterChange = false;
        if(currentFilterIndex+1 < imageFilterNames.size())
        {
            currentFilterIndex++;
        }else
        {
            currentFilterIndex = 0;
        }
        OMX_IMAGEFILTERTYPE filter = GetImageFilter(imageFilterNames[currentFilterIndex]);
        vector<int> params;
        switch(filter)
        {
                 
            case OMX_ImageFilterColourSwap:
            {
                params.push_back(1); //0-1
                omxPlayer.setFilter(filter, params);
                break;
            }  
            case OMX_ImageFilterBlur:
            {
                params.push_back(2);
                omxPlayer.setFilter(filter, params);
                break;
            }   
            case OMX_ImageFilterSharpen:
            {
                params.push_back(2);
                params.push_back(255);
                params.push_back(255);
                omxPlayer.setFilter(filter, params);
                break;
            }
            case OMX_ImageFilterSaturation:
            {
                int value = to_88(2.5);
                ofLog() << "value: " << value;
                params.push_back(value);
                omxPlayer.setFilter(filter, params);
                break;
            }
            case OMX_ImageFilterFilm:
            {
                params.push_back((int)ofRandom(64, 255));
                params.push_back(128);
                params.push_back(128);
                omxPlayer.setFilter(filter, params);
                break;
            }
            case OMX_ImageFilterSolarize:
            case OMX_ImageFilterColourBalance:
            {
                params.push_back((int)ofRandom(0, 255));
                params.push_back((int)ofRandom(0, 255));
                params.push_back((int)ofRandom(0, 255));
                params.push_back((int)ofRandom(0, 255));
                
                omxPlayer.setFilter(filter, params);
                break;
            }
                
            case OMX_ImageFilterAntialias:
            case OMX_ImageFilterDeRing:
            {
                break;
            } 
            case OMX_ImageFilterPosterise:
            {
                params.push_back(1);//0-30
                omxPlayer.setFilter(filter, params);
                break;

            }
            case OMX_ImageFilterDeInterlaceFast:
            case OMX_ImageFilterDeInterlaceAdvanced:
            {
                
#if 0
                params.push_back(3);
                params.push_back(0); // default frame interval
                params.push_back(0); // half framerate
                params.push_back(1); // use qpus
                
                omxPlayer.setFilter(filter, params);
#endif
                break;
            }
            case OMX_ImageFilterSketch:
            case OMX_ImageFilterWatercolor:
            {
                params.push_back(ofRandom(0, 255)); //u  
                params.push_back(ofRandom(0, 255)); //v
                omxPlayer.setFilter(filter, params);
                break;

            }
            case OMX_ImageFilterAnaglyph:
            {
                /*
                OMX_ImageFilterAnaglyphNone,
                OMX_ImageFilterAnaglyphSBStoRedCyan,
                OMX_ImageFilterAnaglyphSBStoCyanRed,
                OMX_ImageFilterAnaglyphSBStoGreenMagenta,
                OMX_ImageFilterAnaglyphSBStoMagentaGreen,
                OMX_ImageFilterAnaglyphTABtoRedCyan,
                OMX_ImageFilterAnaglyphTABtoCyanRed,
                OMX_ImageFilterAnaglyphTABtoGreenMagenta,
                OMX_ImageFilterAnaglyphTABtoMagentaGreen,*/
                
                /*
                params.push_back((int)ofRandom(0, 9));
                omxPlayer.setFilter(filter, params);
                */
                break;
            }
            default:
            {
                
                params.push_back(1);
                params.push_back(ofRandom(0, 255));   
                params.push_back(ofRandom(0, 255));   
                params.push_back(ofRandom(0, 255)); 
//                omxPlayer.setFilter(filter);
                omxPlayer.setFilter(filter, params);

                break;
            }
        }
        
    }
    
    if(doCycle)
    {
        if(omxPlayer.isFrameNew())
        {
            vector<int> params;
            int value = cycleCounter%255;
            params.push_back(ofRandom(0, 255));
            params.push_back(ofRandom(0, 255));
            //params.push_back(value-ofRandom(0, 255));
            cycleCounter++;
            /*params.push_back(ofRandom(0, 255));   
             params.push_back(ofRandom(0, 255));   
             params.push_back(ofRandom(0, 255)); */
            omxPlayer.setFilter(OMX_ImageFilterNoise, params);
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
            doFilterChange = true;
            break;
        }
        case 'x' :
        {
            if(omxPlayerRecorder.isRecordingPaused)
            {
                omxPlayerRecorder.resumeRecording();
            }else
            {
                omxPlayerRecorder.pauseRecording();
            }
            break;
        }
        case 'a':
        {
            
            static int ColourPointCounter =0;
            vector<int> params;
            params.push_back(ColourPointCounter);
            ColourPointCounter++;            
            
            if(ColourPointCounter >=4)
            {
                ColourPointCounter = 0;
            }
            omxPlayer.setFilter(GetImageFilter("ColourPoint"), params);
            break;

        }
        case 's':
        {
            
            vector<int> params;            
            bool value = ofGetFrameNum()%2 == 0;
            params.push_back(value);
            omxPlayer.setFilter(GetImageFilter("ColourSwap"), params);
            break;

        }
        case 'd':
        {
            
            static int balanceCounter =0;
            vector<int> params;
            params.push_back(1); 
            params.push_back(ofRandom(0, 255));   
            params.push_back(ofRandom(0, 255));   
            params.push_back(ofRandom(0, 255)); 
            balanceCounter++;  
            omxPlayer.setFilter(OMX_ImageFilterColourBalance, params);
            break;
            
        }
        case 'f':
        {
            
            static int waterColorCounter =0;

            vector<int> params;
            params.push_back(ofRandom(0, 255));
            params.push_back(ofRandom(0, 255));
            waterColorCounter++;  
            //OMX_ImageFilterSketch
            omxPlayer.setFilter(OMX_ImageFilterWatercolor, params);
            break;
            
        }
        case '5':
        {
            doCycle = !doCycle;
            cycleCounter = 0;
            break;
        }
        default:
            break;
    }
}
