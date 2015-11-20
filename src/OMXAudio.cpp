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



#include "OMXAudio.h"
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


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//***********************************************************************************************
OMXAudio::OMXAudio() :
	isInitialized     (false  ),
	doPause           (false  ),
	m_CanPause        (false  ),
	m_CurrentVolume   (0      ),
	m_Passthrough     (false  ),
	m_HWDecode        (false  ),
	m_normalize_downmix(true   ),
	m_BytesPerSec     (0      ),
	m_BufferLen       (0      ),
	m_ChunkLen        (0      ),
	m_InputChannels   (0      ),
	m_OutputChannels  (0      ),
	m_downmix_channels(0      ),
	m_BitsPerSample   (0      ),
	clockComponent       (NULL   ),
	omxClock        (NULL   ),
	m_external_clock  (false  ),
	doSetStartTime    (false  ),
	m_SampleSize      (0      ),
	isFirstFrame     (true   ),
	m_SampleRate      (0      ),
	m_eEncoding       (OMX_AUDIO_CodingPCM),
	extraData       (NULL   ),
	extraSize       (0      )
{
}

OMXAudio::~OMXAudio()
{
	if(isInitialized)
	{
		Deinitialize();
	}
}


bool OMXAudio::init(string device, 
                    enum PCMChannels *channelMap,
                    OMXStreamInfo& hints, 
                    OMXClock *clock, 
                    EEncoded bPassthrough, 
                    bool bUseHWDecode,
                    bool boostOnDownmix)
{
	m_HWDecode = false;
	m_Passthrough = false;

	if(bPassthrough != OMXAudio::ENCODED_NONE)
	{
		m_Passthrough = true;
		SetCodingType(hints.codec);
	}
	else 
	{
        if(bUseHWDecode)
        {
            m_HWDecode = CanHWDecode(hints.codec);  
        }
		SetCodingType(CODEC_ID_PCM_S16LE);
	}

	if(hints.extrasize > 0 && hints.extradata != NULL)
	{
		extraSize = hints.extrasize;
		extraData = (uint8_t *)malloc(extraSize);
		memcpy(extraData, hints.extradata, hints.extrasize);
	}
    ofLogVerbose(__func__) << "PART 1";
	/*return init(device, hints.channels, channelMap, hints.channels, hints.samplerate, hints.bitspersample, false, boostOnDownmix, false, bPassthrough);*/
    
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

    ofLogVerbose(__func__) << "PART 2";

	m_Passthrough = false;

	if(bPassthrough != OMXAudio::ENCODED_NONE)
	{
		m_Passthrough =true;
	}

	memset(&m_wave_header, 0x0, sizeof(m_wave_header));

	m_CurrentVolume = 0;

	m_downmix_channels = downmixChannels;
	m_normalize_downmix = !boostOnDownmix;

	m_InputChannels = iChannels;
	m_remap.Reset();

	OMX_INIT_STRUCTURE(m_pcm_output);
	m_OutputChannels = 2;
	m_pcm_output.nChannels = m_OutputChannels;
	m_pcm_output.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
	m_pcm_output.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
	m_pcm_output.eChannelMapping[2] = OMX_AUDIO_ChannelMax;

	OMX_INIT_STRUCTURE(m_pcm_input);
	m_pcm_input.nChannels = m_OutputChannels;
	m_pcm_input.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
	m_pcm_input.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
	m_pcm_input.eChannelMapping[2] = OMX_AUDIO_ChannelMax;

	m_wave_header.Format.nChannels  = m_OutputChannels;
	m_wave_header.dwChannelMask     = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;

	// set the input format, and get the channel layout so we know what we need to open
	enum PCMChannels *outLayout = m_remap.SetInputFormat (iChannels, channelMap, uiBitsPerSample / 8, uiSamplesPerSec);;

	if (!m_Passthrough && channelMap && outLayout)
	{
		/* setup output channel map */
		m_OutputChannels = 0;
		int ch = 0, map;
		int chan = 0;
		while(outLayout[ch] != PCM_INVALID && chan < OMX_AUDIO_MAXCHANNELS)
		{
			for(map = 0; map < OMX_MAX_CHANNELS; ++map)
			{
				if (outLayout[ch] == OMXChannelMap[map])
				{
					m_pcm_output.eChannelMapping[chan] = OMXChannels[map];
					chan++;
					break;
				}
			}
			++ch;
		}

		m_OutputChannels = chan;

		/* setup input channel map */
		for (chan=0; chan < OMX_AUDIO_MAXCHANNELS; chan++)
		{
			m_pcm_input.eChannelMapping[chan] = OMX_AUDIO_ChannelNone;
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
					m_pcm_input.eChannelMapping[chan] = OMXChannels[map];
					m_wave_header.dwChannelMask |= WAVEChannels[map];
					chan++;
					break;
				}
			}
			++ch;
		}
	}

	// set the m_pcm_output parameters
	m_pcm_output.eNumData            = OMX_NumericalDataSigned;
	m_pcm_output.eEndian             = OMX_EndianLittle;
	m_pcm_output.bInterleaved        = OMX_TRUE;
	m_pcm_output.nBitPerSample       = uiBitsPerSample;
	m_pcm_output.ePCMMode            = OMX_AUDIO_PCMModeLinear;
	m_pcm_output.nChannels           = m_OutputChannels;
	m_pcm_output.nSamplingRate       = uiSamplesPerSec;

	m_SampleRate    = uiSamplesPerSec;
	m_BitsPerSample = uiBitsPerSample;
	m_BufferLen     = m_BytesPerSec = uiSamplesPerSec * (uiBitsPerSample >> 3) * m_InputChannels;
	m_BufferLen     *= AUDIO_BUFFER_SECONDS;
	m_ChunkLen      = 6144;
	//m_ChunkLen      = 2048;

	m_wave_header.Samples.wValidBitsPerSample = uiBitsPerSample;
	m_wave_header.Samples.wSamplesPerBlock    = 0;
	m_wave_header.Format.nChannels            = m_InputChannels;
	m_wave_header.Format.nBlockAlign          = m_InputChannels * (uiBitsPerSample >> 3);
	m_wave_header.Format.wFormatTag           = WAVE_FORMAT_PCM;
	m_wave_header.Format.nSamplesPerSec       = uiSamplesPerSec;
	m_wave_header.Format.nAvgBytesPerSec      = m_BytesPerSec;
	m_wave_header.Format.wBitsPerSample       = uiBitsPerSample;
	m_wave_header.Samples.wValidBitsPerSample = uiBitsPerSample;
	m_wave_header.Format.cbSize               = 0;
	m_wave_header.SubFormat                   = KSDATAFORMAT_SUBTYPE_PCM;

	//m_SampleSize              = (m_pcm_output.nChannels * m_pcm_output.nBitPerSample * m_pcm_output.nSamplingRate)>>3;
	m_SampleSize              = (m_pcm_input.nChannels * m_pcm_output.nBitPerSample * m_pcm_output.nSamplingRate)>>3;

	m_pcm_input.eNumData              = OMX_NumericalDataSigned;
	m_pcm_input.eEndian               = OMX_EndianLittle;
	m_pcm_input.bInterleaved          = OMX_TRUE;
	m_pcm_input.nBitPerSample         = uiBitsPerSample;
	m_pcm_input.ePCMMode              = OMX_AUDIO_PCMModeLinear;
	m_pcm_input.nChannels             = m_InputChannels;
	m_pcm_input.nSamplingRate         = uiSamplesPerSec;

	PrintPCM(&m_pcm_input);
	PrintPCM(&m_pcm_output);

	OMX_ERRORTYPE error = OMX_ErrorNone;
	std::string componentName = "";

	componentName = "OMX.broadcom.audio_render";
	if(!renderComponent.init(componentName, OMX_IndexParamAudioInit))
	{
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
		return false;
	}

	if(!m_Passthrough)
	{
		componentName = "OMX.broadcom.audio_mixer";
		if(!m_omx_mixer.init(componentName, OMX_IndexParamAudioInit))
		{
			return false;
		}
	}

	if(m_Passthrough)
	{
		OMX_CONFIG_BOOLEANTYPE boolType;
		OMX_INIT_STRUCTURE(boolType);
		boolType.bEnabled = OMX_TRUE;
		error = decoderComponent.setParameter(OMX_IndexParamBrcmDecoderPassThrough, &boolType);
        OMX_TRACE(error);
		if(error != OMX_ErrorNone)
		{
			return false;
		}
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

	port_param.nBufferSize = m_ChunkLen;
	port_param.nBufferCountActual = m_BufferLen / m_ChunkLen;

	error = decoderComponent.setParameter(OMX_IndexParamPortDefinition, &port_param);
    OMX_TRACE(error);
	if(error != OMX_ErrorNone)
	{
		return false;
	}

	if(m_HWDecode)
	{
		OMX_AUDIO_PARAM_PORTFORMATTYPE formatType;
		OMX_INIT_STRUCTURE(formatType);
		formatType.nPortIndex = decoderComponent.getInputPort();

		formatType.eEncoding = m_eEncoding;

		error = decoderComponent.setParameter(OMX_IndexParamAudioPortFormat, &formatType);
        OMX_TRACE(error);
		if(error != OMX_ErrorNone)
		{
			return false;
		}
	}

	if(omxClock == NULL)
	{
		/* no external clock set. generate one */
		m_external_clock = false;

		omxClock = new OMXClock();

		if(!omxClock->init(false, true))
		{
			delete omxClock;
			omxClock = NULL;
			ofLog(OF_LOG_ERROR, "OMXAudio::init error creating av clock");
			return false;
		}
	}

	clockComponent = omxClock->getComponent();

	clockTunnel.init(clockComponent, clockComponent->getInputPort(), &renderComponent, renderComponent.getInputPort()+1);

	error = clockTunnel.Establish(false);
    OMX_TRACE(error);
	if(error != OMX_ErrorNone)
	{
		return false;
	}

	if(!m_external_clock)
	{
		error = clockComponent->setState(OMX_StateExecuting);
        OMX_TRACE(error);
		if (error != OMX_ErrorNone)
		{
			return false;
		}
	}


	error = decoderComponent.allocInputBuffers();
    OMX_TRACE(error);
	if(error != OMX_ErrorNone)
	{
		return false;
	}

	if(!m_Passthrough)
	{
		decoderTunnel.init(&decoderComponent, decoderComponent.getOutputPort(), &m_omx_mixer, m_omx_mixer.getInputPort());
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

		mixerTunnel.init(&m_omx_mixer, m_omx_mixer.getOutputPort(), &renderComponent, renderComponent.getInputPort());
		error = mixerTunnel.Establish(false);
        OMX_TRACE(error);
		if(error != OMX_ErrorNone)
		{
			return false;
		}

		error = m_omx_mixer.setState(OMX_StateExecuting);
        OMX_TRACE(error);
		if(error != OMX_ErrorNone)
		{
			return false;
		}
	}
	else
	{
		decoderTunnel.init(&decoderComponent, decoderComponent.getOutputPort(), &renderComponent, renderComponent.getInputPort());
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
			ofLog(OF_LOG_ERROR, "OMXAudio::init - buffer error");
			return false;
		}

		omxBuffer->nOffset = 0;
		omxBuffer->nFilledLen = sizeof(m_wave_header);
		if(omxBuffer->nFilledLen > omxBuffer->nAllocLen)
		{
			ofLog(OF_LOG_ERROR, "OMXAudio::init - omxBuffer->nFilledLen > omxBuffer->nAllocLen");
			return false;
		}
		memset((unsigned char *)omxBuffer->pBuffer, 0x0, omxBuffer->nAllocLen);
		memcpy((unsigned char *)omxBuffer->pBuffer, &m_wave_header, omxBuffer->nFilledLen);
		omxBuffer->nFlags = OMX_BUFFERFLAG_CODECCONFIG | OMX_BUFFERFLAG_ENDOFFRAME;

		error = decoderComponent.EmptyThisBuffer(omxBuffer);
        OMX_TRACE(error);
		if (error != OMX_ErrorNone)
		{
			return false;
		}
	}
	else if(m_HWDecode)
	{
		// send decoder config
		if(extraSize > 0 && extraData != NULL)
		{
			OMX_BUFFERHEADERTYPE *omxBuffer = decoderComponent.getInputBuffer();

			if(omxBuffer == NULL)
			{
				ofLog(OF_LOG_ERROR, "%s - buffer error", __func__);
				return false;
			}

			omxBuffer->nOffset = 0;
			omxBuffer->nFilledLen = extraSize;
			if(omxBuffer->nFilledLen > omxBuffer->nAllocLen)
			{
				ofLog(OF_LOG_ERROR, "%s - omxBuffer->nFilledLen > omxBuffer->nAllocLen", __func__);
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
	}

	isInitialized   = true;
	doSetStartTime  = true;
	isFirstFrame   = true;

	setCurrentVolume(m_CurrentVolume);

	ofLog(OF_LOG_VERBOSE, "OMXAudio::init Ouput bps %d samplerate %d channels %d device %s buffer size %d bytes per second %d passthrough %d hwdecode %d",
	      (int)m_pcm_output.nBitPerSample, (int)m_pcm_output.nSamplingRate, (int)m_pcm_output.nChannels, deviceuse.c_str(), m_BufferLen, m_BytesPerSec, m_Passthrough, m_HWDecode);

	return true;
}

//***********************************************************************************************
bool OMXAudio::Deinitialize()
{
	if(!isInitialized)
	{
		return true;
	}

	if(!m_external_clock && omxClock != NULL)
	{
		omxClock->stop();
	}

	decoderTunnel.Flush();
	if(!m_Passthrough)
	{
		mixerTunnel.Flush();
	}
	clockTunnel.Flush();

	clockTunnel.Deestablish(true);
	if(!m_Passthrough)
	{
		mixerTunnel.Deestablish(true);
	}
	decoderTunnel.Deestablish(true);

	decoderComponent.flushInput();

	renderComponent.Deinitialize();
	if(!m_Passthrough)
	{
		m_omx_mixer.Deinitialize();
	}
	decoderComponent.Deinitialize();

	isInitialized = false;
	m_BytesPerSec = 0;
	m_BufferLen   = 0;

	if(!m_external_clock && omxClock != NULL)
	{
		delete omxClock;
		omxClock  = NULL;
		m_external_clock = false;
	}

	clockComponent = NULL;
	omxClock  = NULL;

	isInitialized = false;
	m_HWDecode    = false;

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

void OMXAudio::Flush()
{
	if(!isInitialized)
	{
		return;
	}

	decoderComponent.flushInput();
	decoderTunnel.Flush();
	if(!m_Passthrough)
	{
		mixerTunnel.Flush();
	}

	//doSetStartTime  = true;
	//isFirstFrame   = true;
}

//***********************************************************************************************
bool OMXAudio::pause()
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
bool OMXAudio::resume()
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
bool OMXAudio::Stop()
{
	if (!isInitialized)
	{
		return -1;
	}

	Flush();

	doPause = false;

	return true;
}

//***********************************************************************************************
long OMXAudio::getCurrentVolume() const
{
	return m_CurrentVolume;
}

//***********************************************************************************************
void OMXAudio::Mute(bool bMute)
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
		setCurrentVolume(m_CurrentVolume);
	}
}

