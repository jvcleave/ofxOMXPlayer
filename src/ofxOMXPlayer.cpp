/*
 *  ofxOMXPlayer.cpp
 *
 *  Created by jason van cleave on 9/8/13.
 *
 */

#include "ofxOMXPlayer.h"

ofxOMXPlayer::ofxOMXPlayer()
{
    
    OMXInitializer::getInstance().init();
    signals.push_back(SIGINT);
    signals.push_back(SIGQUIT);
    
    engine = NULL;
    isOpen = false;
    textureEnabled = false;

    textureID = 0;
    videoWidth =0;
    videoHeight = 0;
    appEGLWindow = NULL;
    eglImage = NULL;
    context = NULL;
    display = NULL;
    pixels = NULL;
    directDisplay = NULL;
    
    hasNewFrame = false;
    prevFrame = 0;
    doRestart = false;
    doToggle = false;
    didSeek = false;
    speedMultiplier = 1;
    didWarnAboutInaccurateCurrentFrame =false;
    decoderHandle = NULL;
    ofAddListener(ofEvents().update, this, &ofxOMXPlayer::onUpdate);
    
}


#pragma mark setup
void ofxOMXPlayer::generateEGLImage(int videoWidth_, int videoHeight_)
{
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
        return;
    }

    if (appEGLWindow == NULL)
    {
        appEGLWindow = (ofAppEGLWindow *) ofGetWindowPtr();
    }

    if (appEGLWindow == NULL)
    {
        ofLogError(__func__) << "appEGLWindow is NULL - RETURNING";
        return;
    }
    if (display == NULL)
    {
        display = appEGLWindow->getEglDisplay();
    }
    if (context == NULL)
    {
        context = appEGLWindow->getEglContext();
    }

    

    if (needsRegeneration)
    {
        
        fbo.allocate(videoWidth, videoHeight, GL_RGBA);
        texture.allocate(videoWidth, videoHeight, GL_RGBA);
        //Video renders upside down and backwards when Broadcom proprietary tunnels are enabled
        //may be resolved in future firmare
        //https://github.com/raspberrypi/firmware/issues/176

        if (settings.doFlipTexture)
        {
            texture.getTextureData().bFlipTexture = true;
        }
        texture.setTextureWrap(GL_REPEAT, GL_REPEAT);
        textureID = texture.getTextureData().textureID;
    }


    //ofLogVerbose(__func__) << "textureID: " << textureID;
    //ofLogVerbose(__func__) << "tex.isAllocated(): " << texture.isAllocated();

    glEnable(GL_TEXTURE_2D);

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

    //memset(pixels, 0xff, dataSize);  // white texture, opaque

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, videoWidth, videoHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);


    if (eglImage && needsRegeneration)
    {
        destroyEGLImage();
    }

    // Create EGL Image
    eglImage = eglCreateImageKHR(
                   display,
                   context,
                   EGL_GL_TEXTURE_2D_KHR,
                   (EGLClientBuffer)textureID,
                   NULL);
    glDisable(GL_TEXTURE_2D);
    if (eglImage == EGL_NO_IMAGE_KHR)
    {
        ofLogError()    << "Create EGLImage FAIL <---------------- :(";
    }
    else
    {
        ofLogVerbose()  << "Create EGLImage PASS <---------------- :)";

    }
}


bool ofxOMXPlayer::setup(ofxOMXPlayerSettings settings)
{
    this->settings = settings;
    addExitHandler();
    return openEngine();
}

void ofxOMXPlayer::toggleMode()
{
    doToggle = true;
    doRestart = true;
}

bool ofxOMXPlayer::openEngine(int startTimeInSeconds) //default 0
{
    if (engine)
    {
        delete engine;
        engine = NULL;
    }
    
    engine = new ofxOMXPlayerEngine();
    bool setupPassed = engine->setup(settings);
    
    if (setupPassed)
    {
        settings = engine->omxPlayerSettings;

        if (settings.enableTexture)
        {
            textureEnabled = settings.enableTexture;
            generateEGLImage(settings.videoWidth, settings.videoHeight);
            engine->eglImage = eglImage;
        }
        else
        {
            videoWidth  = settings.videoWidth;
            videoHeight = settings.videoHeight;
        }

        engine->openPlayer(startTimeInSeconds);
    }
    else
    {
        ofLogError(__func__) << "engine->setup FAIL";
    }
    if(engine->directPlayer && engine->directPlayer->directDecoder)
    {
        directDisplay = engine->directPlayer->directDecoder->getOMXDisplay();
        
        cropRectangle = &directDisplay->options.cropRectangle;
        drawRectangle = &directDisplay->options.drawRectangle;
        
    }
    if(settings.enableFilters)
    {
        
        decoderHandle = engine->videoPlayer->decoder->decoderComponent.getHandle();
    }
    isOpen = setupPassed;
    return setupPassed;

}

