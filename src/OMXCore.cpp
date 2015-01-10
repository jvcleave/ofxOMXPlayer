#include <math.h>
#include <sys/time.h>
#include "OMXCore.h"
#include "OMXClock.h"
#include "XMemUtils.h"

//#define ENABLE_WAIT_FOR_COMMANDS
//#define OMX_DEBUG_EVENTS
//#define OMX_DEBUG_EVENTHANDLER
#define COMMAND_WAIT_TIMEOUT 20


string printOMXError(OMX_ERRORTYPE error)
{
	return OMXMaps::getInstance().omxErrors[error];
}


string printState(OMX_STATETYPE state)
{
	return OMXMaps::getInstance().omxStates[state];
}

string printEventType(OMX_EVENTTYPE eventType)
{
	return OMXMaps::getInstance().omxEventTypes[eventType];
}

string printCmd(OMX_COMMANDTYPE command)
{

	return OMXMaps::getInstance().omxCommands[command];
}

static void add_timespecs(struct timespec& time, long millisecs)
{
	time.tv_sec  += millisecs / 1000;
	time.tv_nsec += (millisecs % 1000) * 1000000;
	if (time.tv_nsec > 1000000000)
	{
		time.tv_sec  += 1;
		time.tv_nsec -= 1000000000;
	}
}


string COMXCore::getOMXError(OMX_ERRORTYPE error)
{
	return printOMXError(error);
}

#pragma mark COMXCoreTunel

COMXCoreTunel::COMXCoreTunel()
{
	srcName = "UNDEFINED_srcName";
	dstName = "UNDEFINED_dstName";
	m_src_component       = NULL;
	m_dst_component       = NULL;
	m_src_port            = 0;
	m_dst_port            = 0;
	m_portSettingsChanged = false;
	isEstablished = false;

	pthread_mutex_init(&m_lock, NULL);
}

COMXCoreTunel::~COMXCoreTunel()
{
	if(isEstablished)
	{
		ofLogVerbose(__func__) << " ABOUT TO CALL Deestablish()";
		Deestablish();
	}

	pthread_mutex_destroy(&m_lock);
}

void COMXCoreTunel::Lock()
{
	pthread_mutex_lock(&m_lock);
}

void COMXCoreTunel::UnLock()
{
	pthread_mutex_unlock(&m_lock);
}

void COMXCoreTunel::Initialize(COMXCoreComponent *src_component, unsigned int src_port, COMXCoreComponent *dst_component, unsigned int dst_port)
{

	m_src_component  = src_component;
	m_src_port    = src_port;
	m_dst_component  = dst_component;
	m_dst_port    = dst_port;

	srcName = m_src_component->GetName();
	dstName = m_dst_component->GetName();
}

OMX_ERRORTYPE COMXCoreTunel::Flush()
{
	if(!m_src_component || !m_dst_component || !isEstablished)
	{
		return OMX_ErrorUndefined;
	}

	Lock();
	if(m_src_component->GetComponent())
	{
		m_src_component->FlushAll();
	}

	if(m_dst_component->GetComponent())
	{
		m_dst_component->FlushAll();
	}

	UnLock();
	return OMX_ErrorNone;
}

OMX_ERRORTYPE COMXCoreTunel::Deestablish(bool doWait)
{

	if (!isEstablished)
	{
		return OMX_ErrorNone;
	}

	if(!m_src_component || !m_dst_component)
	{
		return OMX_ErrorUndefined;
	}

	Lock();
	OMX_ERRORTYPE error = OMX_ErrorNone;

	if(m_src_component->GetComponent() && m_portSettingsChanged && doWait)
	{
		error = m_src_component->WaitForEvent(OMX_EventPortSettingsChanged);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << " WaitForEvent OMX_EventPortSettingsChanged FAIL " << printOMXError(error);
		}
	}
	if(m_src_component->GetComponent())
	{
		error = m_src_component->DisablePort(m_src_port);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << srcName << " DisablePort FAIL" << " m_src_port: " << m_src_port << " error: " << printOMXError(error);
		}
	}
	if(m_dst_component->GetComponent())
	{
		error = m_dst_component->DisablePort(m_dst_port);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << dstName << " DisablePort FAIL" << " m_src_port: " << m_dst_port << " error: " << printOMXError(error);
		}
	}
	if(m_src_component->GetComponent())
	{
		error = OMX_SetupTunnel(m_src_component->GetComponent(), m_src_port, NULL, 0);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << srcName << " TUNNEL UNSET FAIL: " << printOMXError(error);
		}
	}

	if(m_dst_component->GetComponent())
	{
		error = OMX_SetupTunnel(m_dst_component->GetComponent(), m_dst_port, NULL, 0);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << dstName << " TUNNEL UNSET FAIL: " << printOMXError(error);
		}
	}
	UnLock();
	isEstablished = false;
	return OMX_ErrorNone;
}

