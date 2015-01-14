#pragma once

#include "ofMain.h"
#include <fcntl.h>

class KeyListenerEventData
{
public:
    KeyListenerEventData(char character, void* listener)
    {
        this->character = character;
        this->listener = listener;
    }
    void* listener;
    
    char character;
};

class KeyListener
{
public:
    virtual void onCharacterReceived(KeyListenerEventData& e) = 0;
};


class TerminalListener : public Poco::Runnable
{
private:
    struct termios orig_termios;
public:
    KeyListener* listener;
    Poco::Thread thread;
    TerminalListener()
    {
        
        listener = NULL;
        
    }
    
    
    void setup(KeyListener* listener_)
    {
        listener = listener_;
        struct termios new_termios;
        
        tcgetattr(STDIN_FILENO, &orig_termios);
        
        new_termios = orig_termios;
        new_termios.c_lflag &= ~(ICANON | ECHO | ECHOCTL | ECHONL);
        new_termios.c_cflag |= HUPCL;
        new_termios.c_cc[VMIN] = 0;
        
        tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
        thread.start(*this);
    }
    
    void run()
    {
        
        while (thread.isRunning())
        {
            if (listener != NULL)
            {
                int ch[8];
                int chnum = 0;
                
                while ((ch[chnum] = getchar()) != EOF)
                {
                    chnum++;
                }
                
                if (chnum > 1)
                {
                    ch[0] = ch[chnum - 1];
                }else
                {
                    (ch[chnum - 2] << 8);
                }
                
                if (chnum > 0)
                {
                    //ofLog(OF_LOG_VERBOSE, "TerminalListener: character %c", ch[0]);
                    KeyListenerEventData eventData(ch[0], (void *)this);
                    listener->onCharacterReceived(eventData);
                }
                thread.sleep(100);
            }
        }
    }
    
    void close()
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
        thread.tryJoin(50);
        listener = NULL;
    }
    
    ~TerminalListener()
    {
        close();
        //ofLogVerbose(__func__) << " END";
    }
    
};
