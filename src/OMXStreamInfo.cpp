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
	duration = 0;
	nb_frames = 0;
	channels   = 0;
	samplerate = 0;
	blockalign = 0;
	bitrate    = 0;
	bitspersample = 0;

	identifier = 0;

	framesize  = 0;
	syncword   = 0;
    gop_size = 0;
}

string COMXStreamInfo::toString()
{
	stringstream info;
	info << "width: "				<<	width					<< "\n";
	info << "height: "				<<	height					<< "\n";
	info << "profile: "				<<	profile					<< "\n";
	info << "numFrames: "			<<	nb_frames				<< "\n";
	info << "duration: "			<<	duration				<< "\n";
	info << "channels: "			<<	channels				<< "\n";
	info << "samplerate: "			<<	samplerate				<< "\n";
	info << "blockalign: "			<<	blockalign				<< "\n";
	info << "bitrate: "				<<	bitrate					<< "\n";
	info << "bitspersample: "		<<	bitspersample			<< "\n";
	info << "framesize: "			<<	framesize				<< "\n";
	info << "fpsscale: "			<<	fpsscale				<< "\n";
	info << "fpsrate: "				<<	fpsrate					<< "\n";
	info << "aspect: "				<<	aspect					<< "\n";
	info << "level: "				<<	level					<< "\n";
	info << "ptsinvalid: "			<<	ptsinvalid				<< "\n";
	info << "identifier: "			<<	identifier				<< "\n";
	info << "extrasize: "			<<	extrasize				<< "\n";
    info << "gop_size: "			<<	gop_size				<< "\n";
    

	return info.str();
}