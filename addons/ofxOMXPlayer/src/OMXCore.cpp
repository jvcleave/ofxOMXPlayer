
#include <math.h>
#include <sys/time.h>

#include "OMXCore.h"

#include "OMXClock.h"

#include "XMemUtils.h"

//#define ENABLE_WAIT_FOR_COMMANDS
//#define OMX_DEBUG_EVENTS
//#define OMX_DEBUG_EVENTHANDLER
#define COMMAND_WAIT_TIMEOUT 50

////////////////////////////////////////////////////////////////////////////////////////////
#define CLASSNAME "COMXCoreComponent"

string printOMXError(OMX_ERRORTYPE error)
{
	string errorString = "UNDEFINED_OMX_ERRORTYPE";
	switch(error)
	{
		case OMX_ErrorNone: {errorString = "OMX_ErrorNone"; break; }
		case OMX_ErrorInsufficientResources: {errorString = "OMX_ErrorInsufficientResources"; break; }
		case OMX_ErrorUndefined: {errorString = "OMX_ErrorUndefined"; break; }
		case OMX_ErrorInvalidComponentName: {errorString = "OMX_ErrorInvalidComponentName"; break; }
		case OMX_ErrorComponentNotFound: {errorString = "OMX_ErrorComponentNotFound"; break; }
		case OMX_ErrorInvalidComponent: {errorString = "OMX_ErrorInvalidComponent"; break; }
		case OMX_ErrorBadParameter: {errorString = "OMX_ErrorBadParameter"; break; }
		case OMX_ErrorNotImplemented: {errorString = "OMX_ErrorNotImplemented"; break; }
		case OMX_ErrorUnderflow: {errorString = "OMX_ErrorUnderflow"; break; }
		case OMX_ErrorOverflow: {errorString = "OMX_ErrorOverflow"; break; }
		case OMX_ErrorHardware: {errorString = "OMX_ErrorHardware"; break; }
		case OMX_ErrorInvalidState: {errorString = "OMX_ErrorInvalidState"; break; }
		case OMX_ErrorStreamCorrupt: {errorString = "OMX_ErrorStreamCorrupt"; break; }
		case OMX_ErrorPortsNotCompatible: {errorString = "OMX_ErrorPortsNotCompatible"; break; }
		case OMX_ErrorResourcesLost: {errorString = "OMX_ErrorResourcesLost"; break; }
		case OMX_ErrorNoMore: {errorString = "OMX_ErrorNoMore"; break; }
		case OMX_ErrorVersionMismatch: {errorString = "OMX_ErrorVersionMismatch"; break; }
		case OMX_ErrorNotReady: {errorString = "OMX_ErrorNotReady"; break; }
		case OMX_ErrorTimeout: {errorString = "OMX_ErrorTimeout"; break; }
		case OMX_ErrorSameState: {errorString = "OMX_ErrorSameState"; break; }
		case OMX_ErrorResourcesPreempted: {errorString = "OMX_ErrorResourcesPreempted"; break; }
		case OMX_ErrorPortUnresponsiveDuringAllocation: {errorString = "OMX_ErrorPortUnresponsiveDuringAllocation"; break; }
		case OMX_ErrorPortUnresponsiveDuringDeallocation: {errorString = "OMX_ErrorPortUnresponsiveDuringDeallocation"; break; }
		case OMX_ErrorPortUnresponsiveDuringStop: {errorString = "OMX_ErrorPortUnresponsiveDuringStop"; break; }
		case OMX_ErrorIncorrectStateTransition: {errorString = "OMX_ErrorIncorrectStateTransition"; break; }
		case OMX_ErrorIncorrectStateOperation: {errorString = "OMX_ErrorIncorrectStateOperation"; break; }
		case OMX_ErrorUnsupportedSetting: {errorString = "OMX_ErrorUnsupportedSetting"; break; }
		case OMX_ErrorUnsupportedIndex: {errorString = "OMX_ErrorUnsupportedIndex"; break; }
		case OMX_ErrorBadPortIndex: {errorString = "OMX_ErrorBadPortIndex"; break; }
		case OMX_ErrorPortUnpopulated: {errorString = "OMX_ErrorPortUnpopulated"; break; }
		case OMX_ErrorComponentSuspended: {errorString = "OMX_ErrorComponentSuspended"; break; }
		case OMX_ErrorDynamicResourcesUnavailable: {errorString = "OMX_ErrorDynamicResourcesUnavailable"; break; }
		case OMX_ErrorMbErrorsInFrame: {errorString = "OMX_ErrorMbErrorsInFrame"; break; }
		case OMX_ErrorFormatNotDetected: {errorString = "OMX_ErrorFormatNotDetected"; break; }
		case OMX_ErrorContentPipeOpenFailed: {errorString = "OMX_ErrorContentPipeOpenFailed"; break; }
		case OMX_ErrorContentPipeCreationFailed: {errorString = "OMX_ErrorContentPipeCreationFailed"; break; }
		case OMX_ErrorSeperateTablesUsed: {errorString = "OMX_ErrorSeperateTablesUsed"; break; }
		case OMX_ErrorTunnelingUnsupported: {errorString = "OMX_ErrorTunnelingUnsupported"; break; }
		case OMX_ErrorKhronosExtensions: {errorString = "OMX_ErrorKhronosExtensions"; break; }
		case OMX_ErrorVendorStartUnused: {errorString = "OMX_ErrorVendorStartUnused"; break; }
		case OMX_ErrorDiskFull: {errorString = "OMX_ErrorDiskFull"; break; }
		case OMX_ErrorMaxFileSize: {errorString = "OMX_ErrorMaxFileSize"; break; }
		case OMX_ErrorDrmUnauthorised: {errorString = "OMX_ErrorDrmUnauthorised"; break; }
		case OMX_ErrorDrmExpired: {errorString = "OMX_ErrorDrmExpired"; break; }
		case OMX_ErrorDrmGeneral: {errorString = "OMX_ErrorDrmGeneral"; break; }
		default:{break;}
	}
	if (errorString == "UNDEFINED_OMX_ERRORTYPE") 
	{
		ofLog(OF_LOG_ERROR, "OMX_ERROR 0x%08x ", error);
		if (error == OMX_CommandMax) {
			
			//raise(SIGKILL);
		}
	}
	return errorString;
}


