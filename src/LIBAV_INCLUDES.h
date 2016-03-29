#pragma once

extern "C" 
{
	
	#ifndef HAVE_MMX
		#define HAVE_MMX
	#endif
		
	#ifndef __STDC_CONSTANT_MACROS
		#define __STDC_CONSTANT_MACROS
	#endif
		
	#ifndef __GNUC__
		#pragma warning(disable:4244)
		#pragma warning(push)
		#pragma warning(disable:4244)
		#pragma warning(pop)
	#endif
	
	#include <libswresample/swresample.h>
	#include <libavfilter/avfiltergraph.h>
	#include <libavfilter/buffersink.h>
	#include <libavfilter/buffersrc.h>
	//#include <libavfilter/avcodec.h>
		
	#include <libavutil/avutil.h>
	#include <libavutil/crc.h>
	#include <libavutil/fifo.h>
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
		
		/* From non-public audioconvert.h */
		struct AVAudioConvert;
		typedef struct AVAudioConvert AVAudioConvert;
		AVAudioConvert *av_audio_convert_alloc(enum AVSampleFormat out_fmt, 
											   int out_channels,
											   enum AVSampleFormat in_fmt, 
											   int in_channels,
											   float *matrix, 
											   int flags);
		void av_audio_convert_free(AVAudioConvert *component);
		int av_audio_convert(AVAudioConvert *component,
							 void * out[6], 
							 int out_stride[6],
							 void *  in[6], 
							 int  in_stride[6], int len);
	//#include <libavutil/opt.h>
	//#include <libavcodec/opt.h>
	#include <libavutil/mem.h>
}

#ifndef AVSEEK_FORCE
#define AVSEEK_FORCE 0x20000
#endif

typedef int64_t offset_t;