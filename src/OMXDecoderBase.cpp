#include "OMXDecoderBase.h"



#include "OMXStreamInfo.h"

#include "XMemUtils.h"

#include <sys/time.h>
#include <inttypes.h>


OMXDecoderBase::OMXDecoderBase()
{

	m_is_open           = false;
	m_Pause             = false;
	m_setStartTime      = true;
	m_extradata         = NULL;
	m_extrasize         = 0;
	m_first_frame       = true;
	m_av_clock			= NULL;
	m_omx_clock			= NULL;
	//ofLogVerbose(__func__) << "OMXDecoderBase::CONSTRUCT";

}

OMXDecoderBase::~OMXDecoderBase()
{

	//ofLogVerbose(__func__) << " START ---------";
	//return;
	//TODO fix this?
	try
	{
		decoderTunnel.Flush();
		/*if(m_deinterlace)
		 m_omx_tunnel_image_fx.Flush();*/
		clockTunnel.Flush();
		schedulerTunnel.Flush();

		clockTunnel.Deestablish();
		decoderTunnel.Deestablish();
		/*if(m_deinterlace)
		 m_omx_tunnel_image_fx.Deestablish();*/
		schedulerTunnel.Deestablish();

		m_omx_decoder.flushInput();

		m_omx_sched.Deinitialize(true);
		/*if(m_deinterlace)
		 m_omx_image_fx.Deinitialize();*/
		m_omx_decoder.Deinitialize(true);
		renderComponent.Deinitialize(true);

		m_is_open       = false;

		if(m_extradata)
		{
			free(m_extradata);
		}
		m_extradata = NULL;
		m_extrasize = 0;

		//m_deinterlace       = false;
		m_first_frame       = true;
		m_setStartTime      = true;
	}
	catch (int e)
	{
		ofLogError(__func__) << "An exception occurred. Exception: " << e << '\n';
	}
	m_is_open       = false;
	//ofLogVerbose(__func__) << " END ---------";

	//m_av_clock->OMXStop();
	//m_av_clock->OMXStateIdle();

	//ofLogVerbose(__func__) << "END ---------";
}

bool OMXDecoderBase::NaluFormatStartCodes(enum AVCodecID codec, uint8_t *in_extradata, int in_extrasize)
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
		default:
			break;
	}
	return false;
}



bool OMXDecoderBase::SendDecoderConfig()
{
	OMX_ERRORTYPE error   = OMX_ErrorNone;

	/* send decoder config */
	if(m_extrasize > 0 && m_extradata != NULL)
	{

		OMX_BUFFERHEADERTYPE *omxBuffer = m_omx_decoder.GetInputBuffer();

		if(omxBuffer == NULL)
		{
			ofLogError(__func__) << "buffer error";
			return false;
		}

		omxBuffer->nOffset = 0;
		omxBuffer->nFilledLen = m_extrasize;
		if(omxBuffer->nFilledLen > omxBuffer->nAllocLen)
		{
			ofLogError(__func__) << "omxBuffer->nFilledLen > omxBuffer->nAllocLen";
			return false;
		}

		memset((unsigned char *)omxBuffer->pBuffer, 0x0, omxBuffer->nAllocLen);
		memcpy((unsigned char *)omxBuffer->pBuffer, m_extradata, omxBuffer->nFilledLen);
		omxBuffer->nFlags = OMX_BUFFERFLAG_CODECCONFIG | OMX_BUFFERFLAG_ENDOFFRAME;

		error = m_omx_decoder.EmptyThisBuffer(omxBuffer);
        OMX_TRACE(error);
		if (error != OMX_ErrorNone)
		{
			return false;
		}

	}

	return true;
}
/*
int OMXDecoderBase::GetInputBufferSize()
{
	return m_omx_decoder.GetInputBufferSize();
}
*/
void OMXDecoderBase::SetDropState(bool bDrop)
{
	m_drop_state = bDrop;
}
/*
unsigned int OMXDecoderBase::GetFreeSpace()
{
	return m_omx_decoder.GetInputBufferSpace();
}

unsigned int OMXDecoderBase::GetSize()
{
	return m_omx_decoder.GetInputBufferSize();
}
*/
void OMXDecoderBase::SubmitEOS()
{
	if(!m_is_open)
	{
		return;
	}

	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_BUFFERHEADERTYPE *omxBuffer = m_omx_decoder.GetInputBuffer();

	if(omxBuffer == NULL)
	{
		ofLogError(__func__) << "buffer NULL";
		return;
	}

	omxBuffer->nOffset     = 0;
	omxBuffer->nFilledLen  = 0;
	omxBuffer->nTimeStamp  = ToOMXTime(0LL);

	omxBuffer->nFlags = OMX_BUFFERFLAG_ENDOFFRAME | OMX_BUFFERFLAG_EOS | OMX_BUFFERFLAG_TIME_UNKNOWN;

	error = m_omx_decoder.EmptyThisBuffer(omxBuffer);
    OMX_TRACE(error);

}

