#include "OMXEGLImage.h"



unsigned long long currentFrameTime;
unsigned long long lastFrameTime;

bool isFirstCallback = true;
OMXEGLImage::OMXEGLImage()
{
	ofLogVerbose() << "OMXEGLImage CONSTRUCT";
	eglBuffer = NULL;
	
}

OMX_ERRORTYPE onFillBufferDone(OMX_HANDLETYPE hComponent,
							   OMX_PTR pAppData,
							   OMX_BUFFERHEADERTYPE* pBuffer)
{    
	/*if (isFirstCallback) {
		isFirstCallback = false;
		currentFrameTime = ofGetElapsedTimeMillis();
		ofLogVerbose() << "isFirstCallback";
	}else 
	{
		lastFrameTime = currentFrameTime;
		currentFrameTime = ofGetElapsedTimeMillis();
		ofLogVerbose() << "Frame process time: " << currentFrameTime - lastFrameTime;
		
	}*/

	//ofLogVerbose() << "onFillBufferDone<----------";
	//COMXCoreComponent *ctx = static_cast<COMXCoreComponent*>(pAppData);
	OMX_ERRORTYPE didFillBuffer = OMX_FillThisBuffer(hComponent, pBuffer);
	if (didFillBuffer == OMX_ErrorNone) 
	{
		//OMXDecoderBase *ctx = static_cast<OMXDecoderBase*>(pAppData);
		
		OMXDecoderBase::fillBufferCounter++;
		//ofLogVerbose(__func__) << " fillBufferCounter: " << fillBufferCounter;
	}
	
	return didFillBuffer;
}