#pragma mark getters

ofxOMXPlayerSettings ofxOMXPlayer::getSettings()
{
    if(engine)
    {
        return engine->omxPlayerSettings;
    }
    return settings;
}

bool ofxOMXPlayer::getIsOpen()
{
    return isOpen;
}

bool ofxOMXPlayer::isPaused()
{
    if (engine)
    {
        return engine->isPaused();
    }
    return false;
}

bool ofxOMXPlayer::isPlaying()
{
    if (engine)
    {
        return engine->isPlaying();
    }
    return false;
}

bool ofxOMXPlayer::isFrameNew()
{
    return hasNewFrame;
}

bool ofxOMXPlayer::isTextureEnabled()
{
    return textureEnabled;
}

int ofxOMXPlayer::getHeight()
{
    return videoHeight;
}

int ofxOMXPlayer::getWidth()
{
    return videoWidth;
}

double ofxOMXPlayer::getMediaTime()
{
    if (engine)
    {
        return engine->getMediaTime();
    }
    return 0;
}

float ofxOMXPlayer::getFPS()
{
    if(engine)
    {
        return engine->getFPS();
        
    }
    return 0;
}

float ofxOMXPlayer::getDurationInSeconds()
{
    if (engine)
    {
        return engine->getDurationInSeconds();
    }
    return 0;
}

int ofxOMXPlayer::getCurrentFrame()
{
    if (engine)
    {
        if(didSeek && !didWarnAboutInaccurateCurrentFrame)
        {
            ofLogWarning(__func__) << "UNLESS YOU HAVE A 1:1 KEYFRAME:FRAME RATIO CURRENT FRAME NUMBER WILL LIKELY BE INACCURATE AFTER SEEKING";
            
            didWarnAboutInaccurateCurrentFrame = true;
        }
        return engine->getCurrentFrame();
    }
    return 0;
}

int ofxOMXPlayer::getTotalNumFrames()
{
    if (engine)
    {
        return engine->getTotalNumFrames();
    }
    return 0;
}


StreamInfo& ofxOMXPlayer::getVideoStreamInfo()
{
    
    if (engine)
    {
        return engine->videoStreamInfo;
    }
   
    ofLogError(__func__) << "No engine avail - info returned is invalid";
    return videoInfo;
}



ofTexture& ofxOMXPlayer::getTextureReference()
{
    if (!fbo.isAllocated())
    {
        ofLogError() << "NO FBO!";
    }
    return fbo.getTextureReference();
}

GLuint ofxOMXPlayer::getTextureID()
{
    
    return textureID;
}

string ofxOMXPlayer::getInfo()
{
    stringstream info;
    info <<"\n" <<  "APP FPS: "+ ofToString(ofGetFrameRate());
    info <<"\n" <<  "MEDIA TIME: "          << getMediaTime();
    info <<"\n" <<  "DIMENSIONS: "          << getWidth()<<"x"<<getHeight();
    info <<"\n" <<  "FPS: "                 << getFPS();
    info <<"\n" <<  "DURATION IN SECS: "    << getDurationInSeconds();
    info <<"\n" <<  "TOTAL FRAMES: "        << getTotalNumFrames();
    info <<"\n" <<  "CURRENT FRAME: "       << getCurrentFrame();
    if (getTotalNumFrames() > 0) 
    {
        info <<"\n" <<  "REMAINING FRAMES: "    << getTotalNumFrames() - getCurrentFrame();
    }else 
    {
        info <<"\n" <<  "REMAINING FRAMES: N/A, NO TOTAL FRAMES";
    }
    
    
    info <<"\n" <<  "CURRENT VOLUME: "      << getVolume();
    return info.str();
}

