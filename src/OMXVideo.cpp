/*
 *      Copyright (C) 2010 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#if (defined HAVE_CONFIG_H) && (!defined WIN32)
  #include "config.h"
#elif defined(_WIN32)
#include "system.h"
#endif

#include "OMXVideo.h"

#include "OMXStreamInfo.h"

#include "linux/XMemUtils.h"

#include <sys/time.h>
#include <inttypes.h>

#ifdef CLASSNAME
#undef CLASSNAME
#endif
#define CLASSNAME "COMXVideo"



#define OMX_VIDEO_DECODER       "OMX.broadcom.video_decode"
#define OMX_H264BASE_DECODER    OMX_VIDEO_DECODER
#define OMX_H264MAIN_DECODER    OMX_VIDEO_DECODER
#define OMX_H264HIGH_DECODER    OMX_VIDEO_DECODER
#define OMX_MPEG4_DECODER       OMX_VIDEO_DECODER
#define OMX_MSMPEG4V1_DECODER   OMX_VIDEO_DECODER
#define OMX_MSMPEG4V2_DECODER   OMX_VIDEO_DECODER
#define OMX_MSMPEG4V3_DECODER   OMX_VIDEO_DECODER
#define OMX_MPEG4EXT_DECODER    OMX_VIDEO_DECODER
#define OMX_MPEG2V_DECODER      OMX_VIDEO_DECODER
#define OMX_VC1_DECODER         OMX_VIDEO_DECODER
#define OMX_WMV3_DECODER        OMX_VIDEO_DECODER
#define OMX_VP6_DECODER         OMX_VIDEO_DECODER
#define OMX_VP8_DECODER         OMX_VIDEO_DECODER
#define OMX_THEORA_DECODER      OMX_VIDEO_DECODER
#define OMX_MJPEG_DECODER       OMX_VIDEO_DECODER

#define MAX_TEXT_LENGTH 1024

COMXVideo::COMXVideo()
{
	m_is_open           = false;
	m_Pause             = false;
	m_setStartTime      = true;
	m_setStartTimeText  = true;
	m_extradata         = NULL;
	m_extrasize         = 0;
	m_video_codec_name  = "";
	m_first_frame       = true;
	eglBuffer = NULL;
}

COMXVideo::~COMXVideo()
{
  if (m_is_open)
    Close();
}

bool COMXVideo::SendDecoderConfig()
{
  OMX_ERRORTYPE omx_err   = OMX_ErrorNone;

  /* send decoder config */
  if(m_extrasize > 0 && m_extradata != NULL)
  {
    OMX_BUFFERHEADERTYPE *omx_buffer = m_omx_decoder.GetInputBuffer();

    if(omx_buffer == NULL)
    {
      printf("\n%s::%s - buffer error 0x%08x", CLASSNAME, __func__, omx_err);
      return false;
    }

    omx_buffer->nOffset = 0;
    omx_buffer->nFilledLen = m_extrasize;
    if(omx_buffer->nFilledLen > omx_buffer->nAllocLen)
    {
      printf("\n%s::%s - omx_buffer->nFilledLen > omx_buffer->nAllocLen", CLASSNAME, __func__);
      return false;
    }

    memset((unsigned char *)omx_buffer->pBuffer, 0x0, omx_buffer->nAllocLen);
    memcpy((unsigned char *)omx_buffer->pBuffer, m_extradata, omx_buffer->nFilledLen);
    omx_buffer->nFlags = OMX_BUFFERFLAG_CODECCONFIG | OMX_BUFFERFLAG_ENDOFFRAME;
  
    omx_err = m_omx_decoder.EmptyThisBuffer(omx_buffer);
    if (omx_err != OMX_ErrorNone)
    {
      printf("\n%s::%s - OMX_EmptyThisBuffer() failed with result(0x%x)\n", CLASSNAME, __func__, omx_err);
      return false;
    }else {
		printf("COMXVideo::SendDecoderConfig m_extradata: %i ", m_extradata); 
	}

  }
	
  
  return true;
}

