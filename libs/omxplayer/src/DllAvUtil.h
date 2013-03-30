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
 *  along with XBMC; see the file COPYING.  If not, write to the Free
 *  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "DynamicDll.h"


#ifndef __GNUC__
#pragma warning(push)
#pragma warning(disable:4244)
#endif

extern "C"
{
#include <libavutil/avutil.h>
#include <libavutil/crc.h>
#include <libavutil/fifo.h>
// for LIBAVCODEC_VERSION_INT:
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavcodec/opt.h>
#include <libavutil/mem.h>

}

#ifndef __GNUC__
#pragma warning(pop)
#endif

// calback used for logging
//void ff_avutil_log(void* ptr, int level, const char* format, va_list va);

class DllAvUtilInterface
{
public:
  virtual ~DllAvUtilInterface() {}
  virtual void av_log_set_callback(void (*)(void*, int, const char*, va_list))=0;
  virtual void *av_malloc(unsigned int size)=0;
  virtual void *av_mallocz(unsigned int size)=0;
  virtual void *av_realloc(void *ptr, unsigned int size)=0;
  virtual void av_free(void *ptr)=0;
  virtual void av_freep(void *ptr)=0;
  virtual int64_t av_rescale_rnd(int64_t a, int64_t b, int64_t c, enum AVRounding)=0;
  virtual int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq)=0;
  virtual const AVCRC* av_crc_get_table(AVCRCId crc_id)=0;
  virtual uint32_t av_crc(const AVCRC *ctx, uint32_t crc, const uint8_t *buffer, size_t length)=0;
  virtual int av_opt_set(void *obj, const char *name, const char *val, int search_flags)=0;
  virtual AVFifoBuffer *av_fifo_alloc(unsigned int size) = 0;
  virtual void av_fifo_free(AVFifoBuffer *f) = 0;
  virtual void av_fifo_reset(AVFifoBuffer *f) = 0;
  virtual int av_fifo_size(AVFifoBuffer *f) = 0;
  virtual int av_fifo_generic_read(AVFifoBuffer *f, void *dest, int buf_size, void (*func)(void*, void*, int)) = 0;
  virtual int av_fifo_generic_write(AVFifoBuffer *f, void *src, int size, int (*func)(void*, void*, int)) = 0;
  virtual char *av_strdup(const char *s)=0;
  virtual int av_get_bytes_per_sample(enum AVSampleFormat p1) = 0;
  virtual AVDictionaryEntry *av_dict_get(AVDictionary *m, const char *key, const AVDictionaryEntry *prev, int flags) = 0;
  virtual int av_dict_set(AVDictionary **pm, const char *key, const char *value, int flags)=0;
  virtual int av_samples_get_buffer_size (int *linesize, int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt, int align) = 0;
  virtual int64_t av_get_default_channel_layout(int nb_channels)=0;
  virtual void av_log_set_level(int level) = 0;
};

// Use direct layer
class DllAvUtilBase : public DllDynamic, DllAvUtilInterface
{
public:

  virtual ~DllAvUtilBase() {}
   virtual void av_log_set_callback(void (*foo)(void*, int, const char*, va_list)) { ::av_log_set_callback(foo); }
   virtual void *av_malloc(unsigned int size) { return ::av_malloc(size); }
   virtual void *av_mallocz(unsigned int size) { return ::av_mallocz(size); }
   virtual void *av_realloc(void *ptr, unsigned int size) { return ::av_realloc(ptr, size); }
   virtual void av_free(void *ptr) { ::av_free(ptr); }
   virtual void av_freep(void *ptr) { ::av_freep(ptr); }
   virtual int64_t av_rescale_rnd(int64_t a, int64_t b, int64_t c, enum AVRounding d) { return ::av_rescale_rnd(a, b, c, d); }
   virtual int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq) { return ::av_rescale_q(a, bq, cq); }
   virtual const AVCRC* av_crc_get_table(AVCRCId crc_id) { return ::av_crc_get_table(crc_id); }
   virtual uint32_t av_crc(const AVCRC *ctx, uint32_t crc, const uint8_t *buffer, size_t length) { return ::av_crc(ctx, crc, buffer, length); }
   virtual int av_opt_set(void *obj, const char *name, const char *val, int search_flags) { return ::av_opt_set(obj, name, val, search_flags); }
  virtual AVFifoBuffer *av_fifo_alloc(unsigned int size) {return ::av_fifo_alloc(size); }
  virtual void av_fifo_free(AVFifoBuffer *f) { ::av_fifo_free(f); }
  virtual void av_fifo_reset(AVFifoBuffer *f) { ::av_fifo_reset(f); }
  virtual int av_fifo_size(AVFifoBuffer *f) { return ::av_fifo_size(f); }
  virtual int av_fifo_generic_read(AVFifoBuffer *f, void *dest, int buf_size, void (*func)(void*, void*, int))
    { return ::av_fifo_generic_read(f, dest, buf_size, func); }
  virtual int av_fifo_generic_write(AVFifoBuffer *f, void *src, int size, int (*func)(void*, void*, int))
    { return ::av_fifo_generic_write(f, src, size, func); }
  virtual char *av_strdup(const char *s) { return ::av_strdup(s); }
  virtual int av_get_bytes_per_sample(enum AVSampleFormat p1)
    { return ::av_get_bytes_per_sample(p1); }
  virtual AVDictionaryEntry *av_dict_get(AVDictionary *m, const char *key, const AVDictionaryEntry *prev, int flags){ return ::av_dict_get(m, key, prev, flags); }
  virtual int av_dict_set(AVDictionary **pm, const char *key, const char *value, int flags) { return ::av_dict_set(pm, key, value, flags); }
  virtual int av_samples_get_buffer_size (int *linesize, int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt, int align)
    { return ::av_samples_get_buffer_size(linesize, nb_channels, nb_samples, sample_fmt, align); }
  virtual int64_t av_get_default_channel_layout(int nb_channels) { return ::av_get_default_channel_layout(nb_channels); }
  virtual void av_log_set_level(int level) { ::av_log_set_level(level); };

   // DLL faking.
   virtual bool ResolveExports() { return true; }
   virtual bool Load() {
     printf("\nDllAvUtilBase: Using libavutil system library");
     return true;
   }
   virtual void Unload() {}
};

class DllAvUtil : public DllAvUtilBase
{
public:
  virtual bool Load()
  {
    if( DllAvUtilBase::Load() )
    {
      //DllAvUtilBase::av_log_set_callback(ff_avutil_log);
      return true;
    }
    return false;
  }
};
