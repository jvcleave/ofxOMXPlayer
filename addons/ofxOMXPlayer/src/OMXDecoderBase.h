#pragma once

#include "ofMain.h"

#include "OMXCore.h"
#include "OMXStreamInfo.h"

#include <IL/OMX_Video.h>

#include "OMXClock.h"
#include "OMXReader.h"

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


class DllAvUtil;
class DllAvFormat;
class OMXDecoderBase
{
public:
	OMXDecoderBase();
	
	virtual ~OMXDecoderBase(){};

	OMX_VIDEO_CODINGTYPE m_codingType;

	COMXCoreComponent*		m_omx_clock;
	OMXClock*				m_av_clock;
	COMXCoreComponent		m_omx_decoder;
	COMXCoreComponent		m_omx_render;
	COMXCoreComponent		m_omx_sched;

	COMXCoreTunel			m_omx_tunnel_decoder;
	COMXCoreTunel			m_omx_tunnel_clock;
	COMXCoreTunel			m_omx_tunnel_sched;


	bool					m_is_open;

	bool					m_Pause;
	bool					m_setStartTime;

	bool					m_drop_state;
	unsigned int			m_decoded_width;
	unsigned int			m_decoded_height;

	uint8_t*				m_extradata;
	int						m_extrasize;

	string					m_video_codec_name;

	bool					m_first_frame;
	uint32_t				m_history_valid_pts;

	
	string decoder_name;
	

	virtual int				Decode(uint8_t *pData, int iSize, double dts, double pts)=0;
	
	
	
	void					WaitCompletion();
	
	virtual void Close() = 0;
	
	bool					Resume();
	bool					Pause();
	
	bool					SendDecoderConfig();
	bool					NaluFormatStartCodes(enum CodecID codec, uint8_t *in_extradata, int in_extrasize);
	
	void					SetDropState(bool bDrop);
	unsigned int			GetFreeSpace();
	unsigned int			GetSize();
	int						GetInputBufferSize();
	void					Reset();
	
	void ProcessCodec(COMXStreamInfo &hints);
	static unsigned count_bits(int32_t value)
	{
		unsigned bits = 0;
		for(;value;++bits)
			value &= value - 1;
		return bits;
	}
	std::string GetDecoderName() { return m_video_codec_name; };
	static int fillBufferCounter;

};