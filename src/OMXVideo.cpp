#include "OMXVideo.h"



COMXVideo::COMXVideo()
{

	m_deinterlace       = false;
	m_hdmi_clock_sync   = false;
	frameCounter = 0;
	frameOffset = 0;
}


COMXVideo::~COMXVideo()
{
	ofRemoveListener(ofEvents().update, this, &COMXVideo::onUpdate);
	//ofLogVerbose(__func__) << "removed update listener";
}

bool COMXVideo::Open(COMXStreamInfo& hints, OMXClock *clock, float display_aspect, bool deinterlace, bool hdmi_clock_sync)
{
	OMX_ERRORTYPE error   = OMX_ErrorNone;

	m_codingType            = OMX_VIDEO_CodingUnused;

	m_decoded_width  = hints.width;
	m_decoded_height = hints.height;

	m_hdmi_clock_sync = hdmi_clock_sync;

	if(!m_decoded_width || !m_decoded_height)
	{
		return false;
	}

	if(hints.extrasize > 0 && hints.extradata != NULL)
	{
		m_extrasize = hints.extrasize;
		m_extradata = (uint8_t *)malloc(m_extrasize);
		memcpy(m_extradata, hints.extradata, hints.extrasize);
	}

	ProcessCodec(hints);

	if(deinterlace)
	{
		ofLog(OF_LOG_VERBOSE, "enable deinterlace\n");
		m_deinterlace = true;
	}
	else
	{
		m_deinterlace = false;
	}

	std::string componentName = "OMX.broadcom.video_decode";
	if(!m_omx_decoder.init(componentName, OMX_IndexParamVideoInit))
	{
		return false;
	}

	componentName = "OMX.broadcom.video_render";
	if(!renderComponent.init(componentName, OMX_IndexParamVideoInit))
	{
		return false;
	}

	componentName = "OMX.broadcom.video_scheduler";
	if(!m_omx_sched.init(componentName, OMX_IndexParamVideoInit))
	{
		return false;
	}

	if(m_deinterlace)
	{
		componentName = "OMX.broadcom.image_fx";
		if(!m_omx_image_fx.init(componentName, OMX_IndexParamImageInit))
		{
			return false;
		}
	}

	if(clock == NULL)
	{
		return false;
	}

	m_av_clock = clock;
	m_omx_clock = m_av_clock->GetOMXClock();

	if(m_omx_clock->getComponent() == NULL)
	{
		m_av_clock = NULL;
		m_omx_clock = NULL;
		return false;
	}

	if(m_deinterlace)
	{
		decoderTunnel.init(&m_omx_decoder, m_omx_decoder.getOutputPort(), &m_omx_image_fx, m_omx_image_fx.getInputPort());
		m_omx_tunnel_image_fx.init(&m_omx_image_fx, m_omx_image_fx.getOutputPort(), &m_omx_sched, m_omx_sched.getInputPort());
	}
	else
	{
		decoderTunnel.init(&m_omx_decoder, m_omx_decoder.getOutputPort(), &m_omx_sched, m_omx_sched.getInputPort());
	}

	schedulerTunnel.init(&m_omx_sched, m_omx_sched.getOutputPort(), &renderComponent, renderComponent.getInputPort());
	clockTunnel.init(m_omx_clock, m_omx_clock->getInputPort() + 1, &m_omx_sched, m_omx_sched.getOutputPort() + 1);

	error = clockTunnel.Establish(false);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	error = m_omx_decoder.setState(OMX_StateIdle);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	OMX_VIDEO_PARAM_PORTFORMATTYPE formatType;
	OMX_INIT_STRUCTURE(formatType);
	formatType.nPortIndex = m_omx_decoder.getInputPort();
	formatType.eCompressionFormat = m_codingType;

	if (hints.fpsscale > 0 && hints.fpsrate > 0)
	{
		formatType.xFramerate = (long long)(1<<16)*hints.fpsrate / hints.fpsscale;
	}
	else
	{
		formatType.xFramerate = 25 * (1<<16);
	}
    
	error = m_omx_decoder.setParameter(OMX_IndexParamVideoPortFormat, &formatType);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	OMX_PARAM_PORTDEFINITIONTYPE portParam;
	OMX_INIT_STRUCTURE(portParam);
	portParam.nPortIndex = m_omx_decoder.getInputPort();

	error = m_omx_decoder.getParameter(OMX_IndexParamPortDefinition, &portParam);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	portParam.nPortIndex = m_omx_decoder.getInputPort();
	int videoBuffers = 60;
	portParam.nBufferCountActual = videoBuffers;

	portParam.format.video.nFrameWidth  = m_decoded_width;
	portParam.format.video.nFrameHeight = m_decoded_height;

	error = m_omx_decoder.setParameter(OMX_IndexParamPortDefinition, &portParam);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	OMX_PARAM_BRCMVIDEODECODEERRORCONCEALMENTTYPE concanParam;
	OMX_INIT_STRUCTURE(concanParam);
	concanParam.bStartWithValidFrame = OMX_FALSE;

	error = m_omx_decoder.setParameter(OMX_IndexParamBrcmVideoDecodeErrorConcealment, &concanParam);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	if (m_deinterlace)
	{
		// the deinterlace component requires 3 additional video buffers in addition to the DPB (this is normally 2).
		OMX_PARAM_U32TYPE extra_buffers;
		OMX_INIT_STRUCTURE(extra_buffers);
		extra_buffers.nU32 = 3;

		error = m_omx_decoder.setParameter(OMX_IndexParamBrcmExtraBuffers, &extra_buffers);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone) return false;
	}

	// broadcom omx entension:
	// When enabled, the timestamp fifo mode will change the way incoming timestamps are associated with output images.
	// In this mode the incoming timestamps get used without re-ordering on output images.
	
    // recent firmware will actually automatically choose the timestamp stream with the least variance, so always enable

    OMX_CONFIG_BOOLEANTYPE timeStampMode;
    OMX_INIT_STRUCTURE(timeStampMode);
    timeStampMode.bEnabled = OMX_TRUE;
    error = m_omx_decoder.setParameter((OMX_INDEXTYPE)OMX_IndexParamBrcmVideoTimestampFifo, &timeStampMode);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;


	if(NaluFormatStartCodes(hints.codec, m_extradata, m_extrasize))
	{
		OMX_NALSTREAMFORMATTYPE nalStreamFormat;
		OMX_INIT_STRUCTURE(nalStreamFormat);
		nalStreamFormat.nPortIndex = m_omx_decoder.getInputPort();
		nalStreamFormat.eNaluFormat = OMX_NaluFormatStartCodes;

		error = m_omx_decoder.setParameter((OMX_INDEXTYPE)OMX_IndexParamNalStreamFormatSelect, &nalStreamFormat);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone) return false;
	}

	if(m_hdmi_clock_sync)
	{
		OMX_CONFIG_LATENCYTARGETTYPE latencyTarget;
		OMX_INIT_STRUCTURE(latencyTarget);
		latencyTarget.nPortIndex = renderComponent.getInputPort();
		latencyTarget.bEnabled = OMX_TRUE;
		latencyTarget.nFilter = 2;
		latencyTarget.nTarget = 4000;
		latencyTarget.nShift = 3;
		latencyTarget.nSpeedFactor = -135;
		latencyTarget.nInterFactor = 500;
		latencyTarget.nAdjCap = 20;

		error = renderComponent.setConfig(OMX_IndexConfigLatencyTarget, &latencyTarget);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone) return false;
	}

	// Alloc buffers for the omx input port.
	error = m_omx_decoder.allocInputBuffers();
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	error = decoderTunnel.Establish(false);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	
	error = m_omx_decoder.setState(OMX_StateExecuting);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	if(m_deinterlace)
	{
		OMX_CONFIG_IMAGEFILTERPARAMSTYPE image_filter;
		OMX_INIT_STRUCTURE(image_filter);

		image_filter.nPortIndex = m_omx_image_fx.getOutputPort();
		image_filter.nNumParams = 1;
		image_filter.nParams[0] = 3;
		image_filter.eImageFilter = OMX_ImageFilterDeInterlaceAdvanced;

		error = m_omx_image_fx.setConfig(OMX_IndexConfigCommonImageFilterParameters, &image_filter);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone) return false;

		error = m_omx_tunnel_image_fx.Establish(false);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone) return false;

		error = m_omx_image_fx.setState(OMX_StateExecuting);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone) return false;

		m_omx_image_fx.disablePort(m_omx_image_fx.getInputPort());
		m_omx_image_fx.disablePort(m_omx_image_fx.getOutputPort());

	}

	error = schedulerTunnel.Establish(false);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	
	error = m_omx_sched.setState(OMX_StateExecuting);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;
	
	
	
	error = renderComponent.setState(OMX_StateExecuting);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;
    
	ofAddListener(ofEvents().update, this, &COMXVideo::onUpdate);
	if(!SendDecoderConfig())
	{
		return false;
	}

	m_is_open           = true;
	m_drop_state        = false;
	m_setStartTime      = true;
	OMX_CONFIG_DISPLAYREGIONTYPE configDisplay;
	OMX_INIT_STRUCTURE(configDisplay);
	configDisplay.nPortIndex = renderComponent.getInputPort();
	
	//we provided a rectangle but returned early as we were not ready
	if (displayRect.getWidth()>0) 
	{
		configureDisplay();
	}else 
	{
		
		float fAspect = (float)hints.aspect / (float)m_decoded_width * (float)m_decoded_height;
		float par = hints.aspect ? fAspect/display_aspect : 0.0f;
		// only set aspect when we have a aspect and display doesn't match the aspect
		bool doDisplayChange = true;
		if(doDisplayChange)
		{
			if(par != 0.0f && fabs(par - 1.0f) > 0.01f)
			{
				
				
				AVRational aspect;
				aspect = av_d2q(par, 100);
				configDisplay.set      = OMX_DISPLAY_SET_PIXEL;
				configDisplay.pixel_x  = aspect.num;
				configDisplay.pixel_y  = aspect.den;
				ofLog(OF_LOG_VERBOSE, "Aspect : num %d den %d aspect %f pixel aspect %f\n", aspect.num, aspect.den, hints.aspect, par);
				error = renderComponent.setConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
                OMX_TRACE(error);
                if(error != OMX_ErrorNone) return false;
			}
			
		}
	}

	ofLog(OF_LOG_VERBOSE,
	      "%s::%s - decoder_component(0x%p), input_port(0x%x), output_port(0x%x) deinterlace %d hdmiclocksync %d\n",
	      "OMXVideo", __func__, m_omx_decoder.getComponent(), m_omx_decoder.getInputPort(), m_omx_decoder.getOutputPort(),
	      m_deinterlace, m_hdmi_clock_sync);

	m_first_frame   = true;

	// start from assuming all recent frames had valid pts
	m_history_valid_pts = ~0;
	return true;
}

