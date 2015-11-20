#include "OMXReader.h"
#include "OMXClock.h"
#include "XMemUtils.h"



static bool g_abort = false;

OMXReader::OMXReader()
{
	isOpen        = false;
	fileName    = "";
	isMatroska   = false;
	isAVI        = false;
	g_abort       = false;
	fileObject       = NULL;
	avioContext   = NULL;
	avFormatContext = NULL;
	isEOF           = false;
	chapterCount = 0;
	currentPTS   = DVD_NOPTS_VALUE;
	
	for(int i = 0; i < MAX_STREAMS; i++)
		omxStreams[i].extradata = NULL;
	
	ClearStreams();
	
	pthread_mutex_init(&m_lock, NULL);
}

OMXReader::~OMXReader()
{
	close();
	
	pthread_mutex_destroy(&m_lock);
}

void OMXReader::lock()
{
	pthread_mutex_lock(&m_lock);
}

void OMXReader::unlock()
{
	pthread_mutex_unlock(&m_lock);
}

static int interrupt_cb(void *unused)
{
	if(g_abort)
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

bool OMXReader::open(std::string filename, bool doSkipAvProbe)
{
	
	currentPTS = DVD_NOPTS_VALUE;
	fileName    = filename; 
	speed       = DVD_PLAYSPEED_NORMAL;
	programID     = UINT_MAX;
	AVIOInterruptCB int_cb = { interrupt_cb, NULL };
	
    ClearStreams();

	
	int           result    = -1;
	AVInputFormat *iformat  = NULL;
	unsigned char *buffer   = NULL;
	unsigned int  flags     = READ_TRUNCATED | READ_BITRATE | READ_CHUNKED;
	
	if(fileName.substr(0, 8) == "shout://" )
		fileName.replace(0, 8, "http://");
	
	if(fileName.substr(0,6) == "mms://" || fileName.substr(0,7) == "mmsh://" || fileName.substr(0,7) == "mmst://" || fileName.substr(0,7) == "mmsu://" ||
	   fileName.substr(0,7) == "http://" || 
	   fileName.substr(0,7) == "rtmp://" || fileName.substr(0,6) == "udp://" ||
	   fileName.substr(0,7) == "rtsp://" )
	{
        doSkipAvProbe = false;
		// ffmpeg dislikes the useragent from AirPlay urls
		//int idx = fileName.Find("|User-Agent=AppleCoreMedia");
		size_t idx = fileName.find("|");
		if(idx != string::npos)
			fileName = fileName.substr(0, idx);
		
		AVDictionary *d = NULL;
		// Enable seeking if http
		if(fileName.substr(0,7) == "http://")
		{
			av_dict_set(&d, "seekable", "1", 0);
		}
		ofLog(OF_LOG_VERBOSE, "OMXPlayer::OpenFile - avformat_open_input %s ", fileName.c_str());
		result = avformat_open_input(&avFormatContext, fileName.c_str(), iformat, &d);
		if(av_dict_count(d) == 0)
		{
			ofLog(OF_LOG_VERBOSE, "OMXPlayer::OpenFile - avformat_open_input enabled SEEKING ");
			if(fileName.substr(0,7) == "http://")
				avFormatContext->pb->seekable = AVIO_SEEKABLE_NORMAL;
		}
		av_dict_free(&d);
		if(result < 0)
		{
			ofLog(OF_LOG_ERROR, "OMXPlayer::OpenFile - avformat_open_input %s ", fileName.c_str());
			close();
			return false;
		}
	}
	else
	{
		fileObject = new CFile();
		
		if (!fileObject->open(fileName, flags))
		{
			ofLog(OF_LOG_ERROR, "OMXPlayer::OpenFile - %s ", fileName.c_str());
			close();
			return false;
		}
        
        buffer = (unsigned char*)av_malloc(FFMPEG_FILE_BUFFER_SIZE);
  
		avioContext = avio_alloc_context(buffer, FFMPEG_FILE_BUFFER_SIZE, 0, fileObject, dvd_file_read, NULL, dvd_file_seek);
		avioContext->max_packet_size = 6144;
		if(avioContext->max_packet_size)
			avioContext->max_packet_size *= FFMPEG_FILE_BUFFER_SIZE / avioContext->max_packet_size;
		
		if(fileObject->IoControl(IOCTRL_SEEK_POSSIBLE, NULL) == 0)
			avioContext->seekable = 0;
		
        
		av_probe_input_buffer(avioContext, &iformat, fileName.c_str(), NULL, 0, 0);
		
		if(!iformat)
		{
			ofLog(OF_LOG_ERROR, "OMXPlayer::OpenFile - av_probe_input_buffer %s ", fileName.c_str());
			close();
			return false;
		}
        
        //#warning experimental
        //iformat->flags |= AVFMT_SEEK_TO_PTS;
		avFormatContext     = avformat_alloc_context();
		avFormatContext->pb = avioContext;
        
        
        
        
		result = avformat_open_input(&avFormatContext, fileName.c_str(), iformat, NULL);
		if(result < 0)
		{
			close();
			return false;
		}
	}
	
	// set the interrupt callback, appeared in libavformat 53.15.0
	avFormatContext->interrupt_callback = int_cb;
	
	isMatroska = strncmp(avFormatContext->iformat->name, "matroska", 8) == 0; // for "matroska.webm"
	isAVI = strcmp(avFormatContext->iformat->name, "avi") == 0;
	
	// if format can be nonblocking, let's use that
	avFormatContext->flags |= AVFMT_FLAG_NONBLOCK;
	
	// analyse very short to speed up mjpeg playback start
	if (iformat && (strcmp(iformat->name, "mjpeg") == 0) && avioContext->seekable == 0)
		avFormatContext->max_analyze_duration = 500000;
	
	if(/*isAVI || */isMatroska)
		avFormatContext->max_analyze_duration = 0;
	
    
    if(!doSkipAvProbe)
    {
        unsigned long long startTime = ofGetElapsedTimeMillis();
        result = avformat_find_stream_info(avFormatContext, NULL);
        unsigned long long endTime = ofGetElapsedTimeMillis();
        ofLogNotice(__func__) << "avformat_find_stream_info TOOK " << endTime-startTime <<  " MS";
        
        
        
        if(result < 0)
        {
            close();
            return false;
        }
    }
    
	if(!GetStreams())
	{
		close();
		return false;
	}
	
	if(fileObject)
	{
		int64_t len = fileObject->GetLength();
		int64_t tim = GetStreamLength();
		
		if(len > 0 && tim > 0)
		{
			unsigned rate = len * 1000 / tim;
			unsigned maxrate = rate + 1024 * 1024 / 8;
			if(fileObject->IoControl(IOCTRL_CACHE_SETRATE, &maxrate) >= 0)
				ofLog(OF_LOG_VERBOSE, "OMXPlayer::OpenFile - set cache throttle rate to %u bytes per second", maxrate);
		}
	}
	
	speed       = DVD_PLAYSPEED_NORMAL;
	/*
	if(dump_format)
		av_dump_format(avFormatContext, 0, fileName.c_str(), 0);*/
	
	UpdateCurrentPTS();
	
	isOpen        = true;
	
	return true;
}

void OMXReader::ClearStreams()
{
	audioIndex     = -1;
	videoIndex     = -1;
	subtitleIndex  = -1;
	
	audioCount     = 0;
	videoCount     = 0;
	subtitleCount  = 0;
	
	for(int i = 0; i < MAX_STREAMS; i++)
	{
		if(omxStreams[i].extradata)
			free(omxStreams[i].extradata);
		
		memset(omxStreams[i].language, 0, sizeof(omxStreams[i].language));
		omxStreams[i].codec_name = "";
		omxStreams[i].name       = "";
		omxStreams[i].type       = OMXSTREAM_NONE;
		omxStreams[i].stream     = NULL;
		omxStreams[i].extradata  = NULL;
		omxStreams[i].extrasize  = 0;
		omxStreams[i].index      = 0;
		omxStreams[i].id         = 0;
	}
	
	programID     = UINT_MAX;
}

bool OMXReader::close()
{
	if (avFormatContext)
	{
		if (avioContext && avFormatContext->pb && avFormatContext->pb != avioContext)
		{
			ofLog(OF_LOG_WARNING, "CDVDDemuxFFmpeg::Dispose - demuxer changed our byte context behind our back, possible memleak");
			avioContext = avFormatContext->pb;
		}
		avformat_close_input(&avFormatContext);
	}
	
	if(avioContext)
	{
		av_free(avioContext->buffer);
		av_free(avioContext);
	}
	
	avioContext       = NULL;
	avFormatContext  = NULL;
	
	if(fileObject)
	{
		fileObject->close();
		delete fileObject;
		fileObject = NULL;
	}
	
	avformat_network_deinit();
	
		
	isOpen            = false;
	fileName        = "";
	isMatroska       = false;
	isAVI            = false;
	videoCount     = 0;
	audioCount     = 0;
	subtitleCount  = 0;
	audioIndex     = -1;
	videoIndex     = -1;
	subtitleIndex  = -1;
	isEOF             = false;
	chapterCount   = 0;
	currentPTS     = DVD_NOPTS_VALUE;
	speed           = DVD_PLAYSPEED_NORMAL;
	
	ClearStreams();
	
	return true;
}

/*void OMXReader::FlushRead()
 {
 currentPTS = DVD_NOPTS_VALUE;
 
 if(!avFormatContext)
 return;
 
 ff_read_frame_flush(avFormatContext);
 }*/

bool OMXReader::SeekTime(int time, bool backwords, double *startpts, bool doLoopOnFail)
{
	if(time < 0)
		time = 0;
	
	if(!avFormatContext)
		return false;
	
	if(fileObject && !fileObject->IoControl(IOCTRL_SEEK_POSSIBLE, NULL))
	{
		ofLog(OF_LOG_VERBOSE, "%s - input stream reports it is not seekable", __FUNCTION__);
		return false;
	}
	
	lock();
	
	//FlushRead();
	
	if(avioContext)
    {
        avioContext->buf_ptr = avioContext->buf_end;
    }
	
	int64_t seek_pts = (int64_t)time * (AV_TIME_BASE / 1000);
	if (avFormatContext->start_time != (int64_t)AV_NOPTS_VALUE)
    {
        seek_pts += avFormatContext->start_time;
    };
	
	
	int ret = av_seek_frame(avFormatContext, -1, seek_pts, backwords ? AVSEEK_FLAG_BACKWARD : 0);
	
	if(ret >= 0)
	{
		UpdateCurrentPTS();
	}else {
		//ofLogVerbose(__func__) << "av_seek_frame returned >= 0, no UpdateCurrentPTS" << ret;
		fileObject->rewindFile();
	}

	
	// in this case the start time is requested time
	if(startpts)
    {
        *startpts = DVD_MSEC_TO_TIME(time);
    }
	
	isEOF = false;
	if (fileObject && fileObject->IsgetIsEOF() && ret <= 0)
	{
		isEOF = true;
		ret = 0;
	}

   
	unlock();

	return (ret >= 0);
}

AVMediaType OMXReader::PacketType(OMXPacket *pkt)
{
	if(!avFormatContext || !pkt)
		return AVMEDIA_TYPE_UNKNOWN;
	
	return avFormatContext->streams[pkt->stream_index]->codec->codec_type;
}

OMXPacket* OMXReader::Read()
{
	AVPacket  pkt;
	OMXPacket* omxPacket = NULL;
	int       result = -1;
	
	if(!avFormatContext)
		return NULL;
	
	lock();
	
	// assume we are not eof
	if(avFormatContext->pb)
		avFormatContext->pb->eof_reached = 0;
	
	// keep track if ffmpeg doesn't always set these
	pkt.size = 0;
	pkt.data = NULL;
	pkt.stream_index = MAX_OMX_STREAMS;
	
	result = av_read_frame(avFormatContext, &pkt);
	if (result < 0)
	{
		isEOF = true;
		unlock();
		return NULL;
	}
	else if (pkt.size < 0 || pkt.stream_index >= MAX_OMX_STREAMS)
	{
		// XXX, in some cases ffmpeg returns a negative packet size
		if(avFormatContext->pb && !avFormatContext->pb->eof_reached)
		{
			ofLog(OF_LOG_ERROR, "OMXReader::Read no valid packet");
			//FlushRead();
		}
		
		av_free_packet(&pkt);
		
		isEOF = true;
		unlock();
		return NULL;
	}
	
	AVStream *pStream = avFormatContext->streams[pkt.stream_index];
	
	/* only read packets for active streams */
	/*
	 if(!IsActive(pkt.stream_index))
	 {
	 av_free_packet(&pkt);
	 unlock();
	 return NULL;
	 }
	 */
	
	// lavf sometimes bugs out and gives 0 dts/pts instead of no dts/pts
	// since this could only happens on initial frame under normal
	// circomstances, let's assume it is wrong all the time
#if 0
	if(pkt.dts == 0)
		pkt.dts = AV_NOPTS_VALUE;
	if(pkt.pts == 0)
		pkt.pts = AV_NOPTS_VALUE;
#endif
	if(isMatroska && pStream->codec && pStream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
	{ // matroska can store different timestamps
		// for different formats, for native stored
		// stuff it is pts, but for ms compatibility
		// tracks, it is really dts. sadly ffmpeg
		// sets these two timestamps equal all the
		// time, so we select it here instead
		if(pStream->codec->codec_tag == 0)
			pkt.dts = AV_NOPTS_VALUE;
		else
			pkt.pts = AV_NOPTS_VALUE;
	}
	// we need to get duration slightly different for matroska embedded text subtitels
	if(isMatroska && pStream->codec->codec_id == AV_CODEC_ID_SUBRIP && pkt.convergence_duration != 0)
		pkt.duration = pkt.convergence_duration;
	
	if(isAVI && pStream->codec && pStream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
	{
		// AVI's always have borked pts, specially if avFormatContext->flags includes
		// AVFMT_FLAG_GENPTS so always use dts
		pkt.pts = AV_NOPTS_VALUE;
	}
	
	omxPacket = AllocPacket(pkt.size);
	/* oom error allocation av packet */
	if(!omxPacket)
	{
		isEOF = true;
		av_free_packet(&pkt);
		unlock();
		return NULL;
	}
	
	omxPacket->codec_type = pStream->codec->codec_type;
	
	/* copy content into our own packet */
	omxPacket->size = pkt.size;
	
	if (pkt.data)
		memcpy(omxPacket->data, pkt.data, omxPacket->size);
	
	omxPacket->stream_index = pkt.stream_index;
	getHints(pStream, &omxPacket->hints);
	
	omxPacket->dts = ConvertTimestamp(pkt.dts, pStream->time_base.den, pStream->time_base.num);
	omxPacket->pts = ConvertTimestamp(pkt.pts, pStream->time_base.den, pStream->time_base.num);
	omxPacket->duration = DVD_SEC_TO_TIME((double)pkt.duration * pStream->time_base.num / pStream->time_base.den);

	// used to guess streamlength
	if (omxPacket->dts != DVD_NOPTS_VALUE && (omxPacket->dts > currentPTS || currentPTS == DVD_NOPTS_VALUE))
		currentPTS = omxPacket->dts;
	
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
			duration = av_rescale_rnd(pStream->duration, (int64_t)pStream->time_base.num * AV_TIME_BASE, 
												  pStream->time_base.den, AV_ROUND_NEAR_INF);
			if ((avFormatContext->duration == (int64_t)AV_NOPTS_VALUE)
				||  (avFormatContext->duration != (int64_t)AV_NOPTS_VALUE && duration > avFormatContext->duration))
				avFormatContext->duration = duration;
		}
	}
	
	av_free_packet(&pkt);
	
	unlock();
	return omxPacket;
}

bool OMXReader::GetStreams()
{
	if(!avFormatContext)
		return false;
	
	unsigned int    programID         = UINT_MAX;
	
	ClearStreams();
	
	if (avFormatContext->nb_programs)
	{
		// look for first non empty stream and discard nonselected programs
		for (unsigned int i = 0; i < avFormatContext->nb_programs; i++)
		{
			if(programID == UINT_MAX && avFormatContext->programs[i]->nb_stream_indexes > 0)
				programID = i;
			
			if(i != programID)
				avFormatContext->programs[i]->discard = AVDISCARD_ALL;
		}
		if(programID != UINT_MAX)
		{
			// add streams from selected program
			for (unsigned int i = 0; i < avFormatContext->programs[programID]->nb_stream_indexes; i++)
				AddStream(avFormatContext->programs[programID]->stream_index[i]);
		}
    }
	
	// if there were no programs or they were all empty, add all streams
	if (programID == UINT_MAX)
	{
		for (unsigned int i = 0; i < avFormatContext->nb_streams; i++)
			AddStream(i);
	}
	
	if(videoCount)
		setActiveStreamInternal(OMXSTREAM_VIDEO, 0);
	
	if(audioCount)
		setActiveStreamInternal(OMXSTREAM_AUDIO, 0);
	
	if(subtitleCount)
		setActiveStreamInternal(OMXSTREAM_SUBTITLE, 0);
	
	int i = 0;
	for(i = 0; i < MAX_OMX_CHAPTERS; i++)
	{
		omxChapters[i].name      = "";
		omxChapters[i].seekto_ms = 0;
		omxChapters[i].ts        = 0;
	}
	
	chapterCount = 0;
	
	if(videoIndex != -1)
	{
		//m_current_chapter = 0;
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52,14,0)
		chapterCount = (avFormatContext->nb_chapters > MAX_OMX_CHAPTERS) ? MAX_OMX_CHAPTERS : avFormatContext->nb_chapters;
		for(i = 0; i < chapterCount; i++)
		{
			if(i > MAX_OMX_CHAPTERS)
				break;
			
			AVChapter *chapter = avFormatContext->chapters[i];
			if(!chapter)
				continue;
			
			omxChapters[i].seekto_ms = ConvertTimestamp(chapter->start, chapter->time_base.den, chapter->time_base.num) / 1000;
			omxChapters[i].ts        = omxChapters[i].seekto_ms / 1000;
			
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52,83,0)
			AVDictionaryEntry *titleTag = av_dict_get(avFormatContext->chapters[i]->metadata,"title", NULL, 0);
			if (titleTag)
				omxChapters[i].name = titleTag->value;
#else
			if(avFormatContext->chapters[i]->title)
				omxChapters[i].name = avFormatContext->chapters[i]->title;
#endif
			printf("Chapter : \t%d \t%s \t%8.2f\n", i, omxChapters[i].name.c_str(), omxChapters[i].ts);
		}
	}