bool COMXVideo::Open(COMXStreamInfo &hints, OMXClock *clock, EGLImageKHR eglImage_)
{
	eglImage = eglImage_;
	OMX_ERRORTYPE omx_err   = OMX_ErrorNone;
	std::string decoder_name;

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
	printf("\nm_extrasize::::::::::::::::::::::::::: %u\n", m_extrasize);  
	printf("\nm_extradata as U::::::::::::::::::::::::::: %u\n", m_extradata);
	}

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
	  printf("Vcodec id unknown: %x\n", hints.codec);
	  return false;
	break;
	}


	std::string componentName = "";

	componentName = decoder_name;
	if(!m_omx_decoder.Initialize(componentName, OMX_IndexParamVideoInit))
	return false;

	componentName = "OMX.broadcom.egl_render";
	if(!m_omx_render.Initialize(componentName, OMX_IndexParamVideoInit))
	return false;

	componentName = "OMX.broadcom.video_scheduler";
	if(!m_omx_sched.Initialize(componentName, OMX_IndexParamVideoInit))
	return false;

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

	m_omx_tunnel_decoder.Initialize(&m_omx_decoder, m_omx_decoder.GetOutputPort(), &m_omx_sched, m_omx_sched.GetInputPort());

	m_omx_tunnel_sched.Initialize(&m_omx_sched, m_omx_sched.GetOutputPort(), &m_omx_render, m_omx_render.GetInputPort());
	m_omx_tunnel_clock.Initialize(m_omx_clock, m_omx_clock->GetInputPort() + 1, &m_omx_sched, m_omx_sched.GetOutputPort() + 1);

	omx_err = m_omx_tunnel_clock.Establish(false);
	if(omx_err != OMX_ErrorNone)
	{
	printf("\nCOMXVideo::Open m_omx_tunnel_clock.Establish\n");
	return false;
	}

	omx_err = m_omx_decoder.SetStateForComponent(OMX_StateIdle);
	if (omx_err != OMX_ErrorNone)
	{
		printf("\nCOMXVideo::Open m_omx_decoder.SetStateForComponent\n");
		return false;
	}

	OMX_VIDEO_PARAM_PORTFORMATTYPE formatType;
	OMX_INIT_STRUCTURE(formatType);
	formatType.nPortIndex = m_omx_decoder.GetInputPort();
	formatType.eCompressionFormat = m_codingType;

	if (hints.fpsscale > 0 && hints.fpsrate > 0)
	{
	//formatType.xFramerate = (long long)(1<<16)*hints.fpsrate / hints.fpsscale;
	}
	else
	{
	// formatType.xFramerate = 25 * (1<<16);
	}

	omx_err = m_omx_decoder.SetParameter(OMX_IndexParamVideoPortFormat, &formatType);
	if(omx_err == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_decoder SET OMX_IndexParamVideoPortFormat PASS";
	}else 
	{
		ofLogError() << "m_omx_decoder SET OMX_IndexParamVideoPortFormat FAIL";
		return false;
	}

	

	OMX_PARAM_PORTDEFINITIONTYPE portParam;
	OMX_INIT_STRUCTURE(portParam);
	portParam.nPortIndex = m_omx_decoder.GetInputPort();

	omx_err = m_omx_decoder.GetParameter(OMX_IndexParamPortDefinition, &portParam);
	if(omx_err == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_decoder GET OMX_IndexParamPortDefinition PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_decoder GET OMX_IndexParamPortDefinition FAIL omx_err(0x%08x)\n", omx_err);
		return false;
	}
	portParam.nPortIndex = m_omx_decoder.GetInputPort();
	portParam.nBufferCountActual = VIDEO_BUFFERS;

	portParam.format.video.nFrameWidth  = m_decoded_width;
	portParam.format.video.nFrameHeight = m_decoded_height;

	omx_err = m_omx_decoder.SetParameter(OMX_IndexParamPortDefinition, &portParam);
	if(omx_err == OMX_ErrorNone)
	{
	  ofLogVerbose() << "m_omx_decoder SET OMX_IndexParamPortDefinition PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_decoder SET OMX_IndexParamPortDefinition FAIL omx_err(0x%08x)\n", omx_err);
		return false;
	}


	// Alloc buffers for the omx intput port.
	omx_err = m_omx_decoder.AllocInputBuffers(false);
	if(omx_err == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_decoder AllocInputBuffers PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_decoder AllocInputBuffers FAIL omx_err(0x%08x)\n", omx_err);
		return false;
	}


	omx_err = m_omx_tunnel_decoder.Establish(false);
	if(omx_err == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_tunnel_decoder Establish PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_tunnel_decoder Establish FAIL omx_err(0x%08x)\n", omx_err);
		return false;
	}
	
	m_av_clock->SetSpeed(DVD_PLAYSPEED_NORMAL);
	m_av_clock->OMXStateExecute();
	m_av_clock->OMXStart();
	
	omx_err = m_omx_decoder.SetStateForComponent(OMX_StateExecuting);
	if(omx_err == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_decoder OMX_StateExecuting PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_decoder OMX_StateExecuting FAIL omx_err(0x%08x)", omx_err);
		return false;
	}
	
	
	/*omx_err = m_omx_decoder.WaitForEvent(OMX_EventPortSettingsChanged);	
	if(omx_err == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_decoder WaitForEvent OMX_EventPortSettingsChanged PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_decoder WaitForEvent OMX_EventPortSettingsChanged FAIL omx_err(0x%08x)", omx_err);
		return false;
	}*/


	omx_err = m_omx_tunnel_sched.Establish(false);
	if(omx_err == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_tunnel_sched Establish PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_tunnel_sched Establish FAIL omx_err(0x%08x)", omx_err);
		return false;
	}



	omx_err = m_omx_sched.SetStateForComponent(OMX_StateExecuting);
	if(omx_err == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_sched OMX_StateExecuting PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_sched OMX_StateExecuting FAIL omx_err(0x%08x)", omx_err);
		return false;
	}
	
	omx_err = m_omx_render.SetStateForComponent(OMX_StateIdle);
	if(omx_err == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_render OMX_StateIdle PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_render OMX_StateIdle FAIL omx_err(0x%08x)", omx_err);
		return false;
	}
	
	ofLogVerbose() << "m_omx_render.GetOutputPort(): " << m_omx_render.GetOutputPort();
	m_omx_render.EnablePort(m_omx_render.GetOutputPort(), true);
	if(omx_err == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_render Enable OUTPUT Port PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_render Enable OUTPUT Port  FAIL omx_err(0x%08x)", omx_err);
		return false;
	}
	
	
	
	omx_err = m_omx_render.UseEGLImage(&eglBuffer, m_omx_render.GetOutputPort(), NULL, eglImage);
	if(omx_err == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_render UseEGLImage PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_render UseEGLImage  FAIL omx_err(0x%08x)", omx_err);
		return false;
	}

	
	
	omx_err = m_omx_render.SetStateForComponent(OMX_StateExecuting);
	if(omx_err == OMX_ErrorNone)
	{
		ofLogVerbose() << "m_omx_render OMX_StateExecuting PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "m_omx_render OMX_StateExecuting FAIL omx_err(0x%08x)", omx_err);
		return false;
	}

	if(SendDecoderConfig())
	{
		ofLogVerbose() << "SendDecoderConfig PASS";
	}else 
	{
		ofLog(OF_LOG_ERROR, "SendDecoderConfig");
		return false;
	}

	m_is_open           = true;
	m_drop_state        = false;
	m_setStartTime      = true;
	m_setStartTimeText  = true;


	printf(
	"%s::%s - decoder_component(0x%p), input_port(0x%x), output_port(0x%x)\n",
	CLASSNAME, __func__, m_omx_decoder.GetComponent(), m_omx_decoder.GetInputPort(), m_omx_decoder.GetOutputPort());

	m_first_frame   = true;
	return true;
}

