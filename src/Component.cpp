#include "Component.h"


//#define DEBUG_EVENTS
//#define DEBUG_STATES
//#define DEBUG_COMMANDS
//#define DEBUG_PORTS

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

Component::Component()
{
	
	frameCounter = 0;
	frameOffset = 0;
	inputPort  = 0;
	outputPort = 0;
	handle      = NULL;
    m_input_buffer_size = 0;
	
	doFlushInput         = false;

	CustomFillBufferDoneHandler = NULL;
	CustomEmptyBufferDoneHandler = NULL;

	m_eos                 = false;


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
    if(handle)
    {
       Deinitialize(__func__); 
    }
	

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
    if(!handle) 
{
	ofLogError(__func__) << getName() << " NO HANDLE";
	return OMX_ErrorNone;
}

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
    if(!handle) 
{
	ofLogError(__func__) << getName() << " NO HANDLE";
	return OMX_ErrorNone;
}

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
    if(!handle) 
{
	ofLogError(__func__) << getName() << " NO HANDLE";
	return OMX_ErrorNone;
}
    
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
	if(!handle) 
{
	ofLogError(__func__) << getName() << " NO HANDLE";
}
	lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;
	error = OMX_SendCommand(handle, OMX_CommandFlush, inputPort, NULL);
    OMX_TRACE(error, getName());
	
	unlock();
}

void Component::flushOutput()
{
    if(!handle) 
{
	ofLogError(__func__) << getName() << " NO HANDLE";
}
	lock();

	OMX_ERRORTYPE error = OMX_ErrorNone;
	error = OMX_SendCommand(handle, OMX_CommandFlush, outputPort, NULL);
	OMX_TRACE(error, getName());
    

	unlock();
}

// timeout in milliseconds
OMX_BUFFERHEADERTYPE* Component::getInputBuffer(long timeout)
{
	if(!handle) 
{
	ofLogError(__func__) << getName() << " NO HANDLE";
}
    
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
			ofLogError(__func__) << componentName << " TIMEOUT";
			break;
		}
	}
	pthread_mutex_unlock(&m_omx_input_mutex);

	return omx_input_buffer;
}

OMX_BUFFERHEADERTYPE* Component::getOutputBuffer()
{
    if(!handle) 
{
	ofLogError(__func__) << getName() << " NO HANDLE";
}
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

	if(!handle) 
{
	ofLogError(__func__) << getName() << " NO HANDLE";
	return OMX_ErrorNone;
}
    
	OMX_PARAM_PORTDEFINITIONTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = inputPort;

	error = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &portFormat);
	OMX_TRACE(error);

    m_input_buffer_size   = portFormat.nBufferSize;
    
    //setState(OMX_StateIdle);
    //setState(OMX_StateLoaded);
    setState(OMX_StateIdle);
	/*if(getState() != OMX_StateIdle)
	{
		if(getState() != OMX_StateLoaded)
		{
			setState(OMX_StateLoaded);
		}
		setState(OMX_StateIdle);
	}*/
    
    
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
	
    if(!handle) 
{
	ofLogError(__func__) << getName() << " NO HANDLE";
	return OMX_ErrorNone;
}
    OMX_ERRORTYPE error = OMX_ErrorNone;
	

	OMX_PARAM_PORTDEFINITIONTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = outputPort;

	error = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &portFormat);
    OMX_TRACE(error);
	if(error != OMX_ErrorNone)
	{
		return error;
	}

    setState(OMX_StateIdle);
    setState(OMX_StateLoaded);
    //setState(OMX_StateIdle);
    
	/*if(getState() != OMX_StateIdle)
	{
		if(getState() != OMX_StateLoaded)
		{
			setState(OMX_StateLoaded);
		}
		setState(OMX_StateIdle);
	}*/

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


	return error;
}
OMX_ERRORTYPE Component::disableAllPorts()
{
    if(!handle) 
{
	ofLogError(__func__) << getName() << " NO HANDLE";
	return OMX_ErrorNone;
}
    
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

void Component::removeEvent(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2)
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
	removeEvent(eEvent, nData1, nData2);
	omxEvents.push_back(event);
	// this allows (all) blocked tasks to be awoken
	pthread_cond_broadcast(&m_omx_event_cond);
	pthread_mutex_unlock(&event_mutex);

	return OMX_ErrorNone;
}


