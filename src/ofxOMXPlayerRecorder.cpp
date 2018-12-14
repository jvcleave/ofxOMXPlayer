#include "ofxOMXPlayerRecorder.h"



OMX_ERRORTYPE ofxOMXPlayerRecorder::encoderEventHandlerCallback(OMX_HANDLETYPE encoder,
                                                                OMX_PTR omxPlayerRecorder_,
                                                                OMX_EVENTTYPE event,
                                                                OMX_U32 nData1,
                                                                OMX_U32 nData2,
                                                                OMX_PTR pEventData)
{
    /*
    ofLog() << "ENCODER: " << DebugEventHandlerString(encoder, event, nData1, nData2, pEventData); 
    if(event == OMX_EventBufferFlag)
    {
        ofxOMXPlayerRecorder* omxPlayerRecorder = static_cast<ofxOMXPlayerRecorder*>(omxPlayerRecorder_);
    }*/
    return OMX_ErrorNone;
}


OMX_ERRORTYPE ofxOMXPlayerRecorder::encoderFillBufferDone(OMX_HANDLETYPE encoder,
                                                        OMX_PTR omxPlayerRecorder_,
                                                        OMX_BUFFERHEADERTYPE* encoderOutputBuffer)
{    
    ofxOMXPlayerRecorder* omxPlayerRecorder = static_cast<ofxOMXPlayerRecorder*>(omxPlayerRecorder_);

    
    bool isKeyframeValid = false;
    omxPlayerRecorder->recordedFrameCounter++;
    /*
     The user wants to quit, but don't exit
     the loop until we are certain that we have processed
     a full frame till end of the frame, i.e. we're at the end
     of the current key frame if processing one or until
     the next key frame is detected. This way we should always
     avoid corruption of the last encoded at the expense of
     small delay in exiting.
     */
    if(omxPlayerRecorder->stopRequested && !omxPlayerRecorder->isStopping) 
    {
        ofLogVerbose(__func__) << "Exit signal detected, waiting for next key frame boundry before exiting...";
        omxPlayerRecorder->isStopping = true;
    }
    
    
    isKeyframeValid = encoderOutputBuffer->nFlags & OMX_BUFFERFLAG_SYNCFRAME;

    if((omxPlayerRecorder->isStopping || omxPlayerRecorder->pauseRequested) &&
       (isKeyframeValid ^ (encoderOutputBuffer->nFlags & OMX_BUFFERFLAG_SYNCFRAME))) 
    {
        ofLogVerbose(__func__) << "Key frame boundry reached, exiting loop...";

        if(omxPlayerRecorder->pauseRequested)
        {
            omxPlayerRecorder->isRecordingPaused = true;
            omxPlayerRecorder->pauseRequested = false;
            OMX_ERRORTYPE error = FlushOMXComponent(encoder, VIDEO_ENCODE_INPUT_PORT);
            OMX_TRACE(error);
            
            error = FlushOMXComponent(encoder, VIDEO_ENCODE_OUTPUT_PORT);
            OMX_TRACE(error);
            
            error = FlushOMXComponent(encoder, VIDEO_ENCODE_OUTPUT_PORT);
            OMX_TRACE(error);
            
            
            //Set encoder to Idle
            error = WaitForState(encoder, OMX_StateIdle);
            OMX_TRACE(error);
            
            
            
        }else
        {
            omxPlayerRecorder->writeFile();
        }
        
    }else 
    {
        omxPlayerRecorder->recordingFileBuffer.append((const char*) encoderOutputBuffer->pBuffer + encoderOutputBuffer->nOffset, encoderOutputBuffer->nFilledLen);
        //ofLogVerbose(__func__) << "encoderOutputBuffer->nFilledLen: " << encoderOutputBuffer->nFilledLen;
        ofLog() << omxPlayerRecorder->recordingFileBuffer.size();
        
        if(!omxPlayerRecorder->isRecordingPaused)
        {
            OMX_ERRORTYPE error = OMX_FillThisBuffer(encoder, encoderOutputBuffer);
            if(error != OMX_ErrorNone) 
            {
                ofLog(OF_LOG_ERROR, "encoder OMX_FillThisBuffer FAIL error: 0x%08x", error);
                if(!omxPlayerRecorder->didWriteFile)
                {
                    ofLogError() << "HAD ERROR FILLING BUFFER, JUST WRITING WHAT WE HAVE";
                    omxPlayerRecorder->writeFile();
                    
                }
            }
        }
        
    }
    return OMX_ErrorNone;
}



ofxOMXPlayerRecorder::ofxOMXPlayerRecorder()
{
    omxPlayer = NULL;
    isOpen = false;
    
    stopRequested = false;     
    isStopping = false;
    isRecording = false;
    didWriteFile = false;
    recordedFrameCounter = 0;
    encoder = NULL;
    splitter = NULL;
    encoderOutputBuffer = NULL;
    recordingRateMB = 2.0;
    isRecordingPaused = false;
    pauseRequested = false;
}

void ofxOMXPlayerRecorder::setup(ofxOMXPlayer* omxPlayer_)
{
    omxPlayer = omxPlayer_;
    isOpen = true;
}

