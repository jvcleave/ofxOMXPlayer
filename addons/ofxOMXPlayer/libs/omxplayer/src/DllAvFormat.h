#pragma once

#include "DynamicDll.h"
#include "DllAvCodec.h"

extern "C" {
#ifndef HAVE_MMX
#define HAVE_MMX
#endif
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#ifndef __GNUC__
#pragma warning(disable:4244)
#endif
#include <libavformat/avformat.h>
}

/* Flag introduced without a version bump */
#ifndef AVSEEK_FORCE
#define AVSEEK_FORCE 0x20000
#endif

typedef int64_t offset_t;

class DllAvFormatInterface
{
public:
  virtual ~DllAvFormatInterface() {}
  virtual void av_register_all_dont_call(void)=0;
  virtual AVInputFormat *av_find_input_format(const char *short_name)=0;
  virtual int url_feof(AVIOContext *s)=0;
  virtual void avformat_close_input(AVFormatContext **s)=0;
  virtual int av_read_frame(AVFormatContext *s, AVPacket *pkt)=0;
  virtual int av_read_play(AVFormatContext *s)=0;
  virtual int av_read_pause(AVFormatContext *s)=0;
  virtual int av_seek_frame(AVFormatContext *s, int stream_index, int64_t timestamp, int flags)=0;

  virtual int avformat_open_input(AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options)=0;
  virtual AVIOContext *avio_alloc_context(unsigned char *buffer, int buffer_size, int write_flag, void *opaque,
                            int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
                            int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
                            offset_t (*seek)(void *opaque, offset_t offset, int whence))=0;
  virtual AVInputFormat *av_probe_input_format(AVProbeData *pd, int is_opened)=0;
  virtual AVInputFormat *av_probe_input_format2(AVProbeData *pd, int is_opened, int *score_max)=0;
  virtual int av_probe_input_buffer(AVIOContext *pb, AVInputFormat **fmt, const char *filename, void *logctx, unsigned int offset, unsigned int max_probe_size)=0;
  virtual void av_dump_format(AVFormatContext *ic, int index, const char *url, int is_output)=0;
  virtual int avio_open(AVIOContext **s, const char *filename, int flags)=0;
  virtual int avio_close(AVIOContext *s)=0;
  virtual int avio_open_dyn_buf(AVIOContext **s)=0;
  virtual int avio_close_dyn_buf(AVIOContext *s, uint8_t **pbuffer)=0;
  virtual offset_t avio_seek(AVIOContext *s, offset_t offset, int whence)=0;
  virtual int avio_read(AVIOContext *s, unsigned char *buf, int size)=0;
  virtual void avio_w8(AVIOContext *s, int b)=0;
  virtual void avio_write(AVIOContext *s, const unsigned char *buf, int size)=0;
  virtual void avio_wb24(AVIOContext *s, unsigned int val)=0;
  virtual void avio_wb32(AVIOContext *s, unsigned int val)=0;
  virtual void avio_wb16(AVIOContext *s, unsigned int val)=0;
  virtual AVFormatContext *avformat_alloc_context(void)=0;
  virtual AVStream *avformat_new_stream(AVFormatContext *s, AVCodec *c)=0;
  virtual AVOutputFormat *av_guess_format(const char *short_name, const char *filename, const char *mime_type)=0;
  virtual int avformat_write_header (AVFormatContext *s, AVDictionary **options)=0;
  virtual int av_write_trailer(AVFormatContext *s)=0;
  virtual int av_write_frame  (AVFormatContext *s, AVPacket *pkt)=0;
  virtual int avformat_network_init  (void)=0;
  virtual int avformat_network_deinit  (void)=0;
};