// timeout in milliseconds
OMX_ERRORTYPE Component::waitForEvent(OMX_EVENTTYPE eventType, long timeout)
{
#ifdef DEBUG_EVENTS
    ofLogVerbose(__func__) << "\n" << componentName << "\n" << "eventType: " << OMX_Maps::getInstance().getEvent(eventType) << "\n";
#endif

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
                        #ifdef DEBUG_EVENTS
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
            ofLogError(__func__) << componentName << " waitForEvent Event: " << OMX_Maps::getInstance().getEvent(eventType) << " TIMED OUT at: " << timeout;
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
#ifdef DEBUG_COMMANDS
    ofLogVerbose(__func__) << "\n" << componentName << " command " << OMX_Maps::getInstance().commandNames[command] << "\n";
#endif    
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
			ofLog(OF_LOG_ERROR,
			      "Component::waitForCommand %s		\n \
				  wait timeout event.eEvent %s              \n \
				  event.command %s						\n \
				  event.nData2 %d\n",
			      componentName.c_str(),
			      OMX_Maps::getInstance().getEvent(eEvent).c_str(),
			      OMX_Maps::getInstance().commandNames[command].c_str(),
			      (int)nData2);
			pthread_mutex_unlock(&event_mutex);
			return OMX_ErrorMax;
		}
	}
	pthread_mutex_unlock(&event_mutex);
	return OMX_ErrorNone;
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

OMX_ERRORTYPE Component::setState(OMX_STATETYPE state)
{
    if(!handle) 
{
	ofLogError(__func__) << getName() << " NO HANDLE";
	return OMX_ErrorNone;
}
    
    lock();
    
    OMX_ERRORTYPE error = OMX_ErrorNone;
    OMX_STATETYPE state_actual;
    error = OMX_GetState(handle, &state_actual);
    OMX_TRACE(error);
    
    if(state == state_actual)
    {
        unlock();
        return OMX_ErrorNone;
    }else
    {
#ifdef DEBUG_STATES
        ofLogVerbose(componentName) << OMX_Maps::getInstance().omxStateNames[state] << " IS NOT " << OMX_Maps::getInstance().omxStateNames[state_actual];
#endif
    }
    
    error = OMX_SendCommand(handle, OMX_CommandStateSet, state, 0);
    OMX_TRACE(error);
    if (error != OMX_ErrorNone)
    {
        if(error == OMX_ErrorSameState)
        {
            error = OMX_ErrorNone;
            unlock();
            return error;
        }
        
    }
    else
    {
        error = waitForCommand(OMX_CommandStateSet, state);
        OMX_TRACE(error);
        if(error == OMX_ErrorSameState)
        {
            unlock();
            return error;
        }
        
        if(componentName == "OMX.broadcom.audio_mixer")
        {
            error = OMX_ErrorNone;
            unlock();
            return error;
        }
        /*if(error == OMX_ErrorPortUnpopulated)
         {
         ofLogError(__func__) << componentName <<  " threw OMX_ErrorPortUnpopulated"; 
         unlock();
         return error;
         }*/
    }
    
    unlock();
    
    return error;
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
#ifdef DEBUG_PORTS
    ofLogVerbose(__func__) << componentName << " port: " << port;
#endif
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
#ifdef DEBUG_PORTS
    ofLogVerbose(__func__) << componentName << " port: " << port;
#endif
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

	callbacks.EventHandler    = &Component::EventHandlerCallback;
	callbacks.EmptyBufferDone = &Component::EmptyBufferDoneCallback;
	callbacks.FillBufferDone  = &Component::FillBufferDoneCallback;

	// Get video component handle setting up callbacks, component is in loaded state on return.
	OMX_ERRORTYPE error = OMX_GetHandle(&handle, (char*)component_name.c_str(), this, &callbacks);
    OMX_TRACE(error);
    
	if (error != OMX_ErrorNone)
	{
        ofLogError(__func__) << componentName << " FAIL ";
		Deinitialize(__func__);
		return false;
	}
    
    
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
		inputPort  = 232;
		outputPort = 231;
	}

	if (outputPort > port_param.nStartPortNumber+port_param.nPorts-1)
	{
		outputPort = port_param.nStartPortNumber+port_param.nPorts-1;
	}

	return true;
}

