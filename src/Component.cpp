

#include "Component.h"


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




#pragma mark Component

Component::Component()
{
	
	frameCounter = 0;
	frameOffset = 0;
	inputPort  = 0;
	outputPort = 0;
	handle      = NULL;
    m_input_buffer_size = 0;
	
	doFlushInput         = false;
	doFlushOutput        = false;

	CustomDecoderFillBufferDoneHandler = NULL;
	CustomDecoderEmptyBufferDoneHandler = NULL;

	m_eos                 = false;

	doExit = false;

	pthread_mutex_init(&m_omx_input_mutex, NULL);
	pthread_mutex_init(&m_omx_output_mutex, NULL);
	pthread_mutex_init(&event_mutex, NULL);
	pthread_mutex_init(&eos_mutex, NULL);
	pthread_cond_init(&m_input_buffer_cond, NULL);
	pthread_cond_init(&m_output_buffer_cond, NULL);
	pthread_cond_init(&m_omx_event_cond, NULL);


	pthread_mutex_init(&m_lock, NULL);
	sem_init(&m_omx_fill_buffer_done, 0, 0);
}

Component::~Component()
{
	Deinitialize();

	pthread_mutex_destroy(&m_omx_input_mutex);
	pthread_mutex_destroy(&m_omx_output_mutex);
	pthread_mutex_destroy(&event_mutex);
	pthread_mutex_destroy(&eos_mutex);
	pthread_cond_destroy(&m_input_buffer_cond);
	pthread_cond_destroy(&m_output_buffer_cond);
	pthread_cond_destroy(&m_omx_event_cond);

	pthread_mutex_destroy(&m_lock);
	sem_destroy(&m_omx_fill_buffer_done);

}


int Component::getCurrentFrame()
{
	return frameCounter;
	
	//return frameCounter-frameOffset;
}

void Component::resetFrameCounter()
{
	frameOffset = frameCounter;
	frameCounter = 0;
}

void Component::incrementFrameCounter()
{
	frameCounter++;
}

void Component::resetEOS()
{
	pthread_mutex_lock(&eos_mutex);
	m_eos = false;
	pthread_mutex_unlock(&eos_mutex);
}


void Component::setEOS(bool isEndofStream)
{
	m_eos = isEndofStream;
}
void Component::lock()
{
	pthread_mutex_lock(&m_lock);
}

void Component::unlock()
{
	pthread_mutex_unlock(&m_lock);
}

OMX_ERRORTYPE Component::EmptyThisBuffer(OMX_BUFFERHEADERTYPE *omxBuffer)
{
    assert(!handle);

	if(!omxBuffer)
	{
		return OMX_ErrorUndefined;
	}
	OMX_ERRORTYPE error = OMX_EmptyThisBuffer(handle, omxBuffer);
	OMX_TRACE(error);

	return error;
}

OMX_ERRORTYPE Component::FillThisBuffer(OMX_BUFFERHEADERTYPE *omxBuffer)
{
    assert(!handle);

	if(!omxBuffer)
	{
		return OMX_ErrorUndefined;
	}

	OMX_ERRORTYPE error = OMX_FillThisBuffer(handle, omxBuffer);
	OMX_TRACE(error);

	return error;
}

OMX_ERRORTYPE Component::FreeOutputBuffer(OMX_BUFFERHEADERTYPE *omxBuffer)
{
    assert(!handle);
    
    if(!omxBuffer)
    {
        return OMX_ErrorUndefined;
    }
    
    OMX_ERRORTYPE error = OMX_FreeBuffer(handle, outputPort, omxBuffer);
	OMX_TRACE(error);

	return error;
}


void Component::flushAll()
{
	flushInput();
	flushOutput();
}

void Component::flushInput()
{
	
	lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;
	error = OMX_SendCommand(handle, OMX_CommandFlush, inputPort, NULL);
    OMX_TRACE(error);
	
	unlock();
}

void Component::flushOutput()
{

	lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;
	error = OMX_SendCommand(handle, OMX_CommandFlush, outputPort, NULL);
	OMX_TRACE(error);
    

	unlock();
}

