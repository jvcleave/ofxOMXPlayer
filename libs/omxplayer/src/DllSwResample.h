#pragma once
/*
 *      Copyright (C) 2005-2010 Team XBMC
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


#include "DynamicDll.h"

extern "C" {
#ifndef HAVE_MMX
	#define HAVE_MMX
#endif
#ifndef __STDC_CONSTANT_MACROS
	#define __STDC_CONSTANT_MACROS
#endif

#include <libswresample/swresample.h>
/*#include <libavresample/avresample.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#define SwrContext AVAudioResampleContext*/

}

class DllSwResampleInterface
{
public:
  virtual ~DllSwResampleInterface() {}
  virtual struct SwrContext *swr_alloc_set_opts(struct SwrContext *s, int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate, int64_t in_ch_layout, enum AVSampleFormat in_sample_fmt, int in_sample_rate, int log_offset, void *log_ctx)=0;
  virtual int swr_init(struct SwrContext *s)=0;
  virtual void swr_free(struct SwrContext **s)=0;
  virtual int swr_convert(struct SwrContext *s, uint8_t **out, int out_count, const uint8_t **in , int in_count)=0;
};


// Use direct mapping
class DllSwResample : public DllDynamic, DllSwResampleInterface
{
public:
  virtual ~DllSwResample() {}

  // DLL faking.
  virtual bool ResolveExports() { return true; }
  virtual bool Load() {
    ofLog(OF_LOG_VERBOSE, "DllAvFormat: Using libswresample system library");
    return true;
  }
  virtual void Unload() {}
  virtual struct SwrContext *swr_alloc_set_opts(struct SwrContext *s, int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate, int64_t in_ch_layout, enum AVSampleFormat in_sample_fmt, int in_sample_rate, int log_offset, void *log_ctx) { return ::swr_alloc_set_opts(s, out_ch_layout, out_sample_fmt, out_sample_rate, in_ch_layout, in_sample_fmt, in_sample_rate, log_offset, log_ctx); }
  virtual int swr_init(struct SwrContext *s) { return ::swr_init(s); }
  virtual void swr_free(struct SwrContext **s){ return ::swr_free(s); }
  virtual int swr_convert(struct SwrContext *s, uint8_t **out, int out_count, const uint8_t **in , int in_count){ return ::swr_convert(s, out, out_count, in, in_count); }
};