#endif
	
	return true;
}

void OMXReader::AddStream(int id)
{
	if(id > MAX_STREAMS || !avFormatContext)
		return;
	
	AVStream *pStream = avFormatContext->streams[id];
	// discard PNG stream as we don't support it, and it stops mp3 files playing with album art
	if (pStream->codec->codec_type == AVMEDIA_TYPE_VIDEO && 
		(pStream->codec->codec_id == CODEC_ID_PNG))
		return;
	
	switch (pStream->codec->codec_type)
	{
		case AVMEDIA_TYPE_AUDIO:
			omxStreams[id].stream      = pStream;
			omxStreams[id].type        = OMXSTREAM_AUDIO;
			omxStreams[id].index       = audioCount;
			omxStreams[id].codec_name  = GetStreamCodecName(pStream);
			omxStreams[id].id          = id;
			audioCount++;
			getHints(pStream, &omxStreams[id].hints);
			break;
		case AVMEDIA_TYPE_VIDEO:
			omxStreams[id].stream      = pStream;
			omxStreams[id].type        = OMXSTREAM_VIDEO;
			omxStreams[id].index       = videoCount;
			omxStreams[id].codec_name  = GetStreamCodecName(pStream);
			omxStreams[id].id          = id;
			videoCount++;
			getHints(pStream, &omxStreams[id].hints);
			break;
		case AVMEDIA_TYPE_SUBTITLE:
			omxStreams[id].stream      = pStream;
			omxStreams[id].type        = OMXSTREAM_SUBTITLE;
			omxStreams[id].index       = subtitleCount;
			omxStreams[id].codec_name  = GetStreamCodecName(pStream);
			omxStreams[id].id          = id;
			subtitleCount++;
			getHints(pStream, &omxStreams[id].hints);
			break;
		default:
			return;
	}
	
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52,83,0)
	AVDictionaryEntry *langTag = av_dict_get(pStream->metadata, "language", NULL, 0);
	if (langTag)
		strncpy(omxStreams[id].language, langTag->value, 3);
