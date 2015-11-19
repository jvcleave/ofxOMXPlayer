#include "OMXDecoderBase.h"



#include "OMXStreamInfo.h"

#include "XMemUtils.h"

#include <sys/time.h>
#include <inttypes.h>


OMXDecoderBase::OMXDecoderBase()
{

	isOpen           = false;
	doPause             = false;
	doSetStartTime      = true;
	extraData         = NULL;
	extraSize         = 0;
	isFirstFrame       = true;
	omxClock			= NULL;
	clockComponent			= NULL;
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

		decoderComponent.flushInput();

		schedulerComponent.Deinitialize(true);
		/*if(m_deinterlace)
		 m_omx_image_fx.Deinitialize();*/
		decoderComponent.Deinitialize(true);
		renderComponent.Deinitialize(true);

		isOpen       = false;

		if(extraData)
		{
			free(extraData);
		}
		extraData = NULL;
		extraSize = 0;

		//m_deinterlace       = false;
		isFirstFrame       = true;
		doSetStartTime      = true;
	}
	catch (int e)
	{
		ofLogError(__func__) << "An exception occurred. Exception: " << e << '\n';
	}
	isOpen       = false;
	//ofLogVerbose(__func__) << " END ---------";

	//omxClock->stop();
	//omxClock->setToIdleState();

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
	if(extraSize > 0 && extraData != NULL)
	{

		OMX_BUFFERHEADERTYPE *omxBuffer = decoderComponent.getInputBuffer();

		if(omxBuffer == NULL)
		{
			ofLogError(__func__) << "buffer error";
			return false;
		}

		omxBuffer->nOffset = 0;
		omxBuffer->nFilledLen = extraSize;
		if(omxBuffer->nFilledLen > omxBuffer->nAllocLen)
		{
			ofLogError(__func__) << "omxBuffer->nFilledLen > omxBuffer->nAllocLen";
			return false;
		}

		memset((unsigned char *)omxBuffer->pBuffer, 0x0, omxBuffer->nAllocLen);
		memcpy((unsigned char *)omxBuffer->pBuffer, extraData, omxBuffer->nFilledLen);
		omxBuffer->nFlags = OMX_BUFFERFLAG_CODECCONFIG | OMX_BUFFERFLAG_ENDOFFRAME;

		error = decoderComponent.EmptyThisBuffer(omxBuffer);
        OMX_TRACE(error);
		if (error != OMX_ErrorNone)
		{
			return false;
		}

	}

	return true;
}
/*
int OMXDecoderBase::getInputBufferSize()
{
	return decoderComponent.getInputBufferSize();
}
*/
void OMXDecoderBase::SetDropState(bool bDrop)
{
	m_drop_state = bDrop;
}
/*
unsigned int OMXDecoderBase::GetFreeSpace()
{
	return decoderComponent.getInputBufferSpace();
}

unsigned int OMXDecoderBase::GetSize()
{
	return decoderComponent.getInputBufferSize();
}
*/
void OMXDecoderBase::submitEOS()
{
	if(!isOpen)
	{
		return;
	}

	OMX_ERRORTYPE error = OMX_ErrorNone;
	OMX_BUFFERHEADERTYPE *omxBuffer = decoderComponent.getInputBuffer();

	if(omxBuffer == NULL)
	{
		ofLogError(__func__) << "buffer NULL";
		return;
	}

	omxBuffer->nOffset     = 0;
	omxBuffer->nFilledLen  = 0;
	omxBuffer->nTimeStamp  = ToOMXTime(0LL);

	omxBuffer->nFlags = OMX_BUFFERFLAG_ENDOFFRAME | OMX_BUFFERFLAG_EOS | OMX_BUFFERFLAG_TIME_UNKNOWN;

	error = decoderComponent.EmptyThisBuffer(omxBuffer);
    OMX_TRACE(error);

}

bool OMXDecoderBase::EOS()
{
	bool isEndOfStream = false;
	if(!isOpen)
	{
		isEndOfStream =  true;
	}
	else
	{
		if (decoderComponent.EOS())
		{

			isEndOfStream =  true;

		}
		//return renderComponent.EOS();
	}
	if (isEndOfStream)
	{
		ofLogVerbose("OMXDecoderBase::EOS") << "isEndOfStream: " << isEndOfStream;
	}
	return isEndOfStream;
}

bool OMXDecoderBase::pause()
{
	if(renderComponent.getHandle() == NULL)
	{
		return false;
	}

	if(doPause)
	{
		return true;
	}

	doPause = true;

	schedulerComponent.setState(OMX_StatePause);
	renderComponent.setState(OMX_StatePause);

	return true;
}

bool OMXDecoderBase::resume()
{
	if(renderComponent.getHandle() == NULL)
	{
		return false;
	}

	if(!doPause)
	{
		return true;
	}
	doPause = false;

	schedulerComponent.setState(OMX_StateExecuting);
	renderComponent.setState(OMX_StateExecuting);

	return true;
}

void OMXDecoderBase::Reset()
{
	//ofLogVerbose(__func__) << " START";

	decoderComponent.flushInput();
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