void ofxOMXPlayerRecorder::startRecording(float recordingRateMB_) //default =2.0
{
    if(isRecording)
    {
        ofLogError(__func__) << "ALREADY RECORDING";
        return;
    }
    if(!omxPlayer) return;
    
    recordingRateMB = recordingRateMB_;
    
    if(omxPlayer->getVideoSplitter() != NULL)
    {
        splitter = omxPlayer->getVideoSplitter()->GetComponent();
        if(splitter == NULL)
        {
            ofLogError(__func__) << "SPLITTER HANDLE IS NULL";
            return;
        }
        
        printf("splitter %p", splitter);
    }
    
    OMX_ERRORTYPE error = OMX_ErrorNone;
    isRecording = true;
    
    createEncoder();
    
    error = SetComponentState(splitter, OMX_StateExecuting);
    OMX_TRACE(error);
    
    //Start encoder
    error = SetComponentState(encoder, OMX_StateExecuting);
    OMX_TRACE(error);
    
    error = OMX_FillThisBuffer(encoder, encoderOutputBuffer);
    OMX_TRACE(error);
    if(error !=OMX_ErrorNone)
    {
        ofLogError(__func__) << "RECORDING START FAILED";
        isRecording = false;
        destroyEncoder();
    }
}

void ofxOMXPlayerRecorder::pauseRecording()
{
    if(isRecording)
    {
       pauseRequested = true; 
    }
}

void ofxOMXPlayerRecorder::resumeRecording()
{
    if(!isRecording)
    {
        ofLogError() << "WASN'T RECORDING";
        return;
    }
    
    if(!isRecordingPaused)
    {
        ofLogError() << "WASN'T PAUSED";
        return;
    }
    
    isRecordingPaused = false;
    OMX_ERRORTYPE error;
    
    error = WaitForState(encoder, OMX_StateExecuting);
    OMX_TRACE(error);
    
    
    error = OMX_FillThisBuffer(encoder, encoderOutputBuffer);
    OMX_TRACE(error);
    if(error !=OMX_ErrorNone)
    {
        ofLogError(__func__) << "RECORDING START FAILED";
        isRecording = false;
        destroyEncoder();
    }
}
void ofxOMXPlayerRecorder::createEncoder()
{
#pragma mark ENCODER SETUP  
    
    OMX_ERRORTYPE error;
    
    OMX_CALLBACKTYPE encoderCallbacks;
    encoderCallbacks.EventHandler       = &ofxOMXPlayerRecorder::encoderEventHandlerCallback;
    encoderCallbacks.EmptyBufferDone    = &ofxOMXPlayerRecorder::nullEmptyBufferDone;
    encoderCallbacks.FillBufferDone     = &ofxOMXPlayerRecorder::encoderFillBufferDone;
    
    error =OMX_GetHandle(&encoder, OMX_VIDEO_ENCODER, this , &encoderCallbacks);
    OMX_TRACE(error);
    
    error = DisableAllPortsForComponent(&encoder);
    OMX_TRACE(error);
    
    
    OMX_PARAM_PORTDEFINITIONTYPE encoderOutputPortDefinition;
    OMX_INIT_STRUCTURE(encoderOutputPortDefinition);
    encoderOutputPortDefinition.nPortIndex = VIDEO_ENCODE_OUTPUT_PORT;
    error =OMX_GetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPortDefinition);
    OMX_TRACE(error);
    
    ofLogNotice(__func__) << "VIDEO_ENCODE_OUTPUT_PORT: " << PrintPortDefinition(encoder, VIDEO_ENCODE_OUTPUT_PORT);
    ofLogNotice(__func__) << "VIDEO_ENCODE_INPUT_PORT: " << PrintPortDefinition(encoder, VIDEO_ENCODE_INPUT_PORT);

    int recordingBitRate = MEGABYTE_IN_BITS * 2.0;
    
    encoderOutputPortDefinition.format.video.nBitrate = recordingBitRate;
    error = OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPortDefinition);
    OMX_TRACE(error);
    
    // Configure encoding bitrate
    OMX_VIDEO_PARAM_BITRATETYPE encodingBitrate;
    OMX_INIT_STRUCTURE(encodingBitrate);
    encodingBitrate.eControlRate = OMX_Video_ControlRateVariable;
    //encodingBitrate.eControlRate = OMX_Video_ControlRateConstant;
    
    encodingBitrate.nTargetBitrate = recordingBitRate;
    encodingBitrate.nPortIndex = VIDEO_ENCODE_OUTPUT_PORT;
    
    error = OMX_SetParameter(encoder, OMX_IndexParamVideoBitrate, &encodingBitrate);
    OMX_TRACE(error);
    
    // Configure encoding format
    OMX_VIDEO_PARAM_PORTFORMATTYPE encodingFormat;
    OMX_INIT_STRUCTURE(encodingFormat);
    encodingFormat.nPortIndex = VIDEO_ENCODE_OUTPUT_PORT;
    encodingFormat.eCompressionFormat = OMX_VIDEO_CodingAVC;
    
    error = OMX_SetParameter(encoder, OMX_IndexParamVideoPortFormat, &encodingFormat);
    OMX_TRACE(error);
    
    error = OMX_GetParameter(encoder, OMX_IndexParamVideoPortFormat, &encodingFormat);
    OMX_TRACE(error);
    
    //Set encoder to Idle
    error = SetComponentState(encoder, OMX_StateIdle);
    OMX_TRACE(error);
    
    
    //Set splitter to Idle
    error = SetComponentState(splitter, OMX_StateIdle);
    OMX_TRACE(error);
    
    /*
    OMX_PARAM_BRCMDISABLEPROPRIETARYTUNNELSTYPE tunnelConfig;
    OMX_INIT_STRUCTURE(tunnelConfig);
    tunnelConfig.nPortIndex = VIDEO_SPLITTER_OUTPUT_PORT2;
    tunnelConfig.bUseBuffers = OMX_TRUE;
    
    
    error = OMX_SetParameter(splitter, OMX_IndexParamBrcmDisableProprietaryTunnels, &tunnelConfig);
    OMX_TRACE(error);
    
    */
    // Create splitter->encoder Tunnel
    error = OMX_SetupTunnel(splitter, VIDEO_SPLITTER_OUTPUT_PORT2,
                            encoder, VIDEO_ENCODE_INPUT_PORT);
    
    OMX_TRACE(error);
    
    //Enable splitter output2 port
    error = EnableComponentPort(splitter, VIDEO_SPLITTER_OUTPUT_PORT2);
    OMX_TRACE(error);
    
    //Enable encoder input port
    
    error = EnableComponentPort(encoder, VIDEO_ENCODE_INPUT_PORT);
    OMX_TRACE(error);
    
    
    //Enable encoder output port
    error = EnableComponentPort(encoder, VIDEO_ENCODE_OUTPUT_PORT);
    OMX_TRACE(error);
    
