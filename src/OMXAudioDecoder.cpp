/*
* XBMC Media Center
* Copyright (c) 2002 d7o3g4q and RUNTiME
* Portions Copyright (c) by the authors of ffmpeg and xvid
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/



#include "OMXAudioDecoder.h"
#include "XMemUtils.h"


#ifndef VOLUME_MINIMUM
#define VOLUME_MINIMUM -6000  // -60dB
#endif

#include <algorithm>

static GUID KSDATAFORMAT_SUBTYPE_PCM = {
    WAVE_FORMAT_PCM,
    0x0000, 0x0010,
    {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
};

#define OMX_MAX_CHANNELS 9

static enum PCMChannels OMXChannelMap[OMX_MAX_CHANNELS] =
{
    PCM_FRONT_LEFT  , PCM_FRONT_RIGHT,
    PCM_FRONT_CENTER, PCM_LOW_FREQUENCY,
    PCM_BACK_LEFT   , PCM_BACK_RIGHT,
    PCM_SIDE_LEFT   , PCM_SIDE_RIGHT,
    PCM_BACK_CENTER
};

static enum OMX_AUDIO_CHANNELTYPE OMXChannels[OMX_MAX_CHANNELS] =
{
    OMX_AUDIO_ChannelLF, OMX_AUDIO_ChannelRF,
    OMX_AUDIO_ChannelCF, OMX_AUDIO_ChannelLFE,
    OMX_AUDIO_ChannelLR, OMX_AUDIO_ChannelRR,
    OMX_AUDIO_ChannelLS, OMX_AUDIO_ChannelRS,
    OMX_AUDIO_ChannelCS
};

static unsigned int WAVEChannels[OMX_MAX_CHANNELS] =
{
    SPEAKER_FRONT_LEFT,       SPEAKER_FRONT_RIGHT,
    SPEAKER_TOP_FRONT_CENTER, SPEAKER_LOW_FREQUENCY,
    SPEAKER_BACK_LEFT,        SPEAKER_BACK_RIGHT,
    SPEAKER_SIDE_LEFT,        SPEAKER_SIDE_RIGHT,
    SPEAKER_BACK_CENTER
};


// Dolby 5.1 downmixing coefficients
float downmixing_coefficients_6[16] =
{
    //        L       R
    /* L */   1,      0,
    /* R */   0,      1,
    /* C */   0.7071, 0.7071,
    /* LFE */ 0.7071, 0.7071,
    /* Ls */  0.7071, 0,
    /* Rs */  0,      0.7071,
    /* Lr */  0,      0,
    /* Rr */  0,      0
};

// 7.1 downmixing coefficients
float downmixing_coefficients_8[16] =
{
    //        L       R
    /* L */   1,      0,
    /* R */   0,      1,
    /* C */   0.7071, 0.7071,
    /* LFE */ 0.7071, 0.7071,
    /* Ls */  0.7071, 0,
    /* Rs */  0,      0.7071,
    /* Lr */  0.7071, 0,
    /* Rr */  0,      0.7071
};


OMXAudioDecoder::OMXAudioDecoder()
{
    isInitialized = false;
    doPause = false;
    canPause = false;
    currentVolume = 0;
    doNormalizeDownmix= true;
    bytesPerSecond = 0;
    bufferLength = 0;
    chunkLength = 0;
    numInputChannels = 0;
    numOutputChannels = 0;
    numDownmixChannels= 0;
    m_BitsPerSample = 0;
    clockComponent = NULL;
    omxClock = NULL;
    doSetStartTime = false;
    sampleSize = 0;
    isFirstFrame = true;
    sampleRate = 0;
    m_eEncoding = OMX_AUDIO_CodingPCM;
    extraData = NULL;
    extraSize = 0;
}

OMXAudioDecoder::~OMXAudioDecoder()
{
    if(isInitialized)
    {
        deinit();
    }
}


