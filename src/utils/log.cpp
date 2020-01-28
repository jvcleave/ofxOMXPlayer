/*
 *      Copyright (C) 2005-2008 Team XBMC
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
#define _DEBUG

#include "system.h"
#include "log.h"
#include "stdio_utf8.h"
#include "stat_utf8.h"
#include "utils/StdString.h"
#include "ofLog.h"

static FILE*       m_file           = NULL;
static int         m_repeatCount    = 0;
static int         m_repeatLogLevel = -1;
static std::string m_repeatLine     = "";
static int         m_logLevel       = LOG_LEVEL_NORMAL;
static bool  logToOF = true;

static pthread_mutex_t   m_log_mutex;

static char levelNames[][8] =
{"DEBUG", "INFO", "NOTICE", "WARNING", "ERROR", "SEVERE", "FATAL", "NONE"};

CLog::CLog()
{
    
}

CLog::~CLog()
{}

void CLog::Close()
{
  if (m_file)
  {
    fclose(m_file);
    m_file = NULL;
  }
  m_repeatLine.clear();
  pthread_mutex_destroy(&m_log_mutex);
}

void CLog::Log(int loglevel, const char *format, ... )
{
    m_logLevel       = LOG_LEVEL_DEBUG;

#if 0
    CStdString strPrefix, strData;
    
    strData.reserve(16384);
    va_list va;
    va_start(va, format);
    strData.FormatV(format,va);
    va_end(va);
    
    
    m_logLevel       = LOG_LEVEL_DEBUG;
    ofLog() << "format: " << format << " : " << strData;
    return;
#endif
    pthread_mutex_lock(&m_log_mutex);

  static const char* prefixFormat = "%02.2d:%02.2d:%02.2d T:%" PRIu64 " %7s: ";
#if !(defined(_DEBUG) || defined(PROFILE))
  if (m_logLevel > LOG_LEVEL_NORMAL ||
     (m_logLevel > LOG_LEVEL_NONE && loglevel >= LOGNOTICE))
#endif
  {
    if (!m_file)
    {
      pthread_mutex_unlock(&m_log_mutex);
      return;
    }

    struct timeval now;
    gettimeofday(&now, NULL);
    SYSTEMTIME time;
    time.wHour=(now.tv_sec/3600) % 24;
    time.wMinute=(now.tv_sec/60) % 60;
    time.wSecond=now.tv_sec % 60;
    uint64_t stamp = now.tv_usec + now.tv_sec * 1000000;
    CStdString strPrefix, strData;

    strData.reserve(16384);
    va_list va;
    va_start(va, format);
    strData.FormatV(format,va);
    va_end(va);

    if (m_repeatLogLevel == loglevel && m_repeatLine == strData)
    {
      m_repeatCount++;
      pthread_mutex_unlock(&m_log_mutex);
      return;
    }
    else if (m_repeatCount)
    {
      CStdString strData2;
      strPrefix.Format(prefixFormat, time.wHour, time.wMinute, time.wSecond, stamp, levelNames[m_repeatLogLevel]);

      strData2.Format("Previous line repeats %d times." LINE_ENDING, m_repeatCount);
      fputs(strPrefix.c_str(), m_file);
      fputs(strData2.c_str(), m_file);
      OutputDebugString(strData2);
      m_repeatCount = 0;
    }
    
    m_repeatLine      = strData;
    m_repeatLogLevel  = loglevel;

    unsigned int length = 0;
    while ( length != strData.length() )
    {
      length = strData.length();
      strData.TrimRight(" ");
      strData.TrimRight('\n');
      strData.TrimRight("\r");
    }

    if (!length)
    {
      pthread_mutex_unlock(&m_log_mutex);
      return;
    }
    
    //OutputDebugString(strData);

    /* fixup newline alignment, number of spaces should equal prefix length */
    strData.Replace("\n", LINE_ENDING"                                            ");
    strData += LINE_ENDING;

    strPrefix.Format(prefixFormat, time.wHour, time.wMinute, time.wSecond, stamp, levelNames[loglevel]);

    fputs(strPrefix.c_str(), m_file);
    fputs(strData.c_str(), m_file);
      
      if(logToOF)
      {
          ofLog() << strPrefix << " : " << strData;

      }
    //fputs(strPrefix.c_str(), stdout);
    //fputs(strData.c_str(), stdout);
    fflush(m_file);
  }

  pthread_mutex_unlock(&m_log_mutex);
}

bool CLog::Init(const char* path, bool logToOF_)
{
    logToOF = logToOF_;
  pthread_mutex_init(&m_log_mutex, NULL);
  if (m_logLevel > LOG_LEVEL_NONE) { 
  if (!m_file)
  {
    CStdString strLogFile, strLogFileOld;

    strLogFile.Format("%s/omxplayer.log", path);
    strLogFileOld.Format("%s/omxplayer.old.log", path);

    struct stat info;
    if (stat(strLogFileOld.c_str(),&info) == 0 &&
        remove(strLogFileOld.c_str()) != 0)
      return false;
    if (stat(strLogFile.c_str(),&info) == 0 &&
        rename(strLogFile.c_str(),strLogFileOld.c_str()) != 0)
      return false;

    m_file = fopen(strLogFile.c_str(),"wb");
  }

  if (m_file)
  {
    unsigned char BOM[3] = {0xEF, 0xBB, 0xBF};
    fwrite(BOM, sizeof(BOM), 1, m_file);
  }
  }
  return m_file != NULL;
}

void CLog::MemDump(char *pData, int length)
{
  if (m_logLevel > LOG_LEVEL_NONE) { 
  Log(LOGDEBUG, "MEM_DUMP: Dumping from %p", pData);
  for (int i = 0; i < length; i+=16)
  {
    CStdString strLine;
    strLine.Format("MEM_DUMP: %04x ", i);
    char *alpha = pData;
    for (int k=0; k < 4 && i + 4*k < length; k++)
    {
      for (int j=0; j < 4 && i + 4*k + j < length; j++)
      {
        CStdString strFormat;
        strFormat.Format(" %02x", *pData++);
        strLine += strFormat;
      }
      strLine += " ";
    }
    // pad with spaces
    while (strLine.size() < 13*4 + 16)
      strLine += " ";
    for (int j=0; j < 16 && i + j < length; j++)
    {
      if (*alpha > 31)
        strLine += *alpha;
      else
        strLine += '.';
      alpha++;
    }
    Log(LOGDEBUG, "%s", strLine.c_str());
  }
  }
}

void CLog::SetLogLevel(int level)
{
  if(m_logLevel > LOG_LEVEL_NONE)
    CLog::Log(LOGNOTICE, "Log level changed to %d", m_logLevel);
  m_logLevel = level;
}

int CLog::GetLogLevel()
{
  return m_logLevel;
}

void CLog::OutputDebugString(const std::string& line)
{
#if defined(_DEBUG) || defined(PROFILE)
if(m_logLevel > LOG_LEVEL_NONE) 
{
  //::OutputDebugString(line.c_str());
  //::OutputDebugString("\n");
    ofLog() << line;  
}
#endif
}
