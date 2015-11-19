#include <math.h>
#include <sys/time.h>
#include "OMXCore.h"
#include "OMXClock.h"
#include "XMemUtils.h"

//#define OMX_DEBUG_EVENTS
//#define OMX_DEBUG_EVENTHANDLER
#define COMMAND_WAIT_TIMEOUT 20

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



#pragma mark COMXCoreTunnel

COMXCoreTunnel::COMXCoreTunnel()
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

COMXCoreTunnel::~COMXCoreTunnel()
{
	if(isEstablished)
	{
		Deestablish();
	}

	pthread_mutex_destroy(&m_lock);
}

void COMXCoreTunnel::Lock()
{
	pthread_mutex_lock(&m_lock);
}

void COMXCoreTunnel::UnLock()
{
	pthread_mutex_unlock(&m_lock);
}

void COMXCoreTunnel::Initialize(COMXCoreComponent *src_component, unsigned int src_port, COMXCoreComponent *dst_component, unsigned int dst_port)
{

	m_src_component  = src_component;
	m_src_port    = src_port;
	m_dst_component  = dst_component;
	m_dst_port    = dst_port;

	srcName = m_src_component->GetName();
	dstName = m_dst_component->GetName();
}

OMX_ERRORTYPE COMXCoreTunnel::Flush()
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

OMX_ERRORTYPE COMXCoreTunnel::Deestablish(bool doWait)
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
		OMX_TRACE(error);
	}
	if(m_src_component->GetComponent())
	{
		error = m_src_component->DisablePort(m_src_port);
		OMX_TRACE(error);
	}
	if(m_dst_component->GetComponent())
	{
		error = m_dst_component->DisablePort(m_dst_port);
		OMX_TRACE(error);
	}
	if(m_src_component->GetComponent())
	{
		error = OMX_SetupTunnel(m_src_component->GetComponent(), m_src_port, NULL, 0);
		OMX_TRACE(error);
	}

	if(m_dst_component->GetComponent())
	{
		error = OMX_SetupTunnel(m_dst_component->GetComponent(), m_dst_port, NULL, 0);
		OMX_TRACE(error);
	}
	UnLock();
	isEstablished = false;
	return OMX_ErrorNone;
}

OMX_ERRORTYPE COMXCoreTunnel::Establish(bool portSettingsChanged)
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
        OMX_TRACE(error);
		if(error != OMX_ErrorNone)
		{
			UnLock();
			return error;
		}
	}
    ofLogVerbose(__func__) << m_src_component->GetName() << " TUNNELING TO " << m_dst_component->GetName();
    ofLogVerbose(__func__) << "portSettingsChanged: " << portSettingsChanged;
    
	if(portSettingsChanged)
	{
		error = m_src_component->WaitForEvent(OMX_EventPortSettingsChanged);
        OMX_TRACE(error);
		if(error != OMX_ErrorNone)
		{
			UnLock();
			return error;
		}
	}
	if(m_src_component->GetComponent())
	{
		error = m_src_component->DisablePort(m_src_port);
		OMX_TRACE(error);
	}

	if(m_dst_component->GetComponent())
	{
		error = m_dst_component->DisablePort(m_dst_port);
		OMX_TRACE(error);
	}

	if(m_src_component->GetComponent() && m_dst_component->GetComponent())
	{
		error = OMX_SetupTunnel(m_src_component->GetComponent(), m_src_port, m_dst_component->GetComponent(), m_dst_port);
        OMX_TRACE(error);
		if(error != OMX_ErrorNone)
		{
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
		UnLock();
		return OMX_ErrorUndefined;
	}

	if(m_src_component->GetComponent())
	{
		error = m_src_component->EnablePort(m_src_port, doWait);
        OMX_TRACE(error);
		if(error != OMX_ErrorNone)
		{
			UnLock();
			return error;
		}
	}

	if(m_dst_component->GetComponent())
	{
		error = m_dst_component->EnablePort(m_dst_port, doWait);
        OMX_TRACE(error);
		if(error != OMX_ErrorNone)
		{
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
            OMX_TRACE(error);
			if(error != OMX_ErrorNone)
			{
				UnLock();
				return error;
			}
			error = m_dst_component->SetStateForComponent(OMX_StateIdle);
            OMX_TRACE(error);
			if(error != OMX_ErrorNone)
			{
				UnLock();
				return error;
			}
		}
	}

	if(m_src_component->GetComponent())
	{
		error = m_src_component->WaitForCommand(OMX_CommandPortEnable, m_src_port, COMMAND_WAIT_TIMEOUT);
		if(error != OMX_ErrorNone)
		{
			UnLock();
			return error;
		}
	}

	m_portSettingsChanged = portSettingsChanged;

	UnLock();


	return error;
}

