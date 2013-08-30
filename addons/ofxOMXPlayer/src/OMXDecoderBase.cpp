#include "OMXDecoderBase.h"



#include "OMXStreamInfo.h"

#include "linux/XMemUtils.h"

#include <sys/time.h>
#include <inttypes.h>

int OMXDecoderBase::fillBufferCounter =0;
OMXDecoderBase::OMXDecoderBase()
{
	
	m_is_open           = false;
	m_Pause             = false;
	m_setStartTime      = true;
	m_extradata         = NULL;
	m_extrasize         = 0;
	m_video_codec_name  = "";
	m_first_frame       = true;
	m_av_clock			= NULL;
	m_omx_clock			= NULL;
	decoder_name = OMX_VIDEO_DECODER;
	
	
	ofLogVerbose() << "OMXDecoderBase::CONSTRUCT";
	
}
bool OMXDecoderBase::NaluFormatStartCodes(enum CodecID codec, uint8_t *in_extradata, int in_extrasize)
{
	switch(codec)
	{
		case CODEC_ID_H264:
		{
			if (in_extrasize < 7 || in_extradata == NULL)
			{
				return true;
			}
			// valid avcC atom data always starts with the value 1 (version), otherwise annexb
			else if ( *in_extradata != 1 )
			{
				return true;
			}
		}
		default: break;
	}
	return false;    
}



bool OMXDecoderBase::SendDecoderConfig()
{
	OMX_ERRORTYPE omx_err   = OMX_ErrorNone;
	
	/* send decoder config */
	if(m_extrasize > 0 && m_extradata != NULL)
	{
		ofLogVerbose(__func__) << "m_extrasize: " << m_extrasize;
		ofLogVerbose(__func__) << "m_extradata: " << m_extradata;
		
		OMX_BUFFERHEADERTYPE *omx_buffer = m_omx_decoder.GetInputBuffer();
		
		if(omx_buffer == NULL)
		{
			ofLog(OF_LOG_VERBOSE, "\n%s::%s - buffer error 0x%08x", "OMXDecoderBase", __func__, omx_err);
			return false;
		}
		
		omx_buffer->nOffset = 0;
		omx_buffer->nFilledLen = m_extrasize;
		if(omx_buffer->nFilledLen > omx_buffer->nAllocLen)
		{
			ofLog(OF_LOG_VERBOSE, "\n%s::%s - omx_buffer->nFilledLen > omx_buffer->nAllocLen", "OMXDecoderBase", __func__);
			return false;
		}
		
		memset((unsigned char *)omx_buffer->pBuffer, 0x0, omx_buffer->nAllocLen);
		memcpy((unsigned char *)omx_buffer->pBuffer, m_extradata, omx_buffer->nFilledLen);
		omx_buffer->nFlags = OMX_BUFFERFLAG_CODECCONFIG | OMX_BUFFERFLAG_ENDOFFRAME;
		
		omx_err = m_omx_decoder.EmptyThisBuffer(omx_buffer);
		if (omx_err != OMX_ErrorNone)
		{
			ofLog(OF_LOG_VERBOSE, "\n%s::%s - OMX_EmptyThisBuffer() failed with result(0x%x)\n", "OMXDecoderBase", __func__, omx_err);
			return false;
		}else
		{
			ofLog(OF_LOG_VERBOSE, "OMXDecoderBase::SendDecoderConfig m_extradata: %i ", m_extradata); 
		}
		
	}
	
	return true;
}
int OMXDecoderBase::GetInputBufferSize()
{
	return m_omx_decoder.GetInputBufferSize();
}

void OMXDecoderBase::SetDropState(bool bDrop)
{
	m_drop_state = bDrop;
}

unsigned int OMXDecoderBase::GetFreeSpace()
{
	return m_omx_decoder.GetInputBufferSpace();
}

unsigned int OMXDecoderBase::GetSize()
{
	return m_omx_decoder.GetInputBufferSize();
}


void OMXDecoderBase::WaitCompletion()
{
	if(!m_is_open)
		return;
	
	OMX_ERRORTYPE omx_err = OMX_ErrorNone;
	OMX_BUFFERHEADERTYPE *omx_buffer = m_omx_decoder.GetInputBuffer();
	
	if(omx_buffer == NULL)
	{
		ofLog(OF_LOG_VERBOSE, "%s::%s - buffer error 0x%08x", "OMXDecoderBase", __func__, omx_err);
		return;
	}
	
	omx_buffer->nOffset     = 0;
	omx_buffer->nFilledLen  = 0;
	omx_buffer->nTimeStamp  = ToOMXTime(0LL);
	
	omx_buffer->nFlags = OMX_BUFFERFLAG_ENDOFFRAME | OMX_BUFFERFLAG_EOS | OMX_BUFFERFLAG_TIME_UNKNOWN;
	
	omx_err = m_omx_decoder.EmptyThisBuffer(omx_buffer);
	if (omx_err != OMX_ErrorNone)
	{
		ofLog(OF_LOG_VERBOSE, "%s::%s - OMX_EmptyThisBuffer() failed with result(0x%x)\n", "OMXDecoderBase", __func__, omx_err);
		return;
	}
	
	while(true)
	{
		if(m_omx_render.IsEOS())
		{
			break;
		}
		OMXClock::OMXSleep(50);
	}
	
	return;
}