// timeout in milliseconds
OMX_BUFFERHEADERTYPE *Component::getInputBuffer(long timeout)
{
	assert(!handle);
    
    OMX_BUFFERHEADERTYPE *omx_input_buffer = NULL;

	pthread_mutex_lock(&m_omx_input_mutex);
	struct timespec endtime;
	clock_gettime(CLOCK_REALTIME, &endtime);
	add_timespecs(endtime, timeout);
	while (1 && !doFlushInput)
	{
		if(!inputBuffersAvailable.empty())
		{
			omx_input_buffer = inputBuffersAvailable.front();
			inputBuffersAvailable.pop();
			break;
		}

		int retcode = pthread_cond_timedwait(&m_input_buffer_cond, &m_omx_input_mutex, &endtime);
		if (retcode != 0)
		{
			//ofLogError(__func__) << componentName << " TIMEOUT";
			break;
		}
	}
	pthread_mutex_unlock(&m_omx_input_mutex);

	return omx_input_buffer;
}

OMX_BUFFERHEADERTYPE *Component::getOutputBuffer()
{
    assert(!handle);
	OMX_BUFFERHEADERTYPE *omx_output_buffer = NULL;


	pthread_mutex_lock(&m_omx_output_mutex);
	if(!outputBuffersAvailable.empty())
	{
		omx_output_buffer = outputBuffersAvailable.front();
		outputBuffersAvailable.pop();
	}
	pthread_mutex_unlock(&m_omx_output_mutex);

	return omx_output_buffer;
}

OMX_ERRORTYPE Component::allocInputBuffers()
{
	OMX_ERRORTYPE error = OMX_ErrorNone;
    ofLogVerbose(__func__) << componentName;

	assert(!handle);
    
	OMX_PARAM_PORTDEFINITIONTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = inputPort;

	error = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &portFormat);
	OMX_TRACE(error);

    m_input_buffer_size   = portFormat.nBufferSize;
    
	if(getState() != OMX_StateIdle)
	{
		if(getState() != OMX_StateLoaded)
		{
			setState(OMX_StateLoaded);
		}
		setState(OMX_StateIdle);
	}
    
    
	error = enablePort(inputPort);
    OMX_TRACE(error);
	if(error != OMX_ErrorNone)
	{
		return error;
	}

	for (size_t i = 0; i < portFormat.nBufferCountActual; i++)
	{
		OMX_BUFFERHEADERTYPE *buffer = NULL;
        error = OMX_AllocateBuffer(handle, &buffer, inputPort, NULL, portFormat.nBufferSize);
        OMX_TRACE(error);

		buffer->nInputPortIndex = inputPort;
		buffer->nFilledLen      = 0;
		buffer->nOffset         = 0;
		buffer->pAppPrivate     = (void*)i;
		inputBuffers.push_back(buffer);
		inputBuffersAvailable.push(buffer);
	}

	doFlushInput = false;

	return error;
}

OMX_ERRORTYPE Component::allocOutputBuffers()
{
	
    assert(!handle);
    OMX_ERRORTYPE error = OMX_ErrorNone;
	if(!handle)
	{
		ofLogError(__func__) << "NO HANDLE";
		return OMX_ErrorUndefined;
	}


	OMX_PARAM_PORTDEFINITIONTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = outputPort;

	error = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &portFormat);
    OMX_TRACE(error);
	if(error != OMX_ErrorNone)
	{
		return error;
	}

	if(getState() != OMX_StateIdle)
	{
		if(getState() != OMX_StateLoaded)
		{
			setState(OMX_StateLoaded);
		}
		setState(OMX_StateIdle);
	}

	error = enablePort(outputPort);
    OMX_TRACE(error);
	if(error != OMX_ErrorNone)
	{
		return error;
	}

	for (size_t i = 0; i < portFormat.nBufferCountActual; i++)
	{
		OMX_BUFFERHEADERTYPE *buffer = NULL;
        error = OMX_AllocateBuffer(handle, &buffer, outputPort, NULL, portFormat.nBufferSize);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone)
        {
            return error;
        }
		buffer->nOutputPortIndex = outputPort;
		buffer->nFilledLen       = 0;
		buffer->nOffset          = 0;
		buffer->pAppPrivate      = (void*)i;
        outputBuffers.push_back(buffer);
        outputBuffersAvailable.push(buffer);
	}

	doFlushOutput = false;

	return error;
}
OMX_ERRORTYPE Component::disableAllPorts()
{
    assert(!handle);
    
	lock();

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
		error = OMX_GetParameter(handle, idxTypes[i], &ports);
		if(error == OMX_ErrorNone)
		{

			uint32_t j;
			for(j=0; j<ports.nPorts; j++)
			{
				OMX_PARAM_PORTDEFINITIONTYPE portFormat;
				OMX_INIT_STRUCTURE(portFormat);
				portFormat.nPortIndex = ports.nStartPortNumber+j;

				error = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &portFormat);
				if(error != OMX_ErrorNone)
				{
					if(portFormat.bEnabled == OMX_FALSE)
					{
						continue;
					}
				}

				error = OMX_SendCommand(handle, OMX_CommandPortDisable, ports.nStartPortNumber+j, NULL);
				if(error != OMX_ErrorNone)
				{
					//ofLogError(__func__) << componentName << " OMX_SendCommand OMX_CommandPortDisable FAIL: " << printOMXError(error);
				}
			}
		}
	}

	unlock();

	return OMX_ErrorNone;
}