bool OMXAudioDecoder::init(string device, 
                    enum PCMChannels *channelMap,
                    StreamInfo& hints, 
                    OMXClock *clock,
                    bool boostOnDownmix)
{

    setCodingType(AV_CODEC_ID_PCM_S16LE);
    if(hints.extrasize > 0 && hints.extradata != NULL)
    {
        extraSize = hints.extrasize;
        extraData = (uint8_t *)malloc(extraSize);
        memcpy(extraData, hints.extradata, hints.extrasize);
    }

    
    int iChannels = hints.channels;
    unsigned int downmixChannels = hints.channels;
    unsigned int uiSamplesPerSec = hints.samplerate;
    unsigned int uiBitsPerSample = hints.bitspersample;

    std::string deviceuse;
    if(device == "hdmi")
    {
        deviceuse = "hdmi";
    }
    else
    {
        deviceuse = "local";
    }


    memset(&waveFormat, 0x0, sizeof(waveFormat));

    currentVolume = 0;

    numDownmixChannels = downmixChannels;
    doNormalizeDownmix = !boostOnDownmix;

    numInputChannels = iChannels;
    remapObject.Reset();

    OMX_INIT_STRUCTURE(pcm_output);
    numOutputChannels = 2;
    pcm_output.nChannels = numOutputChannels;
    pcm_output.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
    pcm_output.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
    pcm_output.eChannelMapping[2] = OMX_AUDIO_ChannelMax;

    OMX_INIT_STRUCTURE(pcm_input);
    pcm_input.nChannels = numOutputChannels;
    pcm_input.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
    pcm_input.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
    pcm_input.eChannelMapping[2] = OMX_AUDIO_ChannelMax;

    waveFormat.Format.nChannels  = numOutputChannels;
    waveFormat.dwChannelMask     = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;

    // set the input format, and get the channel layout so we know what we need to open
    enum PCMChannels *outLayout = remapObject.SetInputFormat (iChannels, channelMap, uiBitsPerSample / 8, uiSamplesPerSec);;

    if (channelMap && outLayout)
    {
        /* setup output channel map */
        numOutputChannels = 0;
        int ch = 0, map;
        int chan = 0;
        while(outLayout[ch] != PCM_INVALID && chan < OMX_AUDIO_MAXCHANNELS)
        {
            for(map = 0; map < OMX_MAX_CHANNELS; ++map)
            {
                if (outLayout[ch] == OMXChannelMap[map])
                {
                    pcm_output.eChannelMapping[chan] = OMXChannels[map];
                    chan++;
                    break;
                }
            }
            ++ch;
        }

        numOutputChannels = chan;

        /* setup input channel map */
        for (chan=0; chan < OMX_AUDIO_MAXCHANNELS; chan++)
        {
            pcm_input.eChannelMapping[chan] = OMX_AUDIO_ChannelNone;
        }

        ch = 0;
        map = 0;
        chan = 0;

        while(channelMap[ch] != PCM_INVALID && chan < iChannels)
        {
            for(map = 0; map < OMX_MAX_CHANNELS; ++map)
            {
                if (channelMap[ch] == OMXChannelMap[map])
                {
                    pcm_input.eChannelMapping[chan] = OMXChannels[map];
                    waveFormat.dwChannelMask |= WAVEChannels[map];
                    chan++;
                    break;
                }
            }
            ++ch;
        }
    }

    // set the pcm_output parameters
    pcm_output.eNumData            = OMX_NumericalDataSigned;
    pcm_output.eEndian             = OMX_EndianLittle;
    pcm_output.bInterleaved        = OMX_TRUE;
    pcm_output.nBitPerSample       = uiBitsPerSample;
    pcm_output.ePCMMode            = OMX_AUDIO_PCMModeLinear;
    pcm_output.nChannels           = numOutputChannels;
    pcm_output.nSamplingRate       = uiSamplesPerSec;

    sampleRate    = uiSamplesPerSec;
    m_BitsPerSample = uiBitsPerSample;
    bufferLength     = bytesPerSecond = uiSamplesPerSec * (uiBitsPerSample >> 3) * numInputChannels;
    bufferLength     *= AUDIO_BUFFER_SECONDS;
    chunkLength      = 6144;
    //chunkLength      = 2048;

    waveFormat.Samples.wValidBitsPerSample = uiBitsPerSample;
    waveFormat.Samples.wSamplesPerBlock    = 0;
    waveFormat.Format.nChannels            = numInputChannels;
    waveFormat.Format.nBlockAlign          = numInputChannels * (uiBitsPerSample >> 3);
    waveFormat.Format.wFormatTag           = WAVE_FORMAT_PCM;
    waveFormat.Format.nSamplesPerSec       = uiSamplesPerSec;
    waveFormat.Format.nAvgBytesPerSec      = bytesPerSecond;
    waveFormat.Format.wBitsPerSample       = uiBitsPerSample;
    waveFormat.Samples.wValidBitsPerSample = uiBitsPerSample;
    waveFormat.Format.cbSize               = 0;
    waveFormat.SubFormat                   = KSDATAFORMAT_SUBTYPE_PCM;

    //sampleSize              = (pcm_output.nChannels * pcm_output.nBitPerSample * pcm_output.nSamplingRate)>>3;
    sampleSize              = (pcm_input.nChannels * pcm_output.nBitPerSample * pcm_output.nSamplingRate)>>3;

    pcm_input.eNumData              = OMX_NumericalDataSigned;
    pcm_input.eEndian               = OMX_EndianLittle;
    pcm_input.bInterleaved          = OMX_TRUE;
    pcm_input.nBitPerSample         = uiBitsPerSample;
    pcm_input.ePCMMode              = OMX_AUDIO_PCMModeLinear;
    pcm_input.nChannels             = numInputChannels;
    pcm_input.nSamplingRate         = uiSamplesPerSec;
#ifdef DEBUG_AUDIO
    printPCM(&pcm_input);
    printPCM(&pcm_output);
#endif
    OMX_ERRORTYPE error = OMX_ErrorNone;
    std::string componentName = "";

    componentName = "OMX.broadcom.audio_render";
    if(!renderComponent.init(componentName, OMX_IndexParamAudioInit))
    {
        ofLogError(__func__) << "renderComponent: FAIL";
         return false;
    }

    OMX_CONFIG_BOOLEANTYPE configBool;
    OMX_INIT_STRUCTURE(configBool);
    configBool.bEnabled = OMX_FALSE;

    error = renderComponent.setConfig(OMX_IndexConfigBrcmClockReferenceSource, &configBool);
    OMX_TRACE(error);
    if (error != OMX_ErrorNone)
    {
         return false;
    }

    renderComponent.resetEOS();

    OMX_CONFIG_BRCMAUDIODESTINATIONTYPE audioDest;
    OMX_INIT_STRUCTURE(audioDest);
    strncpy((char *)audioDest.sName, device.c_str(), strlen(device.c_str()));

    error = renderComponent.setConfig(OMX_IndexConfigBrcmAudioDestination, &audioDest);
    OMX_TRACE(error);
    if (error != OMX_ErrorNone)
    {
         return false;
    }

    componentName = "OMX.broadcom.audio_decode";
    if(!decoderComponent.init(componentName, OMX_IndexParamAudioInit))
    {
        ofLogError(__func__) << "decoderComponent: FAIL";
         return false;
    }

    componentName = "OMX.broadcom.audio_mixer";
    if(!mixerComponent.init(componentName, OMX_IndexParamAudioInit))
    {
        ofLogError(__func__) << "mixerComponent: FAIL";
        return false;
    }

    

    // set up the number/size of buffers
    OMX_PARAM_PORTDEFINITIONTYPE port_param;
    OMX_INIT_STRUCTURE(port_param);
    port_param.nPortIndex = decoderComponent.getInputPort();

    error = decoderComponent.getParameter(OMX_IndexParamPortDefinition, &port_param);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
         return false;
    }

    port_param.format.audio.eEncoding = m_eEncoding;

    port_param.nBufferSize = chunkLength;
    port_param.nBufferCountActual = bufferLength / chunkLength;

    error = decoderComponent.setParameter(OMX_IndexParamPortDefinition, &port_param);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
         return false;
    }
    
    clockComponent = omxClock->getComponent();

    clockTunnel.init(clockComponent, clockComponent->getInputPort(), &renderComponent, renderComponent.getInputPort()+1);

    error = clockTunnel.Establish(false);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
         return false;
    }

    //TODO: sketchy
    error = clockComponent->setState(OMX_StateExecuting);
    OMX_TRACE(error);
    if (error != OMX_ErrorNone)
    {
        return false;
    }


    error = decoderComponent.allocInputBuffers();
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
         return false;
    }

    decoderTunnel.init(&decoderComponent, decoderComponent.getOutputPort(), &mixerComponent, mixerComponent.getInputPort());
    error = decoderTunnel.Establish(false);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
        return false;
    }
    
    error = decoderComponent.setState(OMX_StateExecuting);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
        return false;
    }
    
    mixerTunnel.init(&mixerComponent, mixerComponent.getOutputPort(), &renderComponent, renderComponent.getInputPort());
    error = mixerTunnel.Establish(false);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
        return false;
    }
    
    error = mixerComponent.setState(OMX_StateExecuting);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
        return false;
    }

    error = renderComponent.setState(OMX_StateExecuting);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
         return false;
    }

    if(m_eEncoding == OMX_AUDIO_CodingPCM)
    {
        OMX_BUFFERHEADERTYPE *omxBuffer = decoderComponent.getInputBuffer();
        if(omxBuffer == NULL)
        {
            ofLog(OF_LOG_ERROR, "OMXAudioDecoder::init - buffer error");
             return false;
        }

        omxBuffer->nOffset = 0;
        omxBuffer->nFilledLen = sizeof(waveFormat);
        if(omxBuffer->nFilledLen > omxBuffer->nAllocLen)
        {
            ofLog(OF_LOG_ERROR, "OMXAudioDecoder::init - omxBuffer->nFilledLen > omxBuffer->nAllocLen");
             return false;
        }
        memset((unsigned char *)omxBuffer->pBuffer, 0x0, omxBuffer->nAllocLen);
        memcpy((unsigned char *)omxBuffer->pBuffer, &waveFormat, omxBuffer->nFilledLen);
        omxBuffer->nFlags = OMX_BUFFERFLAG_CODECCONFIG | OMX_BUFFERFLAG_ENDOFFRAME;

        error = decoderComponent.EmptyThisBuffer(omxBuffer);
        OMX_TRACE(error);
        if (error != OMX_ErrorNone)
        {
             return false;
        }
    }


    isInitialized   = true;
    doSetStartTime  = true;
    isFirstFrame   = true;

    setCurrentVolume(currentVolume);

    ofLog(OF_LOG_VERBOSE, "OMXAudioDecoder::init Ouptut bps %d samplerate %d channels %d device %s buffer size %d bytes per second %d",
          (int)pcm_output.nBitPerSample, (int)pcm_output.nSamplingRate, (int)pcm_output.nChannels, deviceuse.c_str(), bufferLength, bytesPerSecond);

    return true;
}

