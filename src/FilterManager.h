//
//  FilterManager.h
//
//  Created by jason van cleave on 12/2/15.
//



class FilterManager
{
public:
    
    Component* imageFXComponent;
    
    //OMX_CONFIG_IMAGEFILTERPARAMSTYPE image_filter;
    FilterManager()
    {
        imageFXComponent = NULL;        
    }
    
    void setup(Component* imageFXComponent_)
    {
        imageFXComponent = imageFXComponent_;
        
        
    }
    void setFilter(OMX_IMAGEFILTERTYPE imageFilter_)
    {
        ofLogVerbose(__func__) << "imageFilter_: " << OMX_Maps::getInstance().getImageFilter(imageFilter_);
        OMX_ERRORTYPE error   = OMX_ErrorNone;
        OMX_CONFIG_IMAGEFILTERTYPE imagefilterConfig;
        OMX_INIT_STRUCTURE(imagefilterConfig);
        imagefilterConfig.nPortIndex = imageFXComponent->getOutputPort();
        imagefilterConfig.eImageFilter = OMX_ImageFilterNone;
        error = imageFXComponent->setConfig(OMX_IndexConfigCommonImageFilter, &imagefilterConfig);
        OMX_TRACE(error);
        imagefilterConfig.eImageFilter = imageFilter_;
        error = imageFXComponent->setConfig(OMX_IndexConfigCommonImageFilter, &imagefilterConfig);
        OMX_TRACE(error);
    }
    
    /*void applyFilter()
    {
        OMX_ERRORTYPE error   = OMX_ErrorNone;
        
        error = imageFXComponent->setConfig(OMX_IndexConfigCommonImageFilter, &imagefilterConfig);
        OMX_TRACE(error);

    }*/
    
    
};