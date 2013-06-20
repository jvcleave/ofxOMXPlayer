#pragma once

#include "ofMain.h"

#include "OMXDecoder.h"


class COMXVideo : public OMXDecoder
{
public:
	COMXVideo();
	~COMXVideo();

	// Required overrides
	bool SendDecoderConfig();
	bool NaluFormatStartCodes(enum CodecID codec, uint8_t *in_extradata, int in_extrasize);
	bool Open(COMXStreamInfo &hints, OMXClock *clock, float display_aspect = 0.0f, bool deinterlace = false, bool hdmi_clock_sync = false);
	void Close(void);
	unsigned int GetFreeSpace();
	unsigned int GetSize();
	int  Decode(uint8_t *pData, int iSize, double dts, double pts);
	void Reset(void);
	void SetDropState(bool bDrop);
	bool Pause();
	bool Resume();
	std::string GetDecoderName() { return m_video_codec_name; };
	//void SetVideoRect(const CRect& SrcRect, const CRect& DestRect);
	int GetInputBufferSize();
	void WaitCompletion();


	COMXCoreComponent m_omx_image_fx;
	COMXCoreTunel     m_omx_tunnel_image_fx;



	bool				m_deinterlace;
	bool				m_hdmi_clock_sync;
	bool				m_syncclock;
};