void COMXVideo::onUpdate(ofEventArgs& args)
{
    //TODO: seems to cause hang on exit
    
	//updateFrameCount();
}
void COMXVideo::updateFrameCount()
{
	if (!m_is_open) {
		return;
	}
	OMX_ERRORTYPE error;
	OMX_CONFIG_BRCMPORTSTATSTYPE stats;
	
	OMX_INIT_STRUCTURE(stats);
	
	stats.nPortIndex = renderComponent.getInputPort();
	
	error = renderComponent.getParameter(OMX_IndexConfigBrcmPortStats, &stats);
    OMX_TRACE(error);
	if (error == OMX_ErrorNone)
	{
		/*OMX_U32 nImageCount;
		 OMX_U32 nBufferCount;
		 OMX_U32 nFrameCount;
		 OMX_U32 nFrameSkips;
		 OMX_U32 nDiscards;
		 OMX_U32 nEOS;
		 OMX_U32 nMaxFrameSize;
		 
		 OMX_TICKS nByteCount;
		 OMX_TICKS nMaxTimeDelta;
		 OMX_U32 nCorruptMBs;*/
		//ofLogVerbose(__func__) << "nFrameCount: " << stats.nFrameCount;
		frameCounter = stats.nFrameCount;
	}else
	{
		ofLogError(__func__) << "renderComponent OMX_CONFIG_BRCMPORTSTATSTYPE fail: ";
	}
}