//***********************************************************************************************
bool OMXAudioDecoder::deinit()
{
    if(!isInitialized)
    {
        return true;
    }

    //TODO stop here?
    if(omxClock)
    {
        //omxClock->stop();
    }
    
    
    OMX_ERRORTYPE error = OMX_ErrorNone;
    

    error = clockTunnel.Deestablish();
    OMX_TRACE(error);
    error = mixerTunnel.Deestablish();
    OMX_TRACE(error);
    error = decoderTunnel.Deestablish();
    OMX_TRACE(error);

#if 0
    bool didDeinit = false;

    didDeinit = renderComponent.Deinitialize(__func__);
    if(!didDeinit) ofLogError(__func__) << "didDeinit failed on renderComponent";
    
    didDeinit = mixerComponent.Deinitialize(__func__);
     if(!didDeinit) ofLogError(__func__) << "didDeinit failed on mixerComponent";
    
    didDeinit = decoderComponent.Deinitialize(__func__);
    if(!didDeinit) ofLogError(__func__) << "didDeinit failed on decoderComponent";
#endif

    isInitialized = false;
    bytesPerSecond = 0;
    bufferLength   = 0;

    
    if(omxClock != NULL)
    {
        
        //delete omxClock;
        omxClock  = NULL;
    }
    
    clockComponent = NULL;
    omxClock  = NULL;

    isInitialized = false;

    if(extraData)
    {
        free(extraData);
    }
    extraData = NULL;
    extraSize = 0;


    doSetStartTime  = true;
    isFirstFrame   = true;

    return true;
}

