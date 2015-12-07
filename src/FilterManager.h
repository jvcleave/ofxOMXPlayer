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
    int numTimes;
    FilterManager()
    {
        imageFXComponent = NULL;  
        numTimes = 0;
    }
    ~FilterManager()
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
        if(numTimes+1 > 4)
        {
            ofLogError() << "cannot be set more than 4 times";
            return;
        }
        ofLogVerbose(__func__) << "imageFilter_: " << getImageFilterString(imageFilter_);
        OMX_ERRORTYPE error   = OMX_ErrorNone;
        
        
        imagefilterConfig.eImageFilter = imageFilter_;
        error = imageFXComponent->setConfig(OMX_IndexConfigCommonImageFilter, &imagefilterConfig);
        OMX_TRACE(error);
        numTimes++;
    }
    
    /*void applyFilter()
    {
        OMX_ERRORTYPE error   = OMX_ErrorNone;
        
        error = imageFXComponent->setConfig(OMX_IndexConfigCommonImageFilter, &imagefilterConfig);
        OMX_TRACE(error);

    }*/
    
    
};