OMX_ERRORTYPE Component::freeInputBuffers()
{
    if(!handle) 
{
	ofLogError(__func__) << getName() << " NO HANDLE";
	return OMX_ErrorNone;
}
    
    OMX_ERRORTYPE error = OMX_ErrorNone;
    
    if(inputBuffers.empty())
    {
        return OMX_ErrorNone;
    }
    
    //m_flush_input = true;
    
    pthread_mutex_lock(&m_omx_input_mutex);
    pthread_cond_broadcast(&m_input_buffer_cond);
    
    error = disablePort(inputPort);
    OMX_TRACE(error);
    
    for (size_t i = 0; i < inputBuffers.size(); i++)
    {
        error = OMX_FreeBuffer(handle, inputPort, inputBuffers[i]);
        OMX_TRACE(error);
    }
    
    inputBuffers.clear();

    //error =  waitForCommand(OMX_CommandPortDisable, inputPort);
    //OMX_TRACE(error);
    
    while (!inputBuffersAvailable.empty())
    {
        inputBuffersAvailable.pop();
    }
    
    pthread_mutex_unlock(&m_omx_input_mutex);
    
    return error;
}

OMX_ERRORTYPE Component::freeOutputBuffers()
{
    if(!handle) 
{
	ofLogError(__func__) << getName() << " NO HANDLE";
	return OMX_ErrorNone;
}
    
    OMX_ERRORTYPE error = OMX_ErrorNone;
    
    if(outputBuffers.empty())
    {
        return OMX_ErrorNone;
    }
    
    pthread_mutex_lock(&m_omx_output_mutex);
    pthread_cond_broadcast(&m_output_buffer_cond);
    
    error = disablePort(outputPort);
    OMX_TRACE(error);
    
    for (size_t i = 0; i < outputBuffers.size(); i++)
    {        
        error = OMX_FreeBuffer(handle, outputPort, outputBuffers[i]);
        OMX_TRACE(error);
    }
    outputBuffers.clear();
    
    //error =  waitForCommand(OMX_CommandPortDisable, outputPort);
    //OMX_TRACE(error);
    
    
    while (!outputBuffersAvailable.empty())
    {
        outputBuffersAvailable.pop();
    }
    
    pthread_mutex_unlock(&m_omx_output_mutex);
    
    return error;
}


bool Component::Deinitialize(string caller)
{
    //ofLogVerbose(__func__) << componentName << " by caller: " << caller;

	if(handle)
	{
		
        flushAll();
		

        OMX_ERRORTYPE error = freeOutputBuffers();
        OMX_TRACE(error);
		freeInputBuffers();
        OMX_TRACE(error);
        
        if(componentName != "OMX.broadcom.egl_render")
        {
            /*if(getState() == OMX_StateExecuting)
            {
                setState(OMX_StatePause);
            }*/
            error = setState(OMX_StatePause);
            OMX_TRACE(error);
            error = setState(OMX_StateIdle);
            OMX_TRACE(error);
            error = setState(OMX_StateLoaded);
            OMX_TRACE(error);
            /*
            if(getState() != OMX_StateIdle)
            {
                setState(OMX_StateIdle);
            }
            
            if(getState() != OMX_StateLoaded)
            {
                setState(OMX_StateLoaded);
            }*/
        }
		

		error = OMX_FreeHandle(handle);
        OMX_TRACE(error);


		handle = NULL;
	}else
    {
        ofLogError(__func__) << "NO HANDLE! caller: " << caller;
    }

	inputPort    = 0;
	outputPort   = 0;
	//componentName = "";

    if(CustomFillBufferDoneHandler)
    {
        CustomFillBufferDoneHandler = NULL;
    }
	if(CustomEmptyBufferDoneHandler)
    {
       CustomEmptyBufferDoneHandler = NULL; 
    }
	

    //ofLogVerbose(__func__) << componentName << " END";

	return true;
}