OMX_ERRORTYPE COMXCoreTunel::Establish(bool portSettingsChanged)
{
	Lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_PARAM_U32TYPE param;
	OMX_INIT_STRUCTURE(param);
	bool doWait = false;
	if(!m_src_component || !m_dst_component)
	{
		UnLock();
		return OMX_ErrorUndefined;
	}

	if(m_src_component->GetState() == OMX_StateLoaded)
	{
		error = m_src_component->SetStateForComponent(OMX_StateIdle);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << srcName << " Setting state to idle FAIL: " << printOMXError(error);
			UnLock();
			return error;
		}
	}

	if(portSettingsChanged)
	{
		error = m_src_component->WaitForEvent(OMX_EventPortSettingsChanged);
		if(error != OMX_ErrorNone)
		{
			UnLock();
			ofLogError(__func__) << srcName << " WaitForEvent OMX_EventPortSettingsChanged FAIL: " << printOMXError(error);
			return error;
		}
	}
	if(m_src_component->GetComponent())
	{
		error = m_src_component->DisablePort(m_src_port);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << srcName << " DisablePort FAIL: " << printOMXError(error);
		}
	}

	if(m_dst_component->GetComponent())
	{
		error = m_dst_component->DisablePort(m_dst_port);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << dstName << " DisablePort doWait" << doWait << " FAIL: " << printOMXError(error);
		}
	}

	if(m_src_component->GetComponent() && m_dst_component->GetComponent())
	{
		error = OMX_SetupTunnel(m_src_component->GetComponent(), m_src_port, m_dst_component->GetComponent(), m_dst_port);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << " OMX_SetupTunnel " << srcName << " TO " << dstName << " FAIL: " << printOMXError(error);
			UnLock();
			return error;
		}
		else
		{
			isEstablished =true;
		}
	}
	else
	{
		ofLogError(__func__) << " OMX_SetupTunnel " << srcName << " TO " << dstName << " FAILED: COMPONENT WAS MISSING";
		UnLock();
		return OMX_ErrorUndefined;
	}

	if(m_src_component->GetComponent())
	{
		error = m_src_component->EnablePort(m_src_port, doWait);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__)  << srcName << " EnablePort doWait" << doWait << " FAIL: " << printOMXError(error);
			UnLock();
			return error;
		}
	}

	if(m_dst_component->GetComponent())
	{
		error = m_dst_component->EnablePort(m_dst_port, doWait);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__)  << dstName << " EnablePort doWait" << doWait << " FAIL: " << printOMXError(error);
			UnLock();
			return error;
		}
	}

	if(m_dst_component->GetComponent())
	{
		if(m_dst_component->GetState() == OMX_StateLoaded)
		{
			//important to wait for audio
			error = m_dst_component->WaitForCommand(OMX_CommandPortEnable, m_dst_port, COMMAND_WAIT_TIMEOUT);
			if(error != OMX_ErrorNone)
			{
				ofLogError(__func__)  << dstName << " WaitForCommand OMX_CommandPortEnable FAIL: " << printOMXError(error);
				UnLock();
				return error;
			}
			error = m_dst_component->SetStateForComponent(OMX_StateIdle);
			if(error != OMX_ErrorNone)
			{
				ofLogError(__func__)  << dstName << " SetStateForComponent OMX_StateIdle FAIL: " << printOMXError(error);
				UnLock();
				return error;
			}
		}
		else
		{
			#ifdef ENABLE_WAIT_FOR_COMMANDS
			error = m_dst_component->WaitForCommand(OMX_CommandPortEnable, m_dst_port);
			if(error != OMX_ErrorNone)
			{
				ofLogError(__func__)  << dstName << " WaitForCommand OMX_CommandPortEnable FAIL: " << printOMXError(error);
				UnLock();
				return error;
			}
			#endif
		}
	}

	if(m_src_component->GetComponent())
	{
		error = m_src_component->WaitForCommand(OMX_CommandPortEnable, m_src_port, COMMAND_WAIT_TIMEOUT);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__)  << srcName << " WaitForCommand OMX_CommandPortEnable FAIL: " << printOMXError(error);
			UnLock();
			return error;
		}
	}

	m_portSettingsChanged = portSettingsChanged;

	UnLock();


	return OMX_ErrorNone;
}

#pragma mark COMXCoreComponent

COMXCoreComponent::COMXCoreComponent()
{
	
	frameCounter = 0;
	frameOffset = 0;
	m_input_port  = 0;
	m_output_port = 0;
	m_handle      = NULL;

	m_input_alignment     = 0;
	m_input_buffer_size  = 0;
	m_input_buffer_count  = 0;

	m_output_alignment    = 0;
	m_output_buffer_size  = 0;
	m_output_buffer_count = 0;
	m_flush_input         = false;
	m_flush_output        = false;

	CustomDecoderFillBufferDoneHandler = NULL;
	CustomDecoderEmptyBufferDoneHandler = NULL;

	m_eos                 = false;

	m_exit = false;

	pthread_mutex_init(&m_omx_input_mutex, NULL);
	pthread_mutex_init(&m_omx_output_mutex, NULL);
	pthread_mutex_init(&m_omx_event_mutex, NULL);
	pthread_mutex_init(&m_omx_eos_mutex, NULL);
	pthread_cond_init(&m_input_buffer_cond, NULL);
	pthread_cond_init(&m_output_buffer_cond, NULL);
	pthread_cond_init(&m_omx_event_cond, NULL);

	m_omx_input_use_buffers  = false;
	m_omx_output_use_buffers = false;


	pthread_mutex_init(&m_lock, NULL);
	sem_init(&m_omx_fill_buffer_done, 0, 0);
}

COMXCoreComponent::~COMXCoreComponent()
{
	Deinitialize();

	pthread_mutex_destroy(&m_omx_input_mutex);
	pthread_mutex_destroy(&m_omx_output_mutex);
	pthread_mutex_destroy(&m_omx_event_mutex);
	pthread_mutex_destroy(&m_omx_eos_mutex);
	pthread_cond_destroy(&m_input_buffer_cond);
	pthread_cond_destroy(&m_output_buffer_cond);
	pthread_cond_destroy(&m_omx_event_cond);

	pthread_mutex_destroy(&m_lock);
	sem_destroy(&m_omx_fill_buffer_done);

}


int COMXCoreComponent::getCurrentFrame()
{
	return frameCounter;
	
	//return frameCounter-frameOffset;
}

void COMXCoreComponent::resetFrameCounter()
{
	frameOffset = frameCounter;
	frameCounter = 0;
}

void COMXCoreComponent::incrementFrameCounter()
{
	frameCounter++;
}

void COMXCoreComponent::ResetEos()
{
	pthread_mutex_lock(&m_omx_eos_mutex);
	m_eos = false;
	pthread_mutex_unlock(&m_omx_eos_mutex);
}


void COMXCoreComponent::SetEOS(bool isEndofStream)
{
	m_eos = isEndofStream;
}
void COMXCoreComponent::Lock()
{
	pthread_mutex_lock(&m_lock);
}

void COMXCoreComponent::UnLock()
{
	pthread_mutex_unlock(&m_lock);
}

OMX_ERRORTYPE COMXCoreComponent::EmptyThisBuffer(OMX_BUFFERHEADERTYPE *omx_buffer)
{

	OMX_ERRORTYPE error = OMX_ErrorNone;

	if(!m_handle || !omx_buffer)
	{
		return OMX_ErrorUndefined;
	}

	error = OMX_EmptyThisBuffer(m_handle, omx_buffer);
	if (error != OMX_ErrorNone)
	{
		ofLogError(__func__)  << m_componentName << " OMX_EmptyThisBuffer FAIL: " << printOMXError(error);
	}

	return error;
}

OMX_ERRORTYPE COMXCoreComponent::FillThisBuffer(OMX_BUFFERHEADERTYPE *omx_buffer)
{

	OMX_ERRORTYPE error = OMX_ErrorNone;

	if(!m_handle || !omx_buffer)
	{
		return OMX_ErrorUndefined;
	}

	error = OMX_FillThisBuffer(m_handle, omx_buffer);
	if (error != OMX_ErrorNone)
	{
		ofLogError(__func__)  << m_componentName << " OMX_FillThisBuffer FAIL: " << printOMXError(error);
	}

	return error;
}

OMX_ERRORTYPE COMXCoreComponent::FreeOutputBuffer(OMX_BUFFERHEADERTYPE *omx_buffer)
{
	OMX_ERRORTYPE error = OMX_ErrorNone;

	if(!m_handle || !omx_buffer)
	{
		return OMX_ErrorUndefined;
	}

	error = OMX_FreeBuffer(m_handle, m_output_port, omx_buffer);
	if (error != OMX_ErrorNone)
	{
		ofLogError(__func__)  << m_componentName << " OMX_FreeBuffer FAIL: " << printOMXError(error);
	}

	return error;
}

