#pragma once
#include "ofMain.h"

#include <sys/stat.h>

typedef unsigned char   BYTE;


#define WAVE_FORMAT_UNKNOWN           0x0000
#define WAVE_FORMAT_PCM               0x0001
#define WAVE_FORMAT_ADPCM             0x0002
#define WAVE_FORMAT_IEEE_FLOAT        0x0003
#define WAVE_FORMAT_EXTENSIBLE        0xFFFE

#define SPEAKER_FRONT_LEFT            0x00001
#define SPEAKER_FRONT_RIGHT           0x00002
#define SPEAKER_FRONT_CENTER          0x00004
#define SPEAKER_LOW_FREQUENCY         0x00008
#define SPEAKER_BACK_LEFT             0x00010
#define SPEAKER_BACK_RIGHT            0x00020
#define SPEAKER_FRONT_LEFT_OF_CENTER  0x00040
#define SPEAKER_FRONT_RIGHT_OF_CENTER 0x00080
#define SPEAKER_BACK_CENTER           0x00100
#define SPEAKER_SIDE_LEFT             0x00200
#define SPEAKER_SIDE_RIGHT            0x00400
#define SPEAKER_TOP_CENTER            0x00800
#define SPEAKER_TOP_FRONT_LEFT        0x01000
#define SPEAKER_TOP_FRONT_CENTER      0x02000
#define SPEAKER_TOP_FRONT_RIGHT       0x04000
#define SPEAKER_TOP_BACK_LEFT         0x08000
#define SPEAKER_TOP_BACK_CENTER       0x10000
#define SPEAKER_TOP_BACK_RIGHT        0x20000

typedef struct tGUID
{
    unsigned int Data1;
    unsigned short  Data2, Data3;
    BYTE  Data4[8];
} __attribute__((__packed__)) GUID;

static const GUID KSDATAFORMAT_SUBTYPE_UNKNOWN = {
    WAVE_FORMAT_UNKNOWN,
    0x0000, 0x0000,
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

static const GUID KSDATAFORMAT_SUBTYPE_PCM = {
    WAVE_FORMAT_PCM,
    0x0000, 0x0010,
    {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
};

static const GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = {
    WAVE_FORMAT_IEEE_FLOAT,
    0x0000, 0x0010,
    {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
};

// Audio stuff
typedef struct tWAVEFORMATEX
{
    unsigned short    wFormatTag;
    unsigned short    nChannels;
    unsigned int   nSamplesPerSec;
    unsigned int   nAvgBytesPerSec;
    unsigned short    nBlockAlign;
    unsigned short    wBitsPerSample;
    unsigned short    cbSize;
} __attribute__((__packed__)) WAVEFORMATEX, *PWAVEFORMATEX, *LPWAVEFORMATEX;

typedef struct tWAVEFORMATEXTENSIBLE
{
    WAVEFORMATEX Format;
    union
    {
        unsigned short wValidBitsPerSample;
        unsigned short wSamplesPerBlock;
        unsigned short wReserved;
    } Samples;
    unsigned int dwChannelMask;
    GUID SubFormat;
} __attribute__((__packed__)) WAVEFORMATEXTENSIBLE;


#ifndef __GNUC__
#pragma warning(disable:4244)
#endif


