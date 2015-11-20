#pragma once

#include "ofMain.h"

#include "LIBAV_INCLUDES.h"

#include "OMXStreamInfo.h"
#include "OMXThread.h"
#include <queue>

#include "OMXStreamInfo.h"

#include "File.h"


#include <sys/types.h>
#include <string>

using namespace XFILE;
using namespace std;

#define MAX_OMX_CHAPTERS 2

#define MAX_OMX_STREAMS        100

#ifndef FFMPEG_FILE_BUFFER_SIZE
#define FFMPEG_FILE_BUFFER_SIZE   32768 // default reading size for ffmpeg
#endif
#ifndef MAX_STREAMS
#define MAX_STREAMS 100
#endif

typedef struct OMXChapter
{
	std::string name;
	int64_t     seekto_ms;
	double      ts;
} OMXChapter;

class OMXReader;

typedef struct OMXPacket
{
	double    pts; // pts in DVD_TIME_BASE
	double    dts; // dts in DVD_TIME_BASE
	double    now; // dts in DVD_TIME_BASE
	double    duration; // duration in DVD_TIME_BASE if available
	int       size;
	uint8_t   *data;
	int       stream_index;
	OMXStreamInfo hints;
	enum AVMediaType codec_type;
} OMXPacket;

enum OMXStreamType
{
	OMXSTREAM_NONE      = 0,
	OMXSTREAM_AUDIO     = 1,
	OMXSTREAM_VIDEO     = 2,
	OMXSTREAM_SUBTITLE  = 3
};

typedef struct OMXStream
{
	char language[4];
	std::string name;
	std::string codec_name;
	AVStream    *stream;
	OMXStreamType type;
	int         id;
	void        *extradata;
	unsigned int extrasize;
	unsigned int index;
	OMXStreamInfo hints;
} OMXStream;

class OMXReader
{
	protected:
		int                       videoIndex;
		int                       audioIndex;
		int                       subtitleIndex;
		int                       videoCount;
		int                       audioCount;
		int                       subtitleCount;
		bool                      isOpen;
		std::string               fileName;
		bool                      isMatroska;
		bool                      isAVI;
		XFILE::CFile              *fileObject;
		AVFormatContext           *avFormatContext;
		AVIOContext               *avioContext;
		bool                      isEOF;
		OMXChapter                omxChapters[MAX_OMX_CHAPTERS];
		OMXStream                 omxStreams[MAX_STREAMS];
		int                       chapterCount;
		double                    currentPTS;
		int                       speed;
		unsigned int              programID;
		pthread_mutex_t           m_lock;
		void Lock();
		void UnLock();
		bool setActiveStreamInternal(OMXStreamType type, unsigned int index);
		bool                      m_seek;
	private:
	public:
		OMXReader();
		~OMXReader();
		bool Open(std::string filename, bool doSkipAvProbe);
		void ClearStreams();
		bool close();
		//void FlushRead();
		bool SeekTime(int time, bool backwords, double *startpts, bool doLoopOnFail = true);
		AVMediaType PacketType(OMXPacket *pkt);
		OMXPacket *Read();
		void Process();
		bool GetStreams();
		void AddStream(int id);
		bool IsActive(int stream_index);
		bool IsActive(OMXStreamType type, int stream_index);
		bool getHints(AVStream *stream, OMXStreamInfo *hints);
		bool getHints(OMXStreamType type, unsigned int index, OMXStreamInfo& hints);
		bool getHints(OMXStreamType type, OMXStreamInfo& hints);
		bool getIsEOF();
		int  getNumAudioStreams()
		{
			return audioCount;
		};
		int  getNumVideoStreams()
		{
			return videoCount;
		};
		int  getNumSubtitleStreams()
		{
			return subtitleCount;
		};
		bool setActiveStream(OMXStreamType type, unsigned int index);
		int  getChapterCount()
		{
			return chapterCount;
		};
		OMXChapter getChapter(unsigned int chapter)
		{
			return omxChapters[(chapter > MAX_OMX_CHAPTERS) ? MAX_OMX_CHAPTERS : chapter];
		};
		static void FreePacket(OMXPacket *pkt);
		static OMXPacket *AllocPacket(int size);
		void setSpeed(int iSpeed);
		int getSpeed()
		{
			return speed;
		};
		void UpdateCurrentPTS();
		double ConvertTimestamp(int64_t pts, int den, int num);
		int getChapter();
		void getChapterName(std::string& strChapterName);
		bool SeekChapter(int chapter, double* startpts);
		int GetAudioIndex()
		{
			return (audioIndex >= 0) ? omxStreams[audioIndex].index : -1;
		};
		int GetSubtitleIndex()
		{
			return (subtitleIndex >= 0) ? omxStreams[subtitleIndex].index : -1;
		};

		int GetRelativeIndex(size_t index)
		{
			//assert(index < MAX_STREAMS);
			return omxStreams[index].index;
		}

		int GetStreamLength();
		static double NormalizeFrameduration(double frameduration);
		bool IsMatroska()
		{
			return isMatroska;
		};
		std::string GetCodecName(OMXStreamType type);
		std::string GetCodecName(OMXStreamType type, unsigned int index);
		std::string GetStreamCodecName(AVStream *stream);
		std::string GetStreamLanguage(OMXStreamType type, unsigned int index);
		std::string GetStreamName(OMXStreamType type, unsigned int index);
		std::string GetStreamType(OMXStreamType type, unsigned int index);
		bool CanSeek();
		bool wasFileRewound;
};