void OMXAudioDecoder::flush()
{
    if(!isInitialized)
    {
        return;
    }

    decoderComponent.flushInput();
    decoderTunnel.flush();
    mixerTunnel.flush();

    //doSetStartTime  = true;
    //isFirstFrame   = true;
}

//***********************************************************************************************
bool OMXAudioDecoder::pause()
{
    if (!isInitialized)
    {
        return -1;
    }

    if(doPause)
    {
        return true;
    }
    doPause = true;

    decoderComponent.setState(OMX_StatePause);

    return true;
}

//***********************************************************************************************
bool OMXAudioDecoder::resume()
{
    if (!isInitialized)
    {
        return -1;
    }

    if(!doPause)
    {
        return true;
    }
    doPause = false;

    decoderComponent.setState(OMX_StateExecuting);

    return true;
}

//***********************************************************************************************
bool OMXAudioDecoder::Stop()
{
    if (!isInitialized)
    {
        return -1;
    }

    flush();

    doPause = false;

    return true;
}

//***********************************************************************************************
long OMXAudioDecoder::getCurrentVolume() const
{
    return currentVolume;
}

//***********************************************************************************************
void OMXAudioDecoder::mute(bool bMute)
{
    if(!isInitialized)
    {
        return;
    }

    if (bMute)
    {
        setCurrentVolume(VOLUME_MINIMUM);
    }
    else
    {
        setCurrentVolume(currentVolume);
    }
}