#pragma mark COMXCoreComponent

COMXCoreComponent::COMXCoreComponent()
{
	
	frameCounter = 0;
	frameOffset = 0;
	m_input_port  = 0;
	m_output_port = 0;
	m_handle      = NULL;
    m_input_buffer_size = 0;
	
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
    assert(!m_handle);

	if(!omx_buffer)
	{
		return OMX_ErrorUndefined;
	}
	OMX_ERRORTYPE error = OMX_EmptyThisBuffer(m_handle, omx_buffer);
	OMX_TRACE(error);

	return error;
}

OMX_ERRORTYPE COMXCoreComponent::FillThisBuffer(OMX_BUFFERHEADERTYPE *omx_buffer)
{
    assert(!m_handle);

	if(!omx_buffer)
	{
		return OMX_ErrorUndefined;
	}

	OMX_ERRORTYPE error = OMX_FillThisBuffer(m_handle, omx_buffer);
	OMX_TRACE(error);

	return error;
}

OMX_ERRORTYPE COMXCoreComponent::FreeOutputBuffer(OMX_BUFFERHEADERTYPE *omx_buffer)
{
    assert(!m_handle);
    
    if(!omx_buffer)
    {
        return OMX_ErrorUndefined;
    }
    
    OMX_ERRORTYPE error = OMX_FreeBuffer(m_handle, m_output_port, omx_buffer);
	OMX_TRACE(error);

	return error;
}


void COMXCoreComponent::FlushAll()
{
	FlushInput();
	FlushOutput();
}

void COMXCoreComponent::FlushInput()
{
	
	Lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;
	error = OMX_SendCommand(m_handle, OMX_CommandFlush, m_input_port, NULL);
    OMX_TRACE(error);
	
	UnLock();
}

void COMXCoreComponent::FlushOutput()
{

	Lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;
	error = OMX_SendCommand(m_handle, OMX_CommandFlush, m_output_port, NULL);
	OMX_TRACE(error);
    

	UnLock();
}

// timeout in milliseconds
OMX_BUFFERHEADERTYPE *COMXCoreComponent::GetInputBuffer(long timeout)
{
	assert(!m_handle);
    
    OMX_BUFFERHEADERTYPE *omx_input_buffer = NULL;

	pthread_mutex_lock(&m_omx_input_mutex);
	struct timespec endtime;
	clock_gettime(CLOCK_REALTIME, &endtime);
	add_timespecs(endtime, timeout);
	while (1 && !m_flush_input)
	{
		if(!m_omx_input_available.empty())
		{
			omx_input_buffer = m_omx_input_available.front();
			m_omx_input_available.pop();
			break;
		}

		int retcode = pthread_cond_timedwait(&m_input_buffer_cond, &m_omx_input_mutex, &endtime);
		if (retcode != 0)
		{
			//ofLogError(__func__) << m_componentName << " TIMEOUT";
			break;
		}
	}
	pthread_mutex_unlock(&m_omx_input_mutex);

	return omx_input_buffer;
}