#pragma mark LOOPING

void ofxOMXPlayer::enableLooping()
{
    if (engine)
    {
        engine->enableLooping();
    }
}
void ofxOMXPlayer::disableLooping()
{
    if (engine)
    {
        engine->disableLooping();
    }
}

bool ofxOMXPlayer::isLoopingEnabled()
{
    bool result = false;
    if (engine)
    {
       result =  engine->isLoopingEnabled();
    }
    return result;
}

#pragma mark playback commands

void ofxOMXPlayer::togglePause()
{
    if (engine)
    {
        setPaused(!engine->isPaused());
    }
}

void ofxOMXPlayer::setPaused(bool doPause)
{
    if (engine)
    {
        bool result = engine->setPaused(doPause);
        ofLogVerbose(__func__) << "result: " << result;
    }
}


void ofxOMXPlayer::stepFrameForward()
{
    stepFrame(1);
}

void ofxOMXPlayer::stepFrame(int step)
{
    if (engine)
    {
        engine->stepFrame(step);
    }
}


void ofxOMXPlayer::scrubForward(int step)//default 1
{
    if (engine)
    {
        engine->scrubForward(step);
    }
}

int ofxOMXPlayer::getSpeedMultiplier()
{
    if(engine)
    {
        speedMultiplier = engine->speedMultiplier;
    }
    return speedMultiplier;
}

void ofxOMXPlayer::increaseSpeed()
{
    if(engine)
    {
        speedMultiplier = engine->increaseSpeed();
    }
}

void ofxOMXPlayer::setNormalSpeed()
{
    if(engine)
    {
        engine->setNormalSpeed();
    }
}


void ofxOMXPlayer::rewind()
{
    engine->rewind();
}

void ofxOMXPlayer::loadMovie(string videoPath)
{
    settings.videoPath = videoPath;
    setup(settings);
}

void ofxOMXPlayer::restartMovie()
{
    doRestart = true;
}


void ofxOMXPlayer::seekToTimeInSeconds(int timeInSeconds)
{
    didSeek = true;
    openEngine(timeInSeconds);
}


#pragma mark audio

void ofxOMXPlayer::increaseVolume()
{
    if (engine)
    {
        engine->increaseVolume();
    }
}
void ofxOMXPlayer::decreaseVolume()
{
    if (engine)
    {
        engine->decreaseVolume();
    }
}

void ofxOMXPlayer::setVolume(float volume)
{
    if (engine)
    {
        engine->setVolume(volume);
    }
}

float ofxOMXPlayer::getVolume()
{
    if (engine)
    {
        return engine->getVolume();
    }
    return 0;
}

StreamInfo& ofxOMXPlayer::getAudioStreamInfo()
{
    
    if (engine)
    {
        return engine->audioStreamInfo;
    }
    
    ofLogError(__func__) << "No engine avail - info returned is invalid";
    return audioInfo;
}




#pragma mark convienience methods
void ofxOMXPlayer::saveImage(string imagePath)//default imagePath=""
{
    if(!isTextureEnabled()) return;
    if(imagePath == "")
    {
        imagePath = ofToDataPath(ofGetTimestampString()+".png", true);
    }
    updatePixels();
    //TODO make smarter, re-allocating every time
    ofImage image;
    image.setFromPixels(getPixels(), getWidth(), getHeight(), OF_IMAGE_COLOR_ALPHA);
    image.saveImage(imagePath);
    
    ofLogVerbose() << "SAVED IMAGE TO: " << imagePath;
}

string ofxOMXPlayer::getRandomVideo(string path)
{
    string videoPath = "";
    
    ofDirectory currentVideoDirectory(ofToDataPath(path, true));
    if (currentVideoDirectory.exists())
    {
        currentVideoDirectory.listDir();
        currentVideoDirectory.sort();
        vector<ofFile> files = currentVideoDirectory.getFiles();
        if (files.size()>0)
        {
            if (files.size()==1)
            {
                videoPath = files[0].path();
            }else
            {
                int randomIndex = ofRandom(files.size());
                videoPath = files[randomIndex].path(); 
            }
           
        }
        
    }else
    {
        ofLogError(__func__) << "NO FILES FOUND AT" << currentVideoDirectory.path();
    }
    
    if(videoPath.empty())
    {
        ofLogWarning(__func__) << "returning empty string";
    }
    return videoPath;
}



