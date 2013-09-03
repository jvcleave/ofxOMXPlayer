#include "OMXVideo.h"


COMXVideo::COMXVideo()
{

	m_deinterlace       = false;
	m_hdmi_clock_sync   = false;
    
}


bool COMXVideo::Open(COMXStreamInfo &hints, OMXClock *clock, float display_aspect, bool deinterlace, bool hdmi_clock_sync)
{
	OMX_ERRORTYPE omx_err   = OMX_ErrorNone;

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
	return false;

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

	omx_err = m_omx_tunnel_clock.Establish(false);
	if(omx_err != OMX_ErrorNone)
	{
		ofLog(OF_LOG_VERBOSE, "COMXVideo::Open m_omx_tunnel_clock.Establish\n");
		return false;
	}

	omx_err = m_omx_decoder.SetStateForComponent(OMX_StateIdle);
	if (omx_err != OMX_ErrorNone)
	{
		ofLog(OF_LOG_VERBOSE, "COMXVideo::Open m_omx_decoder.SetStateForComponent\n");
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

	omx_err = m_omx_decoder.SetParameter(OMX_IndexParamVideoPortFormat, &formatType);
	if(omx_err != OMX_ErrorNone)
	{
		return false;
	}

	OMX_PARAM_PORTDEFINITIONTYPE portParam;
	OMX_INIT_STRUCTURE(portParam);
	portParam.nPortIndex = m_omx_decoder.GetInputPort();

	omx_err = m_omx_decoder.GetParameter(OMX_IndexParamPortDefinition, &portParam);
	if(omx_err != OMX_ErrorNone)
	{
		ofLog(OF_LOG_VERBOSE, "COMXVideo::Open error OMX_IndexParamPortDefinition omx_err(0x%08x)\n", omx_err);
		return false;
	}

	portParam.nPortIndex = m_omx_decoder.GetInputPort();
	int videoBuffers = 60;
	portParam.nBufferCountActual = videoBuffers;

	portParam.format.video.nFrameWidth  = m_decoded_width;
	portParam.format.video.nFrameHeight = m_decoded_height;

	omx_err = m_omx_decoder.SetParameter(OMX_IndexParamPortDefinition, &portParam);
	if(omx_err != OMX_ErrorNone)
	{
		ofLog(OF_LOG_VERBOSE, "COMXVideo::Open error OMX_IndexParamPortDefinition omx_err(0x%08x)\n", omx_err);
		return false;
	}

	OMX_PARAM_BRCMVIDEODECODEERRORCONCEALMENTTYPE concanParam;
	OMX_INIT_STRUCTURE(concanParam);
	concanParam.bStartWithValidFrame = OMX_FALSE;

	omx_err = m_omx_decoder.SetParameter(OMX_IndexParamBrcmVideoDecodeErrorConcealment, &concanParam);
	if(omx_err != OMX_ErrorNone)
	{
		ofLog(OF_LOG_VERBOSE, "COMXVideo::Open error OMX_IndexParamBrcmVideoDecodeErrorConcealment omx_err(0x%08x)\n", omx_err);
		return false;
	}

	if (m_deinterlace)
	{
		// the deinterlace component requires 3 additional video buffers in addition to the DPB (this is normally 2).
		OMX_PARAM_U32TYPE extra_buffers;
		OMX_INIT_STRUCTURE(extra_buffers);
		extra_buffers.nU32 = 3;

		omx_err = m_omx_decoder.SetParameter(OMX_IndexParamBrcmExtraBuffers, &extra_buffers);
		if(omx_err != OMX_ErrorNone)
		{
		  ofLog(OF_LOG_VERBOSE, "COMXVideo::Open error OMX_IndexParamBrcmExtraBuffers omx_err(0x%08x)\n", omx_err);
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
		omx_err = m_omx_decoder.SetParameter((OMX_INDEXTYPE)OMX_IndexParamBrcmVideoTimestampFifo, &timeStampMode);
		if (omx_err != OMX_ErrorNone)
		{
		  ofLog(OF_LOG_VERBOSE, "COMXVideo::Open OMX_IndexParamBrcmVideoTimestampFifo error (0%08x)\n", omx_err);
		  return false;
		}
	}

	if(NaluFormatStartCodes(hints.codec, m_extradata, m_extrasize))
	{
		OMX_NALSTREAMFORMATTYPE nalStreamFormat;
		OMX_INIT_STRUCTURE(nalStreamFormat);
		nalStreamFormat.nPortIndex = m_omx_decoder.GetInputPort();
		nalStreamFormat.eNaluFormat = OMX_NaluFormatStartCodes;

		omx_err = m_omx_decoder.SetParameter((OMX_INDEXTYPE)OMX_IndexParamNalStreamFormatSelect, &nalStreamFormat);
		if (omx_err != OMX_ErrorNone)
		{
		  ofLog(OF_LOG_VERBOSE, "COMXVideo::Open OMX_IndexParamNalStreamFormatSelect error (0%08x)\n", omx_err);
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

		omx_err = m_omx_render.SetConfig(OMX_IndexConfigLatencyTarget, &latencyTarget);
		if (omx_err != OMX_ErrorNone)
		{
		  ofLog(OF_LOG_VERBOSE, "COMXVideo::Open OMX_IndexConfigLatencyTarget error (0%08x)\n", omx_err);
		  return false;
		}
	}

	// Alloc buffers for the omx inpput port.
	omx_err = m_omx_decoder.AllocInputBuffers();
	if (omx_err != OMX_ErrorNone)
	{
	ofLog(OF_LOG_VERBOSE, "COMXVideo::Open AllocOMXInputBuffers error (0%08x)\n", omx_err);
	return false;
	}

	omx_err = m_omx_tunnel_decoder.Establish(false);
	if(omx_err != OMX_ErrorNone)
	{
	ofLog(OF_LOG_VERBOSE, "COMXVideo::Open m_omx_tunnel_decoder.Establish\n");
	return false;
	}

	omx_err = m_omx_decoder.SetStateForComponent(OMX_StateExecuting);
	if (omx_err != OMX_ErrorNone)
	{
	ofLog(OF_LOG_VERBOSE, "COMXVideo::Open error m_omx_decoder.SetStateForComponent\n");
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

	omx_err = m_omx_image_fx.SetConfig(OMX_IndexConfigCommonImageFilterParameters, &image_filter);
	if(omx_err != OMX_ErrorNone)
	{
	  ofLog(OF_LOG_VERBOSE, "COMXVideo::Open error OMX_IndexConfigCommonImageFilterParameters omx_err(0x%08x)\n", omx_err);
	  return false;
	}

	omx_err = m_omx_tunnel_image_fx.Establish(false);
	if(omx_err != OMX_ErrorNone)
	{
	  ofLog(OF_LOG_VERBOSE, "COMXVideo::Open m_omx_tunnel_image_fx.Establish\n");
	  return false;
	}

	omx_err = m_omx_image_fx.SetStateForComponent(OMX_StateExecuting);
	if (omx_err != OMX_ErrorNone)
	{
	  ofLog(OF_LOG_VERBOSE, "COMXVideo::Open error m_omx_image_fx.SetStateForComponent\n");
	  return false;
	}

	m_omx_image_fx.DisablePort(m_omx_image_fx.GetInputPort(), false);
	m_omx_image_fx.DisablePort(m_omx_image_fx.GetOutputPort(), false);
		
	}

	omx_err = m_omx_tunnel_sched.Establish(false);
	if(omx_err != OMX_ErrorNone)
	{
		ofLog(OF_LOG_VERBOSE, "COMXVideo::Open m_omx_tunnel_sched.Establish\n");
	return false;
	}


	omx_err = m_omx_sched.SetStateForComponent(OMX_StateExecuting);
	if (omx_err != OMX_ErrorNone)
	{
		ofLog(OF_LOG_VERBOSE, "COMXVideo::Open error m_omx_sched.SetStateForComponent\n");
	return false;
	}

	omx_err = m_omx_render.SetStateForComponent(OMX_StateExecuting);
	if (omx_err != OMX_ErrorNone)
	{
		ofLog(OF_LOG_VERBOSE, "COMXVideo::Open error m_omx_render.SetStateForComponent\n");
	return false;
	}

	if(!SendDecoderConfig())
	return false;

	m_is_open           = true;
	m_drop_state        = false;
	m_setStartTime      = true;

	float fAspect = (float)hints.aspect / (float)m_decoded_width * (float)m_decoded_height; 
	float par = hints.aspect ? fAspect/display_aspect : 0.0f;
	// only set aspect when we have a aspect and display doesn't match the aspect
	bool doDisplayChange = true;
	if(doDisplayChange)
	{
		if(par != 0.0f && fabs(par - 1.0f) > 0.01f)
		{
			OMX_CONFIG_DISPLAYREGIONTYPE configDisplay;
			OMX_INIT_STRUCTURE(configDisplay);
			configDisplay.nPortIndex = m_omx_render.GetInputPort();
			
			AVRational aspect;
			aspect = av_d2q(par, 100);
			configDisplay.set      = OMX_DISPLAY_SET_PIXEL;
			configDisplay.pixel_x  = aspect.num;
			configDisplay.pixel_y  = aspect.den;
			ofLog(OF_LOG_VERBOSE, "Aspect : num %d den %d aspect %f pixel aspect %f\n", aspect.num, aspect.den, hints.aspect, par);
			omx_err = m_omx_render.SetConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
			if(omx_err != OMX_ErrorNone)
				return false;
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

void COMXVideo::Close()
{
  m_omx_tunnel_decoder.Flush();
  if(m_deinterlace)
    m_omx_tunnel_image_fx.Flush();
  m_omx_tunnel_clock.Flush();
  m_omx_tunnel_sched.Flush();

  m_omx_tunnel_clock.Deestablish();
  m_omx_tunnel_decoder.Deestablish();
  if(m_deinterlace)
    m_omx_tunnel_image_fx.Deestablish();
  m_omx_tunnel_sched.Deestablish();

  m_omx_decoder.FlushInput();

  m_omx_sched.Deinitialize();
  if(m_deinterlace)
    m_omx_image_fx.Deinitialize();
  m_omx_decoder.Deinitialize();
  m_omx_render.Deinitialize();

  m_is_open       = false;

  if(m_extradata)
    free(m_extradata);
  m_extradata = NULL;
  m_extrasize = 0;

  m_video_codec_name  = "";
  m_deinterlace       = false;
  m_first_frame       = true;
  m_setStartTime      = true;
}


int COMXVideo::Decode(uint8_t *pData, int iSize, double dts, double pts)
{
  OMX_ERRORTYPE omx_err;

  if (pData || iSize > 0)
  {
    unsigned int demuxer_bytes = (unsigned int)iSize;
    uint8_t *demuxer_content = pData;

    while(demuxer_bytes)
    {
		// 500ms timeout
		OMX_BUFFERHEADERTYPE *omx_buffer = m_omx_decoder.GetInputBuffer(500);
		if(omx_buffer == NULL)
		{
			ofLog(OF_LOG_VERBOSE, "OMXVideo::Decode timeout\n");
			return false;
		}

		/*
		ofLog(OF_LOG_VERBOSE, "COMXVideo::Video VDec : pts %lld omx_buffer 0x%08x buffer 0x%08x number %d\n", 
		  pts, omx_buffer, omx_buffer->pBuffer, (int)omx_buffer->pAppPrivate);
		ofLog(OF_LOG_VERBOSE, "VDec : pts %f omx_buffer 0x%08x buffer 0x%08x number %d\n", 
		  (float)pts / AV_TIME_BASE, omx_buffer, omx_buffer->pBuffer, (int)omx_buffer->pAppPrivate);
		*/

		omx_buffer->nFlags = 0;
		omx_buffer->nOffset = 0;

		// some packed bitstream AVI files set almost all pts values to DVD_NOPTS_VALUE, but have a scattering of real pts values.
		// the valid pts values match the dts values.
		// if a stream has had more than 4 valid pts values in the last 16, the use UNKNOWN, otherwise use dts
		m_history_valid_pts = (m_history_valid_pts << 1) | (pts != DVD_NOPTS_VALUE);
		if(pts == DVD_NOPTS_VALUE && count_bits(m_history_valid_pts & 0xffff) < 4)
		{
			pts = dts;
		}

		if(m_setStartTime)
		{
			// only send dts on first frame to get nearly correct starttime
			if(pts == DVD_NOPTS_VALUE)
			{
				pts = dts;
			}
			omx_buffer->nFlags |= OMX_BUFFERFLAG_STARTTIME;
			ofLog(OF_LOG_VERBOSE, "OMXVideo::Decode VDec : setStartTime %f\n", (pts == DVD_NOPTS_VALUE ? 0.0 : pts) / DVD_TIME_BASE);
			m_setStartTime = false;
		}
		
		if(pts == DVD_NOPTS_VALUE)
		{
			omx_buffer->nFlags |= OMX_BUFFERFLAG_TIME_UNKNOWN;
		}
		
		omx_buffer->nTimeStamp = ToOMXTime(pts == DVD_NOPTS_VALUE ? 0 : pts);
		omx_buffer->nFilledLen = (demuxer_bytes > omx_buffer->nAllocLen) ? omx_buffer->nAllocLen : demuxer_bytes;
		memcpy(omx_buffer->pBuffer, demuxer_content, omx_buffer->nFilledLen);

		demuxer_bytes -= omx_buffer->nFilledLen;
		demuxer_content += omx_buffer->nFilledLen;

		if(demuxer_bytes == 0)
		{
			omx_buffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
		}

		int nRetry = 0;
		while(true)
		{
			omx_err = m_omx_decoder.EmptyThisBuffer(omx_buffer);
			if (omx_err == OMX_ErrorNone)
			{
				break;
			}
			else
			{
				ofLog(OF_LOG_VERBOSE, "%s::%s - OMX_EmptyThisBuffer() failed with result(0x%x)\n", "OMXVideo", __func__, omx_err);
				nRetry++;
			}
			if(nRetry == 5)
			{
				ofLog(OF_LOG_VERBOSE, "%s::%s - OMX_EmptyThisBuffer() finaly failed\n", "OMXVideo", __func__);
				return false;
			}
      }

      /*
      omx_err = m_omx_decoder.EmptyThisBuffer(omx_buffer);

      if(omx_err != OMX_ErrorNone)
      {
        ofLog(OF_LOG_VERBOSE, "%s::%s - OMX_EmptyThisBuffer() failed with result(0x%x)\n", "OMXVideo", __func__, omx_err);

        ofLog(OF_LOG_VERBOSE, "%s::%s - OMX_EmptyThisBuffer() failed with result(0x%x)\n", "OMXVideo", __func__, omx_err);

        return false;
      }
      */

      if(m_first_frame && m_deinterlace)
      {
        OMX_PARAM_PORTDEFINITIONTYPE port_image;
        OMX_INIT_STRUCTURE(port_image);
        port_image.nPortIndex = m_omx_decoder.GetOutputPort();

        omx_err = m_omx_decoder.GetParameter(OMX_IndexParamPortDefinition, &port_image);
        if(omx_err != OMX_ErrorNone)
        {
          ofLog(OF_LOG_VERBOSE, "%s::%s - error OMX_IndexParamPortDefinition 1 omx_err(0x%08x)\n", "OMXVideo", __func__, omx_err);
        }

        /* we assume when the sizes equal we have the first decoded frame */
        if(port_image.format.video.nFrameWidth == m_decoded_width && port_image.format.video.nFrameHeight == m_decoded_height)
        {
          m_first_frame = false;

          omx_err = m_omx_decoder.WaitForEvent(OMX_EventPortSettingsChanged);
          if(omx_err == OMX_ErrorStreamCorrupt)
          {
            ofLog(OF_LOG_VERBOSE, "%s::%s - image not unsupported\n", "OMXVideo", __func__);
            return false;
          }

          m_omx_decoder.DisablePort(m_omx_decoder.GetOutputPort(), false);
          m_omx_sched.DisablePort(m_omx_sched.GetInputPort(), false);

          m_omx_image_fx.DisablePort(m_omx_image_fx.GetOutputPort(), false);
          m_omx_image_fx.DisablePort(m_omx_image_fx.GetInputPort(), false);

          port_image.nPortIndex = m_omx_image_fx.GetInputPort();

          omx_err = m_omx_image_fx.SetParameter(OMX_IndexParamPortDefinition, &port_image);
          if(omx_err != OMX_ErrorNone)
          {
            ofLog(OF_LOG_VERBOSE, "%s::%s - error OMX_IndexParamPortDefinition 2 omx_err(0x%08x)\n", "OMXVideo", __func__, omx_err);
          }

          port_image.nPortIndex = m_omx_image_fx.GetOutputPort();
          omx_err = m_omx_image_fx.SetParameter(OMX_IndexParamPortDefinition, &port_image);
          if(omx_err != OMX_ErrorNone)
          {
            ofLog(OF_LOG_VERBOSE, "%s::%s - error OMX_IndexParamPortDefinition 3 omx_err(0x%08x)\n", "OMXVideo", __func__, omx_err);
          }

          m_omx_decoder.EnablePort(m_omx_decoder.GetOutputPort(), false);

          m_omx_image_fx.EnablePort(m_omx_image_fx.GetOutputPort(), false);

          m_omx_image_fx.EnablePort(m_omx_image_fx.GetInputPort(), false);

          m_omx_sched.EnablePort(m_omx_sched.GetInputPort(), false);
        }
      }
    }

    return true;

  }
  
  return false;
}



///////////////////////////////////////////////////////////////////////////////////////////
/*void COMXVideo::SetVideoRect(const CRect& SrcRect, const CRect& DestRect)
{
  if(!m_is_open)
    return;

  OMX_CONFIG_DISPLAYREGIONTYPE configDisplay;
  OMX_INIT_STRUCTURE(configDisplay);
  configDisplay.nPortIndex = m_omx_render.GetInputPort();

  configDisplay.set     = OMX_DISPLAY_SET_FULLSCREEN;
  configDisplay.fullscreen = OMX_FALSE;

  m_omx_render.SetConfig(OMX_IndexConfigDisplayRegion, &configDisplay);

  configDisplay.set     = OMX_DISPLAY_SET_DEST_RECT;
  configDisplay.dest_rect.x_offset  = DestRect.x1;
  configDisplay.dest_rect.y_offset  = DestRect.y1;
  configDisplay.dest_rect.width     = DestRect.Width();
  configDisplay.dest_rect.height    = DestRect.Height();

  m_omx_render.SetConfig(OMX_IndexConfigDisplayRegion, &configDisplay);

  ofLog(OF_LOG_VERBOSE, "dest_rect.x_offset %d dest_rect.y_offset %d dest_rect.width %d dest_rect.height %d\n",
      configDisplay.dest_rect.x_offset, configDisplay.dest_rect.y_offset, 
      configDisplay.dest_rect.width, configDisplay.dest_rect.height);
}*/




