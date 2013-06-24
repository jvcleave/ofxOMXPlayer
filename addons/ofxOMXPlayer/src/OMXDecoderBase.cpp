#include "OMXDecoderBase.h"



#include "OMXStreamInfo.h"

#include "linux/XMemUtils.h"

#include <sys/time.h>
#include <inttypes.h>


OMXDecoderBase::OMXDecoderBase()
{
	
	m_is_open           = false;
	m_Pause             = false;
	m_setStartTime      = true;
	m_extradata         = NULL;
	m_extrasize         = 0;
	m_video_codec_name  = "";
	m_first_frame       = true;
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

OMXDecoderBase::~OMXDecoderBase()
{
	if (m_is_open)
	{
		Close();
	}
}

bool OMXDecoderBase::SendDecoderConfig()
{
	OMX_ERRORTYPE omx_err   = OMX_ErrorNone;
	
	/* send decoder config */
	if(m_extrasize > 0 && m_extradata != NULL)
	{
		ofLogVerbose(__FUNCTION__) << "m_extrasize: " << m_extrasize;
		ofLogVerbose(__FUNCTION__) << "m_extradata: " << m_extradata;
		
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
	
	m_omx_decoder.FlushInput();
	m_omx_tunnel_decoder.Flush();
	
}

void OMXDecoderBase::Close()
{
	ofLogVerbose() << "OMXDecoderBase Close";
}
