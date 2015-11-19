#pragma once

#include "ofMain.h"




#include "PCMRemap.h"

#include "LIBAV_INCLUDES.h"

#include "OMXClock.h"
#include "Tunnel.h"

#include "OMXStreamInfo.h"
#include <assert.h>

#define AUDIO_BUFFER_SECONDS 2

#define WAVE_FORMAT_UNKNOWN           0x0000
#define WAVE_FORMAT_PCM               0x0001
#define WAVE_FORMAT_ADPCM             0x0002
#define WAVE_FORMAT_IEEE_FLOAT        0x0003
#define WAVE_FORMAT_EXTENSIBLE        0xFFFE

#define SPEAKER_FRONT_LEFT            0x00001
#define SPEAKER_FRONT_RIGHT           0x00002
#define SPEAKER_FRONT_CENTER          0x00004
#define SPEAKER_LOW_FREQUENCY         0x00008
#define SPEAKER_BACK_LEFT             0x00010
#define SPEAKER_BACK_RIGHT            0x00020
#define SPEAKER_FRONT_LEFT_OF_CENTER  0x00040
#define SPEAKER_FRONT_RIGHT_OF_CENTER 0x00080
#define SPEAKER_BACK_CENTER           0x00100
#define SPEAKER_SIDE_LEFT             0x00200
#define SPEAKER_SIDE_RIGHT            0x00400
#define SPEAKER_TOP_CENTER            0x00800
#define SPEAKER_TOP_FRONT_LEFT        0x01000
#define SPEAKER_TOP_FRONT_CENTER      0x02000
#define SPEAKER_TOP_FRONT_RIGHT       0x04000
#define SPEAKER_TOP_BACK_LEFT         0x08000
#define SPEAKER_TOP_BACK_CENTER       0x10000
#define SPEAKER_TOP_BACK_RIGHT        0x20000

typedef struct tGUID
{
    unsigned int Data1;
    unsigned short  Data2, Data3;
    unsigned char  Data4[8];
} __attribute__((__packed__)) GUID;






// Audio stuff
typedef struct tWAVEFORMATEX
{
    unsigned short    wFormatTag;
    unsigned short    nChannels;
    unsigned int   nSamplesPerSec;
    unsigned int   nAvgBytesPerSec;
    unsigned short    nBlockAlign;
    unsigned short    wBitsPerSample;
    unsigned short    cbSize;
} __attribute__((__packed__)) WAVEFORMATEX, *PWAVEFORMATEX, *LPWAVEFORMATEX;

typedef struct tWAVEFORMATEXTENSIBLE
{
    WAVEFORMATEX Format;
    union
    {
        unsigned short wValidBitsPerSample;
        unsigned short wSamplesPerBlock;
        unsigned short wReserved;
    } Samples;
    unsigned int dwChannelMask;
    GUID SubFormat;
} __attribute__((__packed__)) WAVEFORMATEXTENSIBLE;


class COMXAudio
{
	public:
	
		enum EEncoded
		{
			ENCODED_NONE = 0,
			ENCODED_IEC61937_AC3,
			ENCODED_IEC61937_EAC3,
			ENCODED_IEC61937_DTS,
			ENCODED_IEC61937_MPEG,
			ENCODED_IEC61937_UNKNOWN,
		};
		
		unsigned int GetChunkLen();
		//float GetDelay();
		//float GetCacheTime();
		float GetCacheTotal();
		unsigned int GetAudioRenderingLatency();
		COMXAudio();
		bool init(string device,
						enum PCMChannels *channelMap,
		                COMXStreamInfo& hints, OMXClock *clock,
						EEncoded bPassthrough,
						bool bUseHWDecode,
						bool boostOnDownmix);
	
		bool init(string device,
						int iChannels,
						enum PCMChannels *channelMap,
						unsigned int downmixChannels,
						unsigned int uiSamplesPerSec,
						unsigned int uiBitsPerSample,
						bool bResample,
						bool boostOnDownmix,
						bool bIsMusic=false,
						EEncoded bPassthrough = COMXAudio::ENCODED_NONE);
		~COMXAudio();

		unsigned int AddPackets(void* data, unsigned int len);
		unsigned int AddPackets(void* data, unsigned int len, double dts, double pts);
		unsigned int GetSpace();
		bool Deinitialize();
		bool Pause();
		bool Stop();
		bool Resume();

		long getCurrentVolume() const;
		void Mute(bool bMute);
		bool setCurrentVolume(long nVolume);
		void SetDynamicRangeCompression(long drc)
		{
			m_drc = drc;
		}
		float GetCurrentAttenuation()
		{
			return m_remap.GetCurrentAttenuation();
		}
		int SetPlaySpeed(int iSpeed);
		void submitEOS();
		bool EOS();

		void Flush();
		void DoAudioWork();

		void Process();

		bool SetClock(OMXClock *clock);
		void SetCodingType(AVCodecID codec);
		bool CanHWDecode(AVCodecID codec);
		static bool HWDecode(AVCodecID codec);

		void PrintChannels(OMX_AUDIO_CHANNELTYPE eChannelMapping[]);
		void PrintPCM(OMX_AUDIO_PARAM_PCMMODETYPE *pcm);

	private:
		bool          m_Initialized;
		bool          m_Pause;
		bool          m_CanPause;
		long          m_CurrentVolume;
		long          m_drc;
		bool          m_Passthrough;
		bool          m_HWDecode;
		bool          m_normalize_downmix;
		unsigned int  m_BytesPerSec;
		unsigned int  m_BufferLen;
		unsigned int  m_ChunkLen;
		unsigned int  m_InputChannels;
		unsigned int  m_OutputChannels;
		unsigned int  m_downmix_channels;
		unsigned int  m_BitsPerSample;
		Component*    clockComponent;
		OMXClock*     omxClock;
		bool          m_external_clock;
		bool          m_setStartTime;
		int           m_SampleSize;
		bool          m_first_frame;
		int           m_SampleRate;
		OMX_AUDIO_CODINGTYPE m_eEncoding;
		uint8_t       *m_extradata;
		int           m_extrasize;

		OMX_AUDIO_PARAM_PCMMODETYPE m_pcm_output;
		OMX_AUDIO_PARAM_PCMMODETYPE m_pcm_input;
		OMX_AUDIO_PARAM_DTSTYPE     m_dtsParam;
		WAVEFORMATEXTENSIBLE        m_wave_header;

	protected:
		Component renderComponent;
		Component m_omx_mixer;
		Component m_omx_decoder;
		Tunnel     clockTunnel;
		Tunnel     mixerTunnel;
		Tunnel     decoderTunnel;
		CPCMRemap m_remap;
};
