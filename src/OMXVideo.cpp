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
#include "utils/log.h"
//#include "XMemUtils.h"

#include <sys/time.h>
#include <inttypes.h>

#ifdef CLASSNAME
#undef CLASSNAME
#endif
#define CLASSNAME "COMXVideo"


#ifndef OMX_Maps_h

#define OMX_IMAGE_FX (OMX_STRING)"OMX.broadcom.image_fx"
#define IMAGE_FX_INPUT_PORT 190
#define IMAGE_FX_OUTPUT_PORT 191


#define OMX_CAMERA (OMX_STRING)"OMX.broadcom.camera"
#define CAMERA_PREVIEW_PORT        70
#define CAMERA_OUTPUT_PORT        71
#define CAMERA_STILL_OUTPUT_PORT 72

#define OMX_IMAGE_ENCODER (OMX_STRING)"OMX.broadcom.image_encode"
#define IMAGE_ENCODER_INPUT_PORT 340
#define IMAGE_ENCODER_OUTPUT_PORT 341

#define OMX_IMAGE_DECODER (OMX_STRING)"OMX.broadcom.image_decode"
#define IMAGE_DECODER_INPUT_PORT 320
#define IMAGE_DECODER_OUTPUT_PORT 321


#define OMX_RESIZER (OMX_STRING)"OMX.broadcom.resize"
#define RESIZER_INPUT_PORT 60
#define RESIZER_OUTPUT_PORT 61

#define OMX_VIDEO_ENCODER (OMX_STRING)"OMX.broadcom.video_encode"
#define VIDEO_ENCODE_INPUT_PORT 200
#define VIDEO_ENCODE_OUTPUT_PORT 201

#define OMX_VIDEO_DECODER (OMX_STRING)"OMX.broadcom.video_decode"
#define VIDEO_DECODE_INPUT_PORT 130
#define VIDEO_DECODE_OUTPUT_PORT 131


#define OMX_VIDEO_SPLITTER (OMX_STRING)"OMX.broadcom.video_splitter"
#define VIDEO_SPLITTER_INPUT_PORT 250

#define VIDEO_SPLITTER_OUTPUT_PORT1 251
#define VIDEO_SPLITTER_OUTPUT_PORT2 252
#define VIDEO_SPLITTER_OUTPUT_PORT3 253
#define VIDEO_SPLITTER_OUTPUT_PORT4 254

#define OMX_VIDEO_RENDER (OMX_STRING)"OMX.broadcom.video_render"
#define VIDEO_RENDER_INPUT_PORT    90

#define OMX_EGL_RENDER (OMX_STRING)"OMX.broadcom.egl_render"
#define EGL_RENDER_INPUT_PORT    220
#define EGL_RENDER_OUTPUT_PORT    221

#define OMX_NULL_SINK (OMX_STRING)"OMX.broadcom.null_sink"
#define NULL_SINK_INPUT_PORT 240

#define OMX_VIDEO_SCHEDULER (OMX_STRING)"OMX.broadcom.video_scheduler"
#define VIDEO_SCHEDULER_INPUT_PORT 10
#define VIDEO_SCHEDULER_OUTPUT_PORT 11
#define VIDEO_SCHEDULER_CLOCK_PORT 12

#define OMX_CLOCK (OMX_STRING)"OMX.broadcom.clock"
#define OMX_CLOCK_OUTPUT_PORT_0 80
#define OMX_CLOCK_OUTPUT_PORT_1 81
#define OMX_CLOCK_OUTPUT_PORT_2 82
#define OMX_CLOCK_OUTPUT_PORT_3 83
#define OMX_CLOCK_OUTPUT_PORT_4 84
#define OMX_CLOCK_OUTPUT_PORT_5 85



#endif


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


#if !defined(OMX_TRACE)
#define OMX_TRACE(x) ofLog() << __LINE__ << " "  << omxErrorTypes[x]
#endif



std::map<OMX_EVENTTYPE, std::string> eventTypes;