void Component::Remove(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2)
{
	for (std::vector<OMXEvent>::iterator it = omxEvents.begin(); it != omxEvents.end(); )
	{
		OMXEvent event = *it;

		if(event.eEvent == eEvent && event.nData1 == nData1 && event.nData2 == nData2)
		{
			it = omxEvents.erase(it);
			continue;
		}
		++it;
	}
}

OMX_ERRORTYPE Component::addEvent(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2)
{
	OMXEvent event;

	event.eEvent      = eEvent;
	event.nData1      = nData1;
	event.nData2      = nData2;

	pthread_mutex_lock(&event_mutex);
	Remove(eEvent, nData1, nData2);
	omxEvents.push_back(event);
	// this allows (all) blocked tasks to be awoken
	pthread_cond_broadcast(&m_omx_event_cond);
	pthread_mutex_unlock(&event_mutex);

	return OMX_ErrorNone;
}


// timeout in milliseconds
OMX_ERRORTYPE Component::waitForEvent(OMX_EVENTTYPE eventType, long timeout)
{
    ofLogVerbose(__func__) << componentName << " eventType: " << eventType;


	pthread_mutex_lock(&event_mutex);
	struct timespec endtime;
	clock_gettime(CLOCK_REALTIME, &endtime);
	add_timespecs(endtime, timeout);
	while(true)
	{
		for (vector<OMXEvent>::iterator it = omxEvents.begin(); it != omxEvents.end(); it++)
		{
			OMXEvent event = *it;
		
            
            //Same state - disregard
			if(event.eEvent == OMX_EventError && event.nData1 == (OMX_U32)OMX_ErrorSameState && event.nData2 == 1)
			{
				omxEvents.erase(it);
				pthread_mutex_unlock(&event_mutex);
				return OMX_ErrorNone;
            }
            else
            {
                if(event.eEvent == OMX_EventError)
                {
                    omxEvents.erase(it);
                    pthread_mutex_unlock(&event_mutex);
                    return (OMX_ERRORTYPE)event.nData1;
                }
                else
                {
                    //have the event we are looking for 
                    if(event.eEvent == eventType)
                    {
                        #ifdef OMX_DEBUG_EVENTS
                        stringstream finishedInfo;
                        finishedInfo << componentName << "\n";
                        finishedInfo << "RECEIVED EVENT, REMOVING" << "\n";
                        finishedInfo << "event.eEvent: " << OMX_Maps::getInstance().getEvent(event.eEvent) << "\n";
                        finishedInfo << "event.nData1: " << event.nData1 << "\n";
                        finishedInfo << "event.nData2: " << event.nData2 << "\n";
                        ofLogVerbose(__func__) << finishedInfo.str();
                        #endif
                        omxEvents.erase(it);
                        pthread_mutex_unlock(&event_mutex);
                        return OMX_ErrorNone;
                    }
                }
            }
        }

        int retcode = pthread_cond_timedwait(&m_omx_event_cond, &event_mutex, &endtime);
        if (retcode != 0)
        {
            //ofLogError(__func__) << componentName << " waitForEvent Event: " << OMX_Maps::getInstance().getEvent(event.eEvent) << " TIMED OUT at: " << timeout;
            pthread_mutex_unlock(&event_mutex);
            return OMX_ErrorMax;
        }
    }

	pthread_mutex_unlock(&event_mutex);
	return OMX_ErrorNone;
}

