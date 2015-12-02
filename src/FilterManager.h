//
//  FilterManager.h
//
//  Created by jason van cleave on 12/2/15.
//



class FilterManager
{
public:
    
    Component* imageFXComponent;
    OMX_CONFIG_IMAGEFILTERTYPE imagefilterConfig;
    //OMX_CONFIG_IMAGEFILTERPARAMSTYPE image_filter;
    FilterManager()
    {
        imageFXComponent = NULL;        
    }
    
    void setup(Component* imageFXComponent_)
    {
        imageFXComponent = imageFXComponent_;
        OMX_INIT_STRUCTURE(imagefilterConfig);
        imagefilterConfig.nPortIndex = imageFXComponent->getOutputPort();
        
    }
    void setFilter(OMX_IMAGEFILTERTYPE imageFilter_)
    {
        imagefilterConfig.eImageFilter = imageFilter_;
        applyFilter();
    }
    
    void applyFilter()
    {
        OMX_ERRORTYPE error   = OMX_ErrorNone;
        
        error = imageFXComponent->setConfig(OMX_IndexConfigCommonImageFilter, &imagefilterConfig);
        OMX_TRACE(error);

    }
    
    
};