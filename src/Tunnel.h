#pragma once

#include "Component.h"

class Tunnel
{
public:
    Tunnel();
    ~Tunnel();
    
    void init(Component*, unsigned int, Component*, unsigned int);
    OMX_ERRORTYPE flush();
    OMX_ERRORTYPE Deestablish(bool doWait = true);
    OMX_ERRORTYPE Establish(bool portSettingsChanged);
    string sourceComponentName;
    string destinationComponentName;
private:
    bool isEstablished;
    pthread_mutex_t   m_lock;
    bool            havePortSettingsChanged;
    Component*      sourceComponent;
    Component*      destinationComponent;
    unsigned int    sourcePort;
    unsigned int    destinationPort;
    void            lock();
    void            unlock();
};