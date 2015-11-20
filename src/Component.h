#pragma once

#include "OMX_Maps.h"
#include "LIBAV_INCLUDES.h"
#include <math.h>
#include <sys/time.h>
#include "XMemUtils.h"


struct OMXEvent
{
    OMX_EVENTTYPE eEvent;
    OMX_U32 nData1;
    OMX_U32 nData2;
};


class Component
{
public:
    Component();
    ~Component();
    
    OMX_HANDLETYPE    getHandle()
    {
        return handle;
    };
    unsigned int      getInputPort()
    {
        return inputPort;
    };
    unsigned int      getOutputPort()
    {
        return outputPort;
    };
    std::string       getName()
    {
        return componentName;
    };
    
    OMX_ERRORTYPE disableAllPorts();
    void          Remove(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2);
    OMX_ERRORTYPE addEvent(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2);
    OMX_ERRORTYPE waitForEvent(OMX_EVENTTYPE event, long timeout = 20);
    OMX_ERRORTYPE waitForCommand(OMX_COMMANDTYPE command, OMX_U32 nData2, long timeout=20);
    OMX_ERRORTYPE setState(OMX_STATETYPE state);
    OMX_STATETYPE getState();
    OMX_ERRORTYPE setParameter(OMX_INDEXTYPE, OMX_PTR);
    OMX_ERRORTYPE getParameter(OMX_INDEXTYPE, OMX_PTR);
    OMX_ERRORTYPE setConfig(OMX_INDEXTYPE, OMX_PTR);
    OMX_ERRORTYPE getConfig(OMX_INDEXTYPE, OMX_PTR);
    OMX_ERRORTYPE sendCommand(OMX_COMMANDTYPE, OMX_U32, OMX_PTR);
    OMX_ERRORTYPE enablePort(unsigned int);
    OMX_ERRORTYPE disablePort(unsigned int);
    OMX_ERRORTYPE useEGLImage(OMX_BUFFERHEADERTYPE**, OMX_U32, OMX_PTR, void*);
    
    bool init(string&, OMX_INDEXTYPE);
    bool Deinitialize(bool doFlush=true);
    
    // Decoder delegate callback routines.
    static OMX_ERRORTYPE DecoderEventHandlerCallback(OMX_HANDLETYPE, OMX_PTR, OMX_EVENTTYPE, OMX_U32, OMX_U32, OMX_PTR);
    static OMX_ERRORTYPE DecoderEmptyBufferDoneCallback(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);
    static OMX_ERRORTYPE DecoderFillBufferDoneCallback(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);
    
    // OMXCore decoder callback routines.
    OMX_ERRORTYPE DecoderEventHandler(OMX_HANDLETYPE, OMX_PTR, OMX_EVENTTYPE, OMX_U32, OMX_U32, OMX_PTR);
    OMX_ERRORTYPE DecoderEmptyBufferDone(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);
    OMX_ERRORTYPE DecoderFillBufferDone(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);
    
    OMX_ERRORTYPE EmptyThisBuffer(OMX_BUFFERHEADERTYPE*);
    OMX_ERRORTYPE FillThisBuffer(OMX_BUFFERHEADERTYPE*);
    OMX_ERRORTYPE FreeOutputBuffer(OMX_BUFFERHEADERTYPE*);
    
    unsigned int getInputBufferSize();
    int m_input_buffer_size;
    
    unsigned int getInputBufferSpace();
    
    void flushAll();
    void flushInput();
    void flushOutput();
    
    OMX_BUFFERHEADERTYPE* getInputBuffer(long timeout=200);
    OMX_BUFFERHEADERTYPE* getOutputBuffer();
    
    OMX_ERRORTYPE allocInputBuffers();
    OMX_ERRORTYPE allocOutputBuffers();
    
    void resetEOS();
    bool EOS()
    {
        return m_eos;
    };
    void setEOS(bool isEndOfStream);
    void SetCustomDecoderFillBufferDoneHandler(OMX_ERRORTYPE (*p)(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*))
    {
        CustomDecoderFillBufferDoneHandler = p;
    };
    void SetCustomDecoderEmptyBufferDoneHandler(OMX_ERRORTYPE (*p)(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*))
    {
        CustomDecoderEmptyBufferDoneHandler = p;
    };
    
    
    int getCurrentFrame();
    void resetFrameCounter();
    void incrementFrameCounter();
    
private:
    OMX_HANDLETYPE handle;
    string componentName;
    unsigned int inputPort;
    unsigned int outputPort;
    
    pthread_mutex_t	m_lock;
    pthread_mutex_t	event_mutex;
    pthread_mutex_t	eos_mutex;
    
    vector<OMXEvent>	omxEvents;
    
    OMX_CALLBACKTYPE  callbacks;
    
    //additional event handlers
    OMX_ERRORTYPE (*CustomDecoderFillBufferDoneHandler)(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);
    OMX_ERRORTYPE (*CustomDecoderEmptyBufferDoneHandler)(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);
    
    // OMXCore input buffers (demuxer packets)
    pthread_mutex_t   m_omx_input_mutex;
    queue<OMX_BUFFERHEADERTYPE*> inputBuffersAvailable;
    vector<OMX_BUFFERHEADERTYPE*> inputBuffers;
    
    // OMXCore output buffers (video frames)
    pthread_mutex_t   m_omx_output_mutex;
    std::queue<OMX_BUFFERHEADERTYPE*> outputBuffersAvailable;
    std::vector<OMX_BUFFERHEADERTYPE*> outputBuffers;
    sem_t         m_omx_fill_buffer_done;
    
    bool          doExit;
    pthread_cond_t    m_input_buffer_cond;
    pthread_cond_t    m_output_buffer_cond;
    pthread_cond_t    m_omx_event_cond;
    bool          m_eos;
    bool          doFlushInput;
    bool          doFlushOutput;
    void              lock();
    void              unlock();
    
    int frameCounter;
    int frameOffset;
    
};