#include "OMXReader.h"
#include "OMXClock.h"
#include "linux/XMemUtils.h"



bool OMXReader::g_abort = false;

OMXReader::OMXReader()
{
  m_open        = false;
  m_filename    = "";
  m_bAVI        = false;
  m_bMpeg       = false;
  m_pFile       = NULL;
  m_ioContext   = NULL;
  m_pFormatContext = NULL;
  m_eof           = false;
  m_iCurrentPts   = DVD_NOPTS_VALUE;

  for(int i = 0; i < MAX_STREAMS; i++)
    m_streams[i].extradata = NULL;

  ClearStreams();

  pthread_mutex_init(&m_lock, NULL);
}

OMXReader::~OMXReader()
{
  Close();

  pthread_mutex_destroy(&m_lock);
}

void OMXReader::Lock()
{
  pthread_mutex_lock(&m_lock);
}

void OMXReader::UnLock()
{
  pthread_mutex_unlock(&m_lock);
}

static int interrupt_cb(void *unused)
{
  if(OMXReader::g_abort)
    return 1;
  return 0;
}

static int dvd_file_read(void *h, uint8_t* buf, int size)
{
  if(interrupt_cb(NULL))
    return -1;

  XFILE::CFile *pFile = (XFILE::CFile *)h;
  return pFile->Read(buf, size);
}

static offset_t dvd_file_seek(void *h, offset_t pos, int whence)
{
  if(interrupt_cb(NULL))
    return -1;

  XFILE::CFile *pFile = (XFILE::CFile *)h;
  if(whence == AVSEEK_SIZE)
    return pFile->GetLength();
  else
    return pFile->Seek(pos, whence & ~AVSEEK_FORCE);
}

bool OMXReader::Open(std::string filename, bool dump_format)
{
  if (!m_dllAvUtil.Load() || !m_dllAvCodec.Load() || !m_dllAvFormat.Load())
    return false;
  
  m_iCurrentPts = DVD_NOPTS_VALUE;
  m_filename    = filename; 
  m_speed       = DVD_PLAYSPEED_NORMAL;
  m_program     = UINT_MAX;
  const AVIOInterruptCB int_cb = { interrupt_cb, NULL };

  ClearStreams();

  m_dllAvFormat.av_register_all();
  m_dllAvFormat.avformat_network_init();
  m_dllAvUtil.av_log_set_level(AV_LOG_QUIET);

  int           result    = -1;
  AVInputFormat *iformat  = NULL;
  unsigned char *buffer   = NULL;
  unsigned int  flags     = READ_TRUNCATED | READ_BITRATE | READ_CHUNKED;
	m_pFile = new CFile();
	
	if (!m_pFile->Open(m_filename, flags))
	{
		printf("\nCOMXPlayer::OpenFile - %s ", m_filename.c_str());
		Close();
		return false;
	}
	
	buffer = (unsigned char*)m_dllAvUtil.av_malloc(FFMPEG_FILE_BUFFER_SIZE);
	m_ioContext = m_dllAvFormat.avio_alloc_context(buffer, FFMPEG_FILE_BUFFER_SIZE, 0, m_pFile, dvd_file_read, NULL, dvd_file_seek);
	m_ioContext->max_packet_size = 6144;
	if(m_ioContext->max_packet_size)
		m_ioContext->max_packet_size *= FFMPEG_FILE_BUFFER_SIZE / m_ioContext->max_packet_size;
	
	if(m_pFile->IoControl(IOCTRL_SEEK_POSSIBLE, NULL) == 0)
		m_ioContext->seekable = 0;
	
	m_dllAvFormat.av_probe_input_buffer(m_ioContext, &iformat, m_filename.c_str(), NULL, 0, 0);
	
	if(!iformat)
	{
		printf("\nCOMXPlayer::OpenFile - av_probe_input_buffer %s ", m_filename.c_str());
		Close();
		return false;
	}
	
	m_pFormatContext     = m_dllAvFormat.avformat_alloc_context();
	m_pFormatContext->pb = m_ioContext;
	result = m_dllAvFormat.avformat_open_input(&m_pFormatContext, m_filename.c_str(), iformat, NULL);
	if(result < 0)
	{
		Close();
		return false;
	}

  // set the interrupt callback, appeared in libavformat 53.15.0
  m_pFormatContext->interrupt_callback = int_cb;

  m_bAVI = strcmp(m_pFormatContext->iformat->name, "avi") == 0;
  m_bMpeg = strcmp(m_pFormatContext->iformat->name, "mpeg") == 0;

  // if format can be nonblocking, let's use that
  m_pFormatContext->flags |= AVFMT_FLAG_NONBLOCK;

  // analyse very short to speed up mjpeg playback start
  if (iformat && (strcmp(iformat->name, "mjpeg") == 0) && m_ioContext->seekable == 0)
    m_pFormatContext->max_analyze_duration = 500000;

  result = m_dllAvFormat.avformat_find_stream_info(m_pFormatContext, NULL);
  if(result < 0)
  {
    Close();
    return false;
  }

  if(!GetStreams())
  {
    Close();
    return false;
  }

  if(m_pFile)
  {
    int64_t len = m_pFile->GetLength();
    int64_t tim = GetStreamLength();

    if(len > 0 && tim > 0)
    {
      unsigned rate = len * 1000 / tim;
      unsigned maxrate = rate + 1024 * 1024 / 8;
      if(m_pFile->IoControl(IOCTRL_CACHE_SETRATE, &maxrate) >= 0)
        printf("\nCOMXPlayer::OpenFile - set cache throttle rate to %u bytes per second", maxrate);
    }
  }

  /*printf("file : %s result %d format %s audio streams %d video streams %d chapters %d subtitles %d length %d\n", 
      m_filename.c_str(), result, m_pFormatContext->iformat->name, m_audio_count, m_video_count, m_chapter_count, m_subtitle_count, GetStreamLength() / 1000);
*/

  m_speed       = DVD_PLAYSPEED_NORMAL;

  if(dump_format)
    m_dllAvFormat.av_dump_format(m_pFormatContext, 0, m_filename.c_str(), 0);

  UpdateCurrentPTS();

  m_open        = true;

  return true;
}

