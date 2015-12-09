#include "Tunnel.h"

#pragma mark Tunnel

//#define DEBUG_TUNNELS

Tunnel::Tunnel()
{
    sourceComponentName = "UNDEFINED";
    destinationComponentName = "UNDEFINED";
    sourceComponent       = NULL;
    destinationComponent       = NULL;
    sourcePort            = 0;
    destinationPort            = 0;
    havePortSettingsChanged = false;
    isEstablished = false;
    
    pthread_mutex_init(&m_lock, NULL);
}

Tunnel::~Tunnel()
{
    if(isEstablished)
    {
        Deestablish(__func__);
    }
    
    pthread_mutex_destroy(&m_lock);
}

void Tunnel::lock()
{
    pthread_mutex_lock(&m_lock);
}

void Tunnel::unlock()
{
    pthread_mutex_unlock(&m_lock);
}

void Tunnel::init(Component *src_component, unsigned int src_port, Component *destination, unsigned int dst_port)
{
    
    Component sourceNullSink;
    Component destinationNullSink;
    
    std::string componentName = "OMX.broadcom.null_sink";
    if(!sourceNullSink.init(componentName, OMX_IndexParamOtherInit))
    {
        ofLogError() << "sourceNullSink FAIL";
        
        return;
    }
    
    if(!destinationNullSink.init(componentName, OMX_IndexParamOtherInit))
    {
        ofLogError() << "destinationNullSink FAIL";
        return;
    }
    
    
    sourceComponent  = src_component;
    sourcePort    = src_port;
    destinationComponent  = destination;
    destinationPort    = dst_port;
    
    sourceComponentName = sourceComponent->getName();
    destinationComponentName = destinationComponent->getName();
}

OMX_ERRORTYPE Tunnel::flush()
{
    if(!sourceComponent || !destinationComponent || !isEstablished)
    {
        return OMX_ErrorUndefined;
    }
    
    lock();
    
    sourceComponent->flushAll();
    destinationComponent->flushAll();
    
    unlock();
    return OMX_ErrorNone;
}

OMX_ERRORTYPE Tunnel::Deestablish(string caller)
{
    string debugString = sourceComponent->getName() + " : " + destinationComponent->getName();
    ofLogVerbose(__func__)<< " caller: " << caller << " components: " << debugString;
    if (!isEstablished)
    {
        return OMX_ErrorNone;
    }
    
    if(!sourceComponent || !destinationComponent)
    {
        return OMX_ErrorUndefined;
    }
    
    lock();
    OMX_ERRORTYPE error = OMX_ErrorNone;
    if(havePortSettingsChanged)
    {
        error = sourceComponent->waitForEvent(OMX_EventPortSettingsChanged);
        OMX_TRACE(error);
    }

    destinationComponent->setState(OMX_StatePause);
    destinationComponent->setState(OMX_StateIdle);
    destinationComponent->setState(OMX_StateLoaded);
    
    sourceComponent->setState(OMX_StatePause);
    sourceComponent->setState(OMX_StateIdle);
    sourceComponent->setState(OMX_StateLoaded);
 
#if 0 
    error = sourceComponent->disableAllPorts();
    OMX_TRACE(error, debugString);
    
    error = destinationComponent->disableAllPorts();
    OMX_TRACE(error, debugString);
#endif 
   
    error = sourceComponent->disablePort(sourcePort);
    OMX_TRACE(error);
    
    error = destinationComponent->disablePort(destinationPort);
    OMX_TRACE(error, debugString);
      
    
    
    

    
    error = OMX_SetupTunnel(destinationComponent->getHandle(), destinationPort, destinationNullSink.getHandle(), destinationNullSink.getInputPort());
    OMX_TRACE(error, debugString);
    if(error == OMX_ErrorNone)
    {
        ofLogVerbose(__func__) << destinationComponent->getName() << " TUNNELED TO NULL SUCCESS";
    }else
    {
         ofLogVerbose(__func__) << destinationComponent->getName() << " TUNNELED TO NULL FAIL";
        if(error == OMX_ErrorIncorrectStateOperation)
        {
            ofLogVerbose(__func__) << getOMXStateString(destinationComponent->getState());

        }
    }
    
    error = OMX_SetupTunnel(sourceComponent->getHandle(), sourcePort, sourceNullSink.getHandle(), sourceNullSink.getInputPort());
    OMX_TRACE(error, debugString);
    if(error == OMX_ErrorNone)
    {
        ofLogVerbose(__func__) << sourceComponent->getName() << " TUNNELED TO NULL SUCCESS";
    }else
    {
        ofLogVerbose(__func__) << sourceComponent->getName() << " TUNNELED TO NULL FAIL";
        if(error == OMX_ErrorIncorrectStateOperation)
        {
            ofLogVerbose(__func__) << getOMXStateString(sourceComponent->getState());
            
        }
    }
    
    
    

    
    unlock();
    isEstablished = false;
    return error;
}

OMX_ERRORTYPE Tunnel::Establish(bool portSettingsChanged)
{
    lock();
    
    OMX_ERRORTYPE error = OMX_ErrorNone;
    OMX_PARAM_U32TYPE param;
    OMX_INIT_STRUCTURE(param);
    if(!sourceComponent || !destinationComponent || !sourceComponent->getHandle() || !destinationComponent->getHandle())
    {
        unlock();
        return OMX_ErrorUndefined;
    }
    
    error = sourceComponent->setState(OMX_StateIdle);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
        unlock();
        return error;
    }
#ifdef DEBUG_TUNNELS
    ofLogVerbose(__func__) << sourceComponent->getName() << " TUNNELING TO " << destinationComponent->getName();
    ofLogVerbose(__func__) << "portSettingsChanged: " << portSettingsChanged;
#endif
    if(portSettingsChanged)
    {
        error = sourceComponent->waitForEvent(OMX_EventPortSettingsChanged);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone)
        {
            unlock();
            return error;
        }
    }
    
    error = sourceComponent->disablePort(sourcePort);
    OMX_TRACE(error);
    
    
    error = destinationComponent->disablePort(destinationPort);
    OMX_TRACE(error);
    
    error = OMX_SetupTunnel(sourceComponent->getHandle(), sourcePort, destinationComponent->getHandle(), destinationPort);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
        unlock();
        return error;
    }
    else
    {
        isEstablished =true;
    }
    
    error = sourceComponent->enablePort(sourcePort);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
        unlock();
        return error;
    }
    
    error = destinationComponent->enablePort(destinationPort);
    OMX_TRACE(error);
    if(error != OMX_ErrorNone)
    {
        unlock();
        return error;
    }
    
    if(destinationComponent->getState() == OMX_StateLoaded)
    {
        //important to wait for audio
        error = destinationComponent->waitForCommand(OMX_CommandPortEnable, destinationPort);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone)
        {
            unlock();
            return error;
        }
        error = destinationComponent->setState(OMX_StateIdle);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone)
        {
            unlock();
            return error;
        }
    }
    
    error = sourceComponent->waitForCommand(OMX_CommandPortEnable, sourcePort);
    if(error != OMX_ErrorNone)
    {
        unlock();
        return error;
    }
    
    havePortSettingsChanged = portSettingsChanged;
    
    unlock();
    
    
    return error;
}
