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


class OMXAudio
{
	public:
	
        OMXAudio();
        ~OMXAudio();
    
		enum EEncoded
		{
			ENCODED_NONE = 0,
			ENCODED_IEC61937_AC3,
			ENCODED_IEC61937_EAC3,
			ENCODED_IEC61937_DTS,
			ENCODED_IEC61937_MPEG,
			ENCODED_IEC61937_UNKNOWN,
		};
		
		unsigned int getChunkLen();
		//float GetDelay();
		//float GetCacheTime();
		float getCacheTotal();
		unsigned int GetAudioRenderingLatency();
		
		bool init(string device,
						enum PCMChannels *channelMap,
		                OMXStreamInfo& hints, OMXClock *clock,
						EEncoded bPassthrough,
						bool boostOnDownmix);

		

		unsigned int addPackets(void* data, unsigned int len);
		unsigned int addPackets(void* data, unsigned int len, double dts, double pts);
		unsigned int GetSpace();
		bool Deinitialize();
		bool pause();
		bool Stop();
		bool resume();

		long getCurrentVolume() const;
		void mute(bool bMute);
		bool setCurrentVolume(long nVolume);
		void setDynamicRangeCompression(long drc)
		{
			DRC = drc;
		}
		float getCurrentAttenuation()
		{
			return remapObject.getCurrentAttenuation();
		}
		void submitEOS();
		bool EOS();

		void flush();

		void process();

		bool setClock(OMXClock *clock);
		void setCodingType(AVCodecID codec);

		void printChannels(OMX_AUDIO_CHANNELTYPE eChannelMapping[]);
		void printPCM(OMX_AUDIO_PARAM_PCMMODETYPE *pcm);

	private:
		bool          isInitialized;
		bool          doPause;
		bool          canPause;
		long          currentVolume;
		long          DRC;
		bool          doPassthrough;
		bool          doNormalizeDownmix;
		unsigned int  bytesPerSecond;
		unsigned int  bufferLength;
		unsigned int  chunkLength;
		unsigned int  numInputChannels;
		unsigned int  numOutputChannels;
		unsigned int  numDownmixChannels;
		unsigned int  m_BitsPerSample;
		Component*    clockComponent;
		OMXClock*     omxClock;
		bool          hasExternalClock;
		bool          doSetStartTime;
		int           sampleSize;
		bool          isFirstFrame;
		int           sampleRate;
		OMX_AUDIO_CODINGTYPE m_eEncoding;
		uint8_t       *extraData;
		int           extraSize;

		OMX_AUDIO_PARAM_PCMMODETYPE pcm_output;
		OMX_AUDIO_PARAM_PCMMODETYPE pcm_input;
		OMX_AUDIO_PARAM_DTSTYPE     dtsParam;
		WAVEFORMATEXTENSIBLE        waveFormat;

	protected:
		Component renderComponent;
		Component m_omx_mixer;
		Component decoderComponent;
		Tunnel     clockTunnel;
		Tunnel     mixerTunnel;
		Tunnel     decoderTunnel;
		CPCMRemap remapObject;
};
