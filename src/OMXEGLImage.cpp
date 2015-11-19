#include "OMXEGLImage.h"


OMXEGLImage::OMXEGLImage()
{
	frameCounter = 0;
	frameOffset = 0;
}


OMX_ERRORTYPE OMXEGLImage::onFillBufferDone(OMX_HANDLETYPE hComponent,
                               OMX_PTR pAppData,
                               OMX_BUFFERHEADERTYPE* pBuffer)
{

	Component *component = static_cast<Component*>(pAppData);
	
	OMX_ERRORTYPE didFillBuffer = OMX_FillThisBuffer(hComponent, pBuffer);
		
	if (didFillBuffer == OMX_ErrorNone)
	{
	
		component->incrementFrameCounter();
	}

	return didFillBuffer;
}

int OMXEGLImage::getCurrentFrame()
{
	
	return renderComponent.getCurrentFrame();
}
void OMXEGLImage::resetFrameCounter()
{
	//frameOffset = renderComponent.getCurrentFrame();
	renderComponent.resetFrameCounter();
}

bool OMXEGLImage::Open(COMXStreamInfo& hints, OMXClock *clock, EGLImageKHR eglImage)
{


	OMX_ERRORTYPE error   = OMX_ErrorNone;

	videoWidth  = hints.width;
	videoHeight = hints.height;



	if(!videoWidth || !videoHeight)
	{
		return false;
	}

	if(hints.extrasize > 0 && hints.extradata != NULL)
	{
		extraSize = hints.extrasize;
		extraData = (uint8_t *)malloc(extraSize);
		memcpy(extraData, hints.extradata, hints.extrasize);
	}

	ProcessCodec(hints);


	std::string componentName = "OMX.broadcom.video_decode";
	if(!decoderComponent.init(componentName, OMX_IndexParamVideoInit))
	{
		return false;
	}

	componentName = "OMX.broadcom.egl_render";
	if(!renderComponent.init(componentName, OMX_IndexParamVideoInit))
	{
		return false;
	}

	componentName = "OMX.broadcom.video_scheduler";
	if(!schedulerComponent.init(componentName, OMX_IndexParamVideoInit))
	{
		return false;
	}

	if(clock == NULL)
	{
		return false;
	}

	omxClock = clock;
	clockComponent = omxClock->getComponent();

	if(clockComponent->getHandle() == NULL)
	{
		omxClock = NULL;
		clockComponent = NULL;
		return false;
	}

	decoderTunnel.init(&decoderComponent,		decoderComponent.getOutputPort(),		&schedulerComponent,	schedulerComponent.getInputPort());
	schedulerTunnel.init(	&schedulerComponent,		schedulerComponent.getOutputPort(),		&renderComponent,	renderComponent.getInputPort());
	clockTunnel.init(	clockComponent,		clockComponent->getInputPort() + 1,	&schedulerComponent,	schedulerComponent.getOutputPort() + 1);


	error = decoderComponent.setState(OMX_StateIdle);
	if (error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "decoderComponent OMX_StateIdle FAIL";
		return false;
	}

	OMX_VIDEO_PARAM_PORTFORMATTYPE formatType;
	OMX_INIT_STRUCTURE(formatType);
	formatType.nPortIndex = decoderComponent.getInputPort();
	formatType.eCompressionFormat = m_codingType;

	if (hints.fpsscale > 0 && hints.fpsrate > 0)
	{
		formatType.xFramerate = (long long)(1<<16)*hints.fpsrate / hints.fpsscale;
	}
	else
	{
		formatType.xFramerate = 25 * (1<<16);
	}

	error = decoderComponent.setParameter(OMX_IndexParamVideoPortFormat, &formatType);
    OMX_TRACE(error);

	if(error != OMX_ErrorNone)
	{
		return false;
	}
	

	OMX_PARAM_PORTDEFINITIONTYPE portParam;
	OMX_INIT_STRUCTURE(portParam);
	portParam.nPortIndex = decoderComponent.getInputPort();

	error = decoderComponent.getParameter(OMX_IndexParamPortDefinition, &portParam);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	int numVideoBuffers = 32; //20 is minimum - can get up to 80
	portParam.nBufferCountActual = numVideoBuffers;

	portParam.format.video.nFrameWidth  = videoWidth;
	portParam.format.video.nFrameHeight = videoHeight;


	error = decoderComponent.setParameter(OMX_IndexParamPortDefinition, &portParam);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;


	error = clockTunnel.Establish(false);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;


	OMX_PARAM_BRCMVIDEODECODEERRORCONCEALMENTTYPE concanParam;
	OMX_INIT_STRUCTURE(concanParam);
	concanParam.bStartWithValidFrame = OMX_FALSE;

	error = decoderComponent.setParameter(OMX_IndexParamBrcmVideoDecodeErrorConcealment, &concanParam);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	if(NaluFormatStartCodes(hints.codec, extraData, extraSize))
	{
		OMX_NALSTREAMFORMATTYPE nalStreamFormat;
		OMX_INIT_STRUCTURE(nalStreamFormat);
		nalStreamFormat.nPortIndex = decoderComponent.getInputPort();
		nalStreamFormat.eNaluFormat = OMX_NaluFormatStartCodes;

		error = decoderComponent.setParameter((OMX_INDEXTYPE)OMX_IndexParamNalStreamFormatSelect, &nalStreamFormat);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone) return false;

	}

	// broadcom omx entension:
	// When enabled, the timestamp fifo mode will change the way incoming timestamps are associated with output images.
	// In this mode the incoming timestamps get used without re-ordering on output images.
    
    //recent firmware will actually automatically choose the timestamp stream with the least variance, so always enable

    OMX_CONFIG_BOOLEANTYPE timeStampMode;
    OMX_INIT_STRUCTURE(timeStampMode);
    timeStampMode.bEnabled = OMX_TRUE;
    error = decoderComponent.setParameter((OMX_INDEXTYPE)OMX_IndexParamBrcmVideoTimestampFifo, &timeStampMode);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;


	// Alloc buffers for the omx intput port.
	error = decoderComponent.allocInputBuffers();
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;



	error = decoderTunnel.Establish(false);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	error = decoderComponent.setState(OMX_StateExecuting);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;


	error = schedulerTunnel.Establish(false);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	error = schedulerComponent.setState(OMX_StateExecuting);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;


	OMX_PARAM_PORTDEFINITIONTYPE portParamRenderInput;
	OMX_INIT_STRUCTURE(portParamRenderInput);
	portParamRenderInput.nPortIndex = renderComponent.getInputPort();

	error = renderComponent.getParameter(OMX_IndexParamPortDefinition, &portParamRenderInput);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	OMX_PARAM_PORTDEFINITIONTYPE portParamRenderOutput;
	OMX_INIT_STRUCTURE(portParamRenderOutput);
	portParamRenderOutput.nPortIndex = renderComponent.getOutputPort();

	error = renderComponent.getParameter(OMX_IndexParamPortDefinition, &portParamRenderOutput);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;
	
	// Alloc buffers for the renderComponent input port.
	error = renderComponent.allocInputBuffers();
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;
	
	
	error = renderComponent.setState(OMX_StateIdle);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;
	

	//ofLogVerbose(__func__) << "renderComponent.getOutputPort(): " << renderComponent.getOutputPort();
	renderComponent.enablePort(renderComponent.getOutputPort());
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;


	OMX_BUFFERHEADERTYPE* eglBuffer = NULL;
	error = renderComponent.useEGLImage(&eglBuffer, renderComponent.getOutputPort(), NULL, eglImage);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;


	if(SendDecoderConfig())
	{
		//ofLogVerbose(__func__) << "SendDecoderConfig PASS";
	}
	else
	{
		ofLog(OF_LOG_ERROR, "SendDecoderConfig FAIL");
		return false;
	}


	renderComponent.SetCustomDecoderFillBufferDoneHandler(&OMXEGLImage::onFillBufferDone);
	error = renderComponent.setState(OMX_StateExecuting);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;
    
	error = renderComponent.FillThisBuffer(eglBuffer);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	isOpen           = true;
	m_drop_state        = false;
	doSetStartTime      = true;


	ofLog(OF_LOG_VERBOSE,
	      "%s::%s - decoder_component: 0x%p, input_port: 0x%x, output_port: 0x%x \n",
	      "OMXEGLImage", __func__, decoderComponent.getHandle(), decoderComponent.getInputPort(), decoderComponent.getOutputPort());

	isFirstFrame   = true;
	// start from assuming all recent frames had valid pts
	validHistoryPTS = ~0;
	return true;
}

