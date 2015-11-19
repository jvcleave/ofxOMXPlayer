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


typedef struct omx_event
{
	OMX_EVENTTYPE eEvent;
	OMX_U32 nData1;
	OMX_U32 nData2;
} omx_event;


class COMXCoreComponent
{
	public:
		COMXCoreComponent();
		~COMXCoreComponent();

		OMX_HANDLETYPE    GetComponent()
		{
			return m_handle;
		};
		unsigned int      GetInputPort()
		{
			return m_input_port;
		};
		unsigned int      GetOutputPort()
		{
			return m_output_port;
		};
		std::string       GetName()
		{
			return m_componentName;
		};

		OMX_ERRORTYPE DisableAllPorts();
		void          Remove(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2);
		OMX_ERRORTYPE AddEvent(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2);
		OMX_ERRORTYPE WaitForEvent(OMX_EVENTTYPE event, long timeout = 300);
		OMX_ERRORTYPE WaitForCommand(OMX_COMMANDTYPE command, OMX_U32 nData2, long timeout=300);
		OMX_ERRORTYPE SetStateForComponent(OMX_STATETYPE state);
		OMX_STATETYPE GetState();
		OMX_ERRORTYPE SetParameter(OMX_INDEXTYPE paramIndex, OMX_PTR paramStruct);
		OMX_ERRORTYPE GetParameter(OMX_INDEXTYPE paramIndex, OMX_PTR paramStruct);
		OMX_ERRORTYPE SetConfig(OMX_INDEXTYPE configIndex, OMX_PTR configStruct);
		OMX_ERRORTYPE GetConfig(OMX_INDEXTYPE configIndex, OMX_PTR configStruct);
		OMX_ERRORTYPE SendCommand(OMX_COMMANDTYPE cmd, OMX_U32 cmdParam, OMX_PTR cmdParamData);
		OMX_ERRORTYPE EnablePort(unsigned int port, bool wait = false);
		OMX_ERRORTYPE DisablePort(unsigned int port, bool wait = false);
		OMX_ERRORTYPE UseEGLImage(OMX_BUFFERHEADERTYPE** ppBufferHdr, OMX_U32 nPortIndex, OMX_PTR pAppPrivate, void* eglImage);

		bool Initialize( const std::string& component_name, OMX_INDEXTYPE index);
		bool Deinitialize(bool doFlush=true);

		// Decoder delegate callback routines.
		static OMX_ERRORTYPE DecoderEventHandlerCallback(OMX_HANDLETYPE, OMX_PTR, OMX_EVENTTYPE, OMX_U32, OMX_U32, OMX_PTR);
		static OMX_ERRORTYPE DecoderEmptyBufferDoneCallback(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);
		static OMX_ERRORTYPE DecoderFillBufferDoneCallback(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);

		// OMXCore decoder callback routines.
		OMX_ERRORTYPE DecoderEventHandler(OMX_HANDLETYPE, OMX_PTR, OMX_EVENTTYPE, OMX_U32, OMX_U32, OMX_PTR);
		OMX_ERRORTYPE DecoderEmptyBufferDone(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);
		OMX_ERRORTYPE DecoderFillBufferDone(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);

		OMX_ERRORTYPE EmptyThisBuffer(OMX_BUFFERHEADERTYPE *omx_buffer);
		OMX_ERRORTYPE FillThisBuffer(OMX_BUFFERHEADERTYPE *omx_buffer);
		OMX_ERRORTYPE FreeOutputBuffer(OMX_BUFFERHEADERTYPE *omx_buffer);

		unsigned int GetInputBufferSize();
        int m_input_buffer_size;
		//unsigned int GetOutputBufferSize();

		unsigned int GetInputBufferSpace();
		//unsigned int GetOutputBufferSpace();
        
		void FlushAll();
		void FlushInput();
		void FlushOutput();

		OMX_BUFFERHEADERTYPE *GetInputBuffer(long timeout=200);
		OMX_BUFFERHEADERTYPE *GetOutputBuffer();

		OMX_ERRORTYPE AllocInputBuffers();
		OMX_ERRORTYPE AllocOutputBuffers();

		//OMX_ERRORTYPE FreeInputBuffers(bool wait);
		//OMX_ERRORTYPE FreeOutputBuffers(bool wait);
		void ResetEos();
		bool IsEOS()
		{
			return m_eos;
		};
		void SetEOS(bool isEndOfStream);
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
		OMX_HANDLETYPE m_handle;
		unsigned int   m_input_port;
		unsigned int   m_output_port;
		std::string    m_componentName;
		pthread_mutex_t   m_omx_event_mutex;
		pthread_mutex_t   m_omx_eos_mutex;

		pthread_mutex_t   m_lock;
		std::vector<omx_event> m_omx_events;

		OMX_CALLBACKTYPE  m_callbacks;

		//additional event handlers
		OMX_ERRORTYPE (*CustomDecoderFillBufferDoneHandler)(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);
		OMX_ERRORTYPE (*CustomDecoderEmptyBufferDoneHandler)(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);

		// OMXCore input buffers (demuxer packets)
		pthread_mutex_t   m_omx_input_mutex;
		std::queue<OMX_BUFFERHEADERTYPE*> m_omx_input_available;
		std::vector<OMX_BUFFERHEADERTYPE*> m_omx_input_buffers;

		// OMXCore output buffers (video frames)
		pthread_mutex_t   m_omx_output_mutex;
		std::queue<OMX_BUFFERHEADERTYPE*> m_omx_output_available;
		std::vector<OMX_BUFFERHEADERTYPE*> m_omx_output_buffers;
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

class COMXCoreTunnel
{
public:
    COMXCoreTunnel();
    ~COMXCoreTunnel();
    
    void Initialize(COMXCoreComponent *src_component, unsigned int src_port, COMXCoreComponent *dst_component, unsigned int dst_port);
    OMX_ERRORTYPE Flush();
    OMX_ERRORTYPE Deestablish(bool doWait = true);
    OMX_ERRORTYPE Establish(bool portSettingsChanged);
    string srcName;
    string dstName;
private:
    bool isEstablished;
    pthread_mutex_t   m_lock;
    bool              m_portSettingsChanged;
    COMXCoreComponent *m_src_component;
    COMXCoreComponent *m_dst_component;
    unsigned int      m_src_port;
    unsigned int      m_dst_port;
    void              Lock();
    void              UnLock();
};