unsigned int COMXCoreComponent::GetInputBufferSize()
{
	int free = m_input_buffer_count * m_input_buffer_size;
	return free;
}

unsigned int COMXCoreComponent::GetOutputBufferSize()
{
	int free = m_output_buffer_count * m_output_buffer_size;
	return free;
}

unsigned int COMXCoreComponent::GetInputBufferSpace()
{
	int free = m_omx_input_avaliable.size() * m_input_buffer_size;
	return free;
}

unsigned int COMXCoreComponent::GetOutputBufferSpace()
{
	int free = m_omx_output_available.size() * m_output_buffer_size;
	return free;
}

void COMXCoreComponent::FlushAll()
{
	ofLogVerbose(__func__) << m_componentName << " START";
	FlushInput();
	FlushOutput();
	ofLogVerbose(__func__) << m_componentName << " END";
}

void COMXCoreComponent::FlushInput()
{
	if (!m_omx_input_use_buffers)
	{
		return;
	}

	Lock();
	//ofLogVerbose(__func__) << m_componentName << " START";

	OMX_ERRORTYPE error = OMX_ErrorNone;
	error = OMX_SendCommand(m_handle, OMX_CommandFlush, m_input_port, NULL);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " OMX_CommandFlush FAIL: " << printOMXError(error);
	}

	#ifdef ENABLE_WAIT_FOR_COMMANDS
	error = WaitForCommand(OMX_CommandFlush, m_input_port);//TODO timeout here?
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " WaitForCommand OMX_CommandFlush FAIL: " << printOMXError(error);
	}
	#endif

	UnLock();
}

void COMXCoreComponent::FlushOutput()
{
	if (!m_omx_output_use_buffers)
	{
		return;
	}
	Lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;
	error = OMX_SendCommand(m_handle, OMX_CommandFlush, m_output_port, NULL);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " OMX_SendCommand OMX_CommandFlush FAIL: " << printOMXError(error);
	}
	#ifdef ENABLE_WAIT_FOR_COMMANDS
	error = WaitForCommand(OMX_CommandFlush, m_output_port);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " WaitForCommand OMX_CommandFlush FAIL: " << printOMXError(error);
	}
	#endif

	UnLock();
	//ofLogVerbose(__func__) << m_componentName << " END";
}

// timeout in milliseconds
OMX_BUFFERHEADERTYPE *COMXCoreComponent::GetInputBuffer(long timeout)
{
	OMX_BUFFERHEADERTYPE *omx_input_buffer = NULL;

	if(!m_handle)
	{
		ofLogError(__func__) << "NO HANDLE";
		return NULL;
	}

	pthread_mutex_lock(&m_omx_input_mutex);
	struct timespec endtime;
	clock_gettime(CLOCK_REALTIME, &endtime);
	add_timespecs(endtime, timeout);
	while (1 && !m_flush_input)
	{
		if(!m_omx_input_avaliable.empty())
		{
			omx_input_buffer = m_omx_input_avaliable.front();
			m_omx_input_avaliable.pop();
			break;
		}

		int retcode = pthread_cond_timedwait(&m_input_buffer_cond, &m_omx_input_mutex, &endtime);
		if (retcode != 0)
		{
			ofLogError(__func__) << m_componentName << " TIMEOUT";
			break;
		}
	}
	pthread_mutex_unlock(&m_omx_input_mutex);

	return omx_input_buffer;
}

OMX_BUFFERHEADERTYPE *COMXCoreComponent::GetOutputBuffer()
{
	OMX_BUFFERHEADERTYPE *omx_output_buffer = NULL;

	if(!m_handle)
	{
		ofLogError(__func__) << "NO HANDLE";
		return NULL;
	}

	pthread_mutex_lock(&m_omx_output_mutex);
	if(!m_omx_output_available.empty())
	{
		omx_output_buffer = m_omx_output_available.front();
		m_omx_output_available.pop();
	}
	pthread_mutex_unlock(&m_omx_output_mutex);

	return omx_output_buffer;
}

OMX_ERRORTYPE COMXCoreComponent::AllocInputBuffers(bool use_buffers)//use_buffers = false
{
	OMX_ERRORTYPE error = OMX_ErrorNone;

	m_omx_input_use_buffers = use_buffers;

	if(!m_handle)
	{
		ofLogError(__func__) << "NO HANDLE";
		return OMX_ErrorUndefined;
	}

	OMX_PARAM_PORTDEFINITIONTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = m_input_port;

	error = OMX_GetParameter(m_handle, OMX_IndexParamPortDefinition, &portFormat);
	if(error != OMX_ErrorNone)
	{
		return error;
	}

	if(GetState() != OMX_StateIdle)
	{
		if(GetState() != OMX_StateLoaded)
		{
			SetStateForComponent(OMX_StateLoaded);
		}
		SetStateForComponent(OMX_StateIdle);
	}

	error = EnablePort(m_input_port, false);
	if(error != OMX_ErrorNone)
	{
		return error;
	}

	m_input_alignment     = portFormat.nBufferAlignment;
	m_input_buffer_count  = portFormat.nBufferCountActual;
	m_input_buffer_size   = portFormat.nBufferSize;

	/*
	ofLog(OF_LOG_VERBOSE,
		  "COMXCoreComponent::AllocInputBuffers \n \
		  component(%s)							\n \
		  port(%d)								\n \
		  nBufferCountMin(%u)					\n \
		  nBufferCountActual(%u)				\n \
		  nBufferSize(%u)						\n \
		  nBufferAlignmen(%u) \n",
		  m_componentName.c_str(),
		  GetInputPort(),
		  portFormat.nBufferCountMin,
		  portFormat.nBufferCountActual,
		  portFormat.nBufferSize,
		  portFormat.nBufferAlignment);
	 */

	for (size_t i = 0; i < portFormat.nBufferCountActual; i++)
	{
		OMX_BUFFERHEADERTYPE *buffer = NULL;
		OMX_U8* data = NULL;

		if(m_omx_input_use_buffers)
		{
			data = (OMX_U8*)_aligned_malloc(portFormat.nBufferSize, m_input_alignment);
			error = OMX_UseBuffer(m_handle, &buffer, m_input_port, NULL, portFormat.nBufferSize, data);
		}
		else
		{
			error = OMX_AllocateBuffer(m_handle, &buffer, m_input_port, NULL, portFormat.nBufferSize);
		}
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << m_componentName << " OMX_UseBuffer FAIL: " << printOMXError(error);
			if(m_omx_input_use_buffers && data)
			{
				_aligned_free(data);
			}
			return error;
		}

		buffer->nInputPortIndex = m_input_port;
		buffer->nFilledLen      = 0;
		buffer->nOffset         = 0;
		buffer->pAppPrivate     = (void*)i;
		m_omx_input_buffers.push_back(buffer);
		m_omx_input_avaliable.push(buffer);
	}

	//ofLogVerbose(__func__) << m_componentName << " BUFFER SIZE: " << m_omx_input_buffers.size();

	#ifdef ENABLE_WAIT_FOR_COMMANDS
	error = WaitForCommand(OMX_CommandPortEnable, m_input_port);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " WaitForCommand OMX_CommandPortEnable FAIL: " << printOMXError(error);
	}
	#endif

	m_flush_input = false;

	return error;
}

