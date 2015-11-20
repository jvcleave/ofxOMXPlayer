#include "Tunnel.h"

#pragma mark Tunnel

Tunnel::Tunnel()
{
    sourceComponentName = "UNDEFINED_sourceComponentName";
    destinationComponentName = "UNDEFINED_destinationComponentName";
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
        Deestablish();
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
    if(sourceComponent->getHandle())
    {
        sourceComponent->flushAll();
    }
    
    if(destinationComponent->getHandle())
    {
        destinationComponent->flushAll();
    }
    
    unlock();
    return OMX_ErrorNone;
}

OMX_ERRORTYPE Tunnel::Deestablish(bool doWait)
{
    
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
    
    if(sourceComponent->getHandle() && havePortSettingsChanged && doWait)
    {
        error = sourceComponent->waitForEvent(OMX_EventPortSettingsChanged);
        OMX_TRACE(error);
    }
    if(sourceComponent->getHandle())
    {
        error = sourceComponent->disablePort(sourcePort);
        OMX_TRACE(error);
    }
    if(destinationComponent->getHandle())
    {
        error = destinationComponent->disablePort(destinationPort);
        OMX_TRACE(error);
    }
    if(sourceComponent->getHandle())
    {
        error = OMX_SetupTunnel(sourceComponent->getHandle(), sourcePort, NULL, 0);
        OMX_TRACE(error);
    }
    
    if(destinationComponent->getHandle())
    {
        error = OMX_SetupTunnel(destinationComponent->getHandle(), destinationPort, NULL, 0);
        OMX_TRACE(error);
    }
    unlock();
    isEstablished = false;
    return OMX_ErrorNone;
}

OMX_ERRORTYPE Tunnel::Establish(bool portSettingsChanged)
{
    lock();
    
    OMX_ERRORTYPE error = OMX_ErrorNone;
    OMX_PARAM_U32TYPE param;
    OMX_INIT_STRUCTURE(param);
    if(!sourceComponent || !destinationComponent)
    {
        unlock();
        return OMX_ErrorUndefined;
    }
    
    if(sourceComponent->getState() == OMX_StateLoaded)
    {
        error = sourceComponent->setState(OMX_StateIdle);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone)
        {
            unlock();
            return error;
        }
    }
    ofLogVerbose(__func__) << sourceComponent->getName() << " TUNNELING TO " << destinationComponent->getName();
    ofLogVerbose(__func__) << "portSettingsChanged: " << portSettingsChanged;
    
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
    if(sourceComponent->getHandle())
    {
        error = sourceComponent->disablePort(sourcePort);
        OMX_TRACE(error);
    } 
    
    if(destinationComponent->getHandle())
    {
        error = destinationComponent->disablePort(destinationPort);
        OMX_TRACE(error);
    }
    
    if(sourceComponent->getHandle() && destinationComponent->getHandle())
    {
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
    }
    else
    {
        unlock();
        return OMX_ErrorUndefined;
    }
    
    if(sourceComponent->getHandle())
    {
        error = sourceComponent->enablePort(sourcePort);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone)
        {
            unlock();
            return error;
        }
    }
    
    if(destinationComponent->getHandle())
    {
        error = destinationComponent->enablePort(destinationPort);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone)
        {
            unlock();
            return error;
        }
    }
    
    if(destinationComponent->getHandle())
    {
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
    }
    
    if(sourceComponent->getHandle())
    {
        error = sourceComponent->waitForCommand(OMX_CommandPortEnable, sourcePort);
        if(error != OMX_ErrorNone)
        {
            unlock();
            return error;
        }
    }
    
    havePortSettingsChanged = portSettingsChanged;
    
    unlock();
    
    
    return error;
}
