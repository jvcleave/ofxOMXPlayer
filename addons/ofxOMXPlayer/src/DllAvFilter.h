#pragma once
/*
 *      Copyright (C) 2005-2011 Team XBMC
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
#include "DllAvCodec.h"
#include "DllAvFormat.h"
#include "DllSwResample.h"

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

#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/avcodec.h>

}

//#include "threads/SingleLock.h"

class DllAvFilterInterface
{
public:
  virtual ~DllAvFilterInterface() {}
  virtual int avfilter_open(AVFilterContext **filter_ctx, AVFilter *filter, const char *inst_name)=0;
  virtual void avfilter_free(AVFilterContext *filter)=0;
  virtual void avfilter_graph_free(AVFilterGraph **graph)=0;
  virtual int avfilter_graph_create_filter(AVFilterContext **filt_ctx, AVFilter *filt, const char *name, const char *args, void *opaque, AVFilterGraph *graph_ctx)=0;
  virtual AVFilter *avfilter_get_by_name(const char *name)=0;
  virtual AVFilterGraph *avfilter_graph_alloc(void)=0;
  virtual AVFilterInOut *avfilter_inout_alloc()=0;
  virtual void avfilter_inout_free(AVFilterInOut **inout)=0;
  virtual int avfilter_graph_parse(AVFilterGraph *graph, const char *filters, AVFilterInOut **inputs, AVFilterInOut **outputs, void *log_ctx)=0;
  virtual int avfilter_graph_config(AVFilterGraph *graphctx, void *log_ctx)=0;
#if LIBAVFILTER_VERSION_INT < AV_VERSION_INT(3,0,0)
  virtual int av_vsrc_buffer_add_frame(AVFilterContext *buffer_filter, AVFrame *frame, int flags)=0;
#else
  virtual int av_buffersrc_add_frame(AVFilterContext *buffer_filter, AVFrame *frame, int flags)=0;
#endif
  virtual void avfilter_unref_buffer(AVFilterBufferRef *ref)=0;
  virtual int avfilter_link(AVFilterContext *src, unsigned srcpad, AVFilterContext *dst, unsigned dstpad)=0;
  virtual int av_buffersink_get_buffer_ref(AVFilterContext *buffer_sink, AVFilterBufferRef **bufref, int flags)=0;
  virtual AVBufferSinkParams *av_buffersink_params_alloc()=0;
  virtual int av_buffersink_poll_frame(AVFilterContext *ctx)=0;
};

// Use direct mapping
class DllAvFilter : public DllDynamic, DllAvFilterInterface
{
public:
  virtual ~DllAvFilter() {}
  virtual int avfilter_open(AVFilterContext **filter_ctx, AVFilter *filter, const char *inst_name)
  {
    return ::avfilter_open(filter_ctx, filter, inst_name);
  }
  virtual void avfilter_free(AVFilterContext *filter)
  {
    ::avfilter_free(filter);
  }
  virtual void avfilter_graph_free(AVFilterGraph **graph)
  {
    ::avfilter_graph_free(graph);
  }
  void avfilter_register_all()
  {
    ::avfilter_register_all();
  }
  virtual int avfilter_graph_create_filter(AVFilterContext **filt_ctx, AVFilter *filt, const char *name, const char *args, void *opaque, AVFilterGraph *graph_ctx) { return ::avfilter_graph_create_filter(filt_ctx, filt, name, args, opaque, graph_ctx); }
  virtual AVFilter *avfilter_get_by_name(const char *name) { return ::avfilter_get_by_name(name); }
  virtual AVFilterGraph *avfilter_graph_alloc() { return ::avfilter_graph_alloc(); }
  virtual AVFilterInOut *avfilter_inout_alloc()
  {
    return ::avfilter_inout_alloc();
  }
  virtual void avfilter_inout_free(AVFilterInOut **inout)
  {
    ::avfilter_inout_free(inout);
  }
  virtual int avfilter_graph_parse(AVFilterGraph *graph, const char *filters, AVFilterInOut **inputs, AVFilterInOut **outputs, void *log_ctx)
  {
    return ::avfilter_graph_parse(graph, filters, inputs, outputs, log_ctx);
  }
  virtual int avfilter_graph_config(AVFilterGraph *graphctx, void *log_ctx)
  {
    return ::avfilter_graph_config(graphctx, log_ctx);
  }
#if LIBAVFILTER_VERSION_INT < AV_VERSION_INT(3,0,0)
  virtual int av_vsrc_buffer_add_frame(AVFilterContext *buffer_filter, AVFrame *frame, int flags) { return ::av_vsrc_buffer_add_frame(buffer_filter, frame, flags); }
#else
  virtual int av_buffersrc_add_frame(AVFilterContext *buffer_filter, AVFrame* frame, int flags) { return ::av_buffersrc_add_frame(buffer_filter, frame, flags); }
#endif
  virtual void avfilter_unref_buffer(AVFilterBufferRef *ref) { ::avfilter_unref_buffer(ref); }
  virtual int avfilter_link(AVFilterContext *src, unsigned srcpad, AVFilterContext *dst, unsigned dstpad) { return ::avfilter_link(src, srcpad, dst, dstpad); }
  virtual int av_buffersink_get_buffer_ref(AVFilterContext *buffer_sink, AVFilterBufferRef **bufref, int flags) { return ::av_buffersink_get_buffer_ref(buffer_sink, bufref, flags); }
  virtual AVBufferSinkParams *av_buffersink_params_alloc() { return ::av_buffersink_params_alloc(); }
  virtual int av_buffersink_poll_frame(AVFilterContext *ctx) { return ::av_buffersink_poll_frame(ctx); }
  // DLL faking.
  virtual bool ResolveExports() { return true; }
  virtual bool Load() {
    //printf("DllAvFilter: Using libavfilter system library \n");
    return true;
  }
  virtual void Unload() {}
};