void COMXVideo::Close()
{
  m_omx_tunnel_decoder.Flush();
  m_omx_tunnel_clock.Flush();
  m_omx_tunnel_sched.Flush();

  m_omx_tunnel_clock.Deestablish();
  m_omx_tunnel_decoder.Deestablish();
  m_omx_tunnel_sched.Deestablish();

  m_omx_decoder.FlushInput();

  m_omx_sched.Deinitialize();
  m_omx_decoder.Deinitialize();
  m_omx_render.Deinitialize();

  m_is_open       = false;

  if(m_extradata)
    free(m_extradata);
  m_extradata = NULL;
  m_extrasize = 0;

  m_video_codec_name  = "";
  m_first_frame       = true;
}

void COMXVideo::SetDropState(bool bDrop)
{
  m_drop_state = bDrop;
}

unsigned int COMXVideo::GetFreeSpace()
{
  return m_omx_decoder.GetInputBufferSpace();
}

unsigned int COMXVideo::GetSize()
{
  return m_omx_decoder.GetInputBufferSize();
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
        printf("\nOMXVideo::Decode timeout\n");
        printf("COMXVideo::Decode timeout\n");
        return false;
      }

     /* 
      printf("\nCOMXVideo::Video VDec : pts %lld omx_buffer 0x%08x buffer 0x%08x number %d\n", 
          pts, omx_buffer, omx_buffer->pBuffer, (int)omx_buffer->pAppPrivate);
      printf("VDec : pts %f omx_buffer 0x%08x buffer 0x%08x number %d\n", 
          (float)pts / AV_TIME_BASE, omx_buffer, omx_buffer->pBuffer, (int)omx_buffer->pAppPrivate);*/
      

      omx_buffer->nFlags = 0;
      omx_buffer->nOffset = 0;

      uint64_t val  = (uint64_t)(pts == DVD_NOPTS_VALUE) ? 0 : pts;

      if(m_setStartTime)
      {
        omx_buffer->nFlags = OMX_BUFFERFLAG_STARTTIME;
        m_setStartTime = false;
      }
      else
      {
        if(pts == DVD_NOPTS_VALUE)
          omx_buffer->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;
      }

      omx_buffer->nTimeStamp = ToOMXTime(val);

      omx_buffer->nFilledLen = (demuxer_bytes > omx_buffer->nAllocLen) ? omx_buffer->nAllocLen : demuxer_bytes;
      memcpy(omx_buffer->pBuffer, demuxer_content, omx_buffer->nFilledLen);

      demuxer_bytes -= omx_buffer->nFilledLen;
      demuxer_content += omx_buffer->nFilledLen;

      if(demuxer_bytes == 0)
        omx_buffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

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
          printf("\n%s::%s - OMX_EmptyThisBuffer() failed with result(0x%x)\n", CLASSNAME, __func__, omx_err);
          nRetry++;
        }
        if(nRetry == 5)
        {
          printf("\n%s::%s - OMX_EmptyThisBuffer() finaly failed\n", CLASSNAME, __func__);
          printf("%s::%s - OMX_EmptyThisBuffer() finaly failed\n", CLASSNAME, __func__);
          return false;
        }
      }

    }
    return true;
  }
  return false;
}

