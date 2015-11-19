#pragma once

#define __func__ __PRETTY_FUNCTION__


#include "ofMain.h"
#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>
#include <IL/OMX_Index.h>
#include <IL/OMX_Image.h>
#include <IL/OMX_Video.h>
#include <IL/OMX_Broadcom.h>

#include "OMX_Maps.h"

#include <semaphore.h>


//#define OMX_DEBUG_EVENTHANDLER

#define OMX_INIT_STRUCTURE(a) \
  memset(&(a), 0, sizeof(a)); \
  (a).nSize = sizeof(a); \
  (a).nVersion.s.nVersionMajor = OMX_VERSION_MAJOR; \
  (a).nVersion.s.nVersionMinor = OMX_VERSION_MINOR; \
  (a).nVersion.s.nRevision = OMX_VERSION_REVISION; \
  (a).nVersion.s.nStep = OMX_VERSION_STEP

#include "LIBAV_INCLUDES.h"


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

		OMX_HANDLETYPE    GetComponent()
		{
			return handle;
		};
		unsigned int      GetInputPort()
		{
			return inputPort;
		};
		unsigned int      GetOutputPort()
		{
			return outputPort;
		};
		std::string       GetName()
		{
			return componentName;
		};

		OMX_ERRORTYPE DisableAllPorts();
		void          Remove(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2);
		OMX_ERRORTYPE addEvent(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2);
		OMX_ERRORTYPE waitForEvent(OMX_EVENTTYPE event, long timeout = 300);
		OMX_ERRORTYPE waitForCommand(OMX_COMMANDTYPE command, OMX_U32 nData2, long timeout=300);
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

		unsigned int GetInputBufferSize();
        int m_input_buffer_size;

		unsigned int GetInputBufferSpace();
        
		void flushAll();
		void flushInput();
		void flushOutput();

		OMX_BUFFERHEADERTYPE* GetInputBuffer(long timeout=200);
		OMX_BUFFERHEADERTYPE* GetOutputBuffer();

		OMX_ERRORTYPE AllocInputBuffers();
		OMX_ERRORTYPE allocOutputBuffers();

		void resetEOS();
		bool IsEOS()
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
		unsigned int   inputPort;
		unsigned int   outputPort;
		string    componentName;
		pthread_mutex_t   event_mutex;
		pthread_mutex_t   eos_mutex;

		pthread_mutex_t   m_lock;
		vector<OMXEvent> omxEvents;

		OMX_CALLBACKTYPE  m_callbacks;

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

		bool          m_exit;
		pthread_cond_t    m_input_buffer_cond;
		pthread_cond_t    m_output_buffer_cond;
		pthread_cond_t    m_omx_event_cond;
		bool          m_eos;
		bool          m_flush_input;
		bool          m_flush_output;
		void              Lock();
		void              UnLock();
	
	int frameCounter;
	int frameOffset;
	
};

class Tunnel
{
public:
    Tunnel();
    ~Tunnel();
    
    void init(Component*, unsigned int, Component*, unsigned int);
    OMX_ERRORTYPE Flush();
    OMX_ERRORTYPE Deestablish(bool doWait = true);
    OMX_ERRORTYPE Establish(bool portSettingsChanged);
    string srcName;
    string dstName;
private:
    bool isEstablished;
    pthread_mutex_t   m_lock;
    bool            havePortSettingsChanged;
    Component*      sourceComponent;
    Component*      destinationComponent;
    unsigned int    sourcePort;
    unsigned int    destinationPort;
    void            Lock();
    void            UnLock();
};