void OMXReader::ClearStreams()
{
  m_audio_index     = -1;
  m_video_index     = -1;

  m_audio_count     = 0;
  m_video_count     = 0;

  for(int i = 0; i < MAX_STREAMS; i++)
  {
    if(m_streams[i].extradata)
      free(m_streams[i].extradata);

    memset(m_streams[i].language, 0, sizeof(m_streams[i].language));
    m_streams[i].codec_name = "";
    m_streams[i].name       = "";
    m_streams[i].type       = OMXSTREAM_NONE;
    m_streams[i].stream     = NULL;
    m_streams[i].extradata  = NULL;
    m_streams[i].extrasize  = 0;
    m_streams[i].index      = 0;
    m_streams[i].id         = 0;
  }

  m_program     = UINT_MAX;
}

bool OMXReader::Close()
{
  if (m_pFormatContext)
  {
    if (m_ioContext && m_pFormatContext->pb && m_pFormatContext->pb != m_ioContext)
    {
      printf("\nCDVDDemuxFFmpeg::Dispose - demuxer changed our byte context behind our back, possible memleak");
      m_ioContext = m_pFormatContext->pb;
    }
    m_dllAvFormat.avformat_close_input(&m_pFormatContext);
  }

  if(m_ioContext)
  {
    m_dllAvUtil.av_free(m_ioContext->buffer);
    m_dllAvUtil.av_free(m_ioContext);
  }
  
  m_ioContext       = NULL;
  m_pFormatContext  = NULL;

  if(m_pFile)
  {
    m_pFile->Close();
    delete m_pFile;
    m_pFile = NULL;
  }

  m_dllAvFormat.avformat_network_deinit();

  m_dllAvUtil.Unload();
  m_dllAvCodec.Unload();
  m_dllAvFormat.Unload();

  m_open            = false;
  m_filename        = "";
  m_bAVI            = false;
  m_bMpeg           = false;
  m_video_count     = 0;
  m_audio_count     = 0;
  m_audio_index     = -1;
  m_video_index     = -1;
  m_eof             = false;
  m_iCurrentPts     = DVD_NOPTS_VALUE;
  m_speed           = DVD_PLAYSPEED_NORMAL;

  ClearStreams();

  return true;
}