bool OMXEGLImage::Open(COMXStreamInfo &hints, OMXClock *clock)
{
	OMX_ERRORTYPE error   = OMX_ErrorNone;
	

	m_video_codec_name      = "";
	m_codingType            = OMX_VIDEO_CodingUnused;

	m_decoded_width  = hints.width;
	m_decoded_height = hints.height;
	
	

	if(!m_decoded_width || !m_decoded_height)
	return false;

	if(hints.extrasize > 0 && hints.extradata != NULL)
	{
		m_extrasize = hints.extrasize;
		m_extradata = (uint8_t *)malloc(m_extrasize);
		memcpy(m_extradata, hints.extradata, hints.extrasize);
	}
	
	ProcessCodec(hints);
	

	std::string componentName = decoder_name;
	if(!m_omx_decoder.Initialize(componentName, OMX_IndexParamVideoInit))
	{
		return false;
	}

	componentName = "OMX.broadcom.egl_render";
	if(!m_omx_render.Initialize(componentName, OMX_IndexParamVideoInit))
	{
		return false;
	}
	
	componentName = "OMX.broadcom.video_scheduler";
	if(!m_omx_sched.Initialize(componentName, OMX_IndexParamVideoInit))
	{
		return false;
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
	
	m_omx_tunnel_decoder.Initialize(&m_omx_decoder,		m_omx_decoder.GetOutputPort(),		&m_omx_sched,	m_omx_sched.GetInputPort());
	m_omx_tunnel_sched.Initialize(	&m_omx_sched,		m_omx_sched.GetOutputPort(),		&m_omx_render,	m_omx_render.GetInputPort());
	m_omx_tunnel_clock.Initialize(	m_omx_clock,		m_omx_clock->GetInputPort() + 1,	&m_omx_sched,	m_omx_sched.GetOutputPort() + 1);
	

	error = m_omx_decoder.SetStateForComponent(OMX_StateIdle);
	if (error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "m_omx_decoder OMX_StateIdle FAIL";
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
	if(error == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_decoder SET OMX_IndexParamVideoPortFormat PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_decoder GET OMX_IndexParamVideoPortFormat FAIL error: 0x%08x\n", error);
		return false;
	}
	
	OMX_PARAM_PORTDEFINITIONTYPE portParam;
	OMX_INIT_STRUCTURE(portParam);
	portParam.nPortIndex = m_omx_decoder.GetInputPort();

	error = m_omx_decoder.GetParameter(OMX_IndexParamPortDefinition, &portParam);
	if(error == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_decoder GET OMX_IndexParamPortDefinition PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_decoder GET OMX_IndexParamPortDefinition FAIL error: 0x%08x\n", error);
		return false;
	}

	ofLogVerbose(__func__) << "portParam.nBufferCountActual GET VAR --------------------------:" << portParam.nBufferCountActual;
	ofLogVerbose(__func__) << "portParam.format.video.nFrameWidth GET VAR --------------------------:" << portParam.format.video.nFrameWidth;

	int numVideoBuffers = 80; //20 is minimum - can get up to 80
	portParam.nBufferCountActual = numVideoBuffers; 

	portParam.format.video.nFrameWidth  = m_decoded_width;
	portParam.format.video.nFrameHeight = m_decoded_height;

	error = m_omx_decoder.SetParameter(OMX_IndexParamPortDefinition, &portParam);
	if(error == OMX_ErrorNone)
	{
	  ofLogVerbose() << "m_omx_decoder SET OMX_IndexParamPortDefinition PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_decoder SET OMX_IndexParamPortDefinition FAIL error: 0x%08x\n", error);
		return false;
	}
	
	
	error = m_omx_tunnel_clock.Establish(false);
	if(error != OMX_ErrorNone)
	{
		ofLogError(__func__) << "m_omx_tunnel_clock.Establish FAIL";
		return false;
	}
	
	
	OMX_PARAM_BRCMVIDEODECODEERRORCONCEALMENTTYPE concanParam;
	OMX_INIT_STRUCTURE(concanParam);
	concanParam.bStartWithValidFrame = OMX_FALSE;
	
	error = m_omx_decoder.SetParameter(OMX_IndexParamBrcmVideoDecodeErrorConcealment, &concanParam);
	if(error == OMX_ErrorNone)
	{
		ofLogVerbose()	<< "m_omx_decoder OMX_IndexParamBrcmVideoDecodeErrorConcealment PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_decoder OMX_IndexParamBrcmVideoDecodeErrorConcealment FAIL error: 0x%08x\n", error);
		return false;
	}
	
	if(NaluFormatStartCodes(hints.codec, m_extradata, m_extrasize))
	{
		OMX_NALSTREAMFORMATTYPE nalStreamFormat;
		OMX_INIT_STRUCTURE(nalStreamFormat);
		nalStreamFormat.nPortIndex = m_omx_decoder.GetInputPort();
		nalStreamFormat.eNaluFormat = OMX_NaluFormatOneNaluPerBuffer;
		
		error = m_omx_decoder.SetParameter((OMX_INDEXTYPE)OMX_IndexParamNalStreamFormatSelect, &nalStreamFormat);
		if (error == OMX_ErrorNone)
		{
			ofLogVerbose()	<< "Open OMX_IndexParamNalStreamFormatSelect PASS";
		}else 
		{
			ofLog(OF_LOG_ERROR, "Open OMX_IndexParamNalStreamFormatSelect FAIL (0%08x)\n", error);
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
		
		if (error == OMX_ErrorNone)
		{
			ofLogVerbose()	<< "Open OMX_IndexParamBrcmVideoTimestampFifo PASS";
		}else 
		{
			ofLog(OF_LOG_ERROR, "Open OMX_IndexParamBrcmVideoTimestampFifo error (0%08x)\n", error);
			return false;
		}

		
	}
	
	
	// Alloc buffers for the omx intput port.
	error = m_omx_decoder.AllocInputBuffers();
	if(error == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_decoder AllocInputBuffers PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_decoder AllocInputBuffers FAIL error: 0x%08x\n", error);
		return false;
	}


	error = m_omx_tunnel_decoder.Establish(false);
	if(error == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_tunnel_decoder Establish PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_tunnel_decoder Establish FAIL error: 0x%08x\n", error);
		return false;
	}
	
	error = m_omx_decoder.SetStateForComponent(OMX_StateExecuting);
	if(error == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_decoder OMX_StateExecuting PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_decoder OMX_StateExecuting FAIL error: 0x%08x", error);
		return false;
	}
	

	error = m_omx_tunnel_sched.Establish(false);
	if(error == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_tunnel_sched Establish PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_tunnel_sched Establish FAIL error: 0x%08x", error);
		return false;
	}

	error = m_omx_sched.SetStateForComponent(OMX_StateExecuting);
	if(error == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_sched OMX_StateExecuting PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_sched OMX_StateExecuting FAIL error: 0x%08x", error);
		return false;
	}
	
	error = m_omx_render.SetStateForComponent(OMX_StateIdle);
	if(error == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_render OMX_StateIdle PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_render OMX_StateIdle FAIL error: 0x%08x", error);
		return false;
	}
	
	
	ofLogVerbose() << "m_omx_render.GetOutputPort(): " << m_omx_render.GetOutputPort();
	m_omx_render.EnablePort(m_omx_render.GetOutputPort(), false);
	if(error == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_render Enable OUTPUT Port PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_render Enable OUTPUT Port  FAIL error: 0x%08x", error);
		return false;
	}
	error = m_omx_render.UseEGLImage(&eglBuffer, m_omx_render.GetOutputPort(), NULL, GlobalEGLContainer::getInstance().eglImage);
	if(error == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_render UseEGLImage PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_render UseEGLImage  FAIL error: 0x%08x", error);
		return false;
	}

	
	if(SendDecoderConfig())
	{
		ofLogVerbose() << "SendDecoderConfig PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "SendDecoderConfig FAIL");
		return false;
	}
	
	m_omx_render.SetCustomDecoderFillBufferDoneHandler(onFillBufferDone);
	error = m_omx_render.SetStateForComponent(OMX_StateExecuting);
	if(error == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_render OMX_StateExecuting PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_render OMX_StateExecuting FAIL error: 0x%08x", error);
		return false;
	}
	error = m_omx_render.FillThisBuffer(eglBuffer);
	if(error == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_render FillThisBuffer PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_render FillThisBuffer FAIL error: 0x%08x", error);
		if (error == OMX_ErrorIncorrectStateOperation) 
		{
			ofLogError(__func__) << "NEED EGL HACK";
		}
		return false;
	}
	
	m_is_open           = true;
	m_drop_state        = false;
	m_setStartTime      = true;


	ofLog(OF_LOG_VERBOSE, 
	"%s::%s - decoder_component: 0x%p, input_port: 0x%x, output_port: 0x%x \n",
	"OMXEGLImage", __func__, m_omx_decoder.GetComponent(), m_omx_decoder.GetInputPort(), m_omx_decoder.GetOutputPort());

	m_first_frame   = true;
	// start from assuming all recent frames had valid pts
	m_history_valid_pts = ~0;
	return true;
}

OMXEGLImage::~OMXEGLImage()
{
	ofLogVerbose(__func__) << "m_is_open: " << m_is_open;
	if (m_is_open) 
	{
		Close();
	}
}




void OMXEGLImage::Close()
{
	ofLogVerbose(__func__) << " Start";
	m_omx_tunnel_decoder.Flush();
	m_omx_tunnel_clock.Flush();
	m_omx_tunnel_sched.Flush();
	
	bool noWait = true;
	m_omx_tunnel_clock.Deestablish(noWait);
	m_omx_tunnel_decoder.Deestablish(noWait);
	m_omx_tunnel_sched.Deestablish(noWait);
	 
	//This already happened in Flush above
	//m_omx_sched.FlushAll();
	//m_omx_decoder.FlushAll();
	//m_omx_render.DisablePort(m_omx_render.GetOutputPort(), false);

	
	bool doFlush = false;
	m_omx_sched.Deinitialize(doFlush);
	m_omx_decoder.Deinitialize(doFlush);
	m_omx_render.Deinitialize(doFlush);
	m_is_open       = false;

	if(m_extradata)
	{
	  free(m_extradata);
	}
	m_extradata = NULL;
	m_extrasize = 0;

	m_video_codec_name  = "";
	m_first_frame       = true;
	
	
	ofLogVerbose(__func__) << " END";
}


int OMXEGLImage::Decode(uint8_t *pData, int iSize, double dts, double pts)
{
	OMX_ERRORTYPE error;

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
				ofLog(OF_LOG_VERBOSE, "OMXEGLImage::Decode timeout\n");
				return false;
			}

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
				error = m_omx_decoder.EmptyThisBuffer(omx_buffer);
				
				
				if (error == OMX_ErrorNone)
				{
					break;
				}
				else
				{
					ofLog(OF_LOG_VERBOSE, "\n%s::%s - OMX_EmptyThisBuffer() failed with result(0x%x)\n", "OMXEGLImage", __func__, error);
					nRetry++;
				}
				if(nRetry == 5)
				{
					ofLog(OF_LOG_VERBOSE, "\n%s::%s - OMX_EmptyThisBuffer() finaly failed\n", "OMXEGLImage", __func__);
					return false;
				}
			}

		}
		return true;
	}
	return false;
}
