#include "VideoDecoderDirect.h"



VideoDecoderDirect::VideoDecoderDirect()
{

	doHDMISync   = false;
	frameCounter = 0;
	frameOffset = 0;
    doUpdate = false;
}


VideoDecoderDirect::~VideoDecoderDirect()
{
	ofRemoveListener(ofEvents().update, this, &VideoDecoderDirect::onUpdate);
	ofLogVerbose(__func__) << "removed update listener";
}

bool VideoDecoderDirect::open(StreamInfo& streamInfo, OMXClock *clock, ofxOMXPlayerSettings& settings_)
{
	OMX_ERRORTYPE error   = OMX_ErrorNone;

	videoWidth  = streamInfo.width;
	videoHeight = streamInfo.height;

    settings = settings_;
    omxClock = clock;
    clockComponent = omxClock->getComponent();
	doHDMISync = settings.directDisplayOptions.doHDMISync;
    

	if(!videoWidth || !videoHeight)
	{
		return false;
	}

	if(streamInfo.extrasize > 0 && streamInfo.extradata != NULL)
	{
		extraSize = streamInfo.extrasize;
		extraData = (uint8_t *)malloc(extraSize);
		memcpy(extraData, streamInfo.extradata, streamInfo.extrasize);
	}

	processCodec(streamInfo);

	if(settings.enableFilters)
	{
		doFilters = true;
	}
	else
	{
		doFilters = false;
	}

	std::string componentName = "OMX.broadcom.video_decode";
	if(!decoderComponent.init(componentName, OMX_IndexParamVideoInit))
	{
		return false;
	}

	componentName = "OMX.broadcom.video_render";
	if(!renderComponent.init(componentName, OMX_IndexParamVideoInit))
	{
		return false;
	}

	componentName = "OMX.broadcom.video_scheduler";
	if(!schedulerComponent.init(componentName, OMX_IndexParamVideoInit))
	{
		return false;
	}

    if(doFilters)
    {
        componentName = "OMX.broadcom.image_fx";
        if(!imageFXComponent.init(componentName, OMX_IndexParamImageInit))
        {
            return false;
        }
        
        decoderTunnel.init(&decoderComponent, 
                           decoderComponent.getOutputPort(), 
                           &imageFXComponent, 
                           imageFXComponent.getInputPort());
        
        imageFXTunnel.init(&imageFXComponent, 
                           imageFXComponent.getOutputPort(), 
                           &schedulerComponent, 
                           schedulerComponent.getInputPort());
    }
    else
    {
        decoderTunnel.init(&decoderComponent, 
                           decoderComponent.getOutputPort(), 
                           &schedulerComponent, 
                           schedulerComponent.getInputPort());
    }

    schedulerTunnel.init(&schedulerComponent,
                         schedulerComponent.getOutputPort(),
                         &renderComponent,
                         renderComponent.getInputPort());
    
    clockTunnel.init(clockComponent,
                     clockComponent->getInputPort() + 1,
                     &schedulerComponent,
                     schedulerComponent.getOutputPort() + 1);
    
	error = clockTunnel.Establish(false);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	error = decoderComponent.setState(OMX_StateIdle);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	OMX_VIDEO_PARAM_PORTFORMATTYPE formatType;
	OMX_INIT_STRUCTURE(formatType);
	formatType.nPortIndex = decoderComponent.getInputPort();
	formatType.eCompressionFormat = omxCodingType;

	if (streamInfo.fpsscale > 0 && streamInfo.fpsrate > 0)
	{
		formatType.xFramerate = (long long)(1<<16)*streamInfo.fpsrate / streamInfo.fpsscale;
	}
	else
	{
		formatType.xFramerate = 25 * (1<<16);
	}
    
	error = decoderComponent.setParameter(OMX_IndexParamVideoPortFormat, &formatType);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	OMX_PARAM_PORTDEFINITIONTYPE portParam;
	OMX_INIT_STRUCTURE(portParam);
	portParam.nPortIndex = decoderComponent.getInputPort();

	error = decoderComponent.getParameter(OMX_IndexParamPortDefinition, &portParam);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	portParam.nPortIndex = decoderComponent.getInputPort();
	int videoBuffers = 60;
	portParam.nBufferCountActual = videoBuffers;

	portParam.format.video.nFrameWidth  = videoWidth;
	portParam.format.video.nFrameHeight = videoHeight;

	error = decoderComponent.setParameter(OMX_IndexParamPortDefinition, &portParam);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	OMX_PARAM_BRCMVIDEODECODEERRORCONCEALMENTTYPE concanParam;
	OMX_INIT_STRUCTURE(concanParam);
	concanParam.bStartWithValidFrame = OMX_FALSE;

	error = decoderComponent.setParameter(OMX_IndexParamBrcmVideoDecodeErrorConcealment, &concanParam);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

#if 0
	if (doFilters)
	{
		// the deinterlace component requires 3 additional video buffers in addition to the DPB (this is normally 2).
		OMX_PARAM_U32TYPE extra_buffers;
		OMX_INIT_STRUCTURE(extra_buffers);
		extra_buffers.nU32 = 3;

		error = decoderComponent.setParameter(OMX_IndexParamBrcmExtraBuffers, &extra_buffers);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone) return false;
	}