#else
	strcpy( omxStreams[id].language, pStream->language );
#endif
	
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52,83,0)
	AVDictionaryEntry *titleTag = av_dict_get(pStream->metadata,"title", NULL, 0);
	if (titleTag)
		omxStreams[id].name = titleTag->value;
#else
	omxStreams[id].name = pStream->title;
#endif
	
	if( pStream->codec->extradata && pStream->codec->extradata_size > 0 )
	{
		omxStreams[id].extrasize = pStream->codec->extradata_size;
		omxStreams[id].extradata = malloc(pStream->codec->extradata_size);
		memcpy(omxStreams[id].extradata, pStream->codec->extradata, pStream->codec->extradata_size);
	}
}

bool OMXReader::setActiveStreamInternal(OMXStreamType type, unsigned int index)
{
	bool ret = false;
	
	switch(type)
	{
		case OMXSTREAM_AUDIO:
			if((int)index > (audioCount - 1))
				index = (audioCount - 1);
			break;
		case OMXSTREAM_VIDEO:
			if((int)index > (videoCount - 1))
				index = (videoCount - 1);
			break;
		case OMXSTREAM_SUBTITLE:
			if((int)index > (subtitleCount - 1))
				index = (subtitleCount - 1);
			break;
		default:
			break;
	}
	
	for(int i = 0; i < MAX_STREAMS; i++)
	{
		if(omxStreams[i].type == type &&  omxStreams[i].index == index)
		{
			switch(omxStreams[i].type)
			{
				case OMXSTREAM_AUDIO:
					audioIndex = i;
					ret = true;
					break;
				case OMXSTREAM_VIDEO:
					videoIndex = i;
					ret = true;
					break;
				case OMXSTREAM_SUBTITLE:
					subtitleIndex = i;
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
				audioIndex = -1;
				break;
			case OMXSTREAM_VIDEO:
				videoIndex = -1;
				break;
			case OMXSTREAM_SUBTITLE:
				subtitleIndex = -1;
				break;
			default:
				break;
		}
	}
	
	return ret;
}

bool OMXReader::IsActive(int stream_index)
{
	if((audioIndex != -1)    && omxStreams[audioIndex].id      == stream_index)
		return true;
	if((videoIndex != -1)    && omxStreams[videoIndex].id      == stream_index)
		return true;
	if((subtitleIndex != -1) && omxStreams[subtitleIndex].id   == stream_index)
		return true;
	
	return false;
}

bool OMXReader::IsActive(OMXStreamType type, int stream_index)
{
	if((audioIndex != -1)    && omxStreams[audioIndex].id      == stream_index && omxStreams[audioIndex].type == type)
		return true;
	if((videoIndex != -1)    && omxStreams[videoIndex].id      == stream_index && omxStreams[videoIndex].type == type)
		return true;
	if((subtitleIndex != -1) && omxStreams[subtitleIndex].id   == stream_index && omxStreams[subtitleIndex].type == type)
		return true;
	
	return false;
}

bool OMXReader::getHints(AVStream *stream, OMXStreamInfo *hints)
{
	if(!hints || !stream)
		return false;
	
	hints->codec         = stream->codec->codec_id;
	hints->extradata     = stream->codec->extradata;
	hints->extrasize     = stream->codec->extradata_size;
	hints->channels      = stream->codec->channels;
	hints->samplerate    = stream->codec->sample_rate;
	hints->blockalign    = stream->codec->block_align;
	hints->bitrate       = stream->codec->bit_rate;
	hints->bitspersample = stream->codec->bits_per_coded_sample;
    hints->gop_size      = stream->codec->gop_size;
    
	if(hints->bitspersample == 0)
		hints->bitspersample = 16;
	
	hints->width         = stream->codec->width;
	hints->height        = stream->codec->height;
	hints->profile       = stream->codec->profile;
	
	if(stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
	{
		//CUSTOM
		hints->duration		= stream->duration;
		hints->nb_frames	= stream->nb_frames;
		
        
		hints->fpsrate       = stream->r_frame_rate.num;
		hints->fpsscale      = stream->r_frame_rate.den;
		
		if(isMatroska && stream->avg_frame_rate.den && stream->avg_frame_rate.num)
		{
			hints->fpsrate      = stream->avg_frame_rate.num;
			hints->fpsscale     = stream->avg_frame_rate.den;
		}
		else if(stream->r_frame_rate.num && stream->r_frame_rate.den)
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
		if (isAVI && stream->codec->codec_id == CODEC_ID_H264)
			hints->ptsinvalid = true;
	}
	
	return true;
}

bool OMXReader::getHints(OMXStreamType type, unsigned int index, OMXStreamInfo &hints)
{
	for(unsigned int i = 0; i < MAX_STREAMS; i++)
	{
		if(omxStreams[i].type == type && omxStreams[i].index == i)
		{
			hints = omxStreams[i].hints;
			return true;
		}
	}
	
	return false;
}

bool OMXReader::getHints(OMXStreamType type, OMXStreamInfo &hints)
{
	bool ret = false;
	
	switch (type)
	{
		case OMXSTREAM_AUDIO:
			if(audioIndex != -1)
			{
				ret = true;
				hints = omxStreams[audioIndex].hints;
			}
			break;
		case OMXSTREAM_VIDEO:
			if(videoIndex != -1)
			{
				ret = true;
				hints = omxStreams[videoIndex].hints;
			}
			break;
		case OMXSTREAM_SUBTITLE:
			if(subtitleIndex != -1)
			{
				ret = true;
				hints = omxStreams[subtitleIndex].hints;
			}
			break;
		default:
			break;
	}
	
	return ret;
}

bool OMXReader::getIsEOF()
{
	return isEOF;
}

void OMXReader::FreePacket(OMXPacket *pkt)
{
	delete pkt->data;
	pkt->data = NULL;
	delete pkt;
	pkt = NULL;
	/*if(pkt)
	{
		if(pkt->data)
			free(pkt->data);
		free(pkt);
	}*/
}

OMXPacket *OMXReader::AllocPacket(int size)
{
	OMXPacket *pkt = new OMXPacket();
	pkt->data = new uint8_t[size + FF_INPUT_BUFFER_PADDING_SIZE];
	//memset(pkt->data + size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
	pkt->size = size;
	pkt->dts  = DVD_NOPTS_VALUE;
	pkt->pts  = DVD_NOPTS_VALUE;
	pkt->now  = DVD_NOPTS_VALUE;
	pkt->duration = DVD_NOPTS_VALUE;
	
	return pkt;
}

bool OMXReader::setActiveStream(OMXStreamType type, unsigned int index)
{
	bool ret = false;
	lock();
	ret = setActiveStreamInternal(type, index);
	unlock();
	return ret;
}

bool OMXReader::SeekChapter(int chapter, double* startpts)
{
	if(chapter < 1)
		chapter = 1;
	
	if(avFormatContext == NULL)
		return false;
	
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52,14,0)
	if(chapter < 1 || chapter > (int)avFormatContext->nb_chapters)
		return false;
	
	AVChapter *ch = avFormatContext->chapters[chapter-1];
	double dts = ConvertTimestamp(ch->start, ch->time_base.den, ch->time_base.num);
	return SeekTime(DVD_TIME_TO_MSEC(dts), 0, startpts);
#else
	return false;
#endif
}

double OMXReader::ConvertTimestamp(int64_t pts, int den, int num)
{
	if(avFormatContext == NULL)
		return DVD_NOPTS_VALUE;
	
	if (pts == (int64_t)AV_NOPTS_VALUE)
		return DVD_NOPTS_VALUE;
	
	// do calculations in floats as they can easily overflow otherwise
	// we don't care for having a completly exact timestamp anyway
	double timestamp = (double)pts * num  / den;
	double starttime = 0.0f;
	
	if (avFormatContext->start_time != (int64_t)AV_NOPTS_VALUE)
		starttime = (double)avFormatContext->start_time / AV_TIME_BASE;
	
	if(timestamp > starttime)
		timestamp -= starttime;
	else if( timestamp + 0.1f > starttime )
		timestamp = 0;
	
	return timestamp*DVD_TIME_BASE;
}

int OMXReader::getChapter()
{
	if(avFormatContext == NULL
	   || currentPTS == DVD_NOPTS_VALUE)
		return 0;
	
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52,14,0)
	for(unsigned i = 0; i < avFormatContext->nb_chapters; i++)
	{
		AVChapter *chapter = avFormatContext->chapters[i];
		if(currentPTS >= ConvertTimestamp(chapter->start, chapter->time_base.den, chapter->time_base.num)
		   && currentPTS <  ConvertTimestamp(chapter->end,   chapter->time_base.den, chapter->time_base.num))
			return i + 1;
	}
#endif
	return 0;
}