string printState(OMX_STATETYPE state)
{
	string returnString = "UNDEFINED_OMX_STATETYPE"; 
	switch (state) 
	{
		case OMX_StateExecuting:
			returnString = "OMX_StateExecuting";
			break;
		case OMX_StateIdle:
			returnString = "OMX_StateIdle";
			break;
		case OMX_StateLoaded:
			returnString = "OMX_StateLoaded";
			break;
		case OMX_StateInvalid:
			returnString = "OMX_StateInvalid";
			break;
		case OMX_StatePause:
			returnString = "OMX_StatePause";
			break;
		case OMX_StateWaitForResources:
			returnString = "OMX_StateWaitForResources";
			break;
		case OMX_StateKhronosExtensions:
			returnString = "OMX_StateKhronosExtensions";
			break;
		case OMX_StateVendorStartUnused:
			returnString = "OMX_StateVendorStartUnused";
			break;
		case OMX_StateMax:
			returnString = "OMX_StateMax";
			break;
	}
	return returnString;
}

string printEventType(OMX_EVENTTYPE eventType)
{
   	string type = "UNDEFINED";
   	switch(eventType)
   	{
		case OMX_EventCmdComplete: { type = "OMX_EventCmdComplete"; break; }
		case OMX_EventMax: { type = "OMX_EventMax"; break; }
   		case OMX_EventError: { type = "OMX_EventError"; break; }
   		case OMX_EventMark: { type = "OMX_EventMark"; break; }
   		case OMX_EventPortSettingsChanged: { type = "OMX_EventPortSettingsChanged"; break; }
   		case OMX_EventBufferFlag: { type = "OMX_EventBufferFlag"; break; }
   		case OMX_EventResourcesAcquired: { type = "OMX_EventResourcesAcquired"; break; }
   		case OMX_EventComponentResumed: { type = "OMX_EventComponentResumed"; break; }
   		case OMX_EventDynamicResourcesAvailable: { type = "OMX_EventDynamicResourcesAvailable"; break; }
   		case OMX_EventPortFormatDetected: { type = "OMX_EventPortFormatDetected"; break; }
   		case OMX_EventKhronosExtensions: { type = "OMX_EventKhronosExtensions"; break; }
   		case OMX_EventVendorStartUnused: { type = "OMX_EventVendorStartUnused"; break; }
   		case OMX_EventParamOrConfigChanged: { type = "OMX_EventParamOrConfigChanged"; break; }
   	}
   	return type;   
}

string printCmd(OMX_U32 cmd)
{
	
	OMX_COMMANDTYPE coverted = (OMX_COMMANDTYPE) cmd;
   	string type = "UNDEFINED";
   	switch(coverted)
   	{
		case OMX_CommandVendorStartUnused: { type = "OMX_CommandVendorStartUnused"; break; }
		case OMX_CommandMax: { type = "OMX_CommandMax"; break; }	
   		case OMX_CommandStateSet: { type = "OMX_CommandStateSet"; break; }
   		case OMX_CommandFlush: { type = "OMX_CommandFlush"; break; }
   		case OMX_CommandPortDisable: { type = "OMX_CommandPortDisable"; break; }
   		case OMX_CommandPortEnable: { type = "OMX_CommandPortEnable"; break; }
   		case OMX_CommandMarkBuffer: { type = "OMX_CommandMarkBuffer"; break; }
   		case OMX_CommandKhronosExtensions: { type = "OMX_CommandKhronosExtensions"; break; }
   	}
   	return type;   
}

static void add_timespecs(struct timespec &time, long millisecs)
{
   time.tv_sec  += millisecs / 1000;
   time.tv_nsec += (millisecs % 1000) * 1000000;
   if (time.tv_nsec > 1000000000)
   {
      time.tv_sec  += 1;
      time.tv_nsec -= 1000000000;
   }
}


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
		//ofLogVerbose(__func__) << "m_src_component: " << srcName << " m_dst_component: " << dstName << " START";
		
		//OMX_ERRORTYPE omx_err = OMX_ErrorNone;
		if(m_src_component->GetComponent())
		{
			//omx_err = OMX_SendCommand(m_src_component->GetComponent(), OMX_CommandFlush, m_src_port, NULL);
			m_src_component->FlushAll();
		}

		if(m_dst_component->GetComponent())
		{
			//omx_err = OMX_SendCommand(m_dst_component->GetComponent(), OMX_CommandFlush, m_dst_port, NULL);
			m_dst_component->FlushAll();
		}

	UnLock();
	//ofLogVerbose(__func__) << srcName << " END";
  return OMX_ErrorNone;
}

OMX_ERRORTYPE COMXCoreTunel::Deestablish(bool doWait)
{

	if (!isEstablished) 
	{
	  //ofLogVerbose(__func__) << "NOT ESTABLISHED -- BAILING";
	  return OMX_ErrorNone;
	}

	if(!m_src_component || !m_dst_component)
	{
	  return OMX_ErrorUndefined;
	}

	Lock();
		//ofLogVerbose(__func__) << "m_src_component: " << srcName << " m_dst_component: " << dstName << " m_portSettingsChanged " << m_portSettingsChanged;
	
		OMX_ERRORTYPE omx_err = OMX_ErrorNone;
	
		if(m_src_component->GetComponent() && m_portSettingsChanged && doWait)
		{
			//ofLogVerbose(__func__) << srcName << " m_portSettingsChanged " << m_portSettingsChanged << " doWait " << doWait;
			omx_err = m_src_component->WaitForEvent(OMX_EventPortSettingsChanged);
			if(omx_err != OMX_ErrorNone) 
			{
				ofLogError(__func__) << " WaitForEvent OMX_EventPortSettingsChanged FAIL " << printOMXError(omx_err);
			}else 
			{
				//ofLogVerbose(__func__) << " WaitForEvent OMX_EventPortSettingsChanged PASS";
				
			}
		}
	
		//ofLogVerbose(__func__) << srcName << " DISABLE START m_src_port " << m_src_port;
		if(m_src_component->GetComponent())
		{
			omx_err = m_src_component->DisablePort(m_src_port);
			if(omx_err == OMX_ErrorNone)
			{
				//ofLogVerbose(__func__) << srcName << " DisablePort PASS";
			}else 
			{
				ofLogError(__func__) << srcName << " DisablePort FAIL" << " m_src_port: " << m_src_port << " omx_err: " << printOMXError(omx_err);
			}
		}
		//ofLogVerbose(__func__) << srcName << "  DISABLE END";
		//ofLogVerbose(__func__) << dstName << " DISABLE START";
		if(m_dst_component->GetComponent())
		{
			omx_err = m_dst_component->DisablePort(m_dst_port);
			if(omx_err == OMX_ErrorNone)
			{
				//ofLogVerbose(__func__) << dstName << "DisablePort PASS";
			}else 
			{
				ofLogError(__func__) << dstName << " DisablePort FAIL" << " m_src_port: " << m_dst_port << " omx_err: " << printOMXError(omx_err);
			}
			
		}
		//ofLogVerbose(__func__) << dstName << "DISABLE END";
		//ofLogVerbose(__func__) << "TUNNEL UNSET START";
		if(m_src_component->GetComponent())
		{
			omx_err = OMX_SetupTunnel(m_src_component->GetComponent(), m_src_port, NULL, 0);
			if(omx_err == OMX_ErrorNone)
			{
				//ofLogVerbose(__func__) << srcName << " TUNNEL UNSET PASS";
			}else
			{
				ofLogError(__func__) << srcName << " TUNNEL UNSET FAIL: " << printOMXError(omx_err);
			}
		}
	
		if(m_dst_component->GetComponent())
		{
			omx_err = OMX_SetupTunnel(m_dst_component->GetComponent(), m_dst_port, NULL, 0);
			if(omx_err == OMX_ErrorNone)
			{
				//ofLogVerbose(__func__) << dstName << " TUNNEL UNSET PASS";
			}else
			{
				ofLogError(__func__) << dstName << " TUNNEL UNSET FAIL: " << printOMXError(omx_err);
			}
		}
	
		//ofLogVerbose(__func__) << "TUNNEL UNSET END";
	UnLock();
	//ofLogVerbose(__func__) << " END";
	
	isEstablished = false;
	return OMX_ErrorNone;
}