bool OMXDecoderBase::IsEOS()
{
	bool isEndOfStream = false;
	if(!m_is_open)
	{
		isEndOfStream =  true;
	}
	else
	{
		if (m_omx_decoder.IsEOS())
		{

			isEndOfStream =  true;

		}
		//return renderComponent.IsEOS();
	}
	if (isEndOfStream)
	{
		ofLogVerbose("OMXDecoderBase::IsEOS") << "isEndOfStream: " << isEndOfStream;
	}
	return isEndOfStream;
}

bool OMXDecoderBase::Pause()
{
	if(renderComponent.GetComponent() == NULL)
	{
		return false;
	}

	if(m_Pause)
	{
		return true;
	}

	m_Pause = true;

	m_omx_sched.setState(OMX_StatePause);
	renderComponent.setState(OMX_StatePause);

	return true;
}

bool OMXDecoderBase::Resume()
{
	if(renderComponent.GetComponent() == NULL)
	{
		return false;
	}

	if(!m_Pause)
	{
		return true;
	}
	m_Pause = false;

	m_omx_sched.setState(OMX_StateExecuting);
	renderComponent.setState(OMX_StateExecuting);

	return true;
}

void OMXDecoderBase::Reset()
{
	//ofLogVerbose(__func__) << " START";

	m_omx_decoder.flushInput();
	decoderTunnel.Flush();

	//ofLogVerbose(__func__) << " END";
}


void OMXDecoderBase::ProcessCodec(COMXStreamInfo& hints)
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
					m_codingType = OMX_VIDEO_CodingAVC;
					break;
				case FF_PROFILE_H264_MAIN:
					// (role name) video_decoder.avc
					// H.264 Main profile
					m_codingType = OMX_VIDEO_CodingAVC;
					break;
				case FF_PROFILE_H264_HIGH:
					// (role name) video_decoder.avc
					// H.264 Main profile
					m_codingType = OMX_VIDEO_CodingAVC;
					break;
				case FF_PROFILE_UNKNOWN:
					m_codingType = OMX_VIDEO_CodingAVC;
					break;
				default:
					m_codingType = OMX_VIDEO_CodingAVC;
					break;
			}
		}
		break;
		case CODEC_ID_MPEG4:
			// (role name) video_decoder.mpeg4
			// MPEG-4, DivX 4/5 and Xvid compatible
			m_codingType = OMX_VIDEO_CodingMPEG4;
			break;
		case CODEC_ID_MPEG1VIDEO:
		case CODEC_ID_MPEG2VIDEO:
			// (role name) video_decoder.mpeg2
			// MPEG-2
			m_codingType = OMX_VIDEO_CodingMPEG2;
			break;
		case CODEC_ID_H263:
			// (role name) video_decoder.mpeg4
			// MPEG-4, DivX 4/5 and Xvid compatible
			m_codingType = OMX_VIDEO_CodingMPEG4;
			break;
		case CODEC_ID_VP6:
		case CODEC_ID_VP6F:
		case CODEC_ID_VP6A:
			// (role name) video_decoder.vp6
			// VP6
			m_codingType = OMX_VIDEO_CodingVP6;
			break;
		case CODEC_ID_VP8:
			// (role name) video_decoder.vp8
			// VP8
			m_codingType = OMX_VIDEO_CodingVP8;
			break;
		case CODEC_ID_THEORA:
			// (role name) video_decoder.theora
			// theora
			m_codingType = OMX_VIDEO_CodingTheora;
			break;
		case CODEC_ID_MJPEG:
		case CODEC_ID_MJPEGB:
			// (role name) video_decoder.mjpg
			// mjpg
			m_codingType = OMX_VIDEO_CodingMJPEG;
			break;
		case CODEC_ID_VC1:
		case CODEC_ID_WMV3:
			// (role name) video_decoder.vc1
			// VC-1, WMV9
			m_codingType = OMX_VIDEO_CodingWMV;
			break;
		default:
            m_codingType = OMX_VIDEO_CodingUnused;
			ofLog(OF_LOG_VERBOSE, "Video codec id unknown: %x\n", hints.codec);
			break;
	}
}