COMXVideo::COMXVideo() : m_video_codec_name("")
{
    filtersEnabled = false;
    m_is_open           = false;
    m_deinterlace       = false;
    m_drop_state        = false;
    m_omx_clock         = NULL;
    m_av_clock          = NULL;
    m_submitted_eos     = false;
    m_failed_eos        = false;
    m_settings_changed  = false;
    m_setStartTime      = false;
    m_transform         = OMX_DISPLAY_ROT0;
    m_pixel_aspect      = 1.0f;
    frameCounter = 0;
    omxErrorTypes[OMX_ErrorNone] =  "OMX_ErrorNone";
    omxErrorTypes[OMX_ErrorInsufficientResources] =  "OMX_ErrorInsufficientResources";
    omxErrorTypes[OMX_ErrorUndefined] =  "OMX_ErrorUndefined";
    omxErrorTypes[OMX_ErrorInvalidComponentName] =  "OMX_ErrorInvalidComponentName";
    omxErrorTypes[OMX_ErrorComponentNotFound] =  "OMX_ErrorComponentNotFound";
    omxErrorTypes[OMX_ErrorInvalidComponent] =  "OMX_ErrorInvalidComponent";
    omxErrorTypes[OMX_ErrorBadParameter] =  "OMX_ErrorBadParameter";
    omxErrorTypes[OMX_ErrorNotImplemented] =  "OMX_ErrorNotImplemented";
    omxErrorTypes[OMX_ErrorUnderflow] =  "OMX_ErrorUnderflow";
    omxErrorTypes[OMX_ErrorOverflow] =  "OMX_ErrorOverflow";
    omxErrorTypes[OMX_ErrorHardware] =  "OMX_ErrorHardware";
    omxErrorTypes[OMX_ErrorInvalidState] =  "OMX_ErrorInvalidState";
    omxErrorTypes[OMX_ErrorStreamCorrupt] =  "OMX_ErrorStreamCorrupt";
    omxErrorTypes[OMX_ErrorPortsNotCompatible] =  "OMX_ErrorPortsNotCompatible";
    omxErrorTypes[OMX_ErrorResourcesLost] =  "OMX_ErrorResourcesLost";
    omxErrorTypes[OMX_ErrorNoMore] =  "OMX_ErrorNoMore";
    omxErrorTypes[OMX_ErrorVersionMismatch] =  "OMX_ErrorVersionMismatch";
    omxErrorTypes[OMX_ErrorNotReady] =  "OMX_ErrorNotReady";
    omxErrorTypes[OMX_ErrorTimeout] =  "OMX_ErrorTimeout";
    omxErrorTypes[OMX_ErrorSameState] =  "OMX_ErrorSameState";
    omxErrorTypes[OMX_ErrorResourcesPreempted] =  "OMX_ErrorResourcesPreempted";
    omxErrorTypes[OMX_ErrorPortUnresponsiveDuringAllocation] =  "OMX_ErrorPortUnresponsiveDuringAllocation";
    omxErrorTypes[OMX_ErrorPortUnresponsiveDuringDeallocation] =  "OMX_ErrorPortUnresponsiveDuringDeallocation";
    omxErrorTypes[OMX_ErrorPortUnresponsiveDuringStop] =  "OMX_ErrorPortUnresponsiveDuringStop";
    omxErrorTypes[OMX_ErrorIncorrectStateTransition] =  "OMX_ErrorIncorrectStateTransition";
    omxErrorTypes[OMX_ErrorIncorrectStateOperation] =  "OMX_ErrorIncorrectStateOperation";
    omxErrorTypes[OMX_ErrorUnsupportedSetting] =  "OMX_ErrorUnsupportedSetting";
    omxErrorTypes[OMX_ErrorUnsupportedIndex] =  "OMX_ErrorUnsupportedIndex";
    omxErrorTypes[OMX_ErrorBadPortIndex] =  "OMX_ErrorBadPortIndex";
    omxErrorTypes[OMX_ErrorPortUnpopulated] =  "OMX_ErrorPortUnpopulated";
    omxErrorTypes[OMX_ErrorComponentSuspended] =  "OMX_ErrorComponentSuspended";
    omxErrorTypes[OMX_ErrorDynamicResourcesUnavailable] =  "OMX_ErrorDynamicResourcesUnavailable";
    omxErrorTypes[OMX_ErrorMbErrorsInFrame] =  "OMX_ErrorMbErrorsInFrame";
    omxErrorTypes[OMX_ErrorFormatNotDetected] =  "OMX_ErrorFormatNotDetected";
    omxErrorTypes[OMX_ErrorContentPipeOpenFailed] =  "OMX_ErrorContentPipeOpenFailed";
    omxErrorTypes[OMX_ErrorContentPipeCreationFailed] =  "OMX_ErrorContentPipeCreationFailed";
    omxErrorTypes[OMX_ErrorSeperateTablesUsed] =  "OMX_ErrorSeperateTablesUsed";
    omxErrorTypes[OMX_ErrorTunnelingUnsupported] =  "OMX_ErrorTunnelingUnsupported";
    omxErrorTypes[OMX_ErrorKhronosExtensions] =  "OMX_ErrorKhronosExtensions";
    omxErrorTypes[OMX_ErrorVendorStartUnused] =  "OMX_ErrorVendorStartUnused";
    omxErrorTypes[OMX_ErrorDiskFull] =  "OMX_ErrorDiskFull";
    omxErrorTypes[OMX_ErrorMaxFileSize] =  "OMX_ErrorMaxFileSize";
    omxErrorTypes[OMX_ErrorDrmUnauthorised] =  "OMX_ErrorDrmUnauthorised";
    omxErrorTypes[OMX_ErrorDrmExpired] =  "OMX_ErrorDrmExpired";
    omxErrorTypes[OMX_ErrorDrmGeneral] =  "OMX_ErrorDrmGeneral";
    
    
    eventTypes[OMX_EventError] = "OMX_EventError";
    eventTypes[OMX_EventCmdComplete] = "OMX_EventCmdComplete";
    eventTypes[OMX_EventMark] = "OMX_EventMark";
    eventTypes[OMX_EventPortSettingsChanged] = "OMX_EventPortSettingsChanged";
    eventTypes[OMX_EventBufferFlag] = "OMX_EventBufferFlag";
    eventTypes[OMX_EventResourcesAcquired] = "OMX_EventResourcesAcquired";
    eventTypes[OMX_EventComponentResumed] = "OMX_EventComponentResumed";
    eventTypes[OMX_EventDynamicResourcesAvailable] = "OMX_EventDynamicResourcesAvailable";
    eventTypes[OMX_EventKhronosExtensions] = "OMX_EventKhronosExtensions";
    eventTypes[OMX_EventVendorStartUnused] = "OMX_EventVendorStartUnused";
    eventTypes[OMX_EventParamOrConfigChanged] = "OMX_EventParamOrConfigChanged";
}


COMXVideo::~COMXVideo()
{
    Close();
}

bool COMXVideo::SendDecoderConfig()
{
    CSingleLock lock (m_critSection);
    OMX_ERRORTYPE error   = OMX_ErrorNone;
    
    /* send decoder config */
    if(m_config.hints.extrasize > 0 && m_config.hints.extradata != NULL)
    {
        OMX_BUFFERHEADERTYPE *omx_buffer = m_omx_decoder.GetInputBuffer();
        
        if(omx_buffer == NULL)
        {
            ofLog(OF_LOG_NOTICE, "%s::%s - buffer error %s", CLASSNAME, __func__, omxErrorTypes[error].c_str());
            return false;
        }
        
        omx_buffer->nOffset = 0;
        omx_buffer->nFilledLen = std::min((OMX_U32)m_config.hints.extrasize, omx_buffer->nAllocLen);
        
        memset((unsigned char *)omx_buffer->pBuffer, 0x0, omx_buffer->nAllocLen);
        memcpy((unsigned char *)omx_buffer->pBuffer, m_config.hints.extradata, omx_buffer->nFilledLen);
        omx_buffer->nFlags = OMX_BUFFERFLAG_CODECCONFIG | OMX_BUFFERFLAG_ENDOFFRAME;
        
        error = m_omx_decoder.EmptyThisBuffer(omx_buffer);
        if (error != OMX_ErrorNone)
        {
            ofLog(OF_LOG_NOTICE, "%s::%s - OMX_EmptyThisBuffer() failed with result(%s)\n", CLASSNAME, __func__, omxErrorTypes[error].c_str());
            m_omx_decoder.DecoderEmptyBufferDone(m_omx_decoder.GetComponent(), omx_buffer);
            return false;
        }
    }
    return true;
}

bool COMXVideo::NaluFormatStartCodes(enum AVCodecID codec, uint8_t *in_extradata, int in_extrasize)
{
    switch(codec)
    {
        case AV_CODEC_ID_H264:
            if (in_extrasize < 7 || in_extradata == NULL)
                return true;
            // valid avcC atom data always starts with the value 1 (version), otherwise annexb
            else if ( *in_extradata != 1 )
                return true;
        default: break;
    }
    return false;    
}