#pragma mark draw
void ofxOMXPlayer::draw(float x, float y, float width, float height)
{
    if (!engine) return;
    
    if (fbo.isAllocated())
    {
        fbo.draw(x, y, width, height);
    }else
    {
        setDisplayRect(x, y, width, height);
    }
    
}

void ofxOMXPlayer::draw(float x, float y)
{
    draw(x, y, getWidth(), getHeight());
    
}

void ofxOMXPlayer::draw(ofRectangle& drawRect)
{
    draw(drawRect.x, drawRect.y, drawRect.getWidth(), drawRect.getHeight());
    
}

#pragma mark direct only options

void ofxOMXPlayer::cropVideo(ofRectangle& cropRectangle_)
{
    if(*cropRectangle != cropRectangle_)
    {        
        *cropRectangle = cropRectangle_;
    }
    
}

void ofxOMXPlayer::cropVideo(float x, float y, float width, float height)
{
    ofRectangle cropTemp(x, y, width, height);
    cropVideo(cropTemp);
}

void ofxOMXPlayer::rotateVideo(int degrees)
{
    if(directDisplay)
    {
        directDisplay->rotateDisplay(degrees);
    }
}

void ofxOMXPlayer::setMirror(bool doMirror)
{
    if(directDisplay)
    {
        directDisplay->options.doMirror = doMirror;
    }
}

void ofxOMXPlayer::setFullScreen(bool doFullScreen)
{
    if(directDisplay)
    {
        directDisplay->options.doFullScreen = doFullScreen;
        
    }
}

void ofxOMXPlayer::setForceFill(bool doForceFill)
{
    if(directDisplay)
    {
        directDisplay->options.doForceFill = doForceFill;
        
    }
}



void ofxOMXPlayer::setAlpha(int alpha)
{
    if(directDisplay)
    {
        if(alpha >=0 && alpha<=255)
        {
            directDisplay->options.alpha = alpha;
        }
        
    }
}


void ofxOMXPlayer::setDisplayRect(ofRectangle& temp)
{
    
    if(temp.getArea() <= 0) return;
    
    if (*drawRectangle != temp) 
    {        
        *drawRectangle = temp;
    }
}

void ofxOMXPlayer::setDisplayRect(float x, float y, float width, float height)
{
    
    ofRectangle temp(x, y, width, height);
    setDisplayRect(temp);
   
}


 void ofxOMXPlayer::applyFilter(OMX_IMAGEFILTERTYPE filter)
{
    if(engine && engine->videoPlayer)
    {
        engine->videoPlayer->applyFilter(filter);
    }
}


#pragma mark update routines


void ofxOMXPlayer::updateFBO()
{
    if (engine && isTextureEnabled() && isFrameNew())
    {
        fbo.begin();
        ofClear(0, 0, 0, 0);
        texture.draw(0, 0, texture.getWidth(), texture.getHeight()); 
        fbo.end();
    }
}

void ofxOMXPlayer::updateCurrentFrame()
{
    if (engine)
    {
        int currentFrame = engine->getCurrentFrame();
        if (prevFrame != currentFrame) 
        {
            hasNewFrame = true;
            prevFrame = currentFrame;
            
        }else 
        {
            hasNewFrame = false;
        }
        
    }
    else 
    {
        hasNewFrame = false;
    }
}




void ofxOMXPlayer::onUpdate(ofEventArgs& args)
{
    if (doRestart) 
    {
        ofxOMXPlayerSettings currentSettings = getSettings();
        if(doToggle)
        {
            currentSettings.enableTexture = !currentSettings.enableTexture;
            doToggle = false;
        }
        setup(currentSettings);
        doRestart = false;
        return; 
       
    }
    updateCurrentFrame();
    updateFBO();
}