// timeout in milliseconds
OMX_ERRORTYPE Component::waitForCommand(OMX_COMMANDTYPE command, OMX_U32 nData2, long timeout) //timeout default = 2000
{

    ofLogVerbose(__func__) << componentName << " command " << OMX_Maps::getInstance().commandNames[command];
    
	pthread_mutex_lock(&event_mutex);
	struct timespec endtime;
	clock_gettime(CLOCK_REALTIME, &endtime);
	add_timespecs(endtime, timeout);
    OMX_EVENTTYPE eEvent = OMX_EventError;
	while(true)
	{
		for (std::vector<OMXEvent>::iterator it = omxEvents.begin(); it != omxEvents.end(); it++)
		{
			OMXEvent event = *it;
            eEvent = event.eEvent;

            //Ignore same state
			if(event.eEvent == OMX_EventError && event.nData1 == (OMX_U32)OMX_ErrorSameState && event.nData2 == 1)
			{
				omxEvents.erase(it);
				pthread_mutex_unlock(&event_mutex);
				return OMX_ErrorNone;
			}
            else
            {
                //Throw error
                if(event.eEvent == OMX_EventError)
                {                    
                    OMX_TRACE((OMX_ERRORTYPE)event.nData1);
                    omxEvents.erase(it);
                    pthread_mutex_unlock(&event_mutex);
                    return (OMX_ERRORTYPE)event.nData1;
                }
                else
                {
                    //Not an error amd the data we want
                    if(event.eEvent == OMX_EventCmdComplete && event.nData1 == command && event.nData2 == nData2)
                    {
                        omxEvents.erase(it);
                        pthread_mutex_unlock(&event_mutex);
                        return OMX_ErrorNone;
                    }
                }
            }
		}

		int retcode = pthread_cond_timedwait(&m_omx_event_cond, &event_mutex, &endtime);
		if (retcode != 0)
		{
			ofLog(OF_LOG_VERBOSE,
			      "Component::waitForCommand %s		\n \
				  wait timeout event.eEvent %s              \n \
				  event.command 0x%08x						\n \
				  event.nData2 %d\n",
			      componentName.c_str(),
			      OMX_Maps::getInstance().getEvent(eEvent).c_str(),
			      (int)command,
			      (int)nData2);
			pthread_mutex_unlock(&event_mutex);
			return OMX_ErrorMax;
		}
	}
	pthread_mutex_unlock(&event_mutex);
	return OMX_ErrorNone;
}

OMX_ERRORTYPE Component::setState(OMX_STATETYPE state)
{
	assert(!handle);

	lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_STATETYPE state_actual = OMX_StateMax;
    ofLogVerbose(__func__) << componentName << " state requested: " << OMX_Maps::getInstance().omxStateNames[state];
	if(state == state_actual)
	{
		unlock();
		return OMX_ErrorNone;
	}

	error = OMX_SendCommand(handle, OMX_CommandStateSet, state, 0);
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
		error = waitForCommand(OMX_CommandStateSet, state);
        OMX_TRACE(error);
		if(error == OMX_ErrorSameState)
		{
			unlock();
			return OMX_ErrorNone;
		}
	}

	unlock();

	return error;
}

unsigned int Component::getInputBufferSpace()
{
    int free = inputBuffersAvailable.size() * m_input_buffer_size;
    return free;
}

OMX_STATETYPE Component::getState()
{
    lock();
    
    OMX_STATETYPE state;
    OMX_ERRORTYPE error = OMX_GetState(handle, &state);
    OMX_TRACE(error);
    
    unlock();
    return state;

}

OMX_ERRORTYPE Component::setParameter(OMX_INDEXTYPE paramIndex, OMX_PTR paramStruct)
{
	lock();

	OMX_ERRORTYPE error = OMX_SetParameter(handle, paramIndex, paramStruct);
    OMX_TRACE(error);
    
	unlock();
	return error;
}

OMX_ERRORTYPE Component::getParameter(OMX_INDEXTYPE paramIndex, OMX_PTR paramStruct)
{
	lock();

	OMX_ERRORTYPE error = OMX_GetParameter(handle, paramIndex, paramStruct);
    OMX_TRACE(error);


	unlock();
	return error;
}

OMX_ERRORTYPE Component::setConfig(OMX_INDEXTYPE configIndex, OMX_PTR configStruct)
{
    lock();
    
	OMX_ERRORTYPE error = OMX_SetConfig(handle, configIndex, configStruct);
    OMX_TRACE(error);

	unlock();
	return error;
}

OMX_ERRORTYPE Component::getConfig(OMX_INDEXTYPE configIndex, OMX_PTR configStruct)
{
	lock();

	OMX_ERRORTYPE error = OMX_GetConfig(handle, configIndex, configStruct);
    OMX_TRACE(error);
    
	unlock();
	return error;
}

OMX_ERRORTYPE Component::sendCommand(OMX_COMMANDTYPE cmd, OMX_U32 cmdParam, OMX_PTR cmdParamData)
{
	lock();

	OMX_ERRORTYPE error = OMX_SendCommand(handle, cmd, cmdParam, cmdParamData);
    OMX_TRACE(error);
    
	unlock();
	return error;
}