void COMXVideo::PortSettingsChangedLogger(OMX_PARAM_PORTDEFINITIONTYPE port_image, int interlaceEMode)
{
    ofLog(OF_LOG_NOTICE, "%s::%s - %dx%d@%.2f interlace:%d deinterlace:%d anaglyph:%d par:%.2f display:%d layer:%d alpha:%d aspectMode:%d", CLASSNAME, __func__,
          port_image.format.video.nFrameWidth, port_image.format.video.nFrameHeight,
          port_image.format.video.xFramerate / (float)(1<<16), interlaceEMode, m_deinterlace, m_config.anaglyph, m_pixel_aspect, m_config.display,
          m_config.layer, m_config.alpha, m_config.aspectMode);
    
}

bool COMXVideo::PortSettingsChanged()
{
    
    CSingleLock lock (m_critSection);
    OMX_ERRORTYPE error   = OMX_ErrorNone;
    if (m_settings_changed)
    {
        m_omx_decoder.DisablePort(VIDEO_DECODE_OUTPUT_PORT, true);
    }
    
    OMX_PARAM_PORTDEFINITIONTYPE port_image;
    OMX_INIT_STRUCTURE(port_image);
    port_image.nPortIndex = VIDEO_DECODE_OUTPUT_PORT;
    error = m_omx_decoder.GetParameter(OMX_IndexParamPortDefinition, &port_image);
    OMX_TRACE(error);

    
    OMX_CONFIG_POINTTYPE pixel_aspect;
    OMX_INIT_STRUCTURE(pixel_aspect);
    pixel_aspect.nPortIndex = VIDEO_DECODE_OUTPUT_PORT;
    error = m_omx_decoder.GetParameter(OMX_IndexParamBrcmPixelAspectRatio, &pixel_aspect);
    OMX_TRACE(error);

    
    if (pixel_aspect.nX && pixel_aspect.nY && !m_config.hints.forced_aspect)
    {
        float fAspect = (float)pixel_aspect.nX / (float)pixel_aspect.nY;
        m_pixel_aspect = fAspect / m_config.display_aspect;
    }
    
    if (m_settings_changed)
    {
        PortSettingsChangedLogger(port_image, -1);
        SetVideoRect();
        m_omx_decoder.EnablePort(VIDEO_DECODE_OUTPUT_PORT, true);
        return true;
    }
    
    OMX_CONFIG_INTERLACETYPE interlace;
    OMX_INIT_STRUCTURE(interlace);
    interlace.nPortIndex = VIDEO_DECODE_OUTPUT_PORT;
    error = m_omx_decoder.GetConfig(OMX_IndexConfigCommonInterlace, &interlace);
    
    if(filtersEnabled)
    {
        if(m_config.deinterlace == VS_DEINTERLACEMODE_FORCE)
            m_deinterlace = true;
        else if(m_config.deinterlace == VS_DEINTERLACEMODE_OFF)
            m_deinterlace = false;
        else
            m_deinterlace = interlace.eMode != OMX_InterlaceProgressive;
    }

    
    
    
    if(useTexture)
    {
        std::string componentName = "OMX.broadcom.egl_render";
        m_omx_render.Initialize(componentName, OMX_IndexParamVideoInit);
        m_omx_render.fillBufferListener = this;
        
    }else
    {
        string componentName = "OMX.broadcom.video_render";
        m_omx_render.Initialize(componentName, OMX_IndexParamVideoInit);
    }
    
    
    
    m_omx_render.ResetEos();
    
    PortSettingsChangedLogger(port_image, interlace.eMode);
    
    m_omx_sched.Initialize("OMX.broadcom.video_scheduler", OMX_IndexParamVideoInit);
    
    if(filtersEnabled)
    {
        m_omx_image_fx.Initialize("OMX.broadcom.image_fx", OMX_IndexParamImageInit);
    }
    
    if(!useTexture)
    {
        OMX_CONFIG_DISPLAYREGIONTYPE configDisplay;
        OMX_INIT_STRUCTURE(configDisplay);
        configDisplay.nPortIndex = m_omx_render.GetInputPort();
        
        configDisplay.set = (OMX_DISPLAYSETTYPE)(OMX_DISPLAY_SET_ALPHA | OMX_DISPLAY_SET_TRANSFORM | OMX_DISPLAY_SET_LAYER | OMX_DISPLAY_SET_NUM);
        configDisplay.alpha = m_config.alpha;
        configDisplay.num = m_config.display;
        configDisplay.layer = m_config.layer;
        configDisplay.transform = m_transform;
        error = m_omx_render.SetConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
        OMX_TRACE(error);

        
        SetVideoRect(); 
        
        
        if(m_config.hdmi_clock_sync)
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
            OMX_TRACE(error);

        }
    }
    
    if(filtersEnabled)
    {
        if(m_deinterlace || m_config.anaglyph)
        {
            bool advanced_deinterlace = m_config.advanced_hd_deinterlace || port_image.format.video.nFrameWidth * port_image.format.video.nFrameHeight <= 576 * 720;
            /*
            if (m_config.anaglyph != OMX_ImageFilterAnaglyphNone || !advanced_deinterlace)
            {
                // Image_fx assumed 3 frames of context. anaglyph and simple deinterlace don't require this
                OMX_PARAM_U32TYPE extra_buffers;
                OMX_INIT_STRUCTURE(extra_buffers);
                extra_buffers.nU32 = -2;
                
                error = m_omx_image_fx.SetParameter(OMX_IndexParamBrcmExtraBuffers, &extra_buffers);
                if(error != OMX_ErrorNone)
                {
                    ofLog(OF_LOG_NOTICE, "%s::%s error OMX_IndexParamBrcmExtraBuffers error(%s)", CLASSNAME, __func__, omxErrorTypes[error].c_str());
                    return false;
                }
            }
            */
            OMX_CONFIG_IMAGEFILTERPARAMSTYPE image_filter;
            OMX_INIT_STRUCTURE(image_filter);
            
            image_filter.nPortIndex = IMAGE_FX_OUTPUT_PORT;
            if (m_config.anaglyph != OMX_ImageFilterAnaglyphNone)
            {
                image_filter.nNumParams = 1;
                image_filter.nParams[0] = m_config.anaglyph;
                image_filter.eImageFilter = OMX_ImageFilterAnaglyph;
            }
            else
            {
                image_filter.nNumParams = 4;
                image_filter.nParams[0] = 3;
                image_filter.nParams[1] = 0; // default frame interval
                image_filter.nParams[2] = 0; // half framerate
                image_filter.nParams[3] = 1; // use qpus
                if (!advanced_deinterlace)
                    image_filter.eImageFilter = OMX_ImageFilterDeInterlaceFast;
                else
                    image_filter.eImageFilter = OMX_ImageFilterDeInterlaceAdvanced;
            }
            error = m_omx_image_fx.SetConfig(OMX_IndexConfigCommonImageFilterParameters, &image_filter);
            OMX_TRACE(error);

        }else
        {
            SetFilter(m_config.filterType);
        }
    }
    
    
    if(filtersEnabled)
    {
        m_omx_tunnel_decoder.Initialize(&m_omx_decoder, VIDEO_DECODE_OUTPUT_PORT,
                                        &m_omx_image_fx, IMAGE_FX_INPUT_PORT);
        m_omx_tunnel_image_fx.Initialize(&m_omx_image_fx, IMAGE_FX_OUTPUT_PORT,
                                         &m_omx_sched, VIDEO_SCHEDULER_INPUT_PORT);
    }
    else
    {
        m_omx_tunnel_decoder.Initialize(&m_omx_decoder, VIDEO_DECODE_OUTPUT_PORT,
                                        &m_omx_sched, VIDEO_SCHEDULER_INPUT_PORT);
    }
    m_omx_tunnel_sched.Initialize(&m_omx_sched, VIDEO_SCHEDULER_OUTPUT_PORT,
                                  &m_omx_render, m_omx_render.GetInputPort());
    m_omx_tunnel_clock.Initialize(m_omx_clock, OMX_CLOCK_OUTPUT_PORT_1,
                                  &m_omx_sched, VIDEO_SCHEDULER_CLOCK_PORT);
    
    error = m_omx_tunnel_clock.Establish();
    OMX_TRACE(error);

    error = m_omx_tunnel_decoder.Establish();
    OMX_TRACE(error);

    if(filtersEnabled)
    {
        error = m_omx_tunnel_image_fx.Establish();
        OMX_TRACE(error);
    }
    
    error = m_omx_tunnel_sched.Establish();
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;
    
    if(useTexture)
    {
        error = m_omx_render.SetStateForComponent(OMX_StateIdle);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone) return false;
        
        
        m_omx_render.EnablePort(m_omx_render.GetOutputPort(), false);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone) return false;
        
        
        eglBuffer = NULL;
        error = m_omx_render.UseEGLImage(&eglBuffer, m_omx_render.GetOutputPort(), NULL, m_config.eglImage);
        OMX_TRACE(error);

        
    }
    
    error = m_omx_decoder.SetStateForComponent(OMX_StateExecuting);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;
    
    
    if(filtersEnabled)
    {
        error = m_omx_image_fx.SetStateForComponent(OMX_StateExecuting);
        OMX_TRACE(error);

    }
    
    
    error = m_omx_sched.SetStateForComponent(OMX_StateExecuting);
    OMX_TRACE(error);

    
    error = m_omx_render.SetStateForComponent(OMX_StateExecuting);
    OMX_TRACE(error);

    
    if(useTexture)
    {
        //error = m_omx_render.WaitForEvent(OMX_EventPortSettingsChanged, 0);
        error = m_omx_render.FillThisBuffer(eglBuffer);
        OMX_TRACE(error);        
    }
    
    m_settings_changed = true;
    return true;
}