#endif
	// broadcom omx entension:
	// When enabled, the timestamp fifo mode will change the way incoming timestamps are associated with output images.
	// In this mode the incoming timestamps get used without re-ordering on output images.
	
    // recent firmware will actually automatically choose the timestamp stream with the least variance, so always enable

    OMX_CONFIG_BOOLEANTYPE timeStampMode;
    OMX_INIT_STRUCTURE(timeStampMode);
    timeStampMode.bEnabled = OMX_TRUE;
    error = decoderComponent.setParameter((OMX_INDEXTYPE)OMX_IndexParamBrcmVideoTimestampFifo, &timeStampMode);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;


	if(NaluFormatStartCodes(streamInfo.codec, extraData, extraSize))
	{
		OMX_NALSTREAMFORMATTYPE nalStreamFormat;
		OMX_INIT_STRUCTURE(nalStreamFormat);
		nalStreamFormat.nPortIndex = decoderComponent.getInputPort();
		nalStreamFormat.eNaluFormat = OMX_NaluFormatStartCodes;

		error = decoderComponent.setParameter((OMX_INDEXTYPE)OMX_IndexParamNalStreamFormatSelect, &nalStreamFormat);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone) return false;
	}

	if(doHDMISync)
	{
		OMX_CONFIG_LATENCYTARGETTYPE latencyTarget;
		OMX_INIT_STRUCTURE(latencyTarget);
		latencyTarget.nPortIndex = renderComponent.getInputPort();
		latencyTarget.bEnabled = OMX_TRUE;
		latencyTarget.nFilter = 2;
		latencyTarget.nTarget = 4000;
		latencyTarget.nShift = 3;
		latencyTarget.nSpeedFactor = -135;
		latencyTarget.nInterFactor = 500;
		latencyTarget.nAdjCap = 20;

		error = renderComponent.setConfig(OMX_IndexConfigLatencyTarget, &latencyTarget);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone) return false;
	}

   // Alloc buffers for the omx input port.
	error = decoderComponent.allocInputBuffers();
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

    
	error = decoderTunnel.Establish(false);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	
	error = decoderComponent.setState(OMX_StateExecuting);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;


	if(doFilters)
	{
        ofLogVerbose() << "imageFXTunnelState 1: " << OMX_Maps::getInstance().getOMXState(imageFXComponent.getState()) << "!!!!!!!!!!!!!!!!!!!!!!!!!!!";
 
        error = imageFXComponent.allocInputBuffers();
        OMX_TRACE(error);
        if(error != OMX_ErrorNone) return false;
          
		error = imageFXTunnel.Establish(false);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone) return false;
        
        
        ofLogVerbose() << "imageFXTunnelState 2: " << OMX_Maps::getInstance().getOMXState(imageFXComponent.getState()) << "!!!!!!!!!!!!!!!!!!!!!!!!!!!";
        
        
		error = imageFXComponent.setState(OMX_StateExecuting);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone) return false;
        
        filterManager.setup(&imageFXComponent);
        filterManager.setFilter(settings.filter);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone) return false;

	}

	error = schedulerTunnel.Establish(false);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;

	
	error = schedulerComponent.setState(OMX_StateExecuting);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;
	
	
	
	error = renderComponent.setState(OMX_StateExecuting);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone) return false;
    
   
    
	if(!sendDecoderConfig())
	{
		return false;
	}

	isOpen           = true;
	doSetStartTime      = true;
	
	display.setup(&renderComponent, streamInfo, settings);

	isFirstFrame   = true;
    doUpdate = true;
    ofAddListener(ofEvents().update, this, &VideoDecoderDirect::onUpdate);
    
	// start from assuming all recent frames had valid pts
	validHistoryPTS = ~0;
	return true;
}

void VideoDecoderDirect::onUpdate(ofEventArgs& args)
{
    //TODO: seems to cause hang on exit
    if(!doUpdate) return;
	updateFrameCount();
}
void VideoDecoderDirect::updateFrameCount()
{
	if (!isOpen) {
		return;
	}
	OMX_ERRORTYPE error;
	OMX_CONFIG_BRCMPORTSTATSTYPE stats;
	
	OMX_INIT_STRUCTURE(stats);
	
	stats.nPortIndex = renderComponent.getInputPort();
	
	error = renderComponent.getParameter(OMX_IndexConfigBrcmPortStats, &stats);
    OMX_TRACE(error);
	if (error == OMX_ErrorNone)
	{
		/*OMX_U32 nImageCount;
		 OMX_U32 nBufferCount;
		 OMX_U32 nFrameCount;
		 OMX_U32 nFrameSkips;
		 OMX_U32 nDiscards;
		 OMX_U32 nEOS;
		 OMX_U32 nMaxFrameSize;
		 
		 OMX_TICKS nByteCount;
		 OMX_TICKS nMaxTimeDelta;
		 OMX_U32 nCorruptMBs;*/
		//ofLogVerbose(__func__) << "nFrameCount: " << stats.nFrameCount;
		frameCounter = stats.nFrameCount;
	}else
	{
		ofLogError(__func__) << "renderComponent OMX_CONFIG_BRCMPORTSTATSTYPE fail: ";
	}
}

int VideoDecoderDirect::getCurrentFrame()
{
	return frameCounter - frameOffset;
}

void VideoDecoderDirect::resetFrameCounter()
{
	frameOffset = frameCounter;
}







