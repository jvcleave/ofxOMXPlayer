#pragma once

#include "DynamicDll.h"
#include "DllAvUtil.h"


extern "C" {
#ifndef HAVE_MMX
#define HAVE_MMX
#endif
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#ifndef __GNUC__
#pragma warning(disable:4244)
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

  /* From non-public audioconvert.h */
  struct AVAudioConvert;
  typedef struct AVAudioConvert AVAudioConvert;
  AVAudioConvert *av_audio_convert_alloc(enum AVSampleFormat out_fmt, int out_channels,
                                         enum AVSampleFormat in_fmt, int in_channels,
                                         const float *matrix, int flags);
  void av_audio_convert_free(AVAudioConvert *ctx);
  int av_audio_convert(AVAudioConvert *ctx,
                             void * const out[6], const int out_stride[6],
                       const void * const  in[6], const int  in_stride[6], int len);

}


class DllAvCodecInterface
{
public:
  virtual ~DllAvCodecInterface() {}
  virtual void avcodec_register_all(void)=0;
  virtual void avcodec_flush_buffers(AVCodecContext *avctx)=0;
  virtual int avcodec_open2_dont_call(AVCodecContext *avctx, AVCodec *codec, AVDictionary **options)=0;
  virtual AVCodec *avcodec_find_decoder(enum CodecID id)=0;
  virtual AVCodec *avcodec_find_encoder(enum CodecID id)=0;
  virtual int avcodec_close_dont_call(AVCodecContext *avctx)=0;
  virtual AVFrame *avcodec_alloc_frame(void)=0;
  virtual int avpicture_fill(AVPicture *picture, uint8_t *ptr, PixelFormat pix_fmt, int width, int height)=0;
  virtual int avcodec_decode_video2(AVCodecContext *avctx, AVFrame *picture, int *got_picture_ptr, AVPacket *avpkt)=0;
  virtual int avcodec_decode_audio4(AVCodecContext *avctx, AVFrame *frame, int *got_frame_ptr, AVPacket *avpkt)=0;
  virtual int avcodec_decode_subtitle2(AVCodecContext *avctx, AVSubtitle *sub, int *got_sub_ptr, AVPacket *avpkt)=0;
  //virtual int avcodec_encode_audio(AVCodecContext *avctx, uint8_t *buf, int buf_size, const short *samples)=0;
  virtual int avpicture_get_size(PixelFormat pix_fmt, int width, int height)=0;
  virtual AVCodecContext *avcodec_alloc_context3(AVCodec *codec)=0;
  virtual void avcodec_string(char *buf, int buf_size, AVCodecContext *enc, int encode)=0;
  virtual void avcodec_get_context_defaults3(AVCodecContext *s, AVCodec *codec)=0;
  virtual AVCodecParserContext *av_parser_init(int codec_id)=0;
  virtual int av_parser_parse2(AVCodecParserContext *s,AVCodecContext *avctx, uint8_t **poutbuf, int *poutbuf_size,
                    const uint8_t *buf, int buf_size,
                    int64_t pts, int64_t dts, int64_t pos)=0;
  virtual void av_parser_close(AVCodecParserContext *s)=0;
  virtual AVBitStreamFilterContext *av_bitstream_filter_init(const char *name)=0;
  virtual int av_bitstream_filter_filter(AVBitStreamFilterContext *bsfc,
    AVCodecContext *avctx, const char *args,
    uint8_t **poutbuf, int *poutbuf_size,
    const uint8_t *buf, int buf_size, int keyframe) =0;
  virtual void av_bitstream_filter_close(AVBitStreamFilterContext *bsfc) =0;
  virtual void avpicture_free(AVPicture *picture)=0;
  virtual void av_free_packet(AVPacket *pkt)=0;
  virtual int avpicture_alloc(AVPicture *picture, PixelFormat pix_fmt, int width, int height)=0;
  virtual enum PixelFormat avcodec_default_get_format(struct AVCodecContext *s, const enum PixelFormat *fmt)=0;
  virtual int avcodec_default_get_buffer(AVCodecContext *s, AVFrame *pic)=0;
  virtual void avcodec_default_release_buffer(AVCodecContext *s, AVFrame *pic)=0;
  virtual AVCodec *av_codec_next(AVCodec *c)=0;
  virtual AVAudioConvert *av_audio_convert_alloc(enum AVSampleFormat out_fmt, int out_channels,
                                                 enum AVSampleFormat in_fmt , int in_channels,
                                                 const float *matrix        , int flags)=0;
  virtual void av_audio_convert_free(AVAudioConvert *ctx)=0;
  virtual int av_audio_convert(AVAudioConvert *ctx,
                                     void * const out[6], const int out_stride[6],
                               const void * const  in[6], const int  in_stride[6], int len)=0;
  virtual int av_dup_packet(AVPacket *pkt)=0;
  virtual void av_init_packet(AVPacket *pkt)=0;
};


