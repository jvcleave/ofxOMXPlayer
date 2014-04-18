#include "OMXVideo.h"



COMXVideo::COMXVideo()
{

	m_deinterlace       = false;
	m_hdmi_clock_sync   = false;

}

COMXVideo::~COMXVideo()
{
	ofRemoveListener(ofEvents().update, this, &COMXVideo::onUpdate);
	ofLogVerbose(__func__) << "removed update listener";
}

bool COMXVideo::Open(COMXStreamInfo& hints, OMXClock *clock, float display_aspect, bool deinterlace, bool hdmi_clock_sync)
{
	OMX_ERRORTYPE error   = OMX_ErrorNone;

	m_video_codec_name      = "";
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

	std::string componentName = decoder_name;
	if(!m_omx_decoder.Initialize(componentName, OMX_IndexParamVideoInit))
	{
		return false;
	}

	componentName = "OMX.broadcom.video_render";
	if(!m_omx_render.Initialize(componentName, OMX_IndexParamVideoInit))
	{
		return false;
	}

	componentName = "OMX.broadcom.video_scheduler";
	if(!m_omx_sched.Initialize(componentName, OMX_IndexParamVideoInit))
	{
		return false;
	}

	if(m_deinterlace)
	{
		componentName = "OMX.broadcom.image_fx";
		if(!m_omx_image_fx.Initialize(componentName, OMX_IndexParamImageInit))
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

	if(m_omx_clock->GetComponent() == NULL)
	{
		m_av_clock = NULL;
		m_omx_clock = NULL;
		return false;
	}

	if(m_deinterlace)
	{
		m_omx_tunnel_decoder.Initialize(&m_omx_decoder, m_omx_decoder.GetOutputPort(), &m_omx_image_fx, m_omx_image_fx.GetInputPort());
		m_omx_tunnel_image_fx.Initialize(&m_omx_image_fx, m_omx_image_fx.GetOutputPort(), &m_omx_sched, m_omx_sched.GetInputPort());
	}
	else
	{
		m_omx_tunnel_decoder.Initialize(&m_omx_decoder, m_omx_decoder.GetOutputPort(), &m_omx_sched, m_omx_sched.GetInputPort());
	}

	m_omx_tunnel_sched.Initialize(&m_omx_sched, m_omx_sched.GetOutputPort(), &m_omx_render, m_omx_render.GetInputPort());
	m_omx_tunnel_clock.Initialize(m_omx_clock, m_omx_clock->GetInputPort() + 1, &m_omx_sched, m_omx_sched.GetOutputPort() + 1);

	error = m_omx_tunnel_clock.Establish(false);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "m_omx_tunnel_clock Establish FAIL: " << COMXCore::getOMXError(error);

		return false;
	}

	error = m_omx_decoder.SetStateForComponent(OMX_StateIdle);
	if (error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "m_omx_decoder OMX_StateIdle FAIL: " << COMXCore::getOMXError(error);
		return false;
	}

	OMX_VIDEO_PARAM_PORTFORMATTYPE formatType;
	OMX_INIT_STRUCTURE(formatType);
	formatType.nPortIndex = m_omx_decoder.GetInputPort();
	formatType.eCompressionFormat = m_codingType;

	if (hints.fpsscale > 0 && hints.fpsrate > 0)
	{
		formatType.xFramerate = (long long)(1<<16)*hints.fpsrate / hints.fpsscale;
	}
	else
	{
		formatType.xFramerate = 25 * (1<<16);
	}

	error = m_omx_decoder.SetParameter(OMX_IndexParamVideoPortFormat, &formatType);
	if(error != OMX_ErrorNone)
	{
		return false;
	}

	OMX_PARAM_PORTDEFINITIONTYPE portParam;
	OMX_INIT_STRUCTURE(portParam);
	portParam.nPortIndex = m_omx_decoder.GetInputPort();

	error = m_omx_decoder.GetParameter(OMX_IndexParamPortDefinition, &portParam);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "m_omx_decoder Get OMX_IndexParamPortDefinition FAIL: " << COMXCore::getOMXError(error);
		return false;
	}

	portParam.nPortIndex = m_omx_decoder.GetInputPort();
	int videoBuffers = 60;
	portParam.nBufferCountActual = videoBuffers;

	portParam.format.video.nFrameWidth  = m_decoded_width;
	portParam.format.video.nFrameHeight = m_decoded_height;

	error = m_omx_decoder.SetParameter(OMX_IndexParamPortDefinition, &portParam);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "m_omx_decoder Set OMX_IndexParamPortDefinition FAIL: " << COMXCore::getOMXError(error);
		return false;
	}

	OMX_PARAM_BRCMVIDEODECODEERRORCONCEALMENTTYPE concanParam;
	OMX_INIT_STRUCTURE(concanParam);
	concanParam.bStartWithValidFrame = OMX_FALSE;

	error = m_omx_decoder.SetParameter(OMX_IndexParamBrcmVideoDecodeErrorConcealment, &concanParam);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "m_omx_decoder OMX_IndexParamBrcmVideoDecodeErrorConcealment FAIL: " << COMXCore::getOMXError(error);
		return false;
	}

	if (m_deinterlace)
	{
		// the deinterlace component requires 3 additional video buffers in addition to the DPB (this is normally 2).
		OMX_PARAM_U32TYPE extra_buffers;
		OMX_INIT_STRUCTURE(extra_buffers);
		extra_buffers.nU32 = 3;

		error = m_omx_decoder.SetParameter(OMX_IndexParamBrcmExtraBuffers, &extra_buffers);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << "m_omx_decoder OMX_IndexParamBrcmExtraBuffers FAIL: " << COMXCore::getOMXError(error);
			return false;
		}
	}

	// broadcom omx entension:
	// When enabled, the timestamp fifo mode will change the way incoming timestamps are associated with output images.
	// In this mode the incoming timestamps get used without re-ordering on output images.
	if(hints.ptsinvalid)
	{
		OMX_CONFIG_BOOLEANTYPE timeStampMode;
		OMX_INIT_STRUCTURE(timeStampMode);
		timeStampMode.bEnabled = OMX_TRUE;
		error = m_omx_decoder.SetParameter((OMX_INDEXTYPE)OMX_IndexParamBrcmVideoTimestampFifo, &timeStampMode);
		if (error != OMX_ErrorNone)
		{
			ofLogError(__func__) << "m_omx_decoder OMX_IndexParamBrcmVideoTimestampFifo FAIL: " << COMXCore::getOMXError(error);
			return false;
		}
	}

	if(NaluFormatStartCodes(hints.codec, m_extradata, m_extrasize))
	{
		OMX_NALSTREAMFORMATTYPE nalStreamFormat;
		OMX_INIT_STRUCTURE(nalStreamFormat);
		nalStreamFormat.nPortIndex = m_omx_decoder.GetInputPort();
		nalStreamFormat.eNaluFormat = OMX_NaluFormatStartCodes;

		error = m_omx_decoder.SetParameter((OMX_INDEXTYPE)OMX_IndexParamNalStreamFormatSelect, &nalStreamFormat);
		if (error != OMX_ErrorNone)
		{
			ofLogError(__func__) << "m_omx_decoder OMX_IndexParamNalStreamFormatSelect FAIL: " << COMXCore::getOMXError(error);
			return false;
		}
	}

	if(m_hdmi_clock_sync)
	{
		OMX_CONFIG_LATENCYTARGETTYPE latencyTarget;
		OMX_INIT_STRUCTURE(latencyTarget);
		latencyTarget.nPortIndex = m_omx_render.GetInputPort();
		latencyTarget.bEnabled = OMX_TRUE;
		latencyTarget.nFilter = 2;
		latencyTarget.nTarget = 4000;
		latencyTarget.nShift = 3;
		latencyTarget.nSpeedFactor = -135;
		latencyTarget.nInterFactor = 500;
		latencyTarget.nAdjCap = 20;

		error = m_omx_render.SetConfig(OMX_IndexConfigLatencyTarget, &latencyTarget);
		if (error != OMX_ErrorNone)
		{
			ofLogError(__func__) << "m_omx_render OMX_IndexConfigLatencyTarget FAIL: " << COMXCore::getOMXError(error);
			return false;
		}
	}

	// Alloc buffers for the omx input port.
	error = m_omx_decoder.AllocInputBuffers();
	if (error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "m_omx_decoder AllocInputBuffers FAIL: " << COMXCore::getOMXError(error);
		return false;
	}

	error = m_omx_tunnel_decoder.Establish(false);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "m_omx_tunnel_decoder Establish FAIL: " << COMXCore::getOMXError(error);
		return false;
	}

	
	error = m_omx_decoder.SetStateForComponent(OMX_StateExecuting);
	if (error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "m_omx_decoder OMX_StateExecuting FAIL: " << COMXCore::getOMXError(error);
		return false;
	}

	if(m_deinterlace)
	{
		OMX_CONFIG_IMAGEFILTERPARAMSTYPE image_filter;
		OMX_INIT_STRUCTURE(image_filter);

		image_filter.nPortIndex = m_omx_image_fx.GetOutputPort();
		image_filter.nNumParams = 1;
		image_filter.nParams[0] = 3;
		image_filter.eImageFilter = OMX_ImageFilterDeInterlaceAdvanced;

		error = m_omx_image_fx.SetConfig(OMX_IndexConfigCommonImageFilterParameters, &image_filter);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << "m_omx_image_fx SetConfig FAIL: " << COMXCore::getOMXError(error);
			return false;
		}

		error = m_omx_tunnel_image_fx.Establish(false);
		if(error != OMX_ErrorNone)
		{
			ofLogError(__func__) << "m_omx_image_fx Establish FAIL: " << COMXCore::getOMXError(error);
			return false;
		}

		error = m_omx_image_fx.SetStateForComponent(OMX_StateExecuting);
		if (error != OMX_ErrorNone)
		{
			ofLogError(__func__) << "m_omx_image_fx OMX_StateExecuting FAIL: " << COMXCore::getOMXError(error);
			return false;
		}

		m_omx_image_fx.DisablePort(m_omx_image_fx.GetInputPort(), false);
		m_omx_image_fx.DisablePort(m_omx_image_fx.GetOutputPort(), false);

	}

	error = m_omx_tunnel_sched.Establish(false);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "m_omx_tunnel_sched Establish FAIL: " << COMXCore::getOMXError(error);
		return false;
	}

	
	error = m_omx_sched.SetStateForComponent(OMX_StateExecuting);
	if (error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "m_omx_sched OMX_StateExecuting FAIL: " << COMXCore::getOMXError(error);
		return false;
	}
	
	
	
	error = m_omx_render.SetStateForComponent(OMX_StateExecuting);
	if (error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "m_omx_render OMX_StateExecuting FAIL: " << COMXCore::getOMXError(error);
		return false;
	}
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
	configDisplay.nPortIndex = m_omx_render.GetInputPort();
	
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
				error = m_omx_render.SetConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
				if(error != OMX_ErrorNone)
				{
					return false;
				}
			}
			
		}
	}

	ofLog(OF_LOG_VERBOSE,
	      "%s::%s - decoder_component(0x%p), input_port(0x%x), output_port(0x%x) deinterlace %d hdmiclocksync %d\n",
	      "OMXVideo", __func__, m_omx_decoder.GetComponent(), m_omx_decoder.GetInputPort(), m_omx_decoder.GetOutputPort(),
	      m_deinterlace, m_hdmi_clock_sync);

	m_first_frame   = true;

	// start from assuming all recent frames had valid pts
	m_history_valid_pts = ~0;
	return true;
}