void COMXVideo::Reset(void)
{

  m_omx_decoder.FlushInput();
  m_omx_tunnel_decoder.Flush();

}

///////////////////////////////////////////////////////////////////////////////////////////
bool COMXVideo::Pause()
{
  if(m_omx_render.GetComponent() == NULL)
    return false;

  if(m_Pause) return true;
  m_Pause = true;

  m_omx_sched.SetStateForComponent(OMX_StatePause);
  m_omx_render.SetStateForComponent(OMX_StatePause);

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
bool COMXVideo::Resume()
{
  if(m_omx_render.GetComponent() == NULL)
    return false;

  if(!m_Pause) return true;
  m_Pause = false;

  m_omx_sched.SetStateForComponent(OMX_StateExecuting);
  m_omx_render.SetStateForComponent(OMX_StateExecuting);

  return true;
}


int COMXVideo::GetInputBufferSize()
{
  return m_omx_decoder.GetInputBufferSize();
}

void COMXVideo::WaitCompletion()
{
  if(!m_is_open)
    return;

  OMX_ERRORTYPE omx_err = OMX_ErrorNone;
  OMX_BUFFERHEADERTYPE *omx_buffer = m_omx_decoder.GetInputBuffer();
  
  if(omx_buffer == NULL)
  {
    printf("%s::%s - buffer error 0x%08x", CLASSNAME, __func__, omx_err);
    return;
  }
  
  omx_buffer->nOffset     = 0;
  omx_buffer->nFilledLen  = 0;
  omx_buffer->nTimeStamp  = ToOMXTime(0LL);

  omx_buffer->nFlags = OMX_BUFFERFLAG_ENDOFFRAME | OMX_BUFFERFLAG_EOS | OMX_BUFFERFLAG_TIME_UNKNOWN;
  
  omx_err = m_omx_decoder.EmptyThisBuffer(omx_buffer);
  if (omx_err != OMX_ErrorNone)
  {
    printf("\n%s::%s - OMX_EmptyThisBuffer() failed with result(0x%x)\n", CLASSNAME, __func__, omx_err);
    return;
  }


  while(true)
  {
    if(m_omx_render.IsEOS())
      break;

    OMXClock::OMXSleep(50);
  }

  return;
}