OMX_ERRORTYPE Component::enablePort(unsigned int port)//default: wait=false
{
	lock();
	
	OMX_PARAM_PORTDEFINITIONTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = port;

	OMX_ERRORTYPE error = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &portFormat);
    OMX_TRACE(error);

    ofLogVerbose(__func__) << componentName << " port: " << port;
    
	if(portFormat.bEnabled == OMX_FALSE)
	{
		error = OMX_SendCommand(handle, OMX_CommandPortEnable, port, NULL);
        OMX_TRACE(error);

		if(error != OMX_ErrorNone)
		{
			unlock();
            return error;
		}
    }

	unlock();

	return error;
}

OMX_ERRORTYPE Component::disablePort(unsigned int port)//default: wait=false
{

	OMX_ERRORTYPE error = OMX_ErrorUndefined;

	lock();
    ofLogVerbose(__func__) << componentName << " port: " << port;
	error = OMX_SendCommand(handle, OMX_CommandPortDisable, port, NULL);
    OMX_TRACE(error);
	if(error == OMX_ErrorNone)
	{
		unlock();
		return error;
	}
	OMX_PARAM_PORTDEFINITIONTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = port;

	error = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &portFormat);
    OMX_TRACE(error);



	if(portFormat.bEnabled == OMX_TRUE)
	{
		error = OMX_SendCommand(handle, OMX_CommandPortDisable, port, NULL);
        OMX_TRACE(error);

		if(error != OMX_ErrorNone)
		{
			unlock();
			return error;
		}
    }
	unlock();
	return error;
}

OMX_ERRORTYPE Component::useEGLImage(OMX_BUFFERHEADERTYPE** ppBufferHdr, OMX_U32 nPortIndex, OMX_PTR pAppPrivate, void* eglImage)
{
	lock();

	OMX_ERRORTYPE error = OMX_UseEGLImage(handle, ppBufferHdr, nPortIndex, pAppPrivate, eglImage);
    OMX_TRACE(error);

	unlock();
	return error;
}

bool Component::init( std::string& component_name, OMX_INDEXTYPE index)
{

	componentName = component_name;

	callbacks.EventHandler    = &Component::DecoderEventHandlerCallback;
	callbacks.EmptyBufferDone = &Component::DecoderEmptyBufferDoneCallback;
	callbacks.FillBufferDone  = &Component::DecoderFillBufferDoneCallback;

	// Get video component handle setting up callbacks, component is in loaded state on return.
	OMX_ERRORTYPE error = OMX_GetHandle(&handle, (char*)component_name.c_str(), this, &callbacks);
    OMX_TRACE(error);
    
	if (error != OMX_ErrorNone)
	{
		Deinitialize();
		return false;
	}
    ofLogVerbose(__func__) << componentName << " PASS ";
    
	OMX_PORT_PARAM_TYPE port_param;
	OMX_INIT_STRUCTURE(port_param);

	error = OMX_GetParameter(handle, index, &port_param);
    OMX_TRACE(error);


	error = disableAllPorts();
    OMX_TRACE(error);


	inputPort  = port_param.nStartPortNumber;
	outputPort = inputPort + 1;

	if(componentName == "OMX.broadcom.audio_mixer")
	{
		inputPort  = port_param.nStartPortNumber + 1;
		outputPort = port_param.nStartPortNumber;
	}

	if (outputPort > port_param.nStartPortNumber+port_param.nPorts-1)
	{
		outputPort = port_param.nStartPortNumber+port_param.nPorts-1;
	}


	doExit = false;
	doFlushInput   = false;
	doFlushOutput  = false;

	return true;
}

bool Component::Deinitialize(bool doFlush)//doFlush default: true
{
	doExit = true;

	doFlushInput   = true;
	doFlushOutput  = true;

	if(handle)
	{
		if(doFlush)
		{
			flushAll();
		}


		//FreeOutputBuffers(true);
		//FreeInputBuffers(true);

		if(getState() == OMX_StateExecuting)
		{
			setState(OMX_StatePause);
		}

		if(getState() != OMX_StateIdle)
		{
			setState(OMX_StateIdle);
		}

		if(getState() != OMX_StateLoaded)
		{
			setState(OMX_StateLoaded);
		}

		OMX_ERRORTYPE error = OMX_FreeHandle(handle);
        OMX_TRACE(error);


		handle = NULL;
	}

	inputPort    = 0;
	outputPort   = 0;
	componentName = "";

	CustomDecoderFillBufferDoneHandler = NULL;
	CustomDecoderEmptyBufferDoneHandler = NULL;



	return true;
}