void COMXVideo::onUpdate(ofEventArgs& args)
{
	updateFrameCount();
}
void COMXVideo::updateFrameCount()
{
	if (!m_is_open) {
		return;
	}
	OMX_ERRORTYPE error;
	OMX_CONFIG_BRCMPORTSTATSTYPE stats;
	
	OMX_INIT_STRUCTURE(stats);
	
	stats.nPortIndex = m_omx_render.GetInputPort();
	
	error = m_omx_render.GetParameter(OMX_IndexConfigBrcmPortStats, &stats);
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
		frameCounter = stats.nFrameCount-frameOffset;
	}else
	{
		ofLogError(__func__) << "m_omx_render OMX_CONFIG_BRCMPORTSTATSTYPE fail: " << COMXCore::getOMXError(error);
	}
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
			OMX_BUFFERHEADERTYPE *omx_buffer = m_omx_decoder.GetInputBuffer(500);
			if(omx_buffer == NULL)
			{
				ofLogError(__func__) << "Decode timeout";
				return false;
			}

			omx_buffer->nFlags = 0;
			omx_buffer->nOffset = 0;

			if(m_setStartTime)
			{
				omx_buffer->nFlags |= OMX_BUFFERFLAG_STARTTIME;
				ofLog(OF_LOG_VERBOSE, "OMXVideo::Decode VDec : setStartTime %f\n", (pts == DVD_NOPTS_VALUE ? 0.0 : pts) / DVD_TIME_BASE);
				m_setStartTime = false;
			}
			else if(pts == DVD_NOPTS_VALUE)
			{
				omx_buffer->nFlags |= OMX_BUFFERFLAG_TIME_UNKNOWN;
			}

			omx_buffer->nTimeStamp = ToOMXTime((uint64_t)(pts == DVD_NOPTS_VALUE) ? 0 : pts);
			omx_buffer->nFilledLen = (demuxer_bytes > omx_buffer->nAllocLen) ? omx_buffer->nAllocLen : demuxer_bytes;
			memcpy(omx_buffer->pBuffer, demuxer_content, omx_buffer->nFilledLen);

			demuxer_bytes -= omx_buffer->nFilledLen;
			demuxer_content += omx_buffer->nFilledLen;

			if(demuxer_bytes == 0)
			{
				//ofLogVerbose(__func__) << "OMX_BUFFERFLAG_ENDOFFRAME";
				omx_buffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
			}

			int nRetry = 0;
			while(true)
			{
				error = m_omx_decoder.EmptyThisBuffer(omx_buffer);
				if (error == OMX_ErrorNone)
				{
					//ofLog(OF_LOG_VERBOSE, "VideD:  pts:%.0f size:%d)\n", pts, iSize);
					
					break;
				}
				else
				{
					ofLogError(__func__) << "OMX_EmptyThisBuffer() FAIL: " << COMXCore::getOMXError(error);
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
	configDisplay.nPortIndex = m_omx_render.GetInputPort();
	
	
	
	configDisplay.set     = OMX_DISPLAY_SET_FULLSCREEN;
	configDisplay.fullscreen = OMX_FALSE;
	
	m_omx_render.SetConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
	
	configDisplay.set     = OMX_DISPLAY_SET_DEST_RECT;
	configDisplay.dest_rect.x_offset  = displayRect.x;
	configDisplay.dest_rect.y_offset  = displayRect.y;
	configDisplay.dest_rect.width     = displayRect.getWidth();
	configDisplay.dest_rect.height    = displayRect.getHeight();
	
	m_omx_render.SetConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
}
void COMXVideo::setDisplayRect(ofRectangle& rectangle)
{
	
	bool hasChanged = (displayRect != rectangle);
	
	//ofLogVerbose(__func__) << "hasChanged: " << hasChanged << " displayRect: " << displayRect << " rectangle: " << rectangle;
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