#pragma mark ENCODER BUFFERS SETUP  
    
    error =OMX_GetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPortDefinition);
    OMX_TRACE(error);
    
    error =  OMX_AllocateBuffer(encoder, 
                                &encoderOutputBuffer, 
                                VIDEO_ENCODE_OUTPUT_PORT, 
                                NULL, 
                                encoderOutputPortDefinition.nBufferSize);
    
    OMX_TRACE(error);
    
}

void ofxOMXPlayerRecorder::destroyEncoder()
{
    if(!encoder) return;
    OMX_ERRORTYPE error = OMX_ErrorNone;
    
    error = DisableAllPortsForComponent(&encoder);
    OMX_TRACE(error);
    
    error = OMX_SendCommand(encoder, OMX_CommandFlush, OMX_ALL, NULL);
    OMX_TRACE(error);
    
    if(encoderOutputBuffer)
    {
        error = OMX_FreeBuffer(encoder, VIDEO_ENCODE_OUTPUT_PORT, encoderOutputBuffer);
        OMX_TRACE(error);
        encoderOutputBuffer = NULL;
    }
    
    
    error = DisableComponentPort(splitter, VIDEO_SPLITTER_OUTPUT_PORT2);
    OMX_TRACE(error);
    
    error = OMX_SetupTunnel(encoder, VIDEO_ENCODE_INPUT_PORT,
                            NULL, 0);
    OMX_TRACE(error);
    
    error = OMX_FreeHandle(encoder);
    OMX_TRACE(error);
    encoder = NULL;
}


void ofxOMXPlayerRecorder::stopRecording()
{
    stopRequested = true;
}


void ofxOMXPlayerRecorder::writeFile()
{
    OMX_ERRORTYPE error = OMX_ErrorNone;
    
    //stop encoder
    error = SetComponentState(encoder, OMX_StateIdle);
    OMX_TRACE(error);
    
    
    //format is raw H264 NAL Units
    ofLogVerbose(__func__) << "START";
    
    string filePath = ofToDataPath(ofGetTimestampString()+".h264", true);
    
    didWriteFile = ofBufferToFile(filePath, recordingFileBuffer, true);
    
    if(didWriteFile)
    {
        ofLogNotice(__func__) << "WROTE filePath: " << filePath;
    }else
    {
        ofLogError(__func__) << "FAILED TO WRITE TO filePath: " << filePath;

    }
    recordingFileBuffer.clear();
    
    error = WaitForState(splitter, OMX_StateIdle);
    OMX_TRACE(error);
    
    destroyEncoder();
    
    error = WaitForState(splitter, OMX_StateExecuting);
    OMX_TRACE(error);
    
    isRecording = false;
    recordedFrameCounter = 0;
    stopRequested = false;
    isStopping = false;
}

ofxOMXPlayerRecorder::~ofxOMXPlayerRecorder()
{
    omxPlayer = NULL;
    isOpen = false;
    stopRequested = false;     
    isStopping = false;
    isRecording = false;
    didWriteFile = false;
    recordedFrameCounter = 0;
    destroyEncoder();
    encoder = NULL;
    splitter = NULL;
    encoderOutputBuffer = NULL;
    
}