OMX_ERRORTYPE COMXCoreComponent::AllocOutputBuffers(bool use_buffers /* = false */)
{
	OMX_ERRORTYPE error = OMX_ErrorNone;

	if(!m_handle)
	{
		ofLogError(__func__) << "NO HANDLE";
		return OMX_ErrorUndefined;
	}

	m_omx_output_use_buffers = use_buffers;

	OMX_PARAM_PORTDEFINITIONTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = m_output_port;

	error = OMX_GetParameter(m_handle, OMX_IndexParamPortDefinition, &portFormat);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " OMX_GetParameter OMX_IndexParamPortDefinition FAIL: " << printOMXError(error);
		return error;
	}

	if(GetState() != OMX_StateIdle)
	{
		if(GetState() != OMX_StateLoaded)
		{
			SetStateForComponent(OMX_StateLoaded);
		}
		SetStateForComponent(OMX_StateIdle);
	}

	error = EnablePort(m_output_port, false);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " Enable Output Port FAIL: " << printOMXError(error);
		return error;
	}

	m_output_alignment     = portFormat.nBufferAlignment;
	m_output_buffer_count  = portFormat.nBufferCountActual;
	m_output_buffer_size   = portFormat.nBufferSize;
	/*
	 stringstream info;
	info << __func__ << "\n";
	info << m_componentName << "\n";
	info << "port: " << m_output_port << "\n";
	info << "nBufferCountMin: " << portFormat.nBufferCountMin << "\n";
	info << "nBufferCountActual: " << portFormat.nBufferCountActual << "\n";
	info << "nBufferSize: " << portFormat.nBufferSize << "\n";
	info << "nBufferAlignmen: " << portFormat.nBufferAlignment << "\n";
	ofLogVerbose(__func__) << info.str();
	 */

	for (size_t i = 0; i < portFormat.nBufferCountActual; i++)
	{
		OMX_BUFFERHEADERTYPE *buffer = NULL;
		OMX_U8* data = NULL;

		if(m_omx_output_use_buffers)
		{
			data = (OMX_U8*)_aligned_malloc(portFormat.nBufferSize, m_output_alignment);
			error = OMX_UseBuffer(m_handle, &buffer, m_output_port, NULL, portFormat.nBufferSize, data);
			if(error != OMX_ErrorNone)
			{
				ofLogError(__func__) << m_componentName << "OMX_UseBuffer FAIL: " << printOMXError(error);
				if (data)
				{
					_aligned_free(data);
				}
				return error;
			}
		}
		else
		{
			error = OMX_AllocateBuffer(m_handle, &buffer, m_output_port, NULL, portFormat.nBufferSize);
			if(error != OMX_ErrorNone)
			{
				ofLogError(__func__) << m_componentName << "OMX_AllocateBuffer FAIL: " << printOMXError(error);
				return error;
			}
		}
		buffer->nOutputPortIndex = m_output_port;
		buffer->nFilledLen       = 0;
		buffer->nOffset          = 0;
		buffer->pAppPrivate      = (void*)i;
		m_omx_output_buffers.push_back(buffer);
		m_omx_output_available.push(buffer);
	}

	#ifdef ENABLE_WAIT_FOR_COMMANDS
	error = WaitForCommand(OMX_CommandPortEnable, m_output_port);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << "WaitForCommand OMX_CommandPortEnable FAIL: " << printOMXError(error);
	}
	#endif

	m_flush_output = false;

	return error;
}

OMX_ERRORTYPE COMXCoreComponent::FreeInputBuffers(bool wait)
{
	OMX_ERRORTYPE error = OMX_ErrorNone;

	if(!m_handle)
	{
		ofLogError(__func__) << "NO HANDLE";
		return OMX_ErrorUndefined;
	}

	if(m_omx_input_buffers.empty())
	{
		return OMX_ErrorNone;
	}

	m_flush_input = true;

	pthread_mutex_lock(&m_omx_input_mutex);
	pthread_cond_broadcast(&m_input_buffer_cond);

	error = DisablePort(m_input_port);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " DisablePort FAIL: " << printOMXError(error);
	}
	for (size_t i = 0; i < m_omx_input_buffers.size(); i++)
	{
		uint8_t *buf = m_omx_input_buffers[i]->pBuffer;

		error = OMX_FreeBuffer(m_handle, m_input_port, m_omx_input_buffers[i]);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << m_componentName << " OMX_FreeBuffer FAIL: " << printOMXError(error);
		}
		if(m_omx_input_use_buffers && buf)
		{
			_aligned_free(buf);
		}


	}

	#ifdef ENABLE_WAIT_FOR_COMMANDS
	error =  WaitForCommand(OMX_CommandPortDisable, m_input_port);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " WaitForCommand OMX_CommandPortDisable FAIL: " << printOMXError(error);
	}
	#endif

	m_omx_input_buffers.clear();

	while (!m_omx_input_avaliable.empty())
	{
		m_omx_input_avaliable.pop();
	}

	m_input_alignment     = 0;
	m_input_buffer_size   = 0;
	m_input_buffer_count  = 0;

	pthread_mutex_unlock(&m_omx_input_mutex);

	return error;
}

OMX_ERRORTYPE COMXCoreComponent::FreeOutputBuffers(bool wait)
{
	OMX_ERRORTYPE error = OMX_ErrorNone;

	if(!m_handle)
	{
		ofLogError(__func__) << "NO HANDLE";
		return OMX_ErrorUndefined;
	}

	if(m_omx_output_buffers.empty())
	{
		return OMX_ErrorNone;
	}

	m_flush_output = true;

	pthread_mutex_lock(&m_omx_output_mutex);
	pthread_cond_broadcast(&m_output_buffer_cond);

	error = DisablePort(m_output_port);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " DisablePort FAIL: " << printOMXError(error);
	}

	for (size_t i = 0; i < m_omx_output_buffers.size(); i++)
	{
		uint8_t *buf = m_omx_output_buffers[i]->pBuffer;

		error = OMX_FreeBuffer(m_handle, m_output_port, m_omx_output_buffers[i]);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << m_componentName << " OMX_FreeBuffer FAIL: " << printOMXError(error);
		}

		if(m_omx_output_use_buffers && buf)
		{
			_aligned_free(buf);
		}
	}

	#ifdef ENABLE_WAIT_FOR_COMMANDS
	error =  WaitForCommand(OMX_CommandPortDisable, m_output_port);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " WaitForCommand OMX_CommandPortDisable FAIL: " << printOMXError(error);
	}
	#endif

	m_omx_output_buffers.clear();

	while (!m_omx_output_available.empty())
	{
		m_omx_output_available.pop();
	}

	m_output_alignment    = 0;
	m_output_buffer_size  = 0;
	m_output_buffer_count = 0;

	pthread_mutex_unlock(&m_omx_output_mutex);

	return error;
}