void OMXReader::getChapterName(std::string& strChapterName)
{
	strChapterName = "";
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52,14,0)
	int chapterIdx = getChapter();
	if(chapterIdx <= 0)
		return;
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52,83,0)
	// API added on: 2010-10-15
	// (Note that while the function was available earlier, the generic
	// metadata tags were not populated by default)
	AVDictionaryEntry *titleTag = av_dict_get(avFormatContext->chapters[chapterIdx-1]->metadata,
														  "title", NULL, 0);
	if (titleTag)
		strChapterName = titleTag->value;
#else
	if (avFormatContext->chapters[chapterIdx-1]->title)
		strChapterName = avFormatContext->chapters[chapterIdx-1]->title;
#endif
#endif
}

void OMXReader::UpdateCurrentPTS()
{
	currentPTS = DVD_NOPTS_VALUE;
	for(unsigned int i = 0; i < avFormatContext->nb_streams; i++)
	{
		AVStream *stream = avFormatContext->streams[i];
		if(stream && stream->cur_dts != (int64_t)AV_NOPTS_VALUE)
		{
			double ts = ConvertTimestamp(stream->cur_dts, stream->time_base.den, stream->time_base.num);
			if(currentPTS == DVD_NOPTS_VALUE || currentPTS > ts )
            {
                currentPTS = ts;
            }
		}
        //ofLogVerbose(__func__) << "currentPTS: " << currentPTS;
	}
}