// Use direct layer
class DllAvCodec : public DllDynamic, DllAvCodecInterface
{
public:
  virtual ~DllAvCodec() {}
  virtual void avcodec_register_all()
  {
    ::avcodec_register_all();
  }
  virtual void avcodec_flush_buffers(AVCodecContext *avctx) { ::avcodec_flush_buffers(avctx); }
  virtual int avcodec_open2(AVCodecContext *avctx, AVCodec *codec, AVDictionary **options)
  {
    return ::avcodec_open2(avctx, codec, options);
  }
  virtual int avcodec_open2_dont_call(AVCodecContext *avctx, AVCodec *codec, AVDictionary **options) { *(int *)0x0 = 0; return 0; }
  virtual int avcodec_close_dont_call(AVCodecContext *avctx) { *(int *)0x0 = 0; return 0; }
  virtual AVCodec *avcodec_find_decoder(enum CodecID id) { return ::avcodec_find_decoder(id); }
  virtual AVCodec *avcodec_find_encoder(enum CodecID id) { return ::avcodec_find_encoder(id); }
  virtual int avcodec_close(AVCodecContext *avctx)
  {
    return ::avcodec_close(avctx);
  }
  virtual AVFrame *avcodec_alloc_frame() { return ::avcodec_alloc_frame(); }
  virtual int avpicture_fill(AVPicture *picture, uint8_t *ptr, PixelFormat pix_fmt, int width, int height) { return ::avpicture_fill(picture, ptr, pix_fmt, width, height); }
  virtual int avcodec_decode_video2(AVCodecContext *avctx, AVFrame *picture, int *got_picture_ptr, AVPacket *avpkt) { return ::avcodec_decode_video2(avctx, picture, got_picture_ptr, avpkt); }
  virtual int avcodec_decode_audio4(AVCodecContext *avctx, AVFrame *frame, int *got_frame_ptr, AVPacket *avpkt) { return ::avcodec_decode_audio4(avctx, frame, got_frame_ptr, avpkt); }
  virtual int avcodec_decode_subtitle2(AVCodecContext *avctx, AVSubtitle *sub, int *got_sub_ptr, AVPacket *avpkt) { return ::avcodec_decode_subtitle2(avctx, sub, got_sub_ptr, avpkt); }
 // virtual int avcodec_encode_audio(AVCodecContext *avctx, uint8_t *buf, int buf_size, const short *samples) { return ::avcodec_encode_audio(avctx, buf, buf_size, samples); }
  virtual int avpicture_get_size(PixelFormat pix_fmt, int width, int height) { return ::avpicture_get_size(pix_fmt, width, height); }
  virtual AVCodecContext *avcodec_alloc_context3(AVCodec *codec) { return ::avcodec_alloc_context3(codec); }
  virtual void avcodec_string(char *buf, int buf_size, AVCodecContext *enc, int encode) { ::avcodec_string(buf, buf_size, enc, encode); }
  virtual void avcodec_get_context_defaults3(AVCodecContext *s, AVCodec *codec) { ::avcodec_get_context_defaults3(s, codec); }

  virtual AVCodecParserContext *av_parser_init(int codec_id) { return ::av_parser_init(codec_id); }
  virtual int av_parser_parse2(AVCodecParserContext *s,AVCodecContext *avctx, uint8_t **poutbuf, int *poutbuf_size,
                    const uint8_t *buf, int buf_size,
                    int64_t pts, int64_t dts, int64_t pos)
  {
    return ::av_parser_parse2(s, avctx, poutbuf, poutbuf_size, buf, buf_size, pts, dts, pos);
  }
  virtual void av_parser_close(AVCodecParserContext *s) { ::av_parser_close(s); }

  virtual AVBitStreamFilterContext *av_bitstream_filter_init(const char *name) { return ::av_bitstream_filter_init(name); }
  virtual int av_bitstream_filter_filter(AVBitStreamFilterContext *bsfc,
    AVCodecContext *avctx, const char *args,
    uint8_t **poutbuf, int *poutbuf_size,
    const uint8_t *buf, int buf_size, int keyframe) { return ::av_bitstream_filter_filter(bsfc, avctx, args, poutbuf, poutbuf_size, buf, buf_size, keyframe); }
  virtual void av_bitstream_filter_close(AVBitStreamFilterContext *bsfc) { ::av_bitstream_filter_close(bsfc); }

  virtual void avpicture_free(AVPicture *picture) { ::avpicture_free(picture); }
  virtual void av_free_packet(AVPacket *pkt) { ::av_free_packet(pkt); }
  virtual int avpicture_alloc(AVPicture *picture, PixelFormat pix_fmt, int width, int height) { return ::avpicture_alloc(picture, pix_fmt, width, height); }
  virtual int avcodec_default_get_buffer(AVCodecContext *s, AVFrame *pic) { return ::avcodec_default_get_buffer(s, pic); }
  virtual void avcodec_default_release_buffer(AVCodecContext *s, AVFrame *pic) { ::avcodec_default_release_buffer(s, pic); }
  virtual enum PixelFormat avcodec_default_get_format(struct AVCodecContext *s, const enum PixelFormat *fmt) { return ::avcodec_default_get_format(s, fmt); }
  virtual AVCodec *av_codec_next(AVCodec *c) { return ::av_codec_next(c); }
  virtual AVAudioConvert *av_audio_convert_alloc(enum AVSampleFormat out_fmt, int out_channels,
                                                 enum AVSampleFormat in_fmt , int in_channels,
                                                 const float *matrix        , int flags)
          { return ::av_audio_convert_alloc(out_fmt, out_channels, in_fmt, in_channels, matrix, flags); }
  virtual void av_audio_convert_free(AVAudioConvert *ctx)
          { ::av_audio_convert_free(ctx); }

  virtual int av_audio_convert(AVAudioConvert *ctx,
                                     void * const out[6], const int out_stride[6],
                               const void * const  in[6], const int  in_stride[6], int len)
          { return ::av_audio_convert(ctx, out, out_stride, in, in_stride, len); }

  virtual int av_dup_packet(AVPacket *pkt) { return ::av_dup_packet(pkt); }
  virtual void av_init_packet(AVPacket *pkt) { return ::av_init_packet(pkt); }

  // DLL faking.
  virtual bool ResolveExports() { return true; }
  virtual bool Load() {
    //printf("DllAvCodec: Using libavcodec system library \n");
    return true;
  }
  virtual void Unload() {}
};