OMX_ERRORTYPE COMXCoreComponent::DisableAllPorts()
{
	Lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;

	if(!m_handle)
	{
		ofLogError(__func__) << "NO HANDLE";
		UnLock();
		return OMX_ErrorUndefined;
	}

	OMX_INDEXTYPE idxTypes[] =
	{
		OMX_IndexParamAudioInit,
		OMX_IndexParamImageInit,
		OMX_IndexParamVideoInit,
		OMX_IndexParamOtherInit
	};

	OMX_PORT_PARAM_TYPE ports;
	OMX_INIT_STRUCTURE(ports);

	int i;
	for(i=0; i < 4; i++)
	{
		error = OMX_GetParameter(m_handle, idxTypes[i], &ports);
		if(error == OMX_ErrorNone)
		{

			uint32_t j;
			for(j=0; j<ports.nPorts; j++)
			{
				OMX_PARAM_PORTDEFINITIONTYPE portFormat;
				OMX_INIT_STRUCTURE(portFormat);
				portFormat.nPortIndex = ports.nStartPortNumber+j;

				error = OMX_GetParameter(m_handle, OMX_IndexParamPortDefinition, &portFormat);
				if(error != OMX_ErrorNone)
				{
					if(portFormat.bEnabled == OMX_FALSE)
					{
						continue;
					}
				}

				error = OMX_SendCommand(m_handle, OMX_CommandPortDisable, ports.nStartPortNumber+j, NULL);
				if(error != OMX_ErrorNone)
				{
					ofLogError(__func__) << m_componentName << " OMX_SendCommand OMX_CommandPortDisable FAIL: " << printOMXError(error);
				}
				#ifdef ENABLE_WAIT_FOR_COMMANDS
				error = WaitForCommand(OMX_CommandPortDisable, ports.nStartPortNumber+j);
				if(error != OMX_ErrorNone && error != OMX_ErrorSameState)
				{
					UnLock();
					return error;
				}
				#endif
			}
		}
	}

	UnLock();

	return OMX_ErrorNone;
}

void COMXCoreComponent::Remove(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2)
{
	for (std::vector<omx_event>::iterator it = m_omx_events.begin(); it != m_omx_events.end(); )
	{
		omx_event event = *it;

		if(event.eEvent == eEvent && event.nData1 == nData1 && event.nData2 == nData2)
		{
			it = m_omx_events.erase(it);
			continue;
		}
		++it;
	}
}

OMX_ERRORTYPE COMXCoreComponent::AddEvent(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2)
{
	omx_event event;

	event.eEvent      = eEvent;
	event.nData1      = nData1;
	event.nData2      = nData2;

	pthread_mutex_lock(&m_omx_event_mutex);
	Remove(eEvent, nData1, nData2);
	m_omx_events.push_back(event);
	// this allows (all) blocked tasks to be awoken
	pthread_cond_broadcast(&m_omx_event_cond);
	pthread_mutex_unlock(&m_omx_event_mutex);

	#ifdef OMX_DEBUG_EVENTS
	stringstream info;
	info << m_componentName << "\n";
	info << "event.eEvent: " << event.eEvent << "\n";
	info << "event.nData1: " << event.nData1 << "\n";
	info << "event.nData2: " << event.nData2 << "\n";
	//ofLogVerbose(__func__) << info.str();
	#endif

	return OMX_ErrorNone;
}


// timeout in milliseconds
OMX_ERRORTYPE COMXCoreComponent::WaitForEvent(OMX_EVENTTYPE eventType, long timeout)
{
	#ifdef OMX_DEBUG_EVENTS
	//ofLogVerbose(__func__) << m_componentName << " eventType: " << eventType;
	#endif

	pthread_mutex_lock(&m_omx_event_mutex);
	struct timespec endtime;
	clock_gettime(CLOCK_REALTIME, &endtime);
	add_timespecs(endtime, timeout);
	while(true)
	{
		for (std::vector<omx_event>::iterator it = m_omx_events.begin(); it != m_omx_events.end(); it++)
		{
			omx_event event = *it;
			#ifdef OMX_DEBUG_EVENTS
			stringstream info;
			info << m_componentName << "\n";
			info << "event.eEvent: " << printEventType(event.eEvent) << "\n";
			info << "event.nData1: " << event.nData1 << "\n";
			info << "event.nData2: " << event.nData2 << "\n";
			//ofLogVerbose(__func__) << info.str();
			#endif
			if(event.eEvent == OMX_EventError && event.nData1 == (OMX_U32)OMX_ErrorSameState && event.nData2 == 1)
			{
				#ifdef OMX_DEBUG_EVENTS
				stringstream rmInfo;
				rmInfo << m_componentName << "\n";
				rmInfo << "ERROR REMOVING EVENT" << "\n";
				rmInfo << "event.eEvent: " << printEventType(event.eEvent) << "\n";
				rmInfo << "event.nData1: " << event.nData1 << "\n";
				rmInfo << "event.nData2: " << event.nData2 << "\n";
				//ofLogVerbose(__func__) << rmInfo.str();
				#endif
				m_omx_events.erase(it);
				pthread_mutex_unlock(&m_omx_event_mutex);
				return OMX_ErrorNone;
			}
			else if(event.eEvent == OMX_EventError)
			{
				m_omx_events.erase(it);
				pthread_mutex_unlock(&m_omx_event_mutex);
				return (OMX_ERRORTYPE)event.nData1;
			}
			else if(event.eEvent == eventType)
			{
				#ifdef OMX_DEBUG_EVENTS
				stringstream finishedInfo;
				finishedInfo << m_componentName << "\n";
				finishedInfo << "RECEIVED EVENT, REMOVING" << "\n";
				finishedInfo << "event.eEvent: " << printEventType(event.eEvent) << "\n";
				finishedInfo << "event.nData1: " << event.nData1 << "\n";
				finishedInfo << "event.nData2: " << event.nData2 << "\n";
				//ofLogVerbose(__func__) << finishedInfo.str();
				#endif
				m_omx_events.erase(it);
				pthread_mutex_unlock(&m_omx_event_mutex);
				return OMX_ErrorNone;
			}
		}

		int retcode = pthread_cond_timedwait(&m_omx_event_cond, &m_omx_event_mutex, &endtime);
		if (retcode != 0)
		{
			//ofLogError(__func__) << m_componentName << " WaitForEvent Event: " << printEventType(eventType) << " TIMED OUT at: " << timeout;
			pthread_mutex_unlock(&m_omx_event_mutex);
			return OMX_ErrorMax;
		}
	}

	pthread_mutex_unlock(&m_omx_event_mutex);
	return OMX_ErrorNone;
}

