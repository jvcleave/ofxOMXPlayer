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
    
    OMX_HANDLETYPE getHandle()
    {
        return handle;
    };
    unsigned int getInputPort()
    {
        return inputPort;
    };
    unsigned int getOutputPort()
    {
        return outputPort;
    };
    string getName()
    {
        return componentName;
    };
    
    OMX_ERRORTYPE enableAllPorts();
    OMX_ERRORTYPE disableAllPorts();
    void          removeEvent(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2);
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
    bool Deinitialize(string caller="UNDEFINED");
    
    // Decoder delegate callback routines.
    static OMX_ERRORTYPE EventHandlerCallback(OMX_HANDLETYPE, OMX_PTR, OMX_EVENTTYPE, OMX_U32, OMX_U32, OMX_PTR);
    static OMX_ERRORTYPE EmptyBufferDoneCallback(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);
    static OMX_ERRORTYPE FillBufferDoneCallback(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);
    

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
    OMX_ERRORTYPE freeInputBuffers();
    OMX_ERRORTYPE freeOutputBuffers();
    void resetEOS();
    bool EOS()
    {
        return m_eos;
    };
    void setEOS(bool isEndOfStream);
    void setFillBufferDoneHandler(OMX_ERRORTYPE (*p)(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*))
    {
        CustomFillBufferDoneHandler = p;
    };
    void setEmptyBufferDoneHandler(OMX_ERRORTYPE (*p)(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*))
    {
        CustomEmptyBufferDoneHandler = p;
    };
    
    //additional event handlers
    OMX_ERRORTYPE (*CustomFillBufferDoneHandler)(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);
    OMX_ERRORTYPE (*CustomEmptyBufferDoneHandler)(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);
    
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
    

    
    // OMXCore input buffers (demuxer packets)
    pthread_mutex_t   m_omx_input_mutex;
    queue<OMX_BUFFERHEADERTYPE*> inputBuffersAvailable;
    vector<OMX_BUFFERHEADERTYPE*> inputBuffers;
    
    // OMXCore output buffers (video frames)
    pthread_mutex_t   m_omx_output_mutex;
    std::queue<OMX_BUFFERHEADERTYPE*> outputBuffersAvailable;
    std::vector<OMX_BUFFERHEADERTYPE*> outputBuffers;
    sem_t         m_omx_fill_buffer_done;
    
    bool            doExit;
    pthread_cond_t  m_input_buffer_cond;
    pthread_cond_t  m_output_buffer_cond;
    pthread_cond_t  m_omx_event_cond;
    bool            m_eos;
    bool            doFlushInput;
    bool            doFlushOutput;
    void            lock();
    void            unlock();
    
    int frameCounter;
    int frameOffset;
    
};