//***********************************************************************************************
bool OMXAudioDecoder::setCurrentVolume(long nVolume)
{
    if(!isInitialized)
    {
         return false;
    }

    currentVolume = nVolume;

    if((numDownmixChannels == 6 || numDownmixChannels == 8) &&
            numOutputChannels == 2)
    {
        // Convert from millibels to amplitude ratio
        double r = pow(10, nVolume / 2000.0); 

        float* coeff = NULL;

        switch(numDownmixChannels)
        {
            case 6:
                coeff = downmixing_coefficients_6;
                break;
            case 8:
                coeff = downmixing_coefficients_8;
                break;
            default:
                assert(0);
        }

        if(doNormalizeDownmix)
        {
            double sum_L = 0;
            double sum_R = 0;

            for(size_t i = 0; i < 16; ++i)
            {
                if(i & 1)
                {
                    sum_R += coeff[i];
                }
                else
                {
                    sum_L += coeff[i];
                }
            }

            r /= max(sum_L, sum_R);
        }

        OMX_CONFIG_BRCMAUDIODOWNMIXCOEFFICIENTS mix;
        OMX_INIT_STRUCTURE(mix);
        mix.nPortIndex = mixerComponent.getInputPort();

        if(sizeof(mix.coeff)/sizeof(mix.coeff[0]) == 16)
        {
            ofLogError() << "Unexpected OMX_CONFIG_BRCMAUDIODOWNMIXCOEFFICIENTS::coeff length";
        }


        for(size_t i = 0; i < 16; ++i)
        {
            mix.coeff[i] = static_cast<unsigned int>(0x10000 * (coeff[i] * r));
        }

        OMX_ERRORTYPE error = mixerComponent.setConfig(OMX_IndexConfigBrcmAudioDownmixCoefficients, &mix);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone)
        {
             return false;
        }
    }
    else
    {

        OMX_AUDIO_CONFIG_VOLUMETYPE volume;
        OMX_INIT_STRUCTURE(volume);
        volume.nPortIndex = renderComponent.getInputPort();

        volume.sVolume.nValue = nVolume;

        OMX_ERRORTYPE error = renderComponent.setConfig(OMX_IndexConfigAudioVolume, &volume);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone)
        {
             return false;
        }
    }

    return true;
}


//***********************************************************************************************
#if 1
unsigned int OMXAudioDecoder::GetSpace()
{
    int free = decoderComponent.getInputBufferSpace();
    return free;
}
#endif
unsigned int OMXAudioDecoder::addPackets(void* data, unsigned int len)
{
    return addPackets(data, len, 0, 0);
}