bool OMXEGLImage::Decode(uint8_t *pData, int iSize, double pts)
{
	CSingleLock lock (m_critSection);
	OMX_ERRORTYPE error;

	if( m_drop_state || !isOpen )
	{
		return true;
	}

	unsigned int demuxer_bytes = (unsigned int)iSize;
	uint8_t *demuxer_content = pData;

	if (demuxer_content && demuxer_bytes > 0)
	{
		while(demuxer_bytes)
		{
			// 500ms timeout
			OMX_BUFFERHEADERTYPE *omxBuffer = decoderComponent.getInputBuffer(500);
			if(omxBuffer == NULL)
			{
                ofLogVerbose(__func__) << "timeout";
				return false;
			}

			omxBuffer->nFlags = 0;
			omxBuffer->nOffset = 0;

			if(doSetStartTime)
			{
				omxBuffer->nFlags |= OMX_BUFFERFLAG_STARTTIME;
				ofLog(OF_LOG_VERBOSE, "%s : setStartTime %f\n", __func__, (pts == DVD_NOPTS_VALUE ? 0.0 : pts) / DVD_TIME_BASE);
				doSetStartTime = false;
			}
			else if(pts == DVD_NOPTS_VALUE)
			{
				omxBuffer->nFlags |= OMX_BUFFERFLAG_TIME_UNKNOWN;
			}

			omxBuffer->nTimeStamp = ToOMXTime((uint64_t)(pts == DVD_NOPTS_VALUE) ? 0 : pts);
			omxBuffer->nFilledLen = (demuxer_bytes > omxBuffer->nAllocLen) ? omxBuffer->nAllocLen : demuxer_bytes;
			memcpy(omxBuffer->pBuffer, demuxer_content, omxBuffer->nFilledLen);

			demuxer_bytes -= omxBuffer->nFilledLen;
			demuxer_content += omxBuffer->nFilledLen;

			if(demuxer_bytes == 0)
			{
				omxBuffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
			}

			int nRetry = 0;
			while(true)
			{
				error = decoderComponent.EmptyThisBuffer(omxBuffer);
                OMX_TRACE(error);
				if (error == OMX_ErrorNone)
				{
					//ofLog(OF_LOG_VERBOSE, "VideD:  pts:%.0f size:%d)\n", pts, iSize);
					break;
				}
				else
				{
					ofLogError(__func__) << "OMX_EmptyThisBuffer() FAIL";
					nRetry++;
				}
				if(nRetry == 5)
				{
					ofLogError(__func__) << "OMX_EmptyThisBuffer() FAILED 5 TIMES";
					return false;
				}
			}
		}

		return true;
	}
	return false;
}