// Use direct mapping
class DllAvFormat : public DllDynamic, DllAvFormatInterface
{
public:
  virtual ~DllAvFormat() {}
  virtual void av_register_all() 
  { 
    return ::av_register_all();
  } 
  virtual void av_register_all_dont_call() { *(volatile int* )0x0 = 0; } 
  virtual AVInputFormat *av_find_input_format(const char *short_name) { return ::av_find_input_format(short_name); }
  virtual int url_feof(AVIOContext *s) { return ::url_feof(s); }
  virtual void avformat_close_input(AVFormatContext **s) { ::avformat_close_input(s); }
  virtual int av_read_frame(AVFormatContext *s, AVPacket *pkt) { return ::av_read_frame(s, pkt); }
  virtual int av_read_play(AVFormatContext *s) { return ::av_read_play(s); }
  virtual int av_read_pause(AVFormatContext *s) { return ::av_read_pause(s); }
  virtual int av_seek_frame(AVFormatContext *s, int stream_index, int64_t timestamp, int flags) { return ::av_seek_frame(s, stream_index, timestamp, flags); }
  virtual int avformat_find_stream_info(AVFormatContext *ic, AVDictionary **options)
  {
    return ::avformat_find_stream_info(ic, options);
  }
  virtual int avformat_open_input(AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options)
  { return ::avformat_open_input(ps, filename, fmt, options); }
  virtual AVIOContext *avio_alloc_context(unsigned char *buffer, int buffer_size, int write_flag, void *opaque,
                            int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
                            int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
                            offset_t (*seek)(void *opaque, offset_t offset, int whence)) { return ::avio_alloc_context(buffer, buffer_size, write_flag, opaque, read_packet, write_packet, seek); }
  virtual AVInputFormat *av_probe_input_format(AVProbeData *pd, int is_opened) {return ::av_probe_input_format(pd, is_opened); }
  virtual AVInputFormat *av_probe_input_format2(AVProbeData *pd, int is_opened, int *score_max) {*score_max = 100; return ::av_probe_input_format(pd, is_opened); } // Use av_probe_input_format, this is not exported by ffmpeg's headers
  virtual int av_probe_input_buffer(AVIOContext *pb, AVInputFormat **fmt, const char *filename, void *logctx, unsigned int offset, unsigned int max_probe_size) { return ::av_probe_input_buffer(pb, fmt, filename, logctx, offset, max_probe_size); }
  virtual void av_dump_format(AVFormatContext *ic, int index, const char *url, int is_output) { ::av_dump_format(ic, index, url, is_output); }
  virtual int avio_open(AVIOContext **s, const char *filename, int flags) { return ::avio_open(s, filename, flags); }
  virtual int avio_close(AVIOContext *s) { return ::avio_close(s); }
  virtual int avio_open_dyn_buf(AVIOContext **s) { return ::avio_open_dyn_buf(s); }
  virtual int avio_close_dyn_buf(AVIOContext *s, uint8_t **pbuffer) { return ::avio_close_dyn_buf(s, pbuffer); }
  virtual offset_t avio_seek(AVIOContext *s, offset_t offset, int whence) { return ::avio_seek(s, offset, whence); }
  virtual int avio_read(AVIOContext *s, unsigned char *buf, int size) { return ::avio_read(s, buf, size); }
  virtual void avio_w8(AVIOContext *s, int b) { ::avio_w8(s, b); }
  virtual void avio_write(AVIOContext *s, const unsigned char *buf, int size) { ::avio_write(s, buf, size); }
  virtual void avio_wb24(AVIOContext *s, unsigned int val) { ::avio_wb24(s, val); }
  virtual void avio_wb32(AVIOContext *s, unsigned int val) { ::avio_wb32(s, val); }
  virtual void avio_wb16(AVIOContext *s, unsigned int val) { ::avio_wb16(s, val); }
  virtual AVFormatContext *avformat_alloc_context() { return ::avformat_alloc_context(); }
  virtual AVStream *avformat_new_stream(AVFormatContext *s, AVCodec *c) { return ::avformat_new_stream(s, c); }
  virtual AVOutputFormat *av_guess_format(const char *short_name, const char *filename, const char *mime_type) { return ::av_guess_format(short_name, filename, mime_type); }
  virtual int avformat_write_header (AVFormatContext *s, AVDictionary **options) { return ::avformat_write_header (s, options); }
  virtual int av_write_trailer(AVFormatContext *s) { return ::av_write_trailer(s); }
  virtual int av_write_frame  (AVFormatContext *s, AVPacket *pkt) { return ::av_write_frame(s, pkt); }
  virtual int avformat_network_init  (void) { return ::avformat_network_init(); }
  virtual int avformat_network_deinit  (void) { return ::avformat_network_deinit(); }

  // DLL faking.
  virtual bool ResolveExports() { return true; }
  virtual bool Load() {
    //printf("DllAvFormat: Using libavformat system library \n");
    return true;
  }
  virtual void Unload() {}
};