OMX_ERRORTYPE COMXCoreTunel::Establish(bool portSettingsChanged)
{
	Lock();
	
		OMX_ERRORTYPE omx_err = OMX_ErrorNone;
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
			omx_err = m_src_component->SetStateForComponent(OMX_StateIdle);
			if(omx_err != OMX_ErrorNone)
			{
				ofLogError(__func__) << srcName << " Setting state to idle FAIL: " << printOMXError(omx_err);
				UnLock();
				return omx_err;
			}
		}

		if(portSettingsChanged)
		{
			omx_err = m_src_component->WaitForEvent(OMX_EventPortSettingsChanged);
			if(omx_err != OMX_ErrorNone)
			{
				UnLock();
				ofLogError(__func__) << srcName << " WaitForEvent OMX_EventPortSettingsChanged FAIL: " << printOMXError(omx_err);
				return omx_err;
			}
		}
		//ofLogVerbose(__func__) << srcName << " m_src_port: " << m_src_port;
		if(m_src_component->GetComponent())
		{
			omx_err = m_src_component->DisablePort(m_src_port);
			if(omx_err != OMX_ErrorNone) 
			{
				ofLogError(__func__) << srcName << " DisablePort FAIL: " << printOMXError(omx_err);
			}
		}

		if(m_dst_component->GetComponent())
		{
			omx_err = m_dst_component->DisablePort(m_dst_port);
			if(omx_err != OMX_ErrorNone) 
			{
				ofLogError(__func__) << dstName << " DisablePort doWait" << doWait << " FAIL: " << printOMXError(omx_err);
			}
		}

		if(m_src_component->GetComponent() && m_dst_component->GetComponent())
		{
			omx_err = OMX_SetupTunnel(m_src_component->GetComponent(), m_src_port, m_dst_component->GetComponent(), m_dst_port);
			if(omx_err != OMX_ErrorNone) 
			{
				ofLogError(__func__) << " OMX_SetupTunnel " << srcName << " TO " << dstName << " FAIL: " << printOMXError(omx_err);
				UnLock();
				return omx_err;
			}else
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
			omx_err = m_src_component->EnablePort(m_src_port, doWait);
			if(omx_err != OMX_ErrorNone)
			{
				ofLogError(__func__)  << srcName << " EnablePort doWait" << doWait << " FAIL: " << printOMXError(omx_err);
				UnLock();
				return omx_err;
			}
		}

		if(m_dst_component->GetComponent())
		{
			omx_err = m_dst_component->EnablePort(m_dst_port, doWait);
			if(omx_err != OMX_ErrorNone)
			{
				ofLogError(__func__)  << dstName << " EnablePort doWait" << doWait << " FAIL: " << printOMXError(omx_err);
				UnLock();
				return omx_err;
			}
		}

		if(m_dst_component->GetComponent())
		{
			if(m_dst_component->GetState() == OMX_StateLoaded)
			{
				//important to wait for audio
				omx_err = m_dst_component->WaitForCommand(OMX_CommandPortEnable, m_dst_port, COMMAND_WAIT_TIMEOUT);
				if(omx_err != OMX_ErrorNone)
				{
					ofLogError(__func__)  << dstName << " WaitForCommand OMX_CommandPortEnable FAIL: " << printOMXError(omx_err);
					UnLock();
					return omx_err;
				}
				omx_err = m_dst_component->SetStateForComponent(OMX_StateIdle);
				if(omx_err != OMX_ErrorNone)
				{
					ofLogError(__func__)  << dstName << " SetStateForComponent OMX_StateIdle FAIL: " << printOMXError(omx_err);
					UnLock();
					return omx_err;
				}
			}
			else
			{
				#ifdef ENABLE_WAIT_FOR_COMMANDS
				omx_err = m_dst_component->WaitForCommand(OMX_CommandPortEnable, m_dst_port);
				if(omx_err != OMX_ErrorNone)
				{
					ofLogError(__func__)  << dstName << " WaitForCommand OMX_CommandPortEnable FAIL: " << printOMXError(omx_err);
					UnLock();
					return omx_err;
				}
				#endif
			}
		}

		if(m_src_component->GetComponent())
		{
			omx_err = m_src_component->WaitForCommand(OMX_CommandPortEnable, m_src_port, COMMAND_WAIT_TIMEOUT);
			if(omx_err != OMX_ErrorNone)
			{
				ofLogError(__func__)  << srcName << " WaitForCommand OMX_CommandPortEnable FAIL: " << printOMXError(omx_err);
				UnLock();
				return omx_err;
			}
		}

		m_portSettingsChanged = portSettingsChanged;

	UnLock();
	
	
	return OMX_ErrorNone;
}

////////////////////////////////////////////////////////////////////////////////////////////

COMXCoreComponent::COMXCoreComponent()
{
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
	pthread_cond_destroy(&m_input_buffer_cond);
	pthread_cond_destroy(&m_output_buffer_cond);
	pthread_cond_destroy(&m_omx_event_cond);

	pthread_mutex_destroy(&m_lock);
	sem_destroy(&m_omx_fill_buffer_done);

  
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
	
	OMX_ERRORTYPE omx_err = OMX_ErrorNone;

	if(!m_handle || !omx_buffer)
	{
	  return OMX_ErrorUndefined;
	}

	omx_err = OMX_EmptyThisBuffer(m_handle, omx_buffer);
	if (omx_err != OMX_ErrorNone)
	{
		ofLogError(__func__)  << m_componentName << " OMX_EmptyThisBuffer FAIL: " << printOMXError(omx_err);
	}

	return omx_err;
}