//***********************************************************************************************
bool OMXAudio::setCurrentVolume(long nVolume)
{
	if(!isInitialized || m_Passthrough)
	{
		return false;
	}

	m_CurrentVolume = nVolume;

	if((m_downmix_channels == 6 || m_downmix_channels == 8) &&
	        m_OutputChannels == 2)
	{
		// Convert from millibels to amplitude ratio
		double r = pow(10, nVolume / 2000.0);

		float* coeff = NULL;

		switch(m_downmix_channels)
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

		if(m_normalize_downmix)
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
		mix.nPortIndex = m_omx_mixer.getInputPort();

		if(sizeof(mix.coeff)/sizeof(mix.coeff[0]) == 16)
		{
			ofLogError() << "Unexpected OMX_CONFIG_BRCMAUDIODOWNMIXCOEFFICIENTS::coeff length";
		}


		for(size_t i = 0; i < 16; ++i)
		{
			mix.coeff[i] = static_cast<unsigned int>(0x10000 * (coeff[i] * r));
		}

		OMX_ERRORTYPE error = m_omx_mixer.setConfig(OMX_IndexConfigBrcmAudioDownmixCoefficients, &mix);
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
unsigned int OMXAudio::GetSpace()
{
	int free = decoderComponent.getInputBufferSpace();
	return free;
}
#endif
unsigned int OMXAudio::AddPackets(void* data, unsigned int len)
{
	return AddPackets(data, len, 0, 0);
}

//***********************************************************************************************
unsigned int OMXAudio::AddPackets(void* data, unsigned int len, double dts, double pts)
{
	if(!isInitialized)
	{
		ofLog(OF_LOG_ERROR,"OMXAudio::AddPackets - sanity failed. no valid play handle!");
		return len;
	}

	unsigned int demuxer_bytes = (unsigned int)len;
	uint8_t *demuxer_content = (uint8_t *)data;

	OMX_ERRORTYPE error;

	OMX_BUFFERHEADERTYPE *omxBuffer = NULL;

	while(demuxer_bytes)
	{
		// 200ms timeout
		omxBuffer = decoderComponent.getInputBuffer(200);

		if(omxBuffer == NULL)
		{
			ofLog(OF_LOG_ERROR, "OMXAudio::Decode timeout");
			return len;
		}

		omxBuffer->nOffset = 0;
		omxBuffer->nFlags  = 0;

		omxBuffer->nFilledLen = (demuxer_bytes > omxBuffer->nAllocLen) ? omxBuffer->nAllocLen : demuxer_bytes;
		memcpy(omxBuffer->pBuffer, demuxer_content, omxBuffer->nFilledLen);

		/*
		if (m_SampleSize > 0 && pts != DVD_NOPTS_VALUE && !(omxBuffer->nFlags & OMX_BUFFERFLAG_TIME_UNKNOWN) && !m_Passthrough && !m_HWDecode)
		{
		  pts += ((double)omxBuffer->nFilledLen * DVD_TIME_BASE) / m_SampleSize;
		}
		printf("ADec : pts %f omxBuffer 0x%08x buffer 0x%08x number %d\n",
		      (float)pts / AV_TIME_BASE, omxBuffer, omxBuffer->pBuffer, (int)omxBuffer->pAppPrivate);
		*/

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
			if(!m_Passthrough)
			{
				m_omx_mixer.disablePort(m_omx_mixer.getOutputPort());
				m_omx_mixer.disablePort(m_omx_mixer.getInputPort());
			}
			decoderComponent.disablePort(decoderComponent.getOutputPort());

			if(!m_Passthrough)
			{
				if(m_HWDecode)
				{
					OMX_INIT_STRUCTURE(m_pcm_input);
					m_pcm_input.nPortIndex      = decoderComponent.getOutputPort();
					error = decoderComponent.getParameter(OMX_IndexParamAudioPcm, &m_pcm_input);
					OMX_TRACE(error);
				}

				/* setup mixer input */
				m_pcm_input.nPortIndex      = m_omx_mixer.getInputPort();
				error = m_omx_mixer.setParameter(OMX_IndexParamAudioPcm, &m_pcm_input);
				OMX_TRACE(error);
                
				error = m_omx_mixer.getParameter(OMX_IndexParamAudioPcm, &m_pcm_input);
				OMX_TRACE(error);

				/* setup mixer output */
				m_pcm_output.nPortIndex      = m_omx_mixer.getOutputPort();
				error = m_omx_mixer.setParameter(OMX_IndexParamAudioPcm, &m_pcm_output);
				OMX_TRACE(error);
                
				error = m_omx_mixer.getParameter(OMX_IndexParamAudioPcm, &m_pcm_output);
				OMX_TRACE(error);
                
				m_pcm_output.nPortIndex      = renderComponent.getInputPort();
				error = renderComponent.setParameter(OMX_IndexParamAudioPcm, &m_pcm_output);
				OMX_TRACE(error);
                
				error = renderComponent.getParameter(OMX_IndexParamAudioPcm, &m_pcm_output);
				OMX_TRACE(error);

				PrintPCM(&m_pcm_input);
				PrintPCM(&m_pcm_output);
			}
			else
			{
				m_pcm_output.nPortIndex      = decoderComponent.getOutputPort();
				decoderComponent.getParameter(OMX_IndexParamAudioPcm, &m_pcm_output);
				PrintPCM(&m_pcm_output);

				OMX_AUDIO_PARAM_PORTFORMATTYPE formatType;
				OMX_INIT_STRUCTURE(formatType);
				formatType.nPortIndex = renderComponent.getInputPort();

				error = renderComponent.getParameter(OMX_IndexParamAudioPortFormat, &formatType);
				OMX_TRACE(error);

				formatType.eEncoding = m_eEncoding;

				error = renderComponent.setParameter(OMX_IndexParamAudioPortFormat, &formatType);
				OMX_TRACE(error);

				if(m_eEncoding == OMX_AUDIO_CodingDDP)
				{
					OMX_AUDIO_PARAM_DDPTYPE m_ddParam;
					OMX_INIT_STRUCTURE(m_ddParam);

					m_ddParam.nPortIndex      = renderComponent.getInputPort();

					m_ddParam.nChannels       = m_InputChannels; //(m_InputChannels == 6) ? 8 : m_InputChannels;
					m_ddParam.nSampleRate     = m_SampleRate;
					m_ddParam.eBitStreamId    = OMX_AUDIO_DDPBitStreamIdAC3;
					m_ddParam.nBitRate        = 0;

					for(unsigned int i = 0; i < OMX_MAX_CHANNELS; i++)
					{
						if(i >= m_ddParam.nChannels)
						{
							break;
						}

						m_ddParam.eChannelMapping[i] = OMXChannels[i];
					}

					renderComponent.setParameter(OMX_IndexParamAudioDdp, &m_ddParam);
					renderComponent.getParameter(OMX_IndexParamAudioDdp, &m_ddParam);
				}
				else if(m_eEncoding == OMX_AUDIO_CodingDTS)
				{
					m_dtsParam.nPortIndex      = renderComponent.getInputPort();

					m_dtsParam.nChannels       = m_InputChannels; //(m_InputChannels == 6) ? 8 : m_InputChannels;
					m_dtsParam.nBitRate        = 0;

					for(unsigned int i = 0; i < OMX_MAX_CHANNELS; i++)
					{
						if(i >= m_dtsParam.nChannels)
						{
							break;
						}

						m_dtsParam.eChannelMapping[i] = OMXChannels[i];
					}

					renderComponent.setParameter(OMX_IndexParamAudioDts, &m_dtsParam);
					renderComponent.getParameter(OMX_IndexParamAudioDts, &m_dtsParam);
				}
			}

			renderComponent.enablePort(renderComponent.getInputPort());
			if(!m_Passthrough)
			{
				m_omx_mixer.enablePort(m_omx_mixer.getOutputPort());
				m_omx_mixer.enablePort(m_omx_mixer.getInputPort());
			}
			decoderComponent.enablePort(decoderComponent.getOutputPort());
		}

	}

	return len;
}

//***********************************************************************************************
#if 0
float OMXAudio::GetDelay()
{
	unsigned int free = decoderComponent.getInputBufferSize() - decoderComponent.getInputBufferSpace();
	return (float)free / (float)m_BytesPerSec;
}


float OMXAudio::GetCacheTime()
{
	float fBufferLenFull = (float)m_BufferLen - (float)GetSpace();
	if(fBufferLenFull < 0)
	{
		fBufferLenFull = 0;
	}
	float ret = fBufferLenFull / (float)m_BytesPerSec;
	return ret;
}
#endif
float OMXAudio::GetCacheTotal()
{
	return (float)m_BufferLen / (float)m_BytesPerSec;
}

//***********************************************************************************************
unsigned int OMXAudio::GetChunkLen()
{
	return m_ChunkLen;
}
//***********************************************************************************************
int OMXAudio::SetPlaySpeed(int iSpeed)
{
	return 0;
}

unsigned int OMXAudio::GetAudioRenderingLatency()
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

void OMXAudio::submitEOS()
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

bool OMXAudio::EOS()
{
	if(!isInitialized || doPause)
	{
		return false;
	}
	unsigned int latency = GetAudioRenderingLatency();
	return renderComponent.EOS() && latency <= 0;
}




bool OMXAudio::SetClock(OMXClock *clock)
{
	if(omxClock != NULL)
	{
		return false;
	}

	omxClock = clock;
	m_external_clock = true;
	return true;
}

void OMXAudio::SetCodingType(AVCodecID codec)
{
	switch(codec)
	{
		case CODEC_ID_DTS:
			//ofLogVerbose(__func__) << "OMX_AUDIO_CodingDTS";
			m_eEncoding = OMX_AUDIO_CodingDTS;
			break;
		case CODEC_ID_AC3:
		case CODEC_ID_EAC3:
			//ofLogVerbose(__func__) << "OMX_AUDIO_CodingDDP";
			m_eEncoding = OMX_AUDIO_CodingDDP;
			break;
		default:
			//ofLogVerbose(__func__) << "OMX_AUDIO_CodingPCM";
			m_eEncoding = OMX_AUDIO_CodingPCM;
			break;
	}
}

bool OMXAudio::CanHWDecode(AVCodecID codec)
{
	switch(codec)
	{
		/*
		 case CODEC_ID_VORBIS:
		 //ofLogVerbose(__func__) << "OMX_AUDIO_CodingVORBIS";
		 m_eEncoding = OMX_AUDIO_CodingVORBIS;
		 m_HWDecode = true;
		 break;
		 case CODEC_ID_AAC:
		 //ofLogVerbose(__func__) << "OMX_AUDIO_CodingAAC";
		 m_eEncoding = OMX_AUDIO_CodingAAC;
		 m_HWDecode = true;
		 break;
		 */
		case CODEC_ID_MP2:
		case CODEC_ID_MP3:
			//ofLogVerbose(__func__) << "OMX_AUDIO_CodingMP3";
			m_eEncoding = OMX_AUDIO_CodingMP3;
			m_HWDecode = true;
			break;
		case CODEC_ID_DTS:
			//ofLogVerbose(__func__) << "OMX_AUDIO_CodingDTS";
			m_eEncoding = OMX_AUDIO_CodingDTS;
			m_HWDecode = true;
			break;
		case CODEC_ID_AC3:
		case CODEC_ID_EAC3:
			//ofLogVerbose(__func__) << "OMX_AUDIO_CodingDDP";
			m_eEncoding = OMX_AUDIO_CodingDDP;
			m_HWDecode = true;
			break;
		default:
			//ofLogVerbose(__func__) << "OMX_AUDIO_CodingPCM";
			m_eEncoding = OMX_AUDIO_CodingPCM;
			m_HWDecode = false;
			break;
	}

	return m_HWDecode;
}

bool OMXAudio::HWDecode(AVCodecID codec)
{
	bool ret = false;

	switch(codec)
	{
		/*
		 case CODEC_ID_VORBIS:
		 //ofLogVerbose(__func__) << "CODEC_ID_VORBIS";
		 ret = true;
		 break;
		 case CODEC_ID_AAC:
		 //ofLogVerbose(__func__) << "CODEC_ID_AAC";
		 ret = true;
		 break;
		 */
		case CODEC_ID_MP2:
		case CODEC_ID_MP3:
			//ofLogVerbose(__func__) << "CODEC_ID_MP2 / CODEC_ID_MP3";
			ret = true;
			break;
		case CODEC_ID_DTS:
			//ofLogVerbose(__func__) << "CODEC_ID_DTS";
			ret = true;
			break;
		case CODEC_ID_AC3:
		case CODEC_ID_EAC3:
			//ofLogVerbose(__func__) << "CODEC_ID_AC3 / CODEC_ID_EAC3";
			ret = true;
			break;
		default:
			ret = false;
			break;
	}

	return ret;
}

void OMXAudio::PrintChannels(OMX_AUDIO_CHANNELTYPE eChannelMapping[])
{
	for(int i = 0; i < OMX_AUDIO_MAXCHANNELS; i++)
	{
		switch(eChannelMapping[i])
		{
			case OMX_AUDIO_ChannelLF:
				//ofLogVerbose(__func__) << "OMX_AUDIO_ChannelLF";
				break;
			case OMX_AUDIO_ChannelRF:
				//ofLogVerbose(__func__) << "OMX_AUDIO_ChannelRF";
				break;
			case OMX_AUDIO_ChannelCF:
				//ofLogVerbose(__func__) << "OMX_AUDIO_ChannelCF";
				break;
			case OMX_AUDIO_ChannelLS:
				//ofLogVerbose(__func__) << "OMX_AUDIO_ChannelLS";
				break;
			case OMX_AUDIO_ChannelRS:
				//ofLogVerbose(__func__) << "OMX_AUDIO_ChannelRS";
				break;
			case OMX_AUDIO_ChannelLFE:
				//ofLogVerbose(__func__) << "OMX_AUDIO_ChannelLFE";
				break;
			case OMX_AUDIO_ChannelCS:
				//ofLogVerbose(__func__) << "OMX_AUDIO_ChannelCS";
				break;
			case OMX_AUDIO_ChannelLR:
				//ofLogVerbose(__func__) << "OMX_AUDIO_ChannelLR";
				break;
			case OMX_AUDIO_ChannelRR:
				//ofLogVerbose(__func__) << "OMX_AUDIO_ChannelRR";
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

void OMXAudio::PrintPCM(OMX_AUDIO_PARAM_PCMMODETYPE *pcm)
{

	stringstream info;
	info << "PCM PROPERTIES"	<< "\n";
	info << "nPortIndex: "		<< (int)pcm->nPortIndex			<< "\n";
	info << "eNumData: "		<< pcm->eNumData				<< "\n";
	info << "eEndian: "			<< pcm->eEndian					<< "\n";
	info << "bInterleaved: "	<< (int)pcm->bInterleaved		<< "\n";
	info << "nBitPerSample: "	<< (int)pcm->nBitPerSample		<< "\n";
	info << "ePCMMode: "		<< pcm->ePCMMode				<< "\n";
	info << "nChannels: "		<< (int)pcm->nChannels			<< "\n";
	info << "nSamplingRate: "	<< (int)pcm->nSamplingRate		<< "\n";
	//ofLogVerbose(__func__) << "\n" <<  info.str();

	//PrintChannels(pcm->eChannelMapping);
}