bool OMXReader::SeekTime(int64_t seek_ms, int seek_flags, double *startpts)
{
  if(seek_ms < 0)
    seek_ms = 0;

  if(!m_pFile || !m_pFormatContext)
    return false;

  if(!m_pFile->IoControl(IOCTRL_SEEK_POSSIBLE, NULL))
    return false;

  Lock();

  //FlushRead();

  if(m_ioContext)
    m_ioContext->buf_ptr = m_ioContext->buf_end;

  int64_t seek_pts = (int64_t)seek_ms * (AV_TIME_BASE / 1000);
  if (m_pFormatContext->start_time != (int64_t)AV_NOPTS_VALUE)
    seek_pts += m_pFormatContext->start_time;

  // seek behind eof 
	ofLogVerbose() << "GetStreamLength(): " << GetStreamLength();
	ofLogVerbose() << "(seek_pts / AV_TIME_BASE): " << (seek_pts / AV_TIME_BASE);

  if((seek_pts / AV_TIME_BASE) > (GetStreamLength()  / 1000))
  {
    m_eof = true;
    UnLock();
    return true;
  }

  int ret = m_dllAvFormat.av_seek_frame(m_pFormatContext, -1, seek_pts, seek_flags ? AVSEEK_FLAG_BACKWARD : 0);

  if(ret >= 0)
  {
    UpdateCurrentPTS();
    m_eof = false;
  }

  if(m_iCurrentPts == DVD_NOPTS_VALUE)
  {
    ofLogVerbose() << "OMXReader::SeekTime - unknown position after seek";
  }
  else
  {
    ofLog(OF_LOG_VERBOSE, "\nOMXReader::SeekTime - seek ended up on time %d",(int)(m_iCurrentPts / DVD_TIME_BASE * 1000));
  }

  if(startpts)
    *startpts = DVD_MSEC_TO_TIME(seek_ms);

  UnLock();

  return (ret >= 0);
}

AVMediaType OMXReader::PacketType(OMXPacket *pkt)
{
  if(!m_pFormatContext || !pkt)
    return AVMEDIA_TYPE_UNKNOWN;

  return m_pFormatContext->streams[pkt->stream_index]->codec->codec_type;
}

