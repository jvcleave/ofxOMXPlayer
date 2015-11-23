#include <iostream>
#include <stdio.h>

#include "File.h"
using namespace XFILE;
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#ifndef __GNUC__
#pragma warning (disable:4244)
#endif

//*********************************************************************************************
File::File()
{
	fileObject = NULL;
	m_flags = 0;
	m_iLength = 0;
	m_bPipe = false;
}

//*********************************************************************************************
File::~File()
{
	if(fileObject && !m_bPipe)
	{
		fclose(fileObject);
	}
}
void File::rewindFile()
{
	rewind(fileObject);
}
//*********************************************************************************************
bool File::open(std::string& strFileName, unsigned int flags)
{
	m_flags = flags;

	if (strFileName.compare(0, 5, "pipe:") == 0)
	{
		m_bPipe = true;
		fileObject = stdin;
		m_iLength = 0;
		return true;
	}
	fileObject = fopen64(strFileName.c_str(), "r");
	if(!fileObject)
	{
		return false;
	}

	fseeko64(fileObject, 0, SEEK_END);
	m_iLength = ftello64(fileObject);
	fseeko64(fileObject, 0, SEEK_SET);

	return true;
}

bool File::OpenForWrite(std::string& strFileName, bool bOverWrite)
{
	return false;
}

bool File::Exists(std::string& strFileName, bool bUseCache /* = true */)
{
	FILE *fp;

	if (strFileName.compare(0, 5, "pipe:") == 0)
	{
		return true;
	}

	fp = fopen64(strFileName.c_str(), "r");

	if(!fp)
	{
		return false;
	}

	fclose(fp);

	return true;
}

unsigned int File::Read(void *lpBuf, int64_t uiBufSize)
{
	unsigned int ret = 0;

	if(!fileObject)
	{
		return 0;
	}

	ret = fread(lpBuf, 1, uiBufSize, fileObject);

	return ret;
}

//*********************************************************************************************
void File::close()
{
	if(fileObject && !m_bPipe)
	{
		fclose(fileObject);
	}
	fileObject = NULL;
}

//*********************************************************************************************
int64_t File::Seek(int64_t iFilePosition, int iWhence)
{
	if (!fileObject)
	{
		return -1;
	}

	return fseeko64(fileObject, iFilePosition, iWhence);
}

//*********************************************************************************************
int64_t File::GetLength()
{
	return m_iLength;
}

//*********************************************************************************************
int64_t File::GetPosition()
{
	if (!fileObject)
	{
		return -1;
	}

	return ftello64(fileObject);
}

//*********************************************************************************************
int File::Write(void* lpBuf, int64_t uiBufSize)
{
	return -1;
}

int File::IoControl(EIoControl request, void* param)
{
	if(request == IOCTRL_SEEK_POSSIBLE && fileObject)
	{
		if (m_bPipe)
		{
			return false;
		}

		struct stat st;
		if (fstat(fileno(fileObject), &st) == 0)
		{
			return !S_ISFIFO(st.st_mode);
		}
	}

	return -1;
}

bool File::getIsEOF()
{
	if (!fileObject)
	{
		return -1;
	}

	if (m_bPipe)
	{
		return false;
	}

	return feof(fileObject) != 0;
}