int COMXVideo::getCurrentFrame()
{
	return frameCounter - frameOffset;
}

void COMXVideo::resetFrameCounter()
{
	frameOffset = frameCounter;
}


bool COMXVideo::Decode(uint8_t *pData, int iSize, double pts)
{
	CSingleLock lock (m_critSection);
	OMX_ERRORTYPE error;

	if( m_drop_state || !m_is_open )
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
			OMX_BUFFERHEADERTYPE *omxBuffer = m_omx_decoder.getInputBuffer(500);
			if(omxBuffer == NULL)
			{
				ofLogError(__func__) << "Decode timeout";
				return false;
			}

			omxBuffer->nFlags = 0;
			omxBuffer->nOffset = 0;

			if(m_setStartTime)
			{
				omxBuffer->nFlags |= OMX_BUFFERFLAG_STARTTIME;
				ofLog(OF_LOG_VERBOSE, "OMXVideo::Decode VDec : setStartTime %f\n", (pts == DVD_NOPTS_VALUE ? 0.0 : pts) / DVD_TIME_BASE);
				m_setStartTime = false;
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
				//ofLogVerbose(__func__) << "OMX_BUFFERFLAG_ENDOFFRAME";
				omxBuffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
			}

			int nRetry = 0;
			while(true)
			{
				error = m_omx_decoder.EmptyThisBuffer(omxBuffer);
                OMX_TRACE(error);
				if (error == OMX_ErrorNone)
				{
					//ofLog(OF_LOG_VERBOSE, "VideD:  pts:%.0f size:%d)\n", pts, iSize);
					
					break;
				}
				else
				{
					ofLogError(__func__) << "OMX_EmptyThisBuffer() FAIL: ";
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

void COMXVideo::configureDisplay()
{
	OMX_CONFIG_DISPLAYREGIONTYPE configDisplay;
	OMX_INIT_STRUCTURE(configDisplay);
	configDisplay.nPortIndex = renderComponent.getInputPort();
	
	
	
	configDisplay.set     = OMX_DISPLAY_SET_FULLSCREEN;
	configDisplay.fullscreen = OMX_FALSE;
	
	renderComponent.setConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
	
	configDisplay.set     = OMX_DISPLAY_SET_DEST_RECT;
	configDisplay.dest_rect.x_offset  = displayRect.x;
	configDisplay.dest_rect.y_offset  = displayRect.y;
	configDisplay.dest_rect.width     = displayRect.getWidth();
	configDisplay.dest_rect.height    = displayRect.getHeight();
	
	renderComponent.setConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
}
void COMXVideo::setDisplayRect(ofRectangle& rectangle)
{
	
	bool hasChanged = (displayRect != rectangle);
    
    if (hasChanged) 
	{
		displayRect = rectangle;
	}
	
	if(!m_is_open)
	{
		return;
	}
	if (hasChanged) 
	{
		
		configureDisplay();
	}	
}




