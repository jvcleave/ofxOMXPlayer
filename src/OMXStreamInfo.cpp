#include "OMXStreamInfo.h"

COMXStreamInfo::COMXStreamInfo()                                                     
{ 
  extradata = NULL; 
  Clear(); 
}

COMXStreamInfo::~COMXStreamInfo()
{
  extradata = NULL;
  extrasize = 0;
}


void COMXStreamInfo::Clear()
{
  codec = CODEC_ID_NONE;
  software = false;
  codec_tag  = 0;


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


  framesize  = 0;
  syncword   = 0;
}