void OMXReader::setSpeed(int iSpeed)
{
	if(!avFormatContext)
		return;
	
	if(speed != DVD_PLAYSPEED_PAUSE && iSpeed == DVD_PLAYSPEED_PAUSE)
	{
		av_read_pause(avFormatContext);
	}
	else if(speed == DVD_PLAYSPEED_PAUSE && iSpeed != DVD_PLAYSPEED_PAUSE)
	{
		av_read_play(avFormatContext);
	}
	speed = iSpeed;
	
	AVDiscard discard = AVDISCARD_NONE;
	if(speed > 4*DVD_PLAYSPEED_NORMAL)
		discard = AVDISCARD_NONKEY;
	else if(speed > 2*DVD_PLAYSPEED_NORMAL)
		discard = AVDISCARD_BIDIR;
	else if(speed < DVD_PLAYSPEED_PAUSE)
		discard = AVDISCARD_NONKEY;
	
	for(unsigned int i = 0; i < avFormatContext->nb_streams; i++)
	{
		if(avFormatContext->streams[i])
		{
			if(avFormatContext->streams[i]->discard != AVDISCARD_ALL)
				avFormatContext->streams[i]->discard = discard;
		}
	}
}

int OMXReader::GetStreamLength()
{
	if (!avFormatContext)
		return 0;
	
	return (int)(avFormatContext->duration / (AV_TIME_BASE / 1000));
}

