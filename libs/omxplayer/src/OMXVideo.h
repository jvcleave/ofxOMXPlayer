#pragma once
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

#include "ofMain.h"

#include "OMXCore.h"
#include "OMXStreamInfo.h"

#include <IL/OMX_Video.h>

#include "OMXClock.h"
#include "OMXReader.h"

//#include "guilib/Geometry.h"




#define CLASSNAME "COMXVideo"

class DllAvUtil;
class DllAvFormat;
class COMXVideo
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
protected:
  // Video format
  bool              m_drop_state;
  unsigned int      m_decoded_width;
  unsigned int      m_decoded_height;

  OMX_VIDEO_CODINGTYPE m_codingType;

  COMXCoreComponent m_omx_decoder;
  COMXCoreComponent m_omx_render;
  COMXCoreComponent m_omx_sched;
  COMXCoreComponent m_omx_image_fx;
  COMXCoreComponent *m_omx_clock;
  OMXClock           *m_av_clock;

  COMXCoreTunel     m_omx_tunnel_decoder;
  COMXCoreTunel     m_omx_tunnel_clock;
  COMXCoreTunel     m_omx_tunnel_sched;
  COMXCoreTunel     m_omx_tunnel_image_fx;
  bool              m_is_open;

  bool              m_Pause;
  bool              m_setStartTime;

  uint8_t           *m_extradata;
  int               m_extrasize;

  std::string       m_video_codec_name;

  bool              m_deinterlace;
  bool              m_hdmi_clock_sync;
  bool              m_first_frame;
};