OMX_ERRORTYPE COMXCoreComponent::FillThisBuffer(OMX_BUFFERHEADERTYPE *omx_buffer)
{
	
	OMX_ERRORTYPE omx_err = OMX_ErrorNone;

	if(!m_handle || !omx_buffer)
	{
		return OMX_ErrorUndefined;
	}

	omx_err = OMX_FillThisBuffer(m_handle, omx_buffer);
	if (omx_err != OMX_ErrorNone)
	{
		ofLogError(__func__)  << m_componentName << " OMX_FillThisBuffer FAIL: " << printOMXError(omx_err);
	}

	return omx_err;
}

OMX_ERRORTYPE COMXCoreComponent::FreeOutputBuffer(OMX_BUFFERHEADERTYPE *omx_buffer)
{
	OMX_ERRORTYPE omx_err = OMX_ErrorNone;

	if(!m_handle || !omx_buffer)
	{
	  return OMX_ErrorUndefined;
	}

	omx_err = OMX_FreeBuffer(m_handle, m_output_port, omx_buffer);
	if (omx_err != OMX_ErrorNone)
	{
		ofLogError(__func__)  << m_componentName << " OMX_FreeBuffer FAIL: " << printOMXError(omx_err);
	}

	return omx_err;
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
	//ofLogVerbose(__func__) << m_componentName << " START";
	FlushInput();
	FlushOutput();
	//ofLogVerbose(__func__) << m_componentName << " END";
}