double OMXReader::NormalizeFrameduration(double frameduration)
{
	//if the duration is within 20 microseconds of a common duration, use that
	double durations[] = {DVD_TIME_BASE * 1.001 / 24.0, DVD_TIME_BASE / 24.0, DVD_TIME_BASE / 25.0,
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
	
	AVCodec *codec = avcodec_find_decoder(stream->codec->codec_id);
	
	if (codec)
		strStreamName = codec->name;
	
	return strStreamName;
}

std::string OMXReader::GetCodecName(OMXStreamType type)
{
	std::string strStreamName;
	
	lock();
	switch (type)
	{
		case OMXSTREAM_AUDIO:
			if(audioIndex != -1)
				strStreamName = omxStreams[audioIndex].codec_name;
			break;
		case OMXSTREAM_VIDEO:
			if(videoIndex != -1)
				strStreamName = omxStreams[videoIndex].codec_name;
			break;
		case OMXSTREAM_SUBTITLE:
			if(subtitleIndex != -1)
				strStreamName = omxStreams[subtitleIndex].codec_name;
			break;
		default:
			break;
	}
	unlock();
	
	return strStreamName;
}

std::string OMXReader::GetCodecName(OMXStreamType type, unsigned int index)
{
	std::string strStreamName = "";
	
	for(int i = 0; i < MAX_STREAMS; i++)
	{
		if(omxStreams[i].type == type &&  omxStreams[i].index == index)
		{
			strStreamName = omxStreams[i].codec_name;
			break;
		}
	}
	
	return strStreamName;
}

std::string OMXReader::GetStreamLanguage(OMXStreamType type, unsigned int index)
{
	std::string language = "";
	
	for(int i = 0; i < MAX_STREAMS; i++)
	{
		if(omxStreams[i].type == type &&  omxStreams[i].index == index)
		{
			language = omxStreams[i].language;
			break;
		}
	}
	
	return language;
}

std::string OMXReader::GetStreamName(OMXStreamType type, unsigned int index)
{
	std::string name = "";
	
	for(int i = 0; i < MAX_STREAMS; i++)
	{
		if(omxStreams[i].type == type &&  omxStreams[i].index == index)
		{
			name = omxStreams[i].name;
			break;
		}
	}
	
	return name;
}

std::string OMXReader::GetStreamType(OMXStreamType type, unsigned int index)
{
	std::string strInfo;
	char sInfo[64];
	
	for(int i = 0; i < MAX_STREAMS; i++)
	{
		if(omxStreams[i].type == type &&  omxStreams[i].index == index)
		{
			if (omxStreams[i].hints.codec == CODEC_ID_AC3) strcpy(sInfo, "AC3 ");
			else if (omxStreams[i].hints.codec == CODEC_ID_DTS)
			{
#ifdef FF_PROFILE_DTS_HD_MA
				if (omxStreams[i].hints.profile == FF_PROFILE_DTS_HD_MA)
					strcpy(sInfo, "DTS-HD MA ");
				else if (omxStreams[i].hints.profile == FF_PROFILE_DTS_HD_HRA)
					strcpy(sInfo, "DTS-HD HRA ");
				else
#endif
					strcpy(sInfo, "DTS ");
			}
			else if (omxStreams[i].hints.codec == CODEC_ID_MP2) strcpy(sInfo, "MP2 ");
			else strcpy(sInfo, "");
			
			if (omxStreams[i].hints.channels == 1) strcat(sInfo, "Mono");
			else if (omxStreams[i].hints.channels == 2) strcat(sInfo, "Stereo");
			else if (omxStreams[i].hints.channels == 6) strcat(sInfo, "5.1");
			else if (omxStreams[i].hints.channels != 0)
			{
				char temp[32];
				sprintf(temp, " %d %s", omxStreams[i].hints.channels, "Channels");
				strcat(sInfo, temp);
			}
			break;
		}
	}
	
	strInfo = sInfo;
	return strInfo;
}

bool OMXReader::CanSeek()
{
	if(avioContext)
		return avioContext->seekable;
	
	if(!avFormatContext)
		return false;
	
	if(avFormatContext->pb->seekable == AVIO_SEEKABLE_NORMAL)
		return true;
	
	return false;
}