// timeout in milliseconds
OMX_ERRORTYPE COMXCoreComponent::WaitForCommand(OMX_COMMANDTYPE command, OMX_U32 nData2, long timeout) //timeout default = 2000
{
	#ifdef OMX_DEBUG_EVENTS
	//ofLogVerbose(__func__) << m_componentName << " WAITING " << timeout << "MS FOR COMMAND: " <<  printCmd(command);
	#endif

	pthread_mutex_lock(&m_omx_event_mutex);
	struct timespec endtime;
	clock_gettime(CLOCK_REALTIME, &endtime);
	add_timespecs(endtime, timeout);
	while(true)
	{
		for (std::vector<omx_event>::iterator it = m_omx_events.begin(); it != m_omx_events.end(); it++)
		{
			omx_event event = *it;

			#ifdef OMX_DEBUG_EVENTS
			ofLog(OF_LOG_VERBOSE,
			      "COMXCoreComponent::WaitForCommand %s \n \
				  inlist event event.eEvent 0x%08x		\n \
				  event.nData1 0x%08x					\n \
				  event.nData2 %d\n",
			      m_componentName.c_str(),
			      (int)event.eEvent,
			      (int)event.nData1,
			      (int)event.nData2);
			#endif



			if(event.eEvent == OMX_EventError && event.nData1 == (OMX_U32)OMX_ErrorSameState && event.nData2 == 1)
			{
				#ifdef OMX_DEBUG_EVENTS
				ofLogError(__func__) << m_componentName << " SAME STATE FOR COMMAND: " <<  printCmd(command);
				//ofLog(OF_LOG_VERBOSE, "COMXCoreComponent::WaitForCommand %s remove event event.eEvent 0x%08x event.nData1 0x%08x event.nData2 %d\n", m_componentName.c_str(), (int)event.eEvent, (int)event.nData1, (int)event.nData2);
				#endif

				m_omx_events.erase(it);
				pthread_mutex_unlock(&m_omx_event_mutex);
				return OMX_ErrorNone;
			}

			if(event.eEvent == OMX_EventError)
			{
				ofLogError(__func__) << m_componentName << " " <<  printCmd(command);
				m_omx_events.erase(it);
				pthread_mutex_unlock(&m_omx_event_mutex);
				return (OMX_ERRORTYPE)event.nData1;
			}

			if(event.eEvent == OMX_EventCmdComplete && event.nData1 == command && event.nData2 == nData2)
			{

				#ifdef OMX_DEBUG_EVENTS
				//ofLogVerbose(__func__) << m_componentName << " " <<  printCmd(command) << "PASS";
				//ofLog(OF_LOG_VERBOSE, "COMXCoreComponent::WaitForCommand %s remove event event.eEvent 0x%08x event.nData1 0x%08x event.nData2 %d\n", m_componentName.c_str(), (int)event.eEvent, (int)event.nData1, (int)event.nData2);
				#endif

				m_omx_events.erase(it);
				pthread_mutex_unlock(&m_omx_event_mutex);
				return OMX_ErrorNone;
			}
		}

		int retcode = pthread_cond_timedwait(&m_omx_event_cond, &m_omx_event_mutex, &endtime);
		if (retcode != 0)
		{
			ofLog(OF_LOG_VERBOSE,
			      "COMXCoreComponent::WaitForCommand %s		\n \
				  wait timeout event.eEvent 0x%08x			\n \
				  event.command 0x%08x						\n \
				  event.nData2 %d\n",
			      m_componentName.c_str(),
			      (int)OMX_EventCmdComplete,
			      (int)command,
			      (int)nData2);
			pthread_mutex_unlock(&m_omx_event_mutex);
			return OMX_ErrorMax;
		}
	}
	pthread_mutex_unlock(&m_omx_event_mutex);
	return OMX_ErrorNone;
}

OMX_ERRORTYPE COMXCoreComponent::SetStateForComponent(OMX_STATETYPE state)
{
	if(!m_handle)
	{
		return OMX_ErrorUndefined;
	}

	Lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_STATETYPE state_actual = OMX_StateMax;

	if(state == state_actual)
	{
		UnLock();
		return OMX_ErrorNone;
	}

	error = OMX_SendCommand(m_handle, OMX_CommandStateSet, state, 0);
	if (error != OMX_ErrorNone)
	{
		if(error == OMX_ErrorSameState)
		{
			error = OMX_ErrorNone;
		}
		else
		{
			ofLogError(__func__) << " OMX_GetState FAIL " << printOMXError(error);
		}
	}
	else
	{
		error = WaitForCommand(OMX_CommandStateSet, state, COMMAND_WAIT_TIMEOUT);
		if(error == OMX_ErrorSameState)
		{
			ofLogError(__func__) << " OMX_GetState FAIL " << printOMXError(error);
			UnLock();
			return OMX_ErrorNone;
		}
	}

	UnLock();

	return error;
}

OMX_STATETYPE COMXCoreComponent::GetState()
{

	//ofLogVerbose(__func__) << m_componentName << " START";
	if(m_handle)
	{
		Lock();
		OMX_STATETYPE state;
		OMX_ERRORTYPE error = OMX_GetState(m_handle, &state);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << " OMX_GetState FAIL " << printOMXError(error);
		}
		//ofLogVerbose(__func__) << m_componentName << " END";
		UnLock();
		return state;
	}

	return (OMX_STATETYPE)0;
}

OMX_ERRORTYPE COMXCoreComponent::SetParameter(OMX_INDEXTYPE paramIndex, OMX_PTR paramStruct)
{
	Lock();

	OMX_ERRORTYPE error = OMX_SetParameter(m_handle, paramIndex, paramStruct);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " FAIL " << printOMXError(error);
	}

	UnLock();
	return error;
}

OMX_ERRORTYPE COMXCoreComponent::GetParameter(OMX_INDEXTYPE paramIndex, OMX_PTR paramStruct)
{
	Lock();

	OMX_ERRORTYPE error = OMX_GetParameter(m_handle, paramIndex, paramStruct);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " FAIL " << printOMXError(error);
	}

	UnLock();
	return error;
}

