#include "ofxOMXPlayer.h"

class SplitterController
{
public:
    
    ofxOMXPlayer* omxPlayer;
    
    COMXCoreComponent* splitterComponent;
    OMX_HANDLETYPE splitter;
    OMX_HANDLETYPE encoder;
    
    ofBuffer recordingFileBuffer;
    OMX_BUFFERHEADERTYPE* encoderOutputBuffer;
    
    bool isOpen;
    bool didWriteFile;
    bool stopRequested;
    bool isStopping;
    int recordedFrameCounter;
    bool isRecording;

    
    SplitterController();
    void setup(ofxOMXPlayer* omxPlayer_);

    void startRecording();
    void stopRecording();
    
protected:
    void writeFile();

    void createEncoder();
    void destroyEncoder();
    
    static OMX_ERRORTYPE encoderEventHandlerCallback(OMX_HANDLETYPE camera, OMX_PTR videoModeEngine_, OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData);
    static OMX_ERRORTYPE encoderFillBufferDone(OMX_HANDLETYPE encoder, OMX_PTR engine, OMX_BUFFERHEADERTYPE* encoderOutputBuffer);
    static OMX_ERRORTYPE nullEmptyBufferDone(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*){return OMX_ErrorNone;};

};