//All events callback
OMX_ERRORTYPE Component::EventHandlerCallback(OMX_HANDLETYPE hComponent,
                                              OMX_PTR pAppData,
                                              OMX_EVENTTYPE event,
                                              OMX_U32 nData1,
                                              OMX_U32 nData2,
                                              OMX_PTR pEventData)
{
    OMX_ERRORTYPE error = OMX_ErrorNone;
    if(!pAppData)
    {
        return error;
    }

	Component* component = static_cast<Component*>(pAppData);
    component->addEvent(event, nData1, nData2);
    bool debugEvents = false;
    
    if(debugEvents)
    {
        for (int i=0; i<OMX_Maps::getInstance().eventTypes.size(); i++) 
        {
            ofLogVerbose() << component->getName() << OMX_Maps::getInstance().eventTypes[event];
        } 
    }
    bool resourceError = false;
    if(event == OMX_EventError)
    {
        OMX_TRACE((OMX_ERRORTYPE) nData1);
        switch((OMX_S32)nData1)
        {
            case OMX_ErrorInsufficientResources:
            case OMX_ErrorStreamCorrupt:
            {
                resourceError = true;
                break;
            }
            case OMX_ErrorSameState:
            {
#ifdef DEBUG_STATES
                ofLogVerbose() << component->getName() << " OMX_ErrorSameState \n";
#endif
                break;
            }
        }
    }
    if(resourceError)
    {
        pthread_cond_broadcast(&component->m_output_buffer_cond);
        pthread_cond_broadcast(&component->m_input_buffer_cond);
        pthread_cond_broadcast(&component->m_omx_event_cond); 
    }
    
    if (event == OMX_EventBufferFlag)
    {
        if(nData2 & OMX_BUFFERFLAG_EOS)
        {
            
            pthread_mutex_lock(&component->eos_mutex);
            component->m_eos = true;
            pthread_mutex_unlock(&component->eos_mutex);
        }
        
    }
	return error;
}

//Input buffer has been emptied
OMX_ERRORTYPE Component::EmptyBufferDoneCallback(OMX_HANDLETYPE hComponent,
                                                 OMX_PTR pAppData,
                                                 OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE error = OMX_ErrorNone;
    if(!pAppData)
    {
        return error;
    }

	Component *component = static_cast<Component*>(pAppData);

	if(component->CustomEmptyBufferDoneHandler)
	{
		error = (*(component->CustomEmptyBufferDoneHandler))(hComponent, pAppData, pBuffer);
        
	}else
    {
        pthread_mutex_lock(&component->m_omx_input_mutex);
        component->inputBuffersAvailable.push(pBuffer);
        
        // this allows (all) blocked tasks to be awoken
        pthread_cond_broadcast(&component->m_input_buffer_cond);
        
        pthread_mutex_unlock(&component->m_omx_input_mutex);
    }

    OMX_TRACE(error, component->getName());
    return error;
}

//output buffer has been filled
OMX_ERRORTYPE Component::FillBufferDoneCallback(OMX_HANDLETYPE hComponent,
                                                OMX_PTR pAppData,
                                                OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE error = OMX_ErrorNone;
	if(!pAppData)
	{
		return error;
	}

	Component* component = static_cast<Component*>(pAppData);
	if(component->CustomFillBufferDoneHandler)
	{
		error = (*(component->CustomFillBufferDoneHandler))(hComponent, pAppData, pBuffer);
        OMX_TRACE(error);
        
	}else
    {
        pthread_mutex_lock(&component->m_omx_output_mutex);
        component->outputBuffersAvailable.push(pBuffer);
        
        // this allows (all) blocked tasks to be awoken
        pthread_cond_broadcast(&component->m_output_buffer_cond);
        
        pthread_mutex_unlock(&component->m_omx_output_mutex);
        
        sem_post(&component->m_omx_fill_buffer_done);
    }
    
    if (error == OMX_ErrorIncorrectStateOperation) 
    {
        ofLogError() << component->getName() << " THREW OMX_ErrorIncorrectStateOperation";
    }
	return error;
}