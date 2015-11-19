#pragma once

#include "ofMain.h"
#include "ofxOMXPlayerSettings.h"

#include "OMXCore.h"
#include "OMXStreamInfo.h"

#include <IL/OMX_Video.h>

#include "OMXClock.h"
#include "OMXReader.h"
#include "SingleLock.h"




class OMXDecoderBase
{
	public:
		OMXDecoderBase();
		virtual ~OMXDecoderBase();
		OMX_VIDEO_CODINGTYPE m_codingType;

		COMXCoreTunnel			m_omx_tunnel_clock;
		COMXCoreTunnel			m_omx_tunnel_sched;
		COMXCoreTunnel			m_omx_tunnel_decoder;

		COMXCoreComponent		m_omx_decoder;
		COMXCoreComponent		m_omx_render;
		COMXCoreComponent		m_omx_sched;

		COMXCoreComponent*		m_omx_clock;
		OMXClock*				m_av_clock;

		bool					m_is_open;

		bool					m_Pause;
		bool					m_setStartTime;

		bool					m_drop_state;
		unsigned int			m_decoded_width;
		unsigned int			m_decoded_height;

		uint8_t*				m_extradata;
		int						m_extrasize;


		bool					m_first_frame;
		uint32_t				m_history_valid_pts;



		virtual bool			Decode(uint8_t *pData, int iSize, double pts)=0;


		void SubmitEOS();
		bool IsEOS();

		bool					Resume();
		bool					Pause();

		bool					SendDecoderConfig();
		bool					NaluFormatStartCodes(enum AVCodecID codec, uint8_t *in_extradata, int in_extrasize);

		void					SetDropState(bool bDrop);
		unsigned int			GetFreeSpace();
		unsigned int			GetSize();
		//int						GetInputBufferSize();
		void					Reset();

		void ProcessCodec(COMXStreamInfo& hints);
		static unsigned count_bits(int32_t value)
		{
			unsigned bits = 0;
			for(; value; ++bits)
			{
				value &= value - 1;
			}
			return bits;
		}

		
		virtual int getCurrentFrame() = 0;
		virtual void resetFrameCounter() = 0;
		CCriticalSection  m_critSection;
};