OMX_ERRORTYPE COMXCoreComponent::SetConfig(OMX_INDEXTYPE configIndex, OMX_PTR configStruct)
{
	Lock();
	//ofLogVerbose(__func__) << m_componentName << " START";
	OMX_ERRORTYPE error = OMX_SetConfig(m_handle, configIndex, configStruct);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " OMX_SetConfig FAIL " << printOMXError(error);
	}
	else
	{
		//ofLogVerbose(__func__) << m_componentName << " OMX_SetConfig PASS";
	}

	//ofLogVerbose(__func__) << m_componentName << " END";
	UnLock();
	return error;
}

OMX_ERRORTYPE COMXCoreComponent::GetConfig(OMX_INDEXTYPE configIndex, OMX_PTR configStruct)
{
	Lock();

	OMX_ERRORTYPE error = OMX_GetConfig(m_handle, configIndex, configStruct);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " FAIL " << printOMXError(error);
	}

	UnLock();
	return error;
}

OMX_ERRORTYPE COMXCoreComponent::SendCommand(OMX_COMMANDTYPE cmd, OMX_U32 cmdParam, OMX_PTR cmdParamData)
{
	Lock();

	OMX_ERRORTYPE error = OMX_SendCommand(m_handle, cmd, cmdParam, cmdParamData);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " FAIL " << printOMXError(error);
	}

	UnLock();
	return error;
}

OMX_ERRORTYPE COMXCoreComponent::EnablePort(unsigned int port,  bool wait)//default: wait=false
{
	Lock();
	if(!m_handle)
	{
		ofLogError(__func__) << m_componentName << "NO HANDLE ";

	}
	OMX_PARAM_PORTDEFINITIONTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = port;

	OMX_ERRORTYPE error = OMX_GetParameter(m_handle, OMX_IndexParamPortDefinition, &portFormat);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " OMX_GetParameter FAIL " << " port " << printOMXError(error);
	}

	if(portFormat.bEnabled == OMX_FALSE)
	{
		error = OMX_SendCommand(m_handle, OMX_CommandPortEnable, port, NULL);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << m_componentName << "OMX_SendCommand OMX_CommandPortEnable FAIL " << " port " << printOMXError(error);
			UnLock();
			return error;

		}
		else
		{
			if(wait)
			{
				error = WaitForCommand(OMX_CommandPortEnable, port, COMMAND_WAIT_TIMEOUT);
			}
		}
	}

	UnLock();

	return error;
}

OMX_ERRORTYPE COMXCoreComponent::DisablePort(unsigned int port, bool wait)//default: wait=false
{

	OMX_ERRORTYPE error = OMX_ErrorUndefined;

	Lock();
	//ofLogVerbose(__func__) << " componentName: "  << m_componentName << " START" << " port: " << port << " wait: " << wait;

	error = OMX_SendCommand(m_handle, OMX_CommandPortDisable, port, NULL);
	if(error == OMX_ErrorNone)
	{
		//ofLogVerbose(__func__) << m_componentName << " OMX_SendCommand OMX_CommandPortDisable PASS";
		UnLock();
		return error;
	}
	//ofLogVerbose(__func__) << m_componentName << "GOING THE LONG ROUTE";
	OMX_PARAM_PORTDEFINITIONTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = port;

	error = OMX_GetParameter(m_handle, OMX_IndexParamPortDefinition, &portFormat);
	if(error == OMX_ErrorNone)
	{
		//ofLogVerbose(__func__) << m_componentName << " OMX_GetParameter PASS";

	}
	else
	{
		ofLogError(__func__) << m_componentName << " OMX_GetParameter FAIL " << printOMXError(error);
	}


	if(portFormat.bEnabled == OMX_TRUE)
	{
		error = OMX_SendCommand(m_handle, OMX_CommandPortDisable, port, NULL);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << m_componentName << "OMX_SendCommand OMX_CommandPortDisable FAIL " << " port " << printOMXError(error);
			UnLock();
			return error;
		}
		else
		{
			if(wait)
			{
				error = WaitForCommand(OMX_CommandPortDisable, port, COMMAND_WAIT_TIMEOUT);
				if(error != OMX_ErrorNone)
				{
					ofLogError(__func__) << m_componentName << "WaitForCommand OMX_CommandPortDisable FAIL " << " port " << printOMXError(error);
					UnLock();
					return error;
				}
			}
		}
	}
	//ofLogVerbose(__func__) << " componentName: "  << m_componentName << " END" << " port: " << port << " wait: " << wait;
	UnLock();
	return error;
}

OMX_ERRORTYPE COMXCoreComponent::UseEGLImage(OMX_BUFFERHEADERTYPE** ppBufferHdr, OMX_U32 nPortIndex, OMX_PTR pAppPrivate, void* eglImage)
{
	Lock();

	OMX_ERRORTYPE error = OMX_UseEGLImage(m_handle, ppBufferHdr, nPortIndex, pAppPrivate, eglImage);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " FAIL " << printOMXError(error);
	}

	UnLock();
	return error;
}

bool COMXCoreComponent::Initialize( const std::string& component_name, OMX_INDEXTYPE index)
{
	//ofLogVerbose(__func__) << " component_name: " << component_name;

	m_componentName = component_name;

	m_callbacks.EventHandler    = &COMXCoreComponent::DecoderEventHandlerCallback;
	m_callbacks.EmptyBufferDone = &COMXCoreComponent::DecoderEmptyBufferDoneCallback;
	m_callbacks.FillBufferDone  = &COMXCoreComponent::DecoderFillBufferDoneCallback;

	// Get video component handle setting up callbacks, component is in loaded state on return.
	OMX_ERRORTYPE error = OMX_GetHandle(&m_handle, (char*)component_name.c_str(), this, &m_callbacks);

	if (error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << "OMX_GetHandle FAIL " << printOMXError(error);
		Deinitialize();
		return false;
	}

	OMX_PORT_PARAM_TYPE port_param;
	OMX_INIT_STRUCTURE(port_param);

	error = OMX_GetParameter(m_handle, index, &port_param);
	if (error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " OMX_GetParameter FAIL " << printOMXError(error);
	}

	error = DisableAllPorts();
	if (error != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " DisableAllPorts FAIL " << printOMXError(error);
	}

	m_input_port  = port_param.nStartPortNumber;
	m_output_port = m_input_port + 1;

	if(m_componentName == "OMX.broadcom.audio_mixer")
	{
		m_input_port  = port_param.nStartPortNumber + 1;
		m_output_port = port_param.nStartPortNumber;
	}

	if (m_output_port > port_param.nStartPortNumber+port_param.nPorts-1)
	{
		m_output_port = port_param.nStartPortNumber+port_param.nPorts-1;
	}
	//ofLogVerbose(__func__) << " componentName: "  << m_componentName << " Initialized" << " input port: " << m_input_port << " output_port: " << m_output_port;

	m_exit = false;
	m_flush_input   = false;
	m_flush_output  = false;

	return true;
}

