#pragma once


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
