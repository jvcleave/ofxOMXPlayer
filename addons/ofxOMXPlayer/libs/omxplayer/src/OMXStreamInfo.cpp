#include "OMXStreamInfo.h"

COMXStreamInfo::COMXStreamInfo()                                                     
{ 
	extradata = NULL; 
	Clear(); 
}

COMXStreamInfo::~COMXStreamInfo()
{
	//if( extradata && extrasize ) free(extradata);
	
	extradata = NULL;
	extrasize = 0;
}


void COMXStreamInfo::Clear()
{
	codec = CODEC_ID_NONE;
	software = false;
	codec_tag  = 0;
	
	//if( extradata && extrasize ) free(extradata);
	
	extradata = NULL;
	extrasize = 0;
	
	fpsscale = 0;
	fpsrate  = 0;
	height   = 0;
	width    = 0;
	aspect   = 0.0;
	vfr      = false;
	stills   = false;
	level    = 0;
	profile  = 0;
	ptsinvalid = false;
	duration = 0.0;
	nb_frames = 0;
	channels   = 0;
	samplerate = 0;
	blockalign = 0;
	bitrate    = 0;
	bitspersample = 0;
	
	identifier = 0;
	
	framesize  = 0;
	syncword   = 0;
}
