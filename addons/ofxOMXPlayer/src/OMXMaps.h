#pragma once

#include "ofMain.h"

#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>
#include <IL/OMX_Index.h>
#include <IL/OMX_Image.h>
#include <IL/OMX_Video.h>
#include <IL/OMX_Broadcom.h>

class OMXMaps
{
	public:
		static OMXMaps& getInstance()
		{
			static OMXMaps    instance;
			return instance;
		}
		map<OMX_ERRORTYPE, string> omxErrors;
		map<OMX_STATETYPE, string> omxStates;
		map<OMX_EVENTTYPE, string> omxEventTypes;
		map<OMX_COMMANDTYPE, string> omxCommands;
	private:
		OMXMaps()
		{

			omxErrors[OMX_ErrorNone] =  "OMX_ErrorNone";
			omxErrors[OMX_ErrorInsufficientResources] =  "OMX_ErrorInsufficientResources";
			omxErrors[OMX_ErrorUndefined] =  "OMX_ErrorUndefined";
			omxErrors[OMX_ErrorInvalidComponentName] =  "OMX_ErrorInvalidComponentName";
			omxErrors[OMX_ErrorComponentNotFound] =  "OMX_ErrorComponentNotFound";
			omxErrors[OMX_ErrorInvalidComponent] =  "OMX_ErrorInvalidComponent";
			omxErrors[OMX_ErrorBadParameter] =  "OMX_ErrorBadParameter";
			omxErrors[OMX_ErrorNotImplemented] =  "OMX_ErrorNotImplemented";
			omxErrors[OMX_ErrorUnderflow] =  "OMX_ErrorUnderflow";
			omxErrors[OMX_ErrorOverflow] =  "OMX_ErrorOverflow";
			omxErrors[OMX_ErrorHardware] =  "OMX_ErrorHardware";
			omxErrors[OMX_ErrorInvalidState] =  "OMX_ErrorInvalidState";
			omxErrors[OMX_ErrorStreamCorrupt] =  "OMX_ErrorStreamCorrupt";
			omxErrors[OMX_ErrorPortsNotCompatible] =  "OMX_ErrorPortsNotCompatible";
			omxErrors[OMX_ErrorResourcesLost] =  "OMX_ErrorResourcesLost";
			omxErrors[OMX_ErrorNoMore] =  "OMX_ErrorNoMore";
			omxErrors[OMX_ErrorVersionMismatch] =  "OMX_ErrorVersionMismatch";
			omxErrors[OMX_ErrorNotReady] =  "OMX_ErrorNotReady";
			omxErrors[OMX_ErrorTimeout] =  "OMX_ErrorTimeout";
			omxErrors[OMX_ErrorSameState] =  "OMX_ErrorSameState";
			omxErrors[OMX_ErrorResourcesPreempted] =  "OMX_ErrorResourcesPreempted";
			omxErrors[OMX_ErrorPortUnresponsiveDuringAllocation] =  "OMX_ErrorPortUnresponsiveDuringAllocation";
			omxErrors[OMX_ErrorPortUnresponsiveDuringDeallocation] =  "OMX_ErrorPortUnresponsiveDuringDeallocation";
			omxErrors[OMX_ErrorPortUnresponsiveDuringStop] =  "OMX_ErrorPortUnresponsiveDuringStop";
			omxErrors[OMX_ErrorIncorrectStateTransition] =  "OMX_ErrorIncorrectStateTransition";
			omxErrors[OMX_ErrorIncorrectStateOperation] =  "OMX_ErrorIncorrectStateOperation";
			omxErrors[OMX_ErrorUnsupportedSetting] =  "OMX_ErrorUnsupportedSetting";
			omxErrors[OMX_ErrorUnsupportedIndex] =  "OMX_ErrorUnsupportedIndex";
			omxErrors[OMX_ErrorBadPortIndex] =  "OMX_ErrorBadPortIndex";
			omxErrors[OMX_ErrorPortUnpopulated] =  "OMX_ErrorPortUnpopulated";
			omxErrors[OMX_ErrorComponentSuspended] =  "OMX_ErrorComponentSuspended";
			omxErrors[OMX_ErrorDynamicResourcesUnavailable] =  "OMX_ErrorDynamicResourcesUnavailable";
			omxErrors[OMX_ErrorMbErrorsInFrame] =  "OMX_ErrorMbErrorsInFrame";
			omxErrors[OMX_ErrorFormatNotDetected] =  "OMX_ErrorFormatNotDetected";
			omxErrors[OMX_ErrorContentPipeOpenFailed] =  "OMX_ErrorContentPipeOpenFailed";
			omxErrors[OMX_ErrorContentPipeCreationFailed] =  "OMX_ErrorContentPipeCreationFailed";
			omxErrors[OMX_ErrorSeperateTablesUsed] =  "OMX_ErrorSeperateTablesUsed";
			omxErrors[OMX_ErrorTunnelingUnsupported] =  "OMX_ErrorTunnelingUnsupported";
			omxErrors[OMX_ErrorKhronosExtensions] =  "OMX_ErrorKhronosExtensions";
			omxErrors[OMX_ErrorVendorStartUnused] =  "OMX_ErrorVendorStartUnused";
			omxErrors[OMX_ErrorDiskFull] =  "OMX_ErrorDiskFull";
			omxErrors[OMX_ErrorMaxFileSize] =  "OMX_ErrorMaxFileSize";
			omxErrors[OMX_ErrorDrmUnauthorised] =  "OMX_ErrorDrmUnauthorised";
			omxErrors[OMX_ErrorDrmExpired] =  "OMX_ErrorDrmExpired";
			omxErrors[OMX_ErrorDrmGeneral] =  "OMX_ErrorDrmGeneral";




			omxStates[OMX_StateExecuting] = "OMX_StateExecuting";
			omxStates[OMX_StateIdle] = "OMX_StateIdle";
			omxStates[OMX_StateLoaded] = "OMX_StateLoaded";
			omxStates[OMX_StateInvalid] = "OMX_StateInvalid";
			omxStates[OMX_StatePause] = "OMX_StatePause";
			omxStates[OMX_StateWaitForResources] = "OMX_StateWaitForResources";
			omxStates[OMX_StateKhronosExtensions] = "OMX_StateKhronosExtensions";
			omxStates[OMX_StateVendorStartUnused] = "OMX_StateVendorStartUnused";
			omxStates[OMX_StateMax] = "OMX_StateMax";



			omxEventTypes[OMX_EventCmdComplete]= "OMX_EventCmdComplete";
			omxEventTypes[OMX_EventMax]= "OMX_EventMax";
			omxEventTypes[OMX_EventError]= "OMX_EventError";
			omxEventTypes[OMX_EventMark]= "OMX_EventMark";
			omxEventTypes[OMX_EventPortSettingsChanged]= "OMX_EventPortSettingsChanged";
			omxEventTypes[OMX_EventBufferFlag]= "OMX_EventBufferFlag";
			omxEventTypes[OMX_EventResourcesAcquired]= "OMX_EventResourcesAcquired";
			omxEventTypes[OMX_EventComponentResumed]= "OMX_EventComponentResumed";
			omxEventTypes[OMX_EventDynamicResourcesAvailable]= "OMX_EventDynamicResourcesAvailable";
			omxEventTypes[OMX_EventPortFormatDetected]= "OMX_EventPortFormatDetected";
			omxEventTypes[OMX_EventKhronosExtensions]= "OMX_EventKhronosExtensions";
			omxEventTypes[OMX_EventVendorStartUnused]= "OMX_EventVendorStartUnused";
			omxEventTypes[OMX_EventParamOrConfigChanged]= "OMX_EventParamOrConfigChanged";


			omxCommands[OMX_CommandVendorStartUnused]= "OMX_CommandVendorStartUnused";
			omxCommands[OMX_CommandMax]= "OMX_CommandMax";
			omxCommands[OMX_CommandStateSet]= "OMX_CommandStateSet";
			omxCommands[OMX_CommandFlush]= "OMX_CommandFlush";
			omxCommands[OMX_CommandPortDisable]= "OMX_CommandPortDisable";
			omxCommands[OMX_CommandPortEnable]= "OMX_CommandPortEnable";
			omxCommands[OMX_CommandMarkBuffer]= "OMX_CommandMarkBuffer";
			omxCommands[OMX_CommandKhronosExtensions]= "OMX_CommandKhronosExtensions";


		}
		~OMXMaps() {};
		OMXMaps(OMXMaps const&);
		void operator=(OMXMaps const&);

};