OMX_BUFFERHEADERTYPE *COMXCoreComponent::GetOutputBuffer()
{
    assert(!m_handle);
	OMX_BUFFERHEADERTYPE *omx_output_buffer = NULL;


	pthread_mutex_lock(&m_omx_output_mutex);
	if(!m_omx_output_available.empty())
	{
		omx_output_buffer = m_omx_output_available.front();
		m_omx_output_available.pop();
	}
	pthread_mutex_unlock(&m_omx_output_mutex);

	return omx_output_buffer;
}

OMX_ERRORTYPE COMXCoreComponent::AllocInputBuffers()
{
	OMX_ERRORTYPE error = OMX_ErrorNone;
    ofLogVerbose(__func__) << m_componentName;

	assert(!m_handle);
    
	OMX_PARAM_PORTDEFINITIONTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = m_input_port;

	error = OMX_GetParameter(m_handle, OMX_IndexParamPortDefinition, &portFormat);
	OMX_TRACE(error);

    m_input_buffer_size   = portFormat.nBufferSize;
    
	if(GetState() != OMX_StateIdle)
	{
		if(GetState() != OMX_StateLoaded)
		{
			SetStateForComponent(OMX_StateLoaded);
		}
		SetStateForComponent(OMX_StateIdle);
	}
    
    
	error = EnablePort(m_input_port, false);
    OMX_TRACE(error);
	if(error != OMX_ErrorNone)
	{
		return error;
	}

	for (size_t i = 0; i < portFormat.nBufferCountActual; i++)
	{
		OMX_BUFFERHEADERTYPE *buffer = NULL;
        error = OMX_AllocateBuffer(m_handle, &buffer, m_input_port, NULL, portFormat.nBufferSize);
        OMX_TRACE(error);

		buffer->nInputPortIndex = m_input_port;
		buffer->nFilledLen      = 0;
		buffer->nOffset         = 0;
		buffer->pAppPrivate     = (void*)i;
		m_omx_input_buffers.push_back(buffer);
		m_omx_input_available.push(buffer);
	}

	m_flush_input = false;

	return error;
}

OMX_ERRORTYPE COMXCoreComponent::AllocOutputBuffers()
{
	
    assert(!m_handle);
    OMX_ERRORTYPE error = OMX_ErrorNone;
	if(!m_handle)
	{
		ofLogError(__func__) << "NO HANDLE";
		return OMX_ErrorUndefined;
	}


	OMX_PARAM_PORTDEFINITIONTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = m_output_port;

	error = OMX_GetParameter(m_handle, OMX_IndexParamPortDefinition, &portFormat);
    OMX_TRACE(error);
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

	error = EnablePort(m_output_port, false);
    OMX_TRACE(error);
	if(error != OMX_ErrorNone)
	{
		return error;
	}

	for (size_t i = 0; i < portFormat.nBufferCountActual; i++)
	{
		OMX_BUFFERHEADERTYPE *buffer = NULL;
        error = OMX_AllocateBuffer(m_handle, &buffer, m_output_port, NULL, portFormat.nBufferSize);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone)
        {
            return error;
        }
		buffer->nOutputPortIndex = m_output_port;
		buffer->nFilledLen       = 0;
		buffer->nOffset          = 0;
		buffer->pAppPrivate      = (void*)i;
        m_omx_output_buffers.push_back(buffer);
        m_omx_output_available.push(buffer);
	}

	m_flush_output = false;

	return error;
}
OMX_ERRORTYPE COMXCoreComponent::DisableAllPorts()
{
    assert(!m_handle);
    
	Lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;

	

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
					//ofLogError(__func__) << m_componentName << " OMX_SendCommand OMX_CommandPortDisable FAIL: " << printOMXError(error);
				}
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

	return OMX_ErrorNone;
}