// DecoderEventHandler -- OMX event callback
OMX_ERRORTYPE Component::DecoderEventHandlerCallback(
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
	Component *component = static_cast<Component*>(pAppData);
	return component->DecoderEventHandler(hComponent, pAppData, eEvent, nData1, nData2, pEventData);
}

// DecoderEmptyBufferDone -- OMXCore input buffer has been emptied
OMX_ERRORTYPE Component::DecoderEmptyBufferDoneCallback(
    OMX_HANDLETYPE hComponent,
    OMX_PTR pAppData,
    OMX_BUFFERHEADERTYPE* pBuffer)
{
	if(!pAppData)
	{
		return OMX_ErrorNone;
	}

	Component *component = static_cast<Component*>(pAppData);
	#ifdef OMX_DEBUG_EVENTS
		ofLogVerbose(__func__) << component->getName();
	#endif
	if(component->CustomDecoderEmptyBufferDoneHandler)
	{
		OMX_ERRORTYPE error = (*(component->CustomDecoderEmptyBufferDoneHandler))(hComponent, pAppData, pBuffer);
		if(error != OMX_ErrorNone)
		{
			return error;
		}
	}

	return component->DecoderEmptyBufferDone( hComponent, pAppData, pBuffer);
}

// DecoderFillBufferDone -- OMXCore output buffer has been filled
OMX_ERRORTYPE Component::DecoderFillBufferDoneCallback(
    OMX_HANDLETYPE hComponent,
    OMX_PTR pAppData,
    OMX_BUFFERHEADERTYPE* pBuffer)
{
	if(!pAppData)
	{
		return OMX_ErrorNone;
	}

	Component *component = static_cast<Component*>(pAppData);
	if(component->CustomDecoderFillBufferDoneHandler)
	{
		OMX_ERRORTYPE error = (*(component->CustomDecoderFillBufferDoneHandler))(hComponent, pAppData, pBuffer);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone)
		{
			return error;
		}
	}

	return component->DecoderFillBufferDone(hComponent, pAppData, pBuffer);
}

OMX_ERRORTYPE Component::DecoderEmptyBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
	if(!pAppData || doExit)
	{
		return OMX_ErrorNone;
	}

	Component *component = static_cast<Component*>(pAppData);

	pthread_mutex_lock(&component->m_omx_input_mutex);
	component->inputBuffersAvailable.push(pBuffer);

	// this allows (all) blocked tasks to be awoken
	pthread_cond_broadcast(&component->m_input_buffer_cond);

	pthread_mutex_unlock(&component->m_omx_input_mutex);

	return OMX_ErrorNone;
}

OMX_ERRORTYPE Component::DecoderFillBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
	if(!pAppData || doExit)
	{
		return OMX_ErrorNone;
	}

	Component *component = static_cast<Component*>(pAppData);

	pthread_mutex_lock(&component->m_omx_output_mutex);
	component->outputBuffersAvailable.push(pBuffer);

	// this allows (all) blocked tasks to be awoken
	pthread_cond_broadcast(&component->m_output_buffer_cond);

	pthread_mutex_unlock(&component->m_omx_output_mutex);

	sem_post(&component->m_omx_fill_buffer_done);

	return OMX_ErrorNone;
}

// DecoderEmptyBufferDone -- OMXCore input buffer has been emptied
OMX_ERRORTYPE Component::DecoderEventHandler(
    OMX_HANDLETYPE hComponent,
    OMX_PTR pAppData,
    OMX_EVENTTYPE eEvent,
    OMX_U32 nData1,
    OMX_U32 nData2,
    OMX_PTR pEventData)
{
	Component *component = static_cast<Component*>(pAppData);
	addEvent(eEvent, nData1, nData2);
	if (eEvent == OMX_EventBufferFlag)
	{
		if(nData2 & OMX_BUFFERFLAG_EOS)
		{

			pthread_mutex_lock(&component->eos_mutex);
			component->m_eos = true;
			pthread_mutex_unlock(&component->eos_mutex);
		}
		
	}
	else
	{
		if (eEvent == OMX_EventError)
		{
            OMX_TRACE((OMX_ERRORTYPE) nData1);
			sem_post(&component->m_omx_fill_buffer_done);
		}

	}
	return OMX_ErrorNone;
}
