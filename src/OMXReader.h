#pragma once

#include "DllAvUtil.h"
#include "DllAvFormat.h"
#include "DllAvCodec.h"
#include "OMXStreamInfo.h"
#include "OMXThread.h"
#include <queue>

#include "OMXStreamInfo.h"

#include "File.h"


#include <sys/types.h>
#include <string>

using namespace XFILE;
using namespace std;


#define MAX_OMX_STREAMS        100

#define OMX_PLAYSPEED_PAUSE  0
#define OMX_PLAYSPEED_NORMAL 1

#ifndef FFMPEG_FILE_BUFFER_SIZE
#define FFMPEG_FILE_BUFFER_SIZE   32768 // default reading size for ffmpeg
#endif
#ifndef MAX_STREAMS
#define MAX_STREAMS 100
#endif


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
  COMXStreamInfo hints;
  enum AVMediaType codec_type;
} OMXPacket;

enum OMXStreamType
{
  OMXSTREAM_NONE      = 0,
  OMXSTREAM_AUDIO     = 1,
  OMXSTREAM_VIDEO     = 2,
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
  COMXStreamInfo hints;
} OMXStream;

class OMXReader
{
protected:
  int                       m_video_index;
  int                       m_audio_index;
  int                       m_video_count;
  int                       m_audio_count;
  int                       m_subtitle_count;
  DllAvUtil                 m_dllAvUtil;
  DllAvCodec                m_dllAvCodec;
  DllAvFormat               m_dllAvFormat;
  bool                      m_open;
  std::string               m_filename;
  bool                      m_bAVI;
  bool                      m_bMpeg;
  XFILE::CFile              *m_pFile;
  AVFormatContext           *m_pFormatContext;
  AVIOContext               *m_ioContext;
  bool                      m_eof;
  OMXStream                 m_streams[MAX_STREAMS];
  double                    m_iCurrentPts;
  int                       m_speed;
  unsigned int              m_program;
//#ifdef STANDALONE
//  void flush_packet_queue(AVFormatContext *s);
//  void av_read_frame_flush(AVFormatContext *s);
//#endif
  pthread_mutex_t           m_lock;
  void Lock();
  void UnLock();
  bool SetActiveStreamInternal(OMXStreamType type, unsigned int index);
  bool                      m_seek;
private:
public:
  OMXReader();
  ~OMXReader();
  bool Open(std::string filename, bool dump_format);
  void ClearStreams();
  bool Close();
  //void FlushRead();
  bool SeekTime(int64_t seek_ms, int seek_flags, double *startpts);
  AVMediaType PacketType(OMXPacket *pkt);
  OMXPacket *Read();
  void Process();
  bool GetStreams();
  void AddStream(int id);
  bool IsActive(int stream_index);
  bool IsActive(OMXStreamType type, int stream_index);
  bool GetHints(AVStream *stream, COMXStreamInfo *hints);
  bool GetHints(OMXStreamType type, unsigned int index, COMXStreamInfo &hints);
  bool GetHints(OMXStreamType type, COMXStreamInfo &hints);
  bool IsEof();
  int  AudioStreamCount() { return m_audio_count; };
  int  VideoStreamCount() { return m_video_count; };
  bool SetActiveStream(OMXStreamType type, unsigned int index);
  static void FreePacket(OMXPacket *pkt);
  static OMXPacket *AllocPacket(int size);
  void SetSpeed(int iSpeed);
  void UpdateCurrentPTS();
  double ConvertTimestamp(int64_t pts, int den, int num);
  double ConvertTimestamp(int64_t pts, AVRational *time_base);
  int GetAudioIndex() { return (m_audio_index >= 0) ? m_streams[m_audio_index].index : -1; };
  int GetStreamLength();
  static double NormalizeFrameduration(double frameduration);
  bool IsMpegVideo() { return m_bMpeg; };
  std::string GetStreamCodecName(AVStream *stream);
  int GetSourceBitrate();
};