// timeout in milliseconds
OMX_ERRORTYPE COMXCoreComponent::WaitForEvent(OMX_EVENTTYPE eventType, long timeout)
{
    ofLogVerbose(__func__) << m_componentName << " eventType: " << eventType;


	pthread_mutex_lock(&m_omx_event_mutex);
	struct timespec endtime;
	clock_gettime(CLOCK_REALTIME, &endtime);
	add_timespecs(endtime, timeout);
	while(true)
	{
		for (vector<omx_event>::iterator it = m_omx_events.begin(); it != m_omx_events.end(); it++)
		{
			omx_event event = *it;
		
            
            //Same state - disregard
			if(event.eEvent == OMX_EventError && event.nData1 == (OMX_U32)OMX_ErrorSameState && event.nData2 == 1)
			{
				m_omx_events.erase(it);
				pthread_mutex_unlock(&m_omx_event_mutex);
				return OMX_ErrorNone;
            }
            else
            {
                if(event.eEvent == OMX_EventError)
                {
                    m_omx_events.erase(it);
                    pthread_mutex_unlock(&m_omx_event_mutex);
                    return (OMX_ERRORTYPE)event.nData1;
                }
                else
                {
                    //have the event we are looking for 
                    if(event.eEvent == eventType)
                    {
                        #ifdef OMX_DEBUG_EVENTS
                        stringstream finishedInfo;
                        finishedInfo << m_componentName << "\n";
                        finishedInfo << "RECEIVED EVENT, REMOVING" << "\n";
                        finishedInfo << "event.eEvent: " << OMX_Maps::getInstance().getEvent(event.eEvent) << "\n";
                        finishedInfo << "event.nData1: " << event.nData1 << "\n";
                        finishedInfo << "event.nData2: " << event.nData2 << "\n";
                        ofLogVerbose(__func__) << finishedInfo.str();
                        #endif
                        m_omx_events.erase(it);
                        pthread_mutex_unlock(&m_omx_event_mutex);
                        return OMX_ErrorNone;
                    }
                }
            }
        }

        int retcode = pthread_cond_timedwait(&m_omx_event_cond, &m_omx_event_mutex, &endtime);
        if (retcode != 0)
        {
            //ofLogError(__func__) << m_componentName << " WaitForEvent Event: " << OMX_Maps::getInstance().getEvent(event.eEvent) << " TIMED OUT at: " << timeout;
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

    ofLogVerbose(__func__) << m_componentName << " command " << OMX_Maps::getInstance().commandNames[command];
    
	pthread_mutex_lock(&m_omx_event_mutex);
	struct timespec endtime;
	clock_gettime(CLOCK_REALTIME, &endtime);
	add_timespecs(endtime, timeout);
    OMX_EVENTTYPE eEvent = OMX_EventError;
	while(true)
	{
		for (std::vector<omx_event>::iterator it = m_omx_events.begin(); it != m_omx_events.end(); it++)
		{
			omx_event event = *it;
            eEvent = event.eEvent;

            //Ignore same state
			if(event.eEvent == OMX_EventError && event.nData1 == (OMX_U32)OMX_ErrorSameState && event.nData2 == 1)
			{
				m_omx_events.erase(it);
				pthread_mutex_unlock(&m_omx_event_mutex);
				return OMX_ErrorNone;
			}
            else
            {
                //Throw error
                if(event.eEvent == OMX_EventError)
                {                    
                    OMX_TRACE((OMX_ERRORTYPE)event.nData1);
                    m_omx_events.erase(it);
                    pthread_mutex_unlock(&m_omx_event_mutex);
                    return (OMX_ERRORTYPE)event.nData1;
                }
                else
                {
                    //Not an error amd the data we want
                    if(event.eEvent == OMX_EventCmdComplete && event.nData1 == command && event.nData2 == nData2)
                    {
                        m_omx_events.erase(it);
                        pthread_mutex_unlock(&m_omx_event_mutex);
                        return OMX_ErrorNone;
                    }
                }
            }
		}

		int retcode = pthread_cond_timedwait(&m_omx_event_cond, &m_omx_event_mutex, &endtime);
		if (retcode != 0)
		{
			ofLog(OF_LOG_VERBOSE,
			      "COMXCoreComponent::WaitForCommand %s		\n \
				  wait timeout event.eEvent %s              \n \
				  event.command 0x%08x						\n \
				  event.nData2 %d\n",
			      m_componentName.c_str(),
			      OMX_Maps::getInstance().getEvent(eEvent).c_str(),
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
	assert(!m_handle);

	Lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_STATETYPE state_actual = OMX_StateMax;
    ofLogVerbose(__func__) << m_componentName << " state requested: " << OMX_Maps::getInstance().omxStateNames[state];
	if(state == state_actual)
	{
		UnLock();
		return OMX_ErrorNone;
	}

	error = OMX_SendCommand(m_handle, OMX_CommandStateSet, state, 0);
    OMX_TRACE(error);
	if (error != OMX_ErrorNone)
	{
		if(error == OMX_ErrorSameState)
		{
			error = OMX_ErrorNone;
		}
	}
	else
	{
		error = WaitForCommand(OMX_CommandStateSet, state, COMMAND_WAIT_TIMEOUT);
        OMX_TRACE(error);
		if(error == OMX_ErrorSameState)
		{
			UnLock();
			return OMX_ErrorNone;
		}
	}

	UnLock();

	return error;
}

unsigned int COMXCoreComponent::GetInputBufferSpace()
{
    int free = m_omx_input_available.size() * m_input_buffer_size;
    return free;
}

OMX_STATETYPE COMXCoreComponent::GetState()
{
    Lock();
    
    OMX_STATETYPE state;
    OMX_ERRORTYPE error = OMX_GetState(m_handle, &state);
    OMX_TRACE(error);
    
    UnLock();
    return state;

}

OMX_ERRORTYPE COMXCoreComponent::SetParameter(OMX_INDEXTYPE paramIndex, OMX_PTR paramStruct)
{
	Lock();

	OMX_ERRORTYPE error = OMX_SetParameter(m_handle, paramIndex, paramStruct);
    OMX_TRACE(error);
    
	UnLock();
	return error;
}

OMX_ERRORTYPE COMXCoreComponent::GetParameter(OMX_INDEXTYPE paramIndex, OMX_PTR paramStruct)
{
	Lock();

	OMX_ERRORTYPE error = OMX_GetParameter(m_handle, paramIndex, paramStruct);
    OMX_TRACE(error);


	UnLock();
	return error;
}

OMX_ERRORTYPE COMXCoreComponent::SetConfig(OMX_INDEXTYPE configIndex, OMX_PTR configStruct)
{
    Lock();
    
	OMX_ERRORTYPE error = OMX_SetConfig(m_handle, configIndex, configStruct);
    OMX_TRACE(error);

	UnLock();
	return error;
}

OMX_ERRORTYPE COMXCoreComponent::GetConfig(OMX_INDEXTYPE configIndex, OMX_PTR configStruct)
{
	Lock();

	OMX_ERRORTYPE error = OMX_GetConfig(m_handle, configIndex, configStruct);
    OMX_TRACE(error);
    
	UnLock();
	return error;
}

OMX_ERRORTYPE COMXCoreComponent::SendCommand(OMX_COMMANDTYPE cmd, OMX_U32 cmdParam, OMX_PTR cmdParamData)
{
	Lock();

	OMX_ERRORTYPE error = OMX_SendCommand(m_handle, cmd, cmdParam, cmdParamData);
    OMX_TRACE(error);
    
	UnLock();
	return error;
}

OMX_ERRORTYPE COMXCoreComponent::EnablePort(unsigned int port,  bool wait)//default: wait=false
{
	Lock();
	
	OMX_PARAM_PORTDEFINITIONTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = port;

	OMX_ERRORTYPE error = OMX_GetParameter(m_handle, OMX_IndexParamPortDefinition, &portFormat);
    OMX_TRACE(error);

    ofLogVerbose(__func__) << m_componentName << " port: " << port << " wait:" << wait;
    
	if(portFormat.bEnabled == OMX_FALSE)
	{
		error = OMX_SendCommand(m_handle, OMX_CommandPortEnable, port, NULL);
        OMX_TRACE(error);

		if(error != OMX_ErrorNone)
		{
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
    ofLogVerbose(__func__) << m_componentName << " port: " << port << " wait:" << wait;
	error = OMX_SendCommand(m_handle, OMX_CommandPortDisable, port, NULL);
    OMX_TRACE(error);
	if(error == OMX_ErrorNone)
	{
		UnLock();
		return error;
	}
	OMX_PARAM_PORTDEFINITIONTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = port;

	error = OMX_GetParameter(m_handle, OMX_IndexParamPortDefinition, &portFormat);
    OMX_TRACE(error);



	if(portFormat.bEnabled == OMX_TRUE)
	{
		error = OMX_SendCommand(m_handle, OMX_CommandPortDisable, port, NULL);
        OMX_TRACE(error);

		if(error != OMX_ErrorNone)
		{
			UnLock();
			return error;
		}
		else
		{
			if(wait)
			{
				error = WaitForCommand(OMX_CommandPortDisable, port, COMMAND_WAIT_TIMEOUT);
                OMX_TRACE(error);
				if(error != OMX_ErrorNone)
				{
					UnLock();
					return error;
				}
			}
		}
	}
	UnLock();
	return error;
}

OMX_ERRORTYPE COMXCoreComponent::UseEGLImage(OMX_BUFFERHEADERTYPE** ppBufferHdr, OMX_U32 nPortIndex, OMX_PTR pAppPrivate, void* eglImage)
{
	Lock();

	OMX_ERRORTYPE error = OMX_UseEGLImage(m_handle, ppBufferHdr, nPortIndex, pAppPrivate, eglImage);
    OMX_TRACE(error);

	UnLock();
	return error;
}

bool COMXCoreComponent::Initialize( const std::string& component_name, OMX_INDEXTYPE index)
{

	m_componentName = component_name;

	m_callbacks.EventHandler    = &COMXCoreComponent::DecoderEventHandlerCallback;
	m_callbacks.EmptyBufferDone = &COMXCoreComponent::DecoderEmptyBufferDoneCallback;
	m_callbacks.FillBufferDone  = &COMXCoreComponent::DecoderFillBufferDoneCallback;

	// Get video component handle setting up callbacks, component is in loaded state on return.
	OMX_ERRORTYPE error = OMX_GetHandle(&m_handle, (char*)component_name.c_str(), this, &m_callbacks);
    OMX_TRACE(error);
    
	if (error != OMX_ErrorNone)
	{
		Deinitialize();
		return false;
	}
    ofLogVerbose(__func__) << m_componentName << " PASS ";
    
	OMX_PORT_PARAM_TYPE port_param;
	OMX_INIT_STRUCTURE(port_param);

	error = OMX_GetParameter(m_handle, index, &port_param);
    OMX_TRACE(error);


	error = DisableAllPorts();
    OMX_TRACE(error);


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


		//FreeOutputBuffers(true);
		//FreeInputBuffers(true);

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
        OMX_TRACE(error);


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
	if(ctx->CustomDecoderFillBufferDoneHandler)
	{
		OMX_ERRORTYPE error = (*(ctx->CustomDecoderFillBufferDoneHandler))(hComponent, pAppData, pBuffer);
        OMX_TRACE(error);
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
	ctx->m_omx_input_available.push(pBuffer);

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
	AddEvent(eEvent, nData1, nData2);
	if (eEvent == OMX_EventBufferFlag)
	{
		if(nData2 & OMX_BUFFERFLAG_EOS)
		{

			pthread_mutex_lock(&ctx->m_omx_eos_mutex);
			ctx->m_eos = true;
			pthread_mutex_unlock(&ctx->m_omx_eos_mutex);
		}
		
	}
	else
	{
		if (eEvent == OMX_EventError)
		{
            OMX_TRACE((OMX_ERRORTYPE) nData1);
			sem_post(&ctx->m_omx_fill_buffer_done);
		}

	}
	return OMX_ErrorNone;
}
