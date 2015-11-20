#pragma once

#include <sys/stat.h>

#define FFMPEG_FILE_BUFFER_SIZE   32768

namespace XFILE
{

	/* indicate that caller can handle truncated reads, where function returns before entire buffer has been filled */
#define READ_TRUNCATED 0x01

	/* indicate that that caller support read in the minimum defined chunk size, this disables internal cache then */
#define READ_CHUNKED   0x02

	/* use cache to access this file */
#define READ_CACHED     0x04

	/* open without caching. regardless to file type. */
#define READ_NO_CACHE  0x08

	/* calcuate bitrate for file while reading */
#define READ_BITRATE   0x10

	typedef enum
	{
		IOCTRL_NATIVE        = 1, /**< SNativeIoControl structure, containing what should be passed to native ioctrl */
		IOCTRL_SEEK_POSSIBLE = 2, /**< return 0 if known not to work, 1 if it should work */
		IOCTRL_CACHE_STATUS  = 3, /**< SCacheStatus structure */
		IOCTRL_CACHE_SETRATE = 4, /**< unsigned int with with speed limit for caching in bytes per second */
	} EIoControl;

	class CFile
	{
		public:
			CFile();
			~CFile();

			bool Open(std::string& strFileName, unsigned int flags = 0);
			bool OpenForWrite(std::string& strFileName, bool bOverWrite);
			unsigned int Read(void* lpBuf, int64_t uiBufSize);
			int Write(void* lpBuf, int64_t uiBufSize);
			int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET);
			int64_t GetPosition();
			int64_t GetLength();
			void Close();
			static bool Exists(std::string& strFileName, bool bUseCache = true);
			int GetChunkSize()
			{
				return 6144 /*FFMPEG_FILE_BUFFER_SIZE*/;
			};
			int IoControl(EIoControl request, void* param);
			bool IsgetIsEOF();
			void rewindFile();
		private:
			unsigned int m_flags;
			FILE  *fileObject;
			int64_t m_iLength;
			bool m_bPipe;
	};

};