//***********************************************************************************************
unsigned int OMXAudioDecoder::addPackets(void* data, unsigned int len, double dts, double pts)
{
    if(!isInitialized)
    {
        ofLog(OF_LOG_ERROR,"OMXAudioDecoder::addPackets - sanity failed. no valid play handle!");
        return len;
    }

    unsigned int demuxer_bytes = (unsigned int)len;
    uint8_t *demuxer_content = (uint8_t *)data;

    OMX_ERRORTYPE error;

    OMX_BUFFERHEADERTYPE *omxBuffer = NULL;

    while(demuxer_bytes)
    {
        // 200ms timeout
        omxBuffer = decoderComponent.getInputBuffer();

        if(omxBuffer == NULL)
        {
            ofLog(OF_LOG_ERROR, "OMXAudioDecoder::Decode timeout");
            return len;
        }

        omxBuffer->nOffset = 0;
        omxBuffer->nFlags  = 0;

        omxBuffer->nFilledLen = (demuxer_bytes > omxBuffer->nAllocLen) ? omxBuffer->nAllocLen : demuxer_bytes;
        memcpy(omxBuffer->pBuffer, demuxer_content, omxBuffer->nFilledLen);


        uint64_t val  = (uint64_t)(pts == DVD_NOPTS_VALUE) ? 0 : pts;

        if(doSetStartTime)
        {
            omxBuffer->nFlags = OMX_BUFFERFLAG_STARTTIME;

            doSetStartTime = false;
        }
        else if(pts == DVD_NOPTS_VALUE)
        {
            omxBuffer->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;
        }

        omxBuffer->nTimeStamp = ToOMXTime(val);

        demuxer_bytes -= omxBuffer->nFilledLen;
        demuxer_content += omxBuffer->nFilledLen;

        if(demuxer_bytes == 0)
        {
            omxBuffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
        }

        int nRetry = 0;

        while(true)
        {
            error = decoderComponent.EmptyThisBuffer(omxBuffer);
            OMX_TRACE(error);
            if (error == OMX_ErrorNone)
            {
                break;
            }
            else
            {
                nRetry++;
            }
            if(nRetry == 5)
            {
                ofLog(OF_LOG_VERBOSE, "%s - OMX_EmptyThisBuffer() finally failed\n", __func__);
                return 0;
            }
        }

        if(isFirstFrame)
        {
            isFirstFrame = false;
            renderComponent.waitForEvent(OMX_EventPortSettingsChanged);

            renderComponent.disablePort(renderComponent.getInputPort());
            mixerComponent.disablePort(mixerComponent.getOutputPort());
            mixerComponent.disablePort(mixerComponent.getInputPort());
            decoderComponent.disablePort(decoderComponent.getOutputPort());

            /* setup mixer input */
            pcm_input.nPortIndex      = mixerComponent.getInputPort();
            error = mixerComponent.setParameter(OMX_IndexParamAudioPcm, &pcm_input);
            OMX_TRACE(error);
            
            error = mixerComponent.getParameter(OMX_IndexParamAudioPcm, &pcm_input);
            OMX_TRACE(error);
            
            /* setup mixer output */
            pcm_output.nPortIndex      = mixerComponent.getOutputPort();
            error = mixerComponent.setParameter(OMX_IndexParamAudioPcm, &pcm_output);
            OMX_TRACE(error);
            
            error = mixerComponent.getParameter(OMX_IndexParamAudioPcm, &pcm_output);
            OMX_TRACE(error);
            
            pcm_output.nPortIndex      = renderComponent.getInputPort();
            error = renderComponent.setParameter(OMX_IndexParamAudioPcm, &pcm_output);
            OMX_TRACE(error);
            
            error = renderComponent.getParameter(OMX_IndexParamAudioPcm, &pcm_output);
            OMX_TRACE(error);
#ifdef DEBUG_AUDIO
            printPCM(&pcm_input);
            printPCM(&pcm_output);
#endif
            renderComponent.enablePort(renderComponent.getInputPort());
            mixerComponent.enablePort(mixerComponent.getOutputPort());
            mixerComponent.enablePort(mixerComponent.getInputPort());
            decoderComponent.enablePort(decoderComponent.getOutputPort());
        }

    }

    return len;
}


float OMXAudioDecoder::getCacheTotal()
{
    return (float)bufferLength / (float)bytesPerSecond;
}

unsigned int OMXAudioDecoder::getChunkLen()
{
    return chunkLength;
}


unsigned int OMXAudioDecoder::GetAudioRenderingLatency()
{
    OMX_PARAM_U32TYPE param;
    OMX_INIT_STRUCTURE(param);
    param.nPortIndex = renderComponent.getInputPort();

    OMX_ERRORTYPE error = renderComponent.getConfig(OMX_IndexConfigAudioRenderingLatency, &param);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
        return 0;
    }
    return param.nU32;
}

void OMXAudioDecoder::submitEOS()
{
    //ofLogVerbose(__func__) << "START";
    if(!isInitialized || doPause)
    {
        return;
    }

    OMX_ERRORTYPE error = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE *omxBuffer = decoderComponent.getInputBuffer();

    if(omxBuffer == NULL)
    {
        ofLog(OF_LOG_VERBOSE, "%s - buffer error", __func__);
        return;
    }

    omxBuffer->nOffset     = 0;
    omxBuffer->nFilledLen  = 0;
    omxBuffer->nTimeStamp  = ToOMXTime(0LL);

    omxBuffer->nFlags = OMX_BUFFERFLAG_ENDOFFRAME | OMX_BUFFERFLAG_EOS | OMX_BUFFERFLAG_TIME_UNKNOWN;

    error = decoderComponent.EmptyThisBuffer(omxBuffer);
    OMX_TRACE(error);
    if (error != OMX_ErrorNone)
    {
        return;
    }

}