bool OMXDecoderBase::Pause()
{
	if(m_omx_render.GetComponent() == NULL)
	{
		return false;
	}
	
	if(m_Pause)
	{
		return true;
	}
	
	m_Pause = true;
	
	m_omx_sched.SetStateForComponent(OMX_StatePause);
	m_omx_render.SetStateForComponent(OMX_StatePause);
	
	return true;
}

bool OMXDecoderBase::Resume()
{
	if(m_omx_render.GetComponent() == NULL)
		return false;
	
	if(!m_Pause) return true;
	m_Pause = false;
	
	m_omx_sched.SetStateForComponent(OMX_StateExecuting);
	m_omx_render.SetStateForComponent(OMX_StateExecuting);
	
	return true;
}

void OMXDecoderBase::Reset()
{
	ofLogVerbose(__func__) << " START";
	
	//m_omx_decoder.FlushInput();
	//m_omx_tunnel_decoder.Flush();
	
	ofLogVerbose(__func__) << " END";
}


void OMXDecoderBase::ProcessCodec(COMXStreamInfo &hints)
{
	switch (hints.codec)
	{
		case CODEC_ID_H264:
		{
			switch(hints.profile)
			{
				case FF_PROFILE_H264_BASELINE:
					// (role name) video_decoder.avc
					// H.264 Baseline profile
					decoder_name = OMX_H264BASE_DECODER;
					m_codingType = OMX_VIDEO_CodingAVC;
					m_video_codec_name = "omx-h264";
					break;
				case FF_PROFILE_H264_MAIN:
					// (role name) video_decoder.avc
					// H.264 Main profile
					decoder_name = OMX_H264MAIN_DECODER;
					m_codingType = OMX_VIDEO_CodingAVC;
					m_video_codec_name = "omx-h264";
					break;
				case FF_PROFILE_H264_HIGH:
					// (role name) video_decoder.avc
					// H.264 Main profile
					decoder_name = OMX_H264HIGH_DECODER;
					m_codingType = OMX_VIDEO_CodingAVC;
					m_video_codec_name = "omx-h264";
					break;
				case FF_PROFILE_UNKNOWN:
					decoder_name = OMX_H264HIGH_DECODER;
					m_codingType = OMX_VIDEO_CodingAVC;
					m_video_codec_name = "omx-h264";
					break;
				default:
					decoder_name = OMX_H264HIGH_DECODER;
					m_codingType = OMX_VIDEO_CodingAVC;
					m_video_codec_name = "omx-h264";
					break;
			}
		}
			break;
		case CODEC_ID_MPEG4:
			// (role name) video_decoder.mpeg4
			// MPEG-4, DivX 4/5 and Xvid compatible
			decoder_name = OMX_MPEG4_DECODER;
			m_codingType = OMX_VIDEO_CodingMPEG4;
			m_video_codec_name = "omx-mpeg4";
			break;
		case CODEC_ID_MPEG1VIDEO:
		case CODEC_ID_MPEG2VIDEO:
			// (role name) video_decoder.mpeg2
			// MPEG-2
			decoder_name = OMX_MPEG2V_DECODER;
			m_codingType = OMX_VIDEO_CodingMPEG2;
			m_video_codec_name = "omx-mpeg2";
			break;
		case CODEC_ID_H263:
			// (role name) video_decoder.mpeg4
			// MPEG-4, DivX 4/5 and Xvid compatible
			decoder_name = OMX_MPEG4_DECODER;
			m_codingType = OMX_VIDEO_CodingMPEG4;
			m_video_codec_name = "omx-h263";
			break;
		case CODEC_ID_VP6:
		case CODEC_ID_VP6F:
		case CODEC_ID_VP6A:
			// (role name) video_decoder.vp6
			// VP6
			decoder_name = OMX_VP6_DECODER;
			m_codingType = OMX_VIDEO_CodingVP6;
			m_video_codec_name = "omx-vp6";
			break;
		case CODEC_ID_VP8:
			// (role name) video_decoder.vp8
			// VP8
			decoder_name = OMX_VP8_DECODER;
			m_codingType = OMX_VIDEO_CodingVP8;
			m_video_codec_name = "omx-vp8";
			break;
		case CODEC_ID_THEORA:
			// (role name) video_decoder.theora
			// theora
			decoder_name = OMX_THEORA_DECODER;
			m_codingType = OMX_VIDEO_CodingTheora;
			m_video_codec_name = "omx-theora";
			break;
		case CODEC_ID_MJPEG:
		case CODEC_ID_MJPEGB:
			// (role name) video_decoder.mjpg
			// mjpg
			decoder_name = OMX_MJPEG_DECODER;
			m_codingType = OMX_VIDEO_CodingMJPEG;
			m_video_codec_name = "omx-mjpeg";
			break;
		case CODEC_ID_VC1:
		case CODEC_ID_WMV3:
			// (role name) video_decoder.vc1
			// VC-1, WMV9
			decoder_name = OMX_VC1_DECODER;
			m_codingType = OMX_VIDEO_CodingWMV;
			m_video_codec_name = "omx-vc1";
			break;    
		default:
			ofLog(OF_LOG_VERBOSE, "Video codec id unknown: %x\n", hints.codec);
			break;
	}
	
}