void COMXCoreComponent::FlushInput()
{
	if (!m_omx_input_use_buffers) 
	{
		return;
	}
	
	Lock();
		//ofLogVerbose(__func__) << m_componentName << " START";

		OMX_ERRORTYPE omx_err = OMX_ErrorNone;
		omx_err = OMX_SendCommand(m_handle, OMX_CommandFlush, m_input_port, NULL);
		if(omx_err != OMX_ErrorNone)
		{
			ofLogError(__func__) << m_componentName << " OMX_CommandFlush FAIL: " << printOMXError(omx_err);
		}
		
		#ifdef ENABLE_WAIT_FOR_COMMANDS
			omx_err = WaitForCommand(OMX_CommandFlush, m_input_port);//TODO timeout here?
			if(omx_err != OMX_ErrorNone)
			{
				ofLogError(__func__) << m_componentName << " WaitForCommand OMX_CommandFlush FAIL: " << printOMXError(omx_err);
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

		OMX_ERRORTYPE omx_err = OMX_ErrorNone;
		omx_err = OMX_SendCommand(m_handle, OMX_CommandFlush, m_output_port, NULL);
		if(omx_err != OMX_ErrorNone)
		{
			ofLogError(__func__) << m_componentName << " OMX_SendCommand OMX_CommandFlush FAIL: " << printOMXError(omx_err);
		}
		#ifdef ENABLE_WAIT_FOR_COMMANDS
			omx_err = WaitForCommand(OMX_CommandFlush, m_output_port);
			if(omx_err != OMX_ErrorNone)
			{
				ofLogError(__func__) << m_componentName << " WaitForCommand OMX_CommandFlush FAIL: " << printOMXError(omx_err);
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
	OMX_ERRORTYPE omx_err = OMX_ErrorNone;

	m_omx_input_use_buffers = use_buffers; 

	if(!m_handle)
	{
		ofLogError(__func__) << "NO HANDLE";
		return OMX_ErrorUndefined;
	}
	
	OMX_PARAM_PORTDEFINITIONTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = m_input_port;

	omx_err = OMX_GetParameter(m_handle, OMX_IndexParamPortDefinition, &portFormat);
	if(omx_err != OMX_ErrorNone)
	{
		return omx_err;
	}

	if(GetState() != OMX_StateIdle)
	{
		if(GetState() != OMX_StateLoaded)
		{
			SetStateForComponent(OMX_StateLoaded);
		}
		SetStateForComponent(OMX_StateIdle);
	}

	omx_err = EnablePort(m_input_port, false);
	if(omx_err != OMX_ErrorNone)
	{
		return omx_err;
	}

	m_input_alignment     = portFormat.nBufferAlignment;
	m_input_buffer_count  = portFormat.nBufferCountActual;
	m_input_buffer_size   = portFormat.nBufferSize;

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

	for (size_t i = 0; i < portFormat.nBufferCountActual; i++)
	{
		OMX_BUFFERHEADERTYPE *buffer = NULL;
		OMX_U8* data = NULL;

		if(m_omx_input_use_buffers)
		{
			data = (OMX_U8*)_aligned_malloc(portFormat.nBufferSize, m_input_alignment);
			omx_err = OMX_UseBuffer(m_handle, &buffer, m_input_port, NULL, portFormat.nBufferSize, data);
		}
		else
		{
			omx_err = OMX_AllocateBuffer(m_handle, &buffer, m_input_port, NULL, portFormat.nBufferSize);
		}
		if(omx_err != OMX_ErrorNone)
		{
			ofLogError(__func__) << m_componentName << " OMX_UseBuffer FAIL: " << printOMXError(omx_err);
			if(m_omx_input_use_buffers && data)
			{
				_aligned_free(data);
			}
			return omx_err;
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
		omx_err = WaitForCommand(OMX_CommandPortEnable, m_input_port);
		if(omx_err != OMX_ErrorNone)
		{
			ofLogError(__func__) << m_componentName << " WaitForCommand OMX_CommandPortEnable FAIL: " << printOMXError(omx_err);
		}
	#endif

	m_flush_input = false;

	return omx_err;
}

OMX_ERRORTYPE COMXCoreComponent::AllocOutputBuffers(bool use_buffers /* = false */)
{
	OMX_ERRORTYPE omx_err = OMX_ErrorNone;

	if(!m_handle)
	{
		ofLogError(__func__) << "NO HANDLE";
		return OMX_ErrorUndefined;
	}

	m_omx_output_use_buffers = use_buffers; 

	OMX_PARAM_PORTDEFINITIONTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = m_output_port;

	omx_err = OMX_GetParameter(m_handle, OMX_IndexParamPortDefinition, &portFormat);
	if(omx_err != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " OMX_GetParameter OMX_IndexParamPortDefinition FAIL: " << printOMXError(omx_err);
		return omx_err;
	}

	if(GetState() != OMX_StateIdle)
	{
		if(GetState() != OMX_StateLoaded)
		{
			SetStateForComponent(OMX_StateLoaded);
		}
		SetStateForComponent(OMX_StateIdle);
	}

	omx_err = EnablePort(m_output_port, false);
	if(omx_err != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " Enable Output Port FAIL: " << printOMXError(omx_err);
		return omx_err;
	}

	m_output_alignment     = portFormat.nBufferAlignment;
	m_output_buffer_count  = portFormat.nBufferCountActual;
	m_output_buffer_size   = portFormat.nBufferSize;
	stringstream info;
	info << __func__ << "\n";
	info << m_componentName << "\n";
	info << "port: " << m_output_port << "\n";
	info << "nBufferCountMin: " << portFormat.nBufferCountMin << "\n";
	info << "nBufferCountActual: " << portFormat.nBufferCountActual << "\n";
	info << "nBufferSize: " << portFormat.nBufferSize << "\n";
	info << "nBufferAlignmen: " << portFormat.nBufferAlignment << "\n";
	//ofLogVerbose(__func__) << info.str();
	//ofLog(OF_LOG_VERBOSE, 
//		  "COMXCoreComponent::AllocOutputBuffers component(%s) - port(%d), nBufferCountMin(%lu), nBufferCountActual(%lu), nBufferSize(%lu) nBufferAlignmen(%lu)\n", m_componentName.c_str(), m_output_port, portFormat.nBufferCountMin, portFormat.nBufferCountActual, portFormat.nBufferSize, portFormat.nBufferAlignment);

	for (size_t i = 0; i < portFormat.nBufferCountActual; i++)
	{
		OMX_BUFFERHEADERTYPE *buffer = NULL;
		OMX_U8* data = NULL;

		if(m_omx_output_use_buffers)
		{
			data = (OMX_U8*)_aligned_malloc(portFormat.nBufferSize, m_output_alignment);
			omx_err = OMX_UseBuffer(m_handle, &buffer, m_output_port, NULL, portFormat.nBufferSize, data);
			if(omx_err != OMX_ErrorNone)
			{
				ofLogError(__func__) << m_componentName << "OMX_UseBuffer FAIL: " << printOMXError(omx_err);
				if (data) 
				{
					_aligned_free(data);
				}
				return omx_err;
			}
		}
		else
		{
			omx_err = OMX_AllocateBuffer(m_handle, &buffer, m_output_port, NULL, portFormat.nBufferSize);
			if(omx_err != OMX_ErrorNone)
			{
				ofLogError(__func__) << m_componentName << "OMX_AllocateBuffer FAIL: " << printOMXError(omx_err);
				return omx_err;
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
		omx_err = WaitForCommand(OMX_CommandPortEnable, m_output_port);
		if(omx_err != OMX_ErrorNone)
		{
			ofLogError(__func__) << m_componentName << "WaitForCommand OMX_CommandPortEnable FAIL: " << printOMXError(omx_err);
		}
	#endif

	m_flush_output = false;

	return omx_err;
}

OMX_ERRORTYPE COMXCoreComponent::FreeInputBuffers(bool wait)
{
	OMX_ERRORTYPE omx_err = OMX_ErrorNone;

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

		omx_err = DisablePort(m_input_port);
		if(omx_err != OMX_ErrorNone)
		{
			ofLogError(__func__) << m_componentName << " DisablePort FAIL: " << printOMXError(omx_err);
		}
		for (size_t i = 0; i < m_omx_input_buffers.size(); i++)
		{
			uint8_t *buf = m_omx_input_buffers[i]->pBuffer;

			omx_err = OMX_FreeBuffer(m_handle, m_input_port, m_omx_input_buffers[i]);
			if(omx_err != OMX_ErrorNone)
			{
				ofLogError(__func__) << m_componentName << " OMX_FreeBuffer FAIL: " << printOMXError(omx_err);
			}
			if(m_omx_input_use_buffers && buf)
			{
				_aligned_free(buf);
			}

			
		}
		
		#ifdef ENABLE_WAIT_FOR_COMMANDS
			omx_err =  WaitForCommand(OMX_CommandPortDisable, m_input_port);
			if(omx_err != OMX_ErrorNone)
			{
				ofLogError(__func__) << m_componentName << " WaitForCommand OMX_CommandPortDisable FAIL: " << printOMXError(omx_err);
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

  return omx_err;
}

OMX_ERRORTYPE COMXCoreComponent::FreeOutputBuffers(bool wait)
{
	OMX_ERRORTYPE omx_err = OMX_ErrorNone;

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

		omx_err = DisablePort(m_output_port);
		if(omx_err != OMX_ErrorNone)
		{
			ofLogError(__func__) << m_componentName << " DisablePort FAIL: " << printOMXError(omx_err);
		}
	
		for (size_t i = 0; i < m_omx_output_buffers.size(); i++)
		{
			uint8_t *buf = m_omx_output_buffers[i]->pBuffer;

			omx_err = OMX_FreeBuffer(m_handle, m_output_port, m_omx_output_buffers[i]);
			if(omx_err != OMX_ErrorNone)
			{
				ofLogError(__func__) << m_componentName << " OMX_FreeBuffer FAIL: " << printOMXError(omx_err);
			}

			if(m_omx_output_use_buffers && buf)
			{
				_aligned_free(buf);
			}
		}
	
		#ifdef ENABLE_WAIT_FOR_COMMANDS
			omx_err =  WaitForCommand(OMX_CommandPortDisable, m_output_port);
			if(omx_err != OMX_ErrorNone)
			{
				ofLogError(__func__) << m_componentName << " WaitForCommand OMX_CommandPortDisable FAIL: " << printOMXError(omx_err);
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

  return omx_err;
}

OMX_ERRORTYPE COMXCoreComponent::DisableAllPorts()
{
	Lock();

		OMX_ERRORTYPE omx_err = OMX_ErrorNone;

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
			omx_err = OMX_GetParameter(m_handle, idxTypes[i], &ports);
			if(omx_err == OMX_ErrorNone) 
			{

				uint32_t j;
				for(j=0; j<ports.nPorts; j++)
				{
					OMX_PARAM_PORTDEFINITIONTYPE portFormat;
					OMX_INIT_STRUCTURE(portFormat);
					portFormat.nPortIndex = ports.nStartPortNumber+j;

					omx_err = OMX_GetParameter(m_handle, OMX_IndexParamPortDefinition, &portFormat);
					if(omx_err != OMX_ErrorNone)
					{
					  if(portFormat.bEnabled == OMX_FALSE)
						continue;
					}

					omx_err = OMX_SendCommand(m_handle, OMX_CommandPortDisable, ports.nStartPortNumber+j, NULL);
					if(omx_err != OMX_ErrorNone)
					{
						ofLogError(__func__) << m_componentName << " OMX_SendCommand OMX_CommandPortDisable FAIL: " << printOMXError(omx_err);
					}
					#ifdef ENABLE_WAIT_FOR_COMMANDS
						omx_err = WaitForCommand(OMX_CommandPortDisable, ports.nStartPortNumber+j);
						if(omx_err != OMX_ErrorNone && omx_err != OMX_ErrorSameState)
						{
							UnLock();
							return omx_err;
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
			ofLogError(__func__) << m_componentName << " WaitForEvent Event: " << printEventType(eventType) << " TIMED OUT at: " << timeout;
			pthread_mutex_unlock(&m_omx_event_mutex);
			return OMX_ErrorMax;
		}
	}
	
	pthread_mutex_unlock(&m_omx_event_mutex);
	return OMX_ErrorNone;
}

// timeout in milliseconds
OMX_ERRORTYPE COMXCoreComponent::WaitForCommand(OMX_U32 command, OMX_U32 nData2, long timeout) //timeout default = 2000
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
		return OMX_ErrorUndefined;
	
	Lock();
	
	OMX_ERRORTYPE omx_err = OMX_ErrorNone;
	OMX_STATETYPE state_actual = OMX_StateMax;
	
	if(state == state_actual)
	{
		UnLock();
		return OMX_ErrorNone;
	}
	
	omx_err = OMX_SendCommand(m_handle, OMX_CommandStateSet, state, 0);
	if (omx_err != OMX_ErrorNone)
	{
		if(omx_err == OMX_ErrorSameState)
		{
			omx_err = OMX_ErrorNone;
		}
		else
		{
			ofLogError(__func__) << " OMX_GetState FAIL " << printOMXError(omx_err);
		}
	}
	else 
	{
		omx_err = WaitForCommand(OMX_CommandStateSet, state, COMMAND_WAIT_TIMEOUT);
		if(omx_err == OMX_ErrorSameState)
		{
			ofLogError(__func__) << " OMX_GetState FAIL " << printOMXError(omx_err);
			UnLock();
			return OMX_ErrorNone;
		}
	}
	
	UnLock();
	
	return omx_err;
#if 0
	OMX_ERRORTYPE omx_err = OMX_ErrorNone;
	Lock();
		
		OMX_STATETYPE state_actual = OMX_StateMax;

		if(!m_handle)
		{
			ofLogError(__func__) << "NO HANDLE";
			UnLock();
			return OMX_ErrorUndefined;
		}
	
		omx_err = OMX_GetState(m_handle, &state_actual);
		if(omx_err != OMX_ErrorNone) 
		{
			ofLogError(__func__) << " OMX_GetState FAIL " << printOMXError(omx_err);
		}
		if(state == state_actual)
		{
			UnLock();
			return OMX_ErrorNone;
		}

		omx_err = OMX_SendCommand(m_handle, OMX_CommandStateSet, state, 0);
		if (omx_err != OMX_ErrorNone)
		{
			if(omx_err == OMX_ErrorSameState)
			{
				omx_err = OMX_ErrorNone;
			}
			else
			{
				ofLogError(__func__) << m_componentName << " SetStateForComponent : " << printState(state) << " FAIL: " << printOMXError(omx_err);
			}
		}
		else 
		{
			#ifdef ENABLE_WAIT_FOR_COMMANDS
			omx_err = WaitForCommand(OMX_CommandStateSet, state, 50);
			if(omx_err == OMX_ErrorSameState)
			{
				//ofLogVerbose(__func__) << m_componentName << " IGNORING OMX_ErrorSameState"; 
				UnLock();
				return OMX_ErrorNone;
			}
			#endif
		}
	UnLock();

	return omx_err;
#endif
}

OMX_STATETYPE COMXCoreComponent::GetState()
{
	
	//ofLogVerbose(__func__) << m_componentName << " START";
	if(m_handle)
	{
		Lock();
		OMX_STATETYPE state;
		OMX_ERRORTYPE omx_err = OMX_GetState(m_handle, &state);
		if(omx_err != OMX_ErrorNone) 
		{
			ofLogError(__func__) << " OMX_GetState FAIL " << printOMXError(omx_err);
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
	
		OMX_ERRORTYPE omx_err = OMX_SetParameter(m_handle, paramIndex, paramStruct);
		if(omx_err != OMX_ErrorNone) 
		{
			ofLogError(__func__) << m_componentName << " FAIL " << printOMXError(omx_err);
		}
	
	UnLock();
	return omx_err;
}

OMX_ERRORTYPE COMXCoreComponent::GetParameter(OMX_INDEXTYPE paramIndex, OMX_PTR paramStruct)
{
	Lock();
	
		OMX_ERRORTYPE omx_err = OMX_GetParameter(m_handle, paramIndex, paramStruct);
		if(omx_err != OMX_ErrorNone) 
		{
			ofLogError(__func__) << m_componentName << " FAIL " << printOMXError(omx_err);
		}
	
	UnLock();
	return omx_err;
}

OMX_ERRORTYPE COMXCoreComponent::SetConfig(OMX_INDEXTYPE configIndex, OMX_PTR configStruct)
{
	Lock();
	//ofLogVerbose(__func__) << m_componentName << " START";
		OMX_ERRORTYPE omx_err = OMX_SetConfig(m_handle, configIndex, configStruct);
		if(omx_err != OMX_ErrorNone) 
		{
			ofLogError(__func__) << m_componentName << " OMX_SetConfig FAIL " << printOMXError(omx_err);
		}else 
		{
			//ofLogVerbose(__func__) << m_componentName << " OMX_SetConfig PASS";
		}

	//ofLogVerbose(__func__) << m_componentName << " END";
	UnLock();
	return omx_err;
}

OMX_ERRORTYPE COMXCoreComponent::GetConfig(OMX_INDEXTYPE configIndex, OMX_PTR configStruct)
{
	Lock();

		OMX_ERRORTYPE omx_err = OMX_GetConfig(m_handle, configIndex, configStruct);
		if(omx_err != OMX_ErrorNone) 
		{
			ofLogError(__func__) << m_componentName << " FAIL " << printOMXError(omx_err);
		}
	
	UnLock();
	return omx_err;
}

OMX_ERRORTYPE COMXCoreComponent::SendCommand(OMX_COMMANDTYPE cmd, OMX_U32 cmdParam, OMX_PTR cmdParamData)
{
	Lock();

		OMX_ERRORTYPE omx_err = OMX_SendCommand(m_handle, cmd, cmdParam, cmdParamData);
		if(omx_err != OMX_ErrorNone) 
		{
			ofLogError(__func__) << m_componentName << " FAIL " << printOMXError(omx_err);
		}

	UnLock();
	return omx_err;
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

		OMX_ERRORTYPE omx_err = OMX_GetParameter(m_handle, OMX_IndexParamPortDefinition, &portFormat);
		if(omx_err != OMX_ErrorNone)
		{
			ofLogError(__func__) << m_componentName << " OMX_GetParameter FAIL " << " port " << printOMXError(omx_err);
		}

		if(portFormat.bEnabled == OMX_FALSE)
		{
			omx_err = OMX_SendCommand(m_handle, OMX_CommandPortEnable, port, NULL);
			if(omx_err != OMX_ErrorNone)
			{
				ofLogError(__func__) << m_componentName << "OMX_SendCommand OMX_CommandPortEnable FAIL " << " port " << printOMXError(omx_err);
				UnLock();
				return omx_err;
		  
			}
			else
			{
				if(wait)
				{
					omx_err = WaitForCommand(OMX_CommandPortEnable, port, COMMAND_WAIT_TIMEOUT);
				}
			}
		}

	UnLock();

	return omx_err;
}

OMX_ERRORTYPE COMXCoreComponent::DisablePort(unsigned int port, bool wait)//default: wait=false
{
	
	OMX_ERRORTYPE omx_err = OMX_ErrorUndefined;
	
	Lock();
		//ofLogVerbose(__func__) << " componentName: "  << m_componentName << " START" << " port: " << port << " wait: " << wait;

		omx_err = OMX_SendCommand(m_handle, OMX_CommandPortDisable, port, NULL);
		if(omx_err == OMX_ErrorNone)
		{
			//ofLogVerbose(__func__) << m_componentName << " OMX_SendCommand OMX_CommandPortDisable PASS";
			UnLock();
			return omx_err;
		}
		//ofLogVerbose(__func__) << m_componentName << "GOING THE LONG ROUTE";
		OMX_PARAM_PORTDEFINITIONTYPE portFormat;
		OMX_INIT_STRUCTURE(portFormat);
		portFormat.nPortIndex = port;

		 omx_err = OMX_GetParameter(m_handle, OMX_IndexParamPortDefinition, &portFormat);
		if(omx_err == OMX_ErrorNone)
		{
			//ofLogVerbose(__func__) << m_componentName << " OMX_GetParameter PASS";
			
		}else 
		{
			ofLogError(__func__) << m_componentName << " OMX_GetParameter FAIL " << printOMXError(omx_err);
		}


		if(portFormat.bEnabled == OMX_TRUE)
		{
			omx_err = OMX_SendCommand(m_handle, OMX_CommandPortDisable, port, NULL);
			if(omx_err != OMX_ErrorNone)
			{
				ofLogError(__func__) << m_componentName << "OMX_SendCommand OMX_CommandPortDisable FAIL " << " port " << printOMXError(omx_err);
				UnLock();
				return omx_err;
			}
			else
			{
				if(wait)
				{
					omx_err = WaitForCommand(OMX_CommandPortDisable, port, COMMAND_WAIT_TIMEOUT);
					if(omx_err != OMX_ErrorNone)
					{
						ofLogError(__func__) << m_componentName << "WaitForCommand OMX_CommandPortDisable FAIL " << " port " << printOMXError(omx_err);
						UnLock();
						return omx_err;
					}
				}
			}
		}
	//ofLogVerbose(__func__) << " componentName: "  << m_componentName << " END" << " port: " << port << " wait: " << wait;
	UnLock();
	return omx_err;
}

OMX_ERRORTYPE COMXCoreComponent::UseEGLImage(OMX_BUFFERHEADERTYPE** ppBufferHdr, OMX_U32 nPortIndex, OMX_PTR pAppPrivate, void* eglImage)
{
	Lock();

		OMX_ERRORTYPE omx_err = OMX_UseEGLImage(m_handle, ppBufferHdr, nPortIndex, pAppPrivate, eglImage);
		if(omx_err != OMX_ErrorNone) 
		{
		  ofLogError(__func__) << m_componentName << " FAIL " << printOMXError(omx_err);
		}

	UnLock();
	return omx_err;
}

bool COMXCoreComponent::Initialize( const std::string &component_name, OMX_INDEXTYPE index)
{
	//ofLogVerbose(__func__) << " component_name: " << component_name;
	
	m_componentName = component_name;

	m_callbacks.EventHandler    = &COMXCoreComponent::DecoderEventHandlerCallback;
	m_callbacks.EmptyBufferDone = &COMXCoreComponent::DecoderEmptyBufferDoneCallback;
	m_callbacks.FillBufferDone  = &COMXCoreComponent::DecoderFillBufferDoneCallback;

	// Get video component handle setting up callbacks, component is in loaded state on return.
	OMX_ERRORTYPE omx_err = OMX_GetHandle(&m_handle, (char*)component_name.c_str(), this, &m_callbacks);
	
	if (omx_err != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << "OMX_GetHandle FAIL " << printOMXError(omx_err);
		Deinitialize();
		return false;
	}

	OMX_PORT_PARAM_TYPE port_param;
	OMX_INIT_STRUCTURE(port_param);

	omx_err = OMX_GetParameter(m_handle, index, &port_param);
	if (omx_err != OMX_ErrorNone)
	{ 
		ofLogError(__func__) << m_componentName << " OMX_GetParameter FAIL " << printOMXError(omx_err);
	}

	omx_err = DisableAllPorts();
	if (omx_err != OMX_ErrorNone)
	{
		ofLogError(__func__) << m_componentName << " DisableAllPorts FAIL " << printOMXError(omx_err);
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

		OMX_ERRORTYPE omx_err = OMX_FreeHandle(m_handle);
		if (omx_err != OMX_ErrorNone)
		{
			ofLogError(__func__) << m_componentName << " OMX_FreeHandle FAIL " << printOMXError(omx_err);
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

///////////////////////////////////////////////////////////////////////////////////////////
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
    return OMX_ErrorNone;

	if (eEvent == OMX_EventPortSettingsChanged ) 
	{
		//ofLogVerbose() << "OMX_EventPortSettingsChanged at ofGetElapsedTimeMillis: " << ofGetElapsedTimeMillis();
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
    return OMX_ErrorNone;

  COMXCoreComponent *ctx = static_cast<COMXCoreComponent*>(pAppData);

  if(ctx->CustomDecoderEmptyBufferDoneHandler){
    OMX_ERRORTYPE omx_err = (*(ctx->CustomDecoderEmptyBufferDoneHandler))(hComponent, pAppData, pBuffer);
    if(omx_err != OMX_ErrorNone)return omx_err;
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
    return OMX_ErrorNone;

  COMXCoreComponent *ctx = static_cast<COMXCoreComponent*>(pAppData);
 
  if(ctx->CustomDecoderFillBufferDoneHandler){
    OMX_ERRORTYPE omx_err = (*(ctx->CustomDecoderFillBufferDoneHandler))(hComponent, pAppData, pBuffer);
    if(omx_err != OMX_ErrorNone)return omx_err;
  }

  return ctx->DecoderFillBufferDone(hComponent, pAppData, pBuffer);
}

OMX_ERRORTYPE COMXCoreComponent::DecoderEmptyBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
  if(!pAppData || m_exit)
    return OMX_ErrorNone;

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
    return OMX_ErrorNone;
  
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
////////////////////////////////////////////////////////////////////////////////////////////
// Component event handler -- OMX event callback
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
 ofLog(OF_LOG_VERBOSE, "COMXCore::%s - %s eEvent(0x%x), nData1(0x%lx), nData2(0x%lx), pEventData(0x%p)\n", __func__, (char *)ctx->GetName().c_str(), eEvent, nData1, nData2, pEventData);
#endif

  AddEvent(eEvent, nData1, nData2);

  switch (eEvent)
  {
    case OMX_EventCmdComplete:
      
      switch(nData1)
      {
        case OMX_CommandStateSet:
		  #if defined(OMX_DEBUG_EVENTHANDLER)
			  //ofLogVerbose(__func__) << ctx->GetName() << " OMX_CommandStateSet " << printState((int)nData2);
		  #endif
		break;
        case OMX_CommandFlush:
          #if defined(OMX_DEBUG_EVENTHANDLER)
         ofLog(OF_LOG_VERBOSE, "\n%s::%s %s - OMX_CommandFlush, port %d\n", CLASSNAME, __func__, ctx->GetName().c_str(), (int)nData2);
          #endif
        break;
        case OMX_CommandPortDisable:
          #if defined(OMX_DEBUG_EVENTHANDLER)
         ofLog(OF_LOG_VERBOSE, "\n%s::%s %s - OMX_CommandPortDisable, nData1(0x%lx), port %d\n", CLASSNAME, __func__, ctx->GetName().c_str(), nData1, (int)nData2);
          #endif
        break;
        case OMX_CommandPortEnable:
          #if defined(OMX_DEBUG_EVENTHANDLER)
         ofLog(OF_LOG_VERBOSE, "\n%s::%s %s - OMX_CommandPortEnable, nData1(0x%lx), port %d\n", CLASSNAME, __func__, ctx->GetName().c_str(), nData1, (int)nData2);
          #endif
        break;
        #if defined(OMX_DEBUG_EVENTHANDLER)
        case OMX_CommandMarkBuffer:
         ofLog(OF_LOG_VERBOSE, "\n%s::%s %s - OMX_CommandMarkBuffer, nData1(0x%lx), port %d\n", CLASSNAME, __func__, ctx->GetName().c_str(), nData1, (int)nData2);
        break;
        #endif
      }
    break;
    case OMX_EventBufferFlag:
      #if defined(OMX_DEBUG_EVENTHANDLER)
     ofLog(OF_LOG_VERBOSE, "\n%s::%s %s - OMX_EventBufferFlag(input)\n", CLASSNAME, __func__, ctx->GetName().c_str());
      #endif
      if(nData2 & OMX_BUFFERFLAG_EOS)
	  {
		  //ofLogVerbose() << "OMX_EventBufferFlag::OMX_BUFFERFLAG_EOS RECEIVED";
		  ctx->m_eos = true;
	  }
       
    break;
    case OMX_EventPortSettingsChanged:
      #if defined(OMX_DEBUG_EVENTHANDLER)
     ofLog(OF_LOG_VERBOSE, "\n%s::%s %s - OMX_EventPortSettingsChanged(output)\n", CLASSNAME, __func__, ctx->GetName().c_str());
      #endif
    break;
    #if defined(OMX_DEBUG_EVENTHANDLER)
    case OMX_EventMark:
     ofLog(OF_LOG_VERBOSE, "\n%s::%s %s - OMX_EventMark\n", CLASSNAME, __func__, ctx->GetName().c_str());
    break;
    case OMX_EventResourcesAcquired:
     ofLog(OF_LOG_VERBOSE, "\n%s::%s %s- OMX_EventResourcesAcquired\n", CLASSNAME, __func__, ctx->GetName().c_str());
    break;
    #endif
    case OMX_EventError:
      switch((OMX_S32)nData1)
      {
        case OMX_ErrorSameState:
        break;
        case OMX_ErrorInsufficientResources:
         ofLog(OF_LOG_VERBOSE, "\n%s::%s %s - OMX_ErrorInsufficientResources, insufficient resources\n", CLASSNAME, __func__, ctx->GetName().c_str());
        break;
        case OMX_ErrorFormatNotDetected:
         ofLog(OF_LOG_VERBOSE, "\n%s::%s %s - OMX_ErrorFormatNotDetected, cannot parse input stream\n", CLASSNAME, __func__, ctx->GetName().c_str());
        break;
        case OMX_ErrorPortUnpopulated:
         ofLog(OF_LOG_VERBOSE, "\n%s::%s %s - OMX_ErrorPortUnpopulated port %d, cannot parse input stream\n", CLASSNAME, __func__, ctx->GetName().c_str(), (int)nData2);
        break;
        case OMX_ErrorStreamCorrupt:
         ofLog(OF_LOG_VERBOSE, "\n%s::%s %s - OMX_ErrorStreamCorrupt, Bitstream corrupt\n", CLASSNAME, __func__, ctx->GetName().c_str());
        break;
        default:
         ofLog(OF_LOG_VERBOSE, "\n%s::%s %s - OMX_EventError detected, nData1(0x%lx), port %d\n",  CLASSNAME, __func__, ctx->GetName().c_str(), nData1, (int)nData2);
        break;
      }
      sem_post(&ctx->m_omx_fill_buffer_done);
    break;
    default:
     ofLog(OF_LOG_VERBOSE, "\n%s::%s %s - Unknown eEvent(0x%x), nData1(0x%lx), port %d\n", CLASSNAME, __func__, ctx->GetName().c_str(), eEvent, nData1, (int)nData2);
    break;
  }

  return OMX_ErrorNone;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
COMXCore::COMXCore()
{
  m_is_open = false;

}



bool COMXCore::Initialize()
{
	ofLogVerbose() << "COMXCore::Initialize";
	OMX_ERRORTYPE omx_err = OMX_Init();
	
	if (omx_err != OMX_ErrorNone)
	{
		ofLogError(__func__)  << " FAIL " << printOMXError(omx_err);
		return false;
	}
	
	m_is_open = true;
	return true;
}

void COMXCore::Deinitialize()
{
	if(m_is_open)
	{
		ofLogVerbose() << "COMXCore::Deinitialize";
		OMX_ERRORTYPE omx_err = OMX_Deinit();
		if (omx_err != OMX_ErrorNone)
		{
			ofLogError(__func__)  << " FAIL " << printOMXError(omx_err);
		}  
	}
}