bool COMXCoreComponent::Deinitialize(bool doFlush)//doFlush default: true
{
	m_exit = true;

	m_flush_input   = true;
	m_flush_output  = true;

	if(m_handle)
	{
		if(doFlush)
		{
			FlushAll();
		}


		FreeOutputBuffers(true);
		FreeInputBuffers(true);

		if(GetState() == OMX_StateExecuting)
		{
			SetStateForComponent(OMX_StatePause);
		}

		if(GetState() != OMX_StateIdle)
		{
			SetStateForComponent(OMX_StateIdle);
		}

		if(GetState() != OMX_StateLoaded)
		{
			SetStateForComponent(OMX_StateLoaded);
		}

		OMX_ERRORTYPE error = OMX_FreeHandle(m_handle);
		if (error != OMX_ErrorNone)
		{
			ofLogError(__func__) << m_componentName << " OMX_FreeHandle FAIL " << printOMXError(error);
		}

		m_handle = NULL;
	}

	m_input_port    = 0;
	m_output_port   = 0;
	m_componentName = "";

	CustomDecoderFillBufferDoneHandler = NULL;
	CustomDecoderEmptyBufferDoneHandler = NULL;



	return true;
}

// DecoderEventHandler -- OMX event callback
OMX_ERRORTYPE COMXCoreComponent::DecoderEventHandlerCallback(
    OMX_HANDLETYPE hComponent,
    OMX_PTR pAppData,
    OMX_EVENTTYPE eEvent,
    OMX_U32 nData1,
    OMX_U32 nData2,
    OMX_PTR pEventData)
{
	if(!pAppData)
	{
		return OMX_ErrorNone;
	}

	if (eEvent == OMX_EventPortSettingsChanged )
	{
		//ofLogVerbose(__func__) << "OMX_EventPortSettingsChanged at ofGetElapsedTimeMillis: " << ofGetElapsedTimeMillis();
	}
	COMXCoreComponent *ctx = static_cast<COMXCoreComponent*>(pAppData);
	return ctx->DecoderEventHandler(hComponent, pAppData, eEvent, nData1, nData2, pEventData);
}

// DecoderEmptyBufferDone -- OMXCore input buffer has been emptied
OMX_ERRORTYPE COMXCoreComponent::DecoderEmptyBufferDoneCallback(
    OMX_HANDLETYPE hComponent,
    OMX_PTR pAppData,
    OMX_BUFFERHEADERTYPE* pBuffer)
{
	if(!pAppData)
	{
		return OMX_ErrorNone;
	}

	COMXCoreComponent *ctx = static_cast<COMXCoreComponent*>(pAppData);
	#ifdef OMX_DEBUG_EVENTS
		ofLogVerbose(__func__) << ctx->GetName();
	#endif
	if(ctx->CustomDecoderEmptyBufferDoneHandler)
	{
		OMX_ERRORTYPE error = (*(ctx->CustomDecoderEmptyBufferDoneHandler))(hComponent, pAppData, pBuffer);
		if(error != OMX_ErrorNone)
		{
			return error;
		}
	}

	return ctx->DecoderEmptyBufferDone( hComponent, pAppData, pBuffer);
}

// DecoderFillBufferDone -- OMXCore output buffer has been filled
OMX_ERRORTYPE COMXCoreComponent::DecoderFillBufferDoneCallback(
    OMX_HANDLETYPE hComponent,
    OMX_PTR pAppData,
    OMX_BUFFERHEADERTYPE* pBuffer)
{
	if(!pAppData)
	{
		return OMX_ErrorNone;
	}

	COMXCoreComponent *ctx = static_cast<COMXCoreComponent*>(pAppData);
	#ifdef OMX_DEBUG_EVENTS
		ofLogVerbose(__func__) << ctx->GetName();
	#endif
	if(ctx->CustomDecoderFillBufferDoneHandler)
	{
		OMX_ERRORTYPE error = (*(ctx->CustomDecoderFillBufferDoneHandler))(hComponent, pAppData, pBuffer);
		if(error != OMX_ErrorNone)
		{
			return error;
		}
	}

	return ctx->DecoderFillBufferDone(hComponent, pAppData, pBuffer);
}

OMX_ERRORTYPE COMXCoreComponent::DecoderEmptyBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
	if(!pAppData || m_exit)
	{
		return OMX_ErrorNone;
	}

	COMXCoreComponent *ctx = static_cast<COMXCoreComponent*>(pAppData);

	pthread_mutex_lock(&ctx->m_omx_input_mutex);
	ctx->m_omx_input_avaliable.push(pBuffer);

	// this allows (all) blocked tasks to be awoken
	pthread_cond_broadcast(&ctx->m_input_buffer_cond);

	pthread_mutex_unlock(&ctx->m_omx_input_mutex);

	return OMX_ErrorNone;
}

OMX_ERRORTYPE COMXCoreComponent::DecoderFillBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
	if(!pAppData || m_exit)
	{
		return OMX_ErrorNone;
	}

	COMXCoreComponent *ctx = static_cast<COMXCoreComponent*>(pAppData);

	pthread_mutex_lock(&ctx->m_omx_output_mutex);
	ctx->m_omx_output_available.push(pBuffer);

	// this allows (all) blocked tasks to be awoken
	pthread_cond_broadcast(&ctx->m_output_buffer_cond);

	pthread_mutex_unlock(&ctx->m_omx_output_mutex);

	sem_post(&ctx->m_omx_fill_buffer_done);

	return OMX_ErrorNone;
}

// DecoderEmptyBufferDone -- OMXCore input buffer has been emptied
OMX_ERRORTYPE COMXCoreComponent::DecoderEventHandler(
    OMX_HANDLETYPE hComponent,
    OMX_PTR pAppData,
    OMX_EVENTTYPE eEvent,
    OMX_U32 nData1,
    OMX_U32 nData2,
    OMX_PTR pEventData)
{
	COMXCoreComponent *ctx = static_cast<COMXCoreComponent*>(pAppData);

	#ifdef OMX_DEBUG_EVENTS
		ofLogVerbose(__func__) << printEventType(eEvent);
	#endif

	AddEvent(eEvent, nData1, nData2);
	if (eEvent == OMX_EventBufferFlag)
	{
		if(nData2 & OMX_BUFFERFLAG_EOS)
		{

			pthread_mutex_lock(&ctx->m_omx_eos_mutex);
			ofLogVerbose(__func__) << ctx->GetName() << " OMX_EventBufferFlag::OMX_BUFFERFLAG_EOS RECEIVED";
			ctx->m_eos = true;
			pthread_mutex_unlock(&ctx->m_omx_eos_mutex);
		}
		
	}
	else
	{
		if (eEvent == OMX_EventError)
		{
			ofLogVerbose(__func__) << printOMXError((OMX_ERRORTYPE) nData1);
			sem_post(&ctx->m_omx_fill_buffer_done);
		}

	}
	return OMX_ErrorNone;
}