void COMXVideo::processCodec(COMXStreamInfo& hints)
{
    switch (hints.codec)
    {
        case AV_CODEC_ID_H264:
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
        case AV_CODEC_ID_MPEG4:
            // (role name) video_decoder.mpeg4
            // MPEG-4, DivX 4/5 and Xvid compatible
            m_codingType = OMX_VIDEO_CodingMPEG4;
            break;
        case AV_CODEC_ID_MPEG1VIDEO:
        case AV_CODEC_ID_MPEG2VIDEO:
            // (role name) video_decoder.mpeg2
            // MPEG-2
            m_codingType = OMX_VIDEO_CodingMPEG2;
            break;
        case AV_CODEC_ID_H263:
            // (role name) video_decoder.mpeg4
            // MPEG-4, DivX 4/5 and Xvid compatible
            m_codingType = OMX_VIDEO_CodingMPEG4;
            break;
        case AV_CODEC_ID_VP6:
        case AV_CODEC_ID_VP6F:
        case AV_CODEC_ID_VP6A:
            // (role name) video_decoder.vp6
            // VP6
            m_codingType = OMX_VIDEO_CodingVP6;
            break;
        case AV_CODEC_ID_VP8:
            // (role name) video_decoder.vp8
            // VP8
            m_codingType = OMX_VIDEO_CodingVP8;
            break;
        case AV_CODEC_ID_THEORA:
            // (role name) video_decoder.theora
            // theora
            m_codingType = OMX_VIDEO_CodingTheora;
            break;
        case AV_CODEC_ID_MJPEG:
        case AV_CODEC_ID_MJPEGB:
            // (role name) video_decoder.mjpg
            // mjpg
            m_codingType = OMX_VIDEO_CodingMJPEG;
            break;
        case AV_CODEC_ID_VC1:
        case AV_CODEC_ID_WMV3:
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
    



void COMXVideo::onFillBuffer(OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_FillThisBuffer(hComponent, pBuffer);
    frameCounter++;
    //ofLog() << "onFillBuffer: " << frameCounter;
}


bool COMXVideo::Open(OMXClock *clock, const OMXVideoConfig &config)
{
    CSingleLock lock (m_critSection);
   

    Close();
    
    frameCounter = 0;
    
    bool vflip = false;
    OMX_ERRORTYPE error   = OMX_ErrorNone;
    std::string decoder_name;
    m_settings_changed = false;
    m_setStartTime = true;
    
    m_config = config;
    filtersEnabled = m_config.enableFilters;
    useTexture = config.useTexture;
    m_video_codec_name      = "";
    m_codingType            = OMX_VIDEO_CodingUnused;
    
    m_submitted_eos = false;
    m_failed_eos    = false;
    
    if(!m_config.hints.width || !m_config.hints.height)
        return false;
    
    switch (m_config.hints.codec)
    {
        case AV_CODEC_ID_H264:
        {
            switch(m_config.hints.profile)
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
            if (m_config.allow_mvc && m_codingType == OMX_VIDEO_CodingAVC)
            {
                m_codingType = OMX_VIDEO_CodingMVC;
                m_video_codec_name = "omx-mvc";
            }
            break;
        case AV_CODEC_ID_MPEG4:
            // (role name) video_decoder.mpeg4
            // MPEG-4, DivX 4/5 and Xvid compatible
            decoder_name = OMX_MPEG4_DECODER;
            m_codingType = OMX_VIDEO_CodingMPEG4;
            m_video_codec_name = "omx-mpeg4";
            break;
        case AV_CODEC_ID_MPEG1VIDEO:
        case AV_CODEC_ID_MPEG2VIDEO:
            // (role name) video_decoder.mpeg2
            // MPEG-2
            decoder_name = OMX_MPEG2V_DECODER;
            m_codingType = OMX_VIDEO_CodingMPEG2;
            m_video_codec_name = "omx-mpeg2";
            break;
        case AV_CODEC_ID_H263:
            // (role name) video_decoder.mpeg4
            // MPEG-4, DivX 4/5 and Xvid compatible
            decoder_name = OMX_MPEG4_DECODER;
            m_codingType = OMX_VIDEO_CodingMPEG4;
            m_video_codec_name = "omx-h263";
            break;
        case AV_CODEC_ID_VP6:
            // this form is encoded upside down
            vflip = true;
            // fall through
        case AV_CODEC_ID_VP6F:
        case AV_CODEC_ID_VP6A:
            // (role name) video_decoder.vp6
            // VP6
            decoder_name = OMX_VP6_DECODER;
            m_codingType = OMX_VIDEO_CodingVP6;
            m_video_codec_name = "omx-vp6";
            break;
        case AV_CODEC_ID_VP8:
            // (role name) video_decoder.vp8
            // VP8
            decoder_name = OMX_VP8_DECODER;
            m_codingType = OMX_VIDEO_CodingVP8;
            m_video_codec_name = "omx-vp8";
            break;
        case AV_CODEC_ID_THEORA:
            // (role name) video_decoder.theora
            // theora
            decoder_name = OMX_THEORA_DECODER;
            m_codingType = OMX_VIDEO_CodingTheora;
            m_video_codec_name = "omx-theora";
            break;
        case AV_CODEC_ID_MJPEG:
        case AV_CODEC_ID_MJPEGB:
            // (role name) video_decoder.mjpg
            // mjpg
            decoder_name = OMX_MJPEG_DECODER;
            m_codingType = OMX_VIDEO_CodingMJPEG;
            m_video_codec_name = "omx-mjpeg";
            break;
        case AV_CODEC_ID_VC1:
        case AV_CODEC_ID_WMV3:
            // (role name) video_decoder.vc1
            // VC-1, WMV9
            decoder_name = OMX_VC1_DECODER;
            m_codingType = OMX_VIDEO_CodingWMV;
            m_video_codec_name = "omx-vc1";
            break;    
        default:
            ofLog(OF_LOG_NOTICE, "Vcodec id unknown: %x\n", m_config.hints.codec);
            return false;
            break;
    }
    
    if(!m_omx_decoder.Initialize(decoder_name, OMX_IndexParamVideoInit))
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
    
    error = m_omx_decoder.SetStateForComponent(OMX_StateIdle);
    if (error != OMX_ErrorNone)
    {
        ofLog(OF_LOG_NOTICE, "COMXVideo::Open m_omx_decoder.SetStateForComponent\n");
        return false;
    }
    
    OMX_VIDEO_PARAM_PORTFORMATTYPE formatType;
    OMX_INIT_STRUCTURE(formatType);
    formatType.nPortIndex = VIDEO_DECODE_INPUT_PORT;
    formatType.eCompressionFormat = m_codingType;
    
    if (m_config.hints.fpsscale > 0 && m_config.hints.fpsrate > 0)
    {
        formatType.xFramerate = (long long)(1<<16)*m_config.hints.fpsrate / m_config.hints.fpsscale;
    }
    else
    {
        formatType.xFramerate = 25 * (1<<16);
    }
    
    error = m_omx_decoder.SetParameter(OMX_IndexParamVideoPortFormat, &formatType);
    if(error != OMX_ErrorNone)
        return false;
    
    OMX_PARAM_PORTDEFINITIONTYPE portParam;
    OMX_INIT_STRUCTURE(portParam);
    portParam.nPortIndex = VIDEO_DECODE_INPUT_PORT;
    
    error = m_omx_decoder.GetParameter(OMX_IndexParamPortDefinition, &portParam);
    if(error != OMX_ErrorNone)
    {
        ofLog(OF_LOG_NOTICE, "COMXVideo::Open error OMX_IndexParamPortDefinition error(%s)\n", omxErrorTypes[error].c_str());
        return false;
    }
    
    portParam.nPortIndex = VIDEO_DECODE_INPUT_PORT;
    portParam.nBufferCountActual = m_config.fifo_size ? m_config.fifo_size * 1024 * 1024 / portParam.nBufferSize : 80;
    
    portParam.format.video.nFrameWidth  = m_config.hints.width;
    portParam.format.video.nFrameHeight = m_config.hints.height;
    
    error = m_omx_decoder.SetParameter(OMX_IndexParamPortDefinition, &portParam);
    if(error != OMX_ErrorNone)
    {
        ofLog(OF_LOG_NOTICE, "COMXVideo::Open error OMX_IndexParamPortDefinition error(%s)\n", omxErrorTypes[error].c_str());
        return false;
    }
    
    // request portsettingschanged on aspect ratio change
    OMX_CONFIG_REQUESTCALLBACKTYPE notifications;
    OMX_INIT_STRUCTURE(notifications);
    notifications.nPortIndex = VIDEO_DECODE_OUTPUT_PORT;
    notifications.nIndex = OMX_IndexParamBrcmPixelAspectRatio;
    notifications.bEnable = OMX_TRUE;
    
    error = m_omx_decoder.SetParameter((OMX_INDEXTYPE)OMX_IndexConfigRequestCallback, &notifications);
    if (error != OMX_ErrorNone)
    {
        ofLog(OF_LOG_NOTICE, "COMXVideo::Open OMX_IndexConfigRequestCallback error (%s)\n", omxErrorTypes[error].c_str());
        return false;
    }
    
    OMX_PARAM_BRCMVIDEODECODEERRORCONCEALMENTTYPE concanParam;
    OMX_INIT_STRUCTURE(concanParam);
    if(1)
        concanParam.bStartWithValidFrame = OMX_TRUE;
    else
        concanParam.bStartWithValidFrame = OMX_FALSE;
    
    error = m_omx_decoder.SetParameter(OMX_IndexParamBrcmVideoDecodeErrorConcealment, &concanParam);
    if(error != OMX_ErrorNone)
    {
        ofLog(OF_LOG_NOTICE, "COMXVideo::Open error OMX_IndexParamBrcmVideoDecodeErrorConcealment error(%s)\n", omxErrorTypes[error].c_str());
        return false;
    }
    
    if(NaluFormatStartCodes(m_config.hints.codec, (uint8_t *)m_config.hints.extradata, m_config.hints.extrasize))
    {
        OMX_NALSTREAMFORMATTYPE nalStreamFormat;
        OMX_INIT_STRUCTURE(nalStreamFormat);
        nalStreamFormat.nPortIndex = VIDEO_DECODE_INPUT_PORT;
        nalStreamFormat.eNaluFormat = OMX_NaluFormatStartCodes;
        
        error = m_omx_decoder.SetParameter((OMX_INDEXTYPE)OMX_IndexParamNalStreamFormatSelect, &nalStreamFormat);
        if (error != OMX_ErrorNone)
        {
            ofLog(OF_LOG_NOTICE, "COMXVideo::Open OMX_IndexParamNalStreamFormatSelect error (%s)\n", omxErrorTypes[error].c_str());
            return false;
        }
    }else
    {
        //ofLog() << "NaluFormatStartCodes FAILED";
    }
    
    // Alloc buffers for the omx input port.
    error = m_omx_decoder.AllocInputBuffers();
    if (error != OMX_ErrorNone)
    {
        ofLog(OF_LOG_NOTICE, "COMXVideo::Open AllocOMXInputBuffers error (%s)\n", omxErrorTypes[error].c_str());
        return false;
    }
    
    if(filtersEnabled)
    {
        OMX_PARAM_U32TYPE extra_buffers;
        OMX_INIT_STRUCTURE(extra_buffers);
        extra_buffers.nU32 = 5;
        
        error = m_omx_decoder.SetParameter(OMX_IndexParamBrcmExtraBuffers, &extra_buffers);
        if (error != OMX_ErrorNone)
        {
            ofLog(OF_LOG_NOTICE, "COMXVideo::Open OMX_IndexParamBrcmExtraBuffers error (%s)\n", omxErrorTypes[error].c_str());
            return false;
        }
    }
    
    
    error = m_omx_decoder.SetStateForComponent(OMX_StateExecuting);
    if (error != OMX_ErrorNone)
    {
        ofLog(OF_LOG_NOTICE, "COMXVideo::Open error m_omx_decoder.SetStateForComponent\n");
        return false;
    }
 
    
    SendDecoderConfig();
    
    m_is_open           = true;
    m_drop_state        = false;
    m_setStartTime      = true;
    
    switch(m_config.hints.orientation)
    {
        case 90:
            m_transform = OMX_DISPLAY_ROT90;
            break;
        case 180:
            m_transform = OMX_DISPLAY_ROT180;
            break;
        case 270:
            m_transform = OMX_DISPLAY_ROT270;
            break;
        case 1:
            m_transform = OMX_DISPLAY_MIRROR_ROT0;
            break;
        case 91:
            m_transform = OMX_DISPLAY_MIRROR_ROT90;
            break;
        case 181:
            m_transform = OMX_DISPLAY_MIRROR_ROT180;
            break;
        case 271:
            m_transform = OMX_DISPLAY_MIRROR_ROT270;
            break;
        default:
            m_transform = OMX_DISPLAY_ROT0;
            break;
    }
    if (vflip)
        m_transform = OMX_DISPLAY_MIRROR_ROT180;
    
    if(m_omx_decoder.BadState())
        return false;
    
    ofLog(OF_LOG_NOTICE,
          "%s::%s - decoder_component(0x%p), input_port(0x%x), output_port(0x%x) deinterlace %d hdmiclocksync %d\n",
          CLASSNAME, __func__, m_omx_decoder.GetComponent(), VIDEO_DECODE_INPUT_PORT, VIDEO_DECODE_OUTPUT_PORT,
          m_config.deinterlace, m_config.hdmi_clock_sync);
    
    float fAspect = m_config.hints.aspect ? (float)m_config.hints.aspect / (float)m_config.hints.width * (float)m_config.hints.height : 1.0f;
    m_pixel_aspect = fAspect / m_config.display_aspect;
    
    return true;
}

void COMXVideo::Close()
{
    CSingleLock lock (m_critSection);
    m_omx_tunnel_clock.Deestablish();
    m_omx_tunnel_decoder.Deestablish();
    if(filtersEnabled)
    {
        m_omx_tunnel_image_fx.Deestablish();
    }
    m_omx_tunnel_sched.Deestablish();
    
    m_omx_decoder.FlushInput();
    
    m_omx_sched.Deinitialize();
    m_omx_decoder.Deinitialize();
    if(filtersEnabled)
    {
        m_omx_image_fx.Deinitialize();
    }
    m_omx_render.Deinitialize();
    
    m_is_open       = false;
    
    m_video_codec_name  = "";
    m_deinterlace       = false;
    m_config.anaglyph          = OMX_ImageFilterAnaglyphNone;
    m_av_clock          = NULL;
    filtersEnabled = false;
}

void COMXVideo::SetDropState(bool bDrop)
{
    m_drop_state = bDrop;
}

unsigned int COMXVideo::GetFreeSpace()
{
    CSingleLock lock (m_critSection);
    return m_omx_decoder.GetInputBufferSpace();
}

unsigned int COMXVideo::GetSize()
{
    CSingleLock lock (m_critSection);
    return m_omx_decoder.GetInputBufferSize();
}

int COMXVideo::Decode(uint8_t *pData, int iSize, double dts, double pts)
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
        OMX_U32 nFlags = 0;
        
        if(m_setStartTime)
        {
            nFlags |= OMX_BUFFERFLAG_STARTTIME;
            ofLog(OF_LOG_NOTICE, "OMXVideo::Decode VDec : setStartTime %f\n", (pts == DVD_NOPTS_VALUE ? 0.0 : pts) / DVD_TIME_BASE);
            m_setStartTime = false;
        }
        if (pts == DVD_NOPTS_VALUE && dts == DVD_NOPTS_VALUE)
            nFlags |= OMX_BUFFERFLAG_TIME_UNKNOWN;
        else if (pts == DVD_NOPTS_VALUE)
            nFlags |= OMX_BUFFERFLAG_TIME_IS_DTS;
        
        while(demuxer_bytes)
        {
            // 500ms timeout
            OMX_BUFFERHEADERTYPE *omx_buffer = m_omx_decoder.GetInputBuffer(500);
            if(omx_buffer == NULL)
            {
                ofLog(OF_LOG_NOTICE, "OMXVideo::Decode timeout\n");
                return false;
            }
            
            omx_buffer->nFlags = nFlags;
            omx_buffer->nOffset = 0;
  
            
            omx_buffer->nTimeStamp = ToOMXTime((uint64_t)(pts != DVD_NOPTS_VALUE ? pts : dts != DVD_NOPTS_VALUE ? dts : 0));
            omx_buffer->nFilledLen = std::min((OMX_U32)demuxer_bytes, omx_buffer->nAllocLen);
            memcpy(omx_buffer->pBuffer, demuxer_content, omx_buffer->nFilledLen);
            
            demuxer_bytes -= omx_buffer->nFilledLen;
            demuxer_content += omx_buffer->nFilledLen;
            
            if(demuxer_bytes == 0)
                omx_buffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
            
            error = m_omx_decoder.EmptyThisBuffer(omx_buffer);
            if (error != OMX_ErrorNone)
            {
                ofLog(OF_LOG_NOTICE, "%s::%s - OMX_EmptyThisBuffer() failed with result(%s)\n", CLASSNAME, __func__, omxErrorTypes[error].c_str());
                m_omx_decoder.DecoderEmptyBufferDone(m_omx_decoder.GetComponent(), omx_buffer);
                return false;
            }
            //ofLog(OF_LOG_NOTICE, "VideD: dts:%.0f pts:%.0f size:%d)\n", dts, pts, iSize);
            
            error = m_omx_decoder.WaitForEvent(OMX_EventPortSettingsChanged, 0);
            if (error == OMX_ErrorNone)
            {
                if(!PortSettingsChanged())
                {
                    ofLog(OF_LOG_NOTICE, "%s::%s - error PortSettingsChanged error(%s)\n", CLASSNAME, __func__, omxErrorTypes[error].c_str());
                    return false;
                }
            }
            error = m_omx_decoder.WaitForEvent(OMX_EventParamOrConfigChanged, 0);
            if (error == OMX_ErrorNone)
            {
                if(!PortSettingsChanged())
                {
                    ofLog(OF_LOG_NOTICE, "%s::%s - error PortSettingsChanged (EventParamOrConfigChanged) error(%s)\n", CLASSNAME, __func__, omxErrorTypes[error].c_str());
                }
            }
        }
        return true;
    }
    
    return false;
}

void COMXVideo::Reset(void)
{
    CSingleLock lock (m_critSection);
    if(!m_is_open)
        return;
    
    m_setStartTime      = true;
    m_omx_decoder.FlushInput();
    if(filtersEnabled)
    {
        m_omx_image_fx.FlushInput();
    }
    m_omx_render.ResetEos();
}

///////////////////////////////////////////////////////////////////////////////////////////
void COMXVideo::SetVideoRect(const CRect& SrcRect, const CRect& DestRect)
{
    m_config.src_rect = SrcRect;
    m_config.dst_rect = DestRect;
    SetVideoRect();
}

void COMXVideo::SetVideoRect(int aspectMode)
{
    m_config.aspectMode = aspectMode;
    SetVideoRect();
}

void COMXVideo::SetVideoRect()
{
    CSingleLock lock (m_critSection);
    if(!m_is_open)
    {
        return;
    }
    
    OMX_ERRORTYPE error;
    OMX_CONFIG_DISPLAYREGIONTYPE configDisplay;
    OMX_INIT_STRUCTURE(configDisplay);
    configDisplay.nPortIndex = m_omx_render.GetInputPort();
    
    configDisplay.set        = (OMX_DISPLAYSETTYPE)(OMX_DISPLAY_SET_NOASPECT | OMX_DISPLAY_SET_MODE | OMX_DISPLAY_SET_SRC_RECT | OMX_DISPLAY_SET_FULLSCREEN | OMX_DISPLAY_SET_PIXEL);
    configDisplay.noaspect   = m_config.aspectMode == 3 ? OMX_TRUE : OMX_FALSE;
    configDisplay.mode       = m_config.aspectMode == 2 ? OMX_DISPLAY_MODE_FILL : OMX_DISPLAY_MODE_LETTERBOX;
    
    configDisplay.src_rect.x_offset   = (int)(m_config.src_rect.x1+0.5f);
    configDisplay.src_rect.y_offset   = (int)(m_config.src_rect.y1+0.5f);
    configDisplay.src_rect.width      = (int)(m_config.src_rect.Width()+0.5f);
    configDisplay.src_rect.height     = (int)(m_config.src_rect.Height()+0.5f);
    
    if (m_config.dst_rect.x2 > m_config.dst_rect.x1 && m_config.dst_rect.y2 > m_config.dst_rect.y1) {
        configDisplay.set        = (OMX_DISPLAYSETTYPE)(configDisplay.set | OMX_DISPLAY_SET_DEST_RECT);
        configDisplay.fullscreen = OMX_FALSE;
        
        if (m_config.aspectMode != 1 && m_config.aspectMode != 2 && m_config.aspectMode != 3) {
            configDisplay.noaspect = OMX_TRUE;
        }
        
        configDisplay.dest_rect.x_offset  = (int)(m_config.dst_rect.x1+0.5f);
        configDisplay.dest_rect.y_offset  = (int)(m_config.dst_rect.y1+0.5f);
        configDisplay.dest_rect.width     = (int)(m_config.dst_rect.Width()+0.5f);
        configDisplay.dest_rect.height    = (int)(m_config.dst_rect.Height()+0.5f);
    } else {
        configDisplay.fullscreen = OMX_TRUE;
    }
    
    if (configDisplay.noaspect == OMX_FALSE && m_pixel_aspect != 0.0f) {
        AVRational aspect = av_d2q(m_pixel_aspect, 100);
        configDisplay.pixel_x = aspect.num;
        configDisplay.pixel_y = aspect.den;
    } else {
        configDisplay.pixel_x = 0;
        configDisplay.pixel_y = 0;
    }
    
    error = m_omx_render.SetConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
    if (error != OMX_ErrorNone) {
        ofLog(OF_LOG_NOTICE, "COMXVideo::Open error OMX_IndexConfigDisplayRegion error(%s)\n", omxErrorTypes[error].c_str());
    }
}

void COMXVideo::SetLayer(int layer)
{
    CSingleLock lock (m_critSection);
    if(!m_is_open)
        return;
    
    OMX_ERRORTYPE error;
    OMX_CONFIG_DISPLAYREGIONTYPE configDisplay;
    OMX_INIT_STRUCTURE(configDisplay);
    
    configDisplay.nPortIndex = m_omx_render.GetInputPort();
    configDisplay.set = OMX_DISPLAY_SET_LAYER;
    configDisplay.layer = layer;
    
    error = m_omx_render.SetConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
    if(error != OMX_ErrorNone)
    {
        ofLog(OF_LOG_NOTICE, "COMXVideo::LAYER::Open error OMX_IndexConfigDisplayRegion error(%s)\n", omxErrorTypes[error].c_str());
    }
    
}

void COMXVideo::SetOrientation(int degreesClockWise, bool doMirror)
{
    CSingleLock lock (m_critSection);
    if(!m_is_open)
    {
        return;
    }
    if(useTexture)
    {
        return;
    }

    if(degreesClockWise<0)
    {
        m_transform = OMX_DISPLAY_ROT0;
    }
    if(degreesClockWise >=90 && degreesClockWise < 180)
    {
        m_transform = OMX_DISPLAY_ROT90;
    }
    if(degreesClockWise >=180 && degreesClockWise < 270)
    {
        m_transform = OMX_DISPLAY_ROT270;
    }
    
    if(doMirror)
    {
        switch (m_transform) 
        {
            case OMX_DISPLAY_ROT0:
            {
                m_transform = OMX_DISPLAY_MIRROR_ROT0;
                break;
            }
            case OMX_DISPLAY_ROT90:
            {
                m_transform = OMX_DISPLAY_MIRROR_ROT90;
                break;
            }
            case OMX_DISPLAY_ROT270:
            {
                m_transform = OMX_DISPLAY_MIRROR_ROT270;
                break;
            }
                
            default:
                break;
        }
    }
    
    
    OMX_CONFIG_DISPLAYREGIONTYPE configDisplay;
    OMX_INIT_STRUCTURE(configDisplay);
    configDisplay.nPortIndex = m_omx_render.GetInputPort();
    
    configDisplay.set = OMX_DISPLAY_SET_TRANSFORM;
    configDisplay.transform = m_transform;
    OMX_ERRORTYPE error = m_omx_render.SetConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
    if(error != OMX_ErrorNone)
    {
        ofLog(OF_LOG_NOTICE, "%s::%s - could not set transform : %d", CLASSNAME, __func__, m_transform);
        return;
    }
    
}

void COMXVideo::SetAlpha(int alpha)
{
    CSingleLock lock (m_critSection);
    if(!m_is_open)
        return;
    
    OMX_ERRORTYPE error;
    OMX_CONFIG_DISPLAYREGIONTYPE configDisplay;
    OMX_INIT_STRUCTURE(configDisplay);
    
    configDisplay.nPortIndex = m_omx_render.GetInputPort();
    configDisplay.set = OMX_DISPLAY_SET_ALPHA;
    configDisplay.alpha = alpha;
    
    error = m_omx_render.SetConfig(OMX_IndexConfigDisplayRegion, &configDisplay);
    if(error != OMX_ErrorNone)
    {
        ofLog(OF_LOG_NOTICE, "COMXVideo::ALPHA::Open error OMX_IndexConfigDisplayRegion error(%s)\n", omxErrorTypes[error].c_str());
    }
    
}


void COMXVideo::SetFilter(OMX_IMAGEFILTERTYPE filterType)
{
    CSingleLock lock (m_critSection);
    if(!m_is_open) return;
    if(!filtersEnabled) return;
    
    OMX_ERRORTYPE error = OMX_ErrorNone;

    //m_omx_decoder.FlushInput();
    m_omx_image_fx.FlushInput();

    OMX_CONFIG_IMAGEFILTERPARAMSTYPE image_filter;
    OMX_INIT_STRUCTURE(image_filter);
    
    image_filter.nPortIndex = IMAGE_FX_OUTPUT_PORT;
    image_filter.eImageFilter = filterType;
    error = m_omx_image_fx.SetConfig(OMX_IndexConfigCommonImageFilterParameters, &image_filter);
    if(error != OMX_ErrorNone)
    {
        ofLog(OF_LOG_NOTICE, "%s::%s - OMX_IndexConfigCommonImageFilterParameters error(%s)", CLASSNAME, __func__, omxErrorTypes[error].c_str());
        return;
    }
}

int COMXVideo::GetInputBufferSize()
{
    CSingleLock lock (m_critSection);
    return m_omx_decoder.GetInputBufferSize();
}

void COMXVideo::SubmitEOS()
{
    CSingleLock lock (m_critSection);
    if(!m_is_open)
        return;
    
    m_submitted_eos = true;
    m_failed_eos = false;
    
    OMX_ERRORTYPE error = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE *omx_buffer = m_omx_decoder.GetInputBuffer(1000);
    
    if(omx_buffer == NULL)
    {
        ofLog(OF_LOG_NOTICE, "%s::%s - buffer error %s", CLASSNAME, __func__, omxErrorTypes[error].c_str());
        m_failed_eos = true;
        return;
    }
    
    omx_buffer->nOffset     = 0;
    omx_buffer->nFilledLen  = 0;
    omx_buffer->nTimeStamp  = ToOMXTime(0LL);
    
    omx_buffer->nFlags = OMX_BUFFERFLAG_ENDOFFRAME | OMX_BUFFERFLAG_EOS | OMX_BUFFERFLAG_TIME_UNKNOWN;
    
    error = m_omx_decoder.EmptyThisBuffer(omx_buffer);
    if (error != OMX_ErrorNone)
    {
        ofLog(OF_LOG_NOTICE, "%s::%s - OMX_EmptyThisBuffer() failed with result(%s)\n", CLASSNAME, __func__, omxErrorTypes[error].c_str());
        m_omx_decoder.DecoderEmptyBufferDone(m_omx_decoder.GetComponent(), omx_buffer);
        return;
    }
    
    ofLog(OF_LOG_NOTICE, "%s::%s", CLASSNAME, __func__);
}

bool COMXVideo::IsEOS()
{
    CSingleLock lock (m_critSection);
    if(!m_is_open)
        return true;
    if (!m_failed_eos && !m_omx_render.IsEOS())
        return false;
    if (m_submitted_eos)
    {
        ofLog(OF_LOG_NOTICE, "%s::%s", CLASSNAME, __func__);
        m_submitted_eos = false;
    }
    return true;
}