OMXPacket *OMXReader::Read(bool doRestart)
{
  //assert(!IsEof());
  if(IsEof())
  {
	  if (doRestart)
	  {
		  m_eof = false;
		  Open(m_filename, false);
		  return NULL;
	  }
	 
	  //SeekTime(1, AVSEEK_FLAG_BACKWARD, NULL);
	  /*switch (currentLoopState) 
	   {
	   case OF_LOOP_NORMAL:
	   {
	   ofLogVerbose() << "OMXReader::Read Applying OF_LOOP_NORMAL";
	   m_eof = false;
	   SeekTime(0.0, AVSEEK_FLAG_BACKWARD, NULL);
	   break; 
	   }
	   case OF_LOOP_PALINDROME:
	   {
	   ofLogVerbose() << "OMXReader::Read OF_LOOP_PALINDROME Not Implemented";
	   break;
	   }
	   case OF_LOOP_NONE:
	   {
	   break;
	   }
	   default:
	   {
	   break;
	   }  
	   }*/
  }
  AVPacket  pkt;
  OMXPacket *m_omx_pkt = NULL;
  int       result = -1;

  if(!m_pFormatContext)
    return NULL;

  Lock();

  // assume we are not eof
  if(m_pFormatContext->pb)
    m_pFormatContext->pb->eof_reached = 0;

  // keep track if ffmpeg doesn't always set these
  pkt.size = 0;
  pkt.data = NULL;
  pkt.stream_index = MAX_OMX_STREAMS;

  result = m_dllAvFormat.av_read_frame(m_pFormatContext, &pkt);
  if (result < 0)
  {
    m_eof = true;
    //FlushRead();
    //m_dllAvCodec.av_free_packet(&pkt);
    UnLock();
    return NULL;
  }
  else if (pkt.size < 0 || pkt.stream_index >= MAX_OMX_STREAMS)
  {
    // XXX, in some cases ffmpeg returns a negative packet size
    if(m_pFormatContext->pb && !m_pFormatContext->pb->eof_reached)
    {
      printf("\nOMXReader::Read no valid packet");
      //FlushRead();
    }

    m_dllAvCodec.av_free_packet(&pkt);

    m_eof = true;
    UnLock();
    return NULL;
  }

  AVStream *pStream = m_pFormatContext->streams[pkt.stream_index];


  // lavf sometimes bugs out and gives 0 dts/pts instead of no dts/pts
  // since this could only happens on initial frame under normal
  // circomstances, let's assume it is wrong all the time
  if(pkt.dts == 0)
    pkt.dts = AV_NOPTS_VALUE;
  if(pkt.pts == 0)
    pkt.pts = AV_NOPTS_VALUE;


  if(m_bAVI && pStream->codec && pStream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
  {
    // AVI's always have borked pts, specially if m_pFormatContext->flags includes
    // AVFMT_FLAG_GENPTS so always use dts
    pkt.pts = AV_NOPTS_VALUE;
  }

  m_omx_pkt = AllocPacket(pkt.size);
  /* oom error allocation av packet */
  if(!m_omx_pkt)
  {
    m_eof = true;
    m_dllAvCodec.av_free_packet(&pkt);
    UnLock();
    return NULL;
  }

  m_omx_pkt->codec_type = pStream->codec->codec_type;

  /* copy content into our own packet */
  m_omx_pkt->size = pkt.size;

  if (pkt.data)
    memcpy(m_omx_pkt->data, pkt.data, m_omx_pkt->size);

  m_omx_pkt->stream_index = pkt.stream_index;
  GetHints(pStream, &m_omx_pkt->hints);

  m_omx_pkt->dts = ConvertTimestamp(pkt.dts, &pStream->time_base);
  m_omx_pkt->pts = ConvertTimestamp(pkt.pts, &pStream->time_base);
  m_omx_pkt->duration = DVD_SEC_TO_TIME((double)pkt.duration * pStream->time_base.num / pStream->time_base.den);

  // used to guess streamlength
  if (m_omx_pkt->dts != DVD_NOPTS_VALUE && (m_omx_pkt->dts > m_iCurrentPts || m_iCurrentPts == DVD_NOPTS_VALUE))
    m_iCurrentPts = m_omx_pkt->dts;

  // check if stream has passed full duration, needed for live streams
  if(pkt.dts != (int64_t)AV_NOPTS_VALUE)
  {
    int64_t duration;
    duration = pkt.dts;
    if(pStream->start_time != (int64_t)AV_NOPTS_VALUE)
      duration -= pStream->start_time;

    if(duration > pStream->duration)
    {
      pStream->duration = duration;
      duration = m_dllAvUtil.av_rescale_rnd(pStream->duration, (int64_t)pStream->time_base.num * AV_TIME_BASE, 
                                            pStream->time_base.den, AV_ROUND_NEAR_INF);
      if ((m_pFormatContext->duration == (int64_t)AV_NOPTS_VALUE)
          ||  (m_pFormatContext->duration != (int64_t)AV_NOPTS_VALUE && duration > m_pFormatContext->duration))
        m_pFormatContext->duration = duration;
    }
  }

  m_dllAvCodec.av_free_packet(&pkt);

  UnLock();
  return m_omx_pkt;
}

bool OMXReader::GetStreams()
{
  if(!m_pFormatContext)
    return false;

  unsigned int    m_program         = UINT_MAX;

  ClearStreams();

  if (m_pFormatContext->nb_programs)
  {
    // look for first non empty stream and discard nonselected programs
    for (unsigned int i = 0; i < m_pFormatContext->nb_programs; i++)
    {
      if(m_program == UINT_MAX && m_pFormatContext->programs[i]->nb_stream_indexes > 0)
        m_program = i;

      if(i != m_program)
        m_pFormatContext->programs[i]->discard = AVDISCARD_ALL;
    }
      if(m_program != UINT_MAX)
      {
        // add streams from selected program
        for (unsigned int i = 0; i < m_pFormatContext->programs[m_program]->nb_stream_indexes; i++)
          AddStream(m_pFormatContext->programs[m_program]->stream_index[i]);
      }
    }

  // if there were no programs or they were all empty, add all streams
  if (m_program == UINT_MAX)
  {
    for (unsigned int i = 0; i < m_pFormatContext->nb_streams; i++)
      AddStream(i);
  }

  if(m_video_count)
    SetActiveStreamInternal(OMXSTREAM_VIDEO, 0);

  if(m_audio_count)
    SetActiveStreamInternal(OMXSTREAM_AUDIO, 0);


  return true;
}

void OMXReader::AddStream(int id)
{
  if(id > MAX_STREAMS || !m_pFormatContext)
    return;

  AVStream *pStream = m_pFormatContext->streams[id];
  // discard PNG stream as we don't support it, and it stops mp3 files playing with album art
  if (pStream->codec->codec_type == AVMEDIA_TYPE_VIDEO && 
    (pStream->codec->codec_id == CODEC_ID_PNG))
    return;

  switch (pStream->codec->codec_type)
  {
    case AVMEDIA_TYPE_AUDIO:
      m_streams[id].stream      = pStream;
      m_streams[id].type        = OMXSTREAM_AUDIO;
      m_streams[id].index       = m_audio_count;
      m_streams[id].codec_name  = GetStreamCodecName(pStream);
      m_streams[id].id          = id;
      m_audio_count++;
      GetHints(pStream, &m_streams[id].hints);
      break;
    case AVMEDIA_TYPE_VIDEO:
      m_streams[id].stream      = pStream;
      m_streams[id].type        = OMXSTREAM_VIDEO;
      m_streams[id].index       = m_video_count;
      m_streams[id].codec_name  = GetStreamCodecName(pStream);
      m_streams[id].id          = id;
      m_video_count++;
      GetHints(pStream, &m_streams[id].hints);
      break;
    default:
      return;
  }

  m_streams[id].name = "whogivesashit";


  if( pStream->codec->extradata && pStream->codec->extradata_size > 0 )
  {
    m_streams[id].extrasize = pStream->codec->extradata_size;
    m_streams[id].extradata = malloc(pStream->codec->extradata_size);
    memcpy(m_streams[id].extradata, pStream->codec->extradata, pStream->codec->extradata_size);
  }
}

bool OMXReader::SetActiveStreamInternal(OMXStreamType type, unsigned int index)
{
  bool ret = false;

  switch(type)
  {
    case OMXSTREAM_AUDIO:
      if((int)index > (m_audio_count - 1))
        index = (m_audio_count - 1);
      break;
    case OMXSTREAM_VIDEO:
      if((int)index > (m_video_count - 1))
        index = (m_video_count - 1);
      break;
    default:
      break;
  }

  for(int i = 0; i < MAX_STREAMS; i++)
  {
    if(m_streams[i].type == type &&  m_streams[i].index == index)
    {
      switch(m_streams[i].type)
      {
        case OMXSTREAM_AUDIO:
          m_audio_index = i;
          ret = true;
          break;
        case OMXSTREAM_VIDEO:
          m_video_index = i;
          ret = true;
          break;
        default:
          break;
      }
    }
  }

  if(!ret)
  {
    switch(type)
    {
      case OMXSTREAM_AUDIO:
        m_audio_index = -1;
        break;
      case OMXSTREAM_VIDEO:
        m_video_index = -1;
        break;
      default:
        break;
    }
  }

  return ret;
}

bool OMXReader::IsActive(int stream_index)
{
  if((m_audio_index != -1)    && m_streams[m_audio_index].id      == stream_index)
    return true;
  if((m_video_index != -1)    && m_streams[m_video_index].id      == stream_index)
    return true;

  return false;
}

bool OMXReader::IsActive(OMXStreamType type, int stream_index)
{
  if((m_audio_index != -1)    && m_streams[m_audio_index].id      == stream_index && m_streams[m_audio_index].type == type)
    return true;
  if((m_video_index != -1)    && m_streams[m_video_index].id      == stream_index && m_streams[m_video_index].type == type)
    return true;

  return false;
}

bool OMXReader::GetHints(AVStream *stream, COMXStreamInfo *hints)
{
  if(!hints || !stream)
    return false;

  hints->codec         = stream->codec->codec_id;
  hints->extradata     = stream->codec->extradata;
  hints->extrasize     = stream->codec->extradata_size;

  hints->width         = stream->codec->width;
  hints->height        = stream->codec->height;
  hints->profile       = stream->codec->profile;
  hints->duration       = stream->duration;
	hints->nb_frames = stream->nb_frames;
  if(stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
  {
    hints->fpsrate       = stream->r_frame_rate.num;
    hints->fpsscale      = stream->r_frame_rate.den;

    if(stream->r_frame_rate.num && stream->r_frame_rate.den)
    {
      hints->fpsrate      = stream->r_frame_rate.num;
      hints->fpsscale     = stream->r_frame_rate.den;
    }
    else
    {
      hints->fpsscale     = 0;
      hints->fpsrate      = 0;
    }

    if (stream->sample_aspect_ratio.num != 0)
      hints->aspect = av_q2d(stream->sample_aspect_ratio) * stream->codec->width / stream->codec->height;
    else if (stream->codec->sample_aspect_ratio.num != 0)
      hints->aspect = av_q2d(stream->codec->sample_aspect_ratio) * stream->codec->width / stream->codec->height;
    else
      hints->aspect = 0.0f;
    if (m_bAVI && stream->codec->codec_id == CODEC_ID_H264)
      hints->ptsinvalid = true;
  }

  return true;
}

bool OMXReader::GetHints(OMXStreamType type, unsigned int index, COMXStreamInfo &hints)
{
  for(unsigned int i = 0; i < MAX_STREAMS; i++)
  {
    if(m_streams[i].type == type && m_streams[i].index == i)
    {
      hints = m_streams[i].hints;
      return true;
    }
  }

  return false;
}

bool OMXReader::GetHints(OMXStreamType type, COMXStreamInfo &hints)
{
  bool ret = false;

  switch (type)
  {
    case OMXSTREAM_AUDIO:
      if(m_audio_index != -1)
      {
        ret = true;
        hints = m_streams[m_audio_index].hints;
      }
      break;
    case OMXSTREAM_VIDEO:
      if(m_video_index != -1)
      {
        ret = true;
        hints = m_streams[m_video_index].hints;
      }
      break;
    default:
      break;
  }

  return ret;
}

bool OMXReader::IsEof()
{
  return m_eof;
}

void OMXReader::FreePacket(OMXPacket *pkt)
{
  if(pkt)
  {
    if(pkt->data)
      free(pkt->data);
    free(pkt);
  }
}

OMXPacket *OMXReader::AllocPacket(int size)
{
  OMXPacket *pkt = (OMXPacket *)malloc(sizeof(OMXPacket));
  if(pkt)
  {
    memset(pkt, 0, sizeof(OMXPacket));

    pkt->data = (uint8_t*) malloc(size + FF_INPUT_BUFFER_PADDING_SIZE);
    if(!pkt->data)
    {
      free(pkt);
      pkt = NULL;
    }
    else
    {
      memset(pkt->data + size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
      pkt->size = size;
      pkt->dts  = DVD_NOPTS_VALUE;
      pkt->pts  = DVD_NOPTS_VALUE;
      pkt->now  = DVD_NOPTS_VALUE;
      pkt->duration = DVD_NOPTS_VALUE;
    }
  }
  return pkt;
}

bool OMXReader::SetActiveStream(OMXStreamType type, unsigned int index)
{
  bool ret = false;
  Lock();
  ret = SetActiveStreamInternal(type, index);
  UnLock();
  return ret;
}

double OMXReader::ConvertTimestamp(int64_t pts, int den, int num)
{
  if(m_pFormatContext == NULL)
    return false;

  if (pts == (int64_t)AV_NOPTS_VALUE)
    return DVD_NOPTS_VALUE;

  // do calculations in floats as they can easily overflow otherwise
  // we don't care for having a completly exact timestamp anyway
  double timestamp = (double)pts * num  / den;
  double starttime = 0.0f;

  if (m_pFormatContext->start_time != (int64_t)AV_NOPTS_VALUE)
    starttime = (double)m_pFormatContext->start_time / AV_TIME_BASE;

  if(timestamp > starttime)
    timestamp -= starttime;
  else if( timestamp + 0.1f > starttime )
    timestamp = 0;

  return timestamp*DVD_TIME_BASE;
}

double OMXReader::ConvertTimestamp(int64_t pts, AVRational *time_base)
{
  double new_pts = pts;

  if(m_pFormatContext == NULL)
    return false;

  if (pts == (int64_t)AV_NOPTS_VALUE)
    return DVD_NOPTS_VALUE;

  if (m_pFormatContext->start_time != (int64_t)AV_NOPTS_VALUE)
    new_pts += m_pFormatContext->start_time;

  new_pts *= av_q2d(*time_base);

  new_pts *= (double)DVD_TIME_BASE;

  return new_pts;
}


void OMXReader::UpdateCurrentPTS()
{
  m_iCurrentPts = DVD_NOPTS_VALUE;
  for(unsigned int i = 0; i < m_pFormatContext->nb_streams; i++)
  {
    AVStream *stream = m_pFormatContext->streams[i];
	  if (stream) 
	  {
		  ofLogVerbose() << "stream->cur_dts: " << stream->cur_dts;
		  ofLogVerbose() << "stream->first_dts: " <<  stream->first_dts;
	  }
    if(stream && stream->cur_dts != (int64_t)AV_NOPTS_VALUE)
    {
      //double ts = ConvertTimestamp(stream->cur_dts, stream->time_base.den, stream->time_base.num);
      double ts = ConvertTimestamp(stream->cur_dts, &stream->time_base);
      if(m_iCurrentPts == DVD_NOPTS_VALUE || m_iCurrentPts > ts )
        m_iCurrentPts = ts;
    }
  }
}

void OMXReader::SetSpeed(int iSpeed)
{
  if(!m_pFormatContext)
    return;

  if(m_speed != DVD_PLAYSPEED_PAUSE && iSpeed == DVD_PLAYSPEED_PAUSE)
  {
    m_dllAvFormat.av_read_pause(m_pFormatContext);
  }
  else if(m_speed == DVD_PLAYSPEED_PAUSE && iSpeed != DVD_PLAYSPEED_PAUSE)
  {
    m_dllAvFormat.av_read_play(m_pFormatContext);
  }
  m_speed = iSpeed;

  AVDiscard discard = AVDISCARD_NONE;
  if(m_speed > 4*DVD_PLAYSPEED_NORMAL)
    discard = AVDISCARD_NONKEY;
  else if(m_speed > 2*DVD_PLAYSPEED_NORMAL)
    discard = AVDISCARD_BIDIR;
  else if(m_speed < DVD_PLAYSPEED_PAUSE)
    discard = AVDISCARD_NONKEY;

  for(unsigned int i = 0; i < m_pFormatContext->nb_streams; i++)
  {
    if(m_pFormatContext->streams[i])
    {
      if(m_pFormatContext->streams[i]->discard != AVDISCARD_ALL)
        m_pFormatContext->streams[i]->discard = discard;
    }
  }
}

int OMXReader::GetStreamLength()
{
  if (!m_pFormatContext)
    return 0;

  return (int)(m_pFormatContext->duration / (AV_TIME_BASE / 1000));
}

double OMXReader::NormalizeFrameduration(double frameduration)
{
  //if the duration is within 20 microseconds of a common duration, use that
  const double durations[] = {DVD_TIME_BASE * 1.001 / 24.0, DVD_TIME_BASE / 24.0, DVD_TIME_BASE / 25.0,
                              DVD_TIME_BASE * 1.001 / 30.0, DVD_TIME_BASE / 30.0, DVD_TIME_BASE / 50.0,
                              DVD_TIME_BASE * 1.001 / 60.0, DVD_TIME_BASE / 60.0};

  double lowestdiff = DVD_TIME_BASE;
  int    selected   = -1;
  for (size_t i = 0; i < sizeof(durations) / sizeof(durations[0]); i++)
  {
    double diff = fabs(frameduration - durations[i]);
    if (diff < DVD_MSEC_TO_TIME(0.02) && diff < lowestdiff)
    {
      selected = i;
      lowestdiff = diff;
    }
  }

  if (selected != -1)
    return durations[selected];
  else
    return frameduration;
}

std::string OMXReader::GetStreamCodecName(AVStream *stream)
{
  std::string strStreamName = "";

  if(!stream)
    return strStreamName;

  unsigned int in = stream->codec->codec_tag;
  // FourCC codes are only valid on video streams, audio codecs in AVI/WAV
  // are 2 bytes and audio codecs in transport streams have subtle variation
  // e.g AC-3 instead of ac3
  if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO && in != 0)
  {
    char fourcc[5];
    memcpy(fourcc, &in, 4);
    fourcc[4] = 0;
    // fourccs have to be 4 characters
    if (strlen(fourcc) == 4)
    {
      strStreamName = fourcc;
      return strStreamName;
    }
  }

#ifdef FF_PROFILE_DTS_HD_MA
  /* use profile to determine the DTS type */
  if (stream->codec->codec_id == CODEC_ID_DTS)
  {
    if (stream->codec->profile == FF_PROFILE_DTS_HD_MA)
      strStreamName = "dtshd_ma";
    else if (stream->codec->profile == FF_PROFILE_DTS_HD_HRA)
      strStreamName = "dtshd_hra";
    else
      strStreamName = "dca";
    return strStreamName;
  }
#endif

  AVCodec *codec = m_dllAvCodec.avcodec_find_decoder(stream->codec->codec_id);

  if (codec)
    strStreamName = codec->name;

  return strStreamName;
}