#pragma mark pixels
void ofxOMXPlayer::updatePixels()
{
    if (!isTextureEnabled())
    {
        return;
    }
    fbo.begin();
    ofClear(0, 0, 0, 0);
    texture.draw(0, 0);
    //ofLogVerbose() << "updatePixels";
    glReadPixels(0,0,videoWidth, videoHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    fbo.end();
}

unsigned char * ofxOMXPlayer::getPixels()
{
    return pixels;
}

#pragma mark shutdown

void ofxOMXPlayer::destroyEGLImage()
{
    
    if (eglImage)
    {
        if (!eglDestroyImageKHR(display, eglImage))
        {
            ofLogError(__func__) << "eglDestroyImageKHR FAIL <---------------- :(";
        }
        eglImage = NULL;
    }
    
}


struct sigaction old_action;
bool ofxOMXPlayer::doExit = false;

void ofxOMXPlayer::close()
{
    ofLogVerbose(__func__)  << "";
    //ofLogVerbose(__func__) << " isOpen: " << isOpen;
    if (!isOpen)
    {
        return;
    }
    
    if(engine)
    {
        if(engine->directPlayer && engine->directPlayer->directDecoder)
        {
            engine->directPlayer->directDecoder->doUpdate = false;
            engine->directPlayer->directDecoder->display.isReady = false;
        }
    }
    
    ofRemoveListener(ofEvents().update, this, &ofxOMXPlayer::onUpdateDuringExit);
    
    
    if (!ofxOMXPlayer::doExit)
    {
        ofLogVerbose(__func__) << "signals.size() " << signals.size();
        for (size_t i=0; i<signals.size(); i++)
        {
            int SIGNAL_TO_BLOCK = signals[i];
            sigaction(SIGNAL_TO_BLOCK, &old_action, NULL);
            ofLogVerbose(__func__) << " RESTORED SIGNAL";
        }
    }
    
    if(engine)
    {
        delete engine;
        engine = NULL;
    }
    
    destroyEGLImage();
    if (pixels)
    {
        delete[] pixels;
        pixels = NULL;
    }
    drawRectangle = NULL;
    cropRectangle = NULL;
    directDisplay = NULL;
    isOpen = false;
    
}

ofxOMXPlayer::~ofxOMXPlayer()
{
    ofRemoveListener(ofEvents().update, this, &ofxOMXPlayer::onUpdate);

    close();
}


void ofxOMXPlayer::signal_handler(int signum)
{
    ofxOMXPlayer::doExit = true;
}

void ofxOMXPlayer::onUpdateDuringExit(ofEventArgs& args)
{
    if (ofxOMXPlayer::doExit)
    {
        ofLogVerbose(__func__) << " EXITING VIA SIGNAL";
        
        close();
        ofxOMXPlayer::doExit = false;
        if(decoderHandle)
        {
            OMX_ERRORTYPE error = OMX_FreeHandle(decoderHandle);
            OMX_TRACE(error);
            decoderHandle = NULL;
        }
        OMXInitializer::getInstance().deinit();
        ofExit();
    }
}

void ofxOMXPlayer::addExitHandler()
{

    for (size_t i=0; i<signals.size(); i++)
    {
        int SIGNAL_TO_BLOCK = signals[i];
        //http://stackoverflow.com/questions/11465148/using-sigaction-c-cpp
        
        //Struct for the new action associated to the SIGNAL_TO_BLOCK
        struct sigaction new_action;
        new_action.sa_handler = ofxOMXPlayer::signal_handler;
        
        //Empty the sa_mask. This means that no signal is blocked while the signal_handler runs.
        sigemptyset(&new_action.sa_mask);
        
        //Block the SEGTERM signal so while the signal_handler runs, the SIGTERM signal is ignored
        sigaddset(&new_action.sa_mask, SIGTERM);
        
        //Remove any flag from sa_flag. See documentation for flags allowed
        new_action.sa_flags = 0;
        
        
        //Read the old signal associated to SIGNAL_TO_BLOCK
        sigaction(SIGNAL_TO_BLOCK, NULL, &old_action);
        
        //If the old handler wasn't SIG_IGN it is a handler that just "ignores" the signal
        if (old_action.sa_handler != SIG_IGN)
        {
            //Replace the signal handler of SIGNAL_TO_BLOCK with the one described by new_action
            sigaction(SIGNAL_TO_BLOCK, &new_action, NULL);
        }
        
    }
    ofAddListener(ofEvents().update, this, &ofxOMXPlayer::onUpdateDuringExit);
}