bool OMXAudioDecoder::EOS()
{
    if(!isInitialized || doPause)
    {
         return false;
    }
    unsigned int latency = GetAudioRenderingLatency();
    return renderComponent.EOS() && latency <= 0;
}




bool OMXAudioDecoder::setClock(OMXClock *clock)
{
    if(omxClock != NULL)
    {
        ofLogError(__func__) << "NULL CLOCK PASSED";
        return false;
    }

    omxClock = clock;
    return true;
}

void OMXAudioDecoder::setCodingType(AVCodecID codec)
{
    switch(codec)
    {
        case AV_CODEC_ID_DTS:
            //ofLogVerbose(__func__) << "OMX_AUDIO_CodingDTS";
            m_eEncoding = OMX_AUDIO_CodingDTS;
            break;
        case AV_CODEC_ID_AC3:
        case AV_CODEC_ID_EAC3:
            //ofLogVerbose(__func__) << "OMX_AUDIO_CodingDDP";
            m_eEncoding = OMX_AUDIO_CodingDDP;
            break;
        default:
            //ofLogVerbose(__func__) << "OMX_AUDIO_CodingPCM";
            m_eEncoding = OMX_AUDIO_CodingPCM;
            break;
    }
}

void OMXAudioDecoder::printChannels(OMX_AUDIO_CHANNELTYPE eChannelMapping[])
{
    for(int i = 0; i < OMX_AUDIO_MAXCHANNELS; i++)
    {
        switch(eChannelMapping[i])
        {
            case OMX_AUDIO_ChannelLF:
                ofLogVerbose(__func__) << "OMX_AUDIO_ChannelLF";
                break;
            case OMX_AUDIO_ChannelRF:
                ofLogVerbose(__func__) << "OMX_AUDIO_ChannelRF";
                break;
            case OMX_AUDIO_ChannelCF:
                ofLogVerbose(__func__) << "OMX_AUDIO_ChannelCF";
                break;
            case OMX_AUDIO_ChannelLS:
                ofLogVerbose(__func__) << "OMX_AUDIO_ChannelLS";
                break;
            case OMX_AUDIO_ChannelRS:
                ofLogVerbose(__func__) << "OMX_AUDIO_ChannelRS";
                break;
            case OMX_AUDIO_ChannelLFE:
                ofLogVerbose(__func__) << "OMX_AUDIO_ChannelLFE";
                break;
            case OMX_AUDIO_ChannelCS:
                ofLogVerbose(__func__) << "OMX_AUDIO_ChannelCS";
                break;
            case OMX_AUDIO_ChannelLR:
                ofLogVerbose(__func__) << "OMX_AUDIO_ChannelLR";
                break;
            case OMX_AUDIO_ChannelRR:
                ofLogVerbose(__func__) << "OMX_AUDIO_ChannelRR";
                break;
            case OMX_AUDIO_ChannelNone:
            case OMX_AUDIO_ChannelKhronosExtensions:
            case OMX_AUDIO_ChannelVendorStartUnused:
            case OMX_AUDIO_ChannelMax:
            default:
                break;
        }
    }
}

void OMXAudioDecoder::printPCM(OMX_AUDIO_PARAM_PCMMODETYPE *pcm)
{

    stringstream info;
    info << "PCM PROPERTIES"    << "\n";
    info << "nPortIndex: "      << (int)pcm->nPortIndex         << "\n";
    info << "eNumData: "        << pcm->eNumData                << "\n";
    info << "eEndian: "         << pcm->eEndian                 << "\n";
    info << "bInterleaved: "    << (int)pcm->bInterleaved       << "\n";
    info << "nBitPerSample: "   << (int)pcm->nBitPerSample      << "\n";
    info << "ePCMMode: "        << pcm->ePCMMode                << "\n";
    info << "nChannels: "       << (int)pcm->nChannels          << "\n";
    info << "nSamplingRate: "   << (int)pcm->nSamplingRate      << "\n";
    ofLogVerbose(__func__) << "\n" <<  info.str();

    printChannels(pcm->eChannelMapping);
}


