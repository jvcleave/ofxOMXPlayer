#pragma once

#include "ofMain.h"

class SSHKeyListenerEventData
{
public:
	SSHKeyListenerEventData(char character_, void* listener_)
	{
		character = character_;
		listener = listener_;
	}
	void* listener;
	
	char character;
};

class SSHKeyListener
{
public:
	virtual void onCharacterReceived(SSHKeyListenerEventData& e) = 0;
};

class ConsoleListener: public Poco::Runnable
{
public:
	SSHKeyListener* listener;
	Poco::Thread thread;
	ConsoleListener()
	{
		listener = NULL;
		
	}
	
	~ConsoleListener()
	{
		thread.tryJoin(50);
		ofLogVerbose() << "~ConsoleListener END";
	}
	
	void setup(SSHKeyListener* listener_)
	{
		listener = listener_;
		//thread.setOSPriority(Poco::Thread::getMinOSPriority());
		thread.start(*this);
	}
	void run()
	{
		
		while (thread.isRunning()) 
		{
			if (listener != NULL) 
			{
				
				char buffer[10];
				if (fgets(buffer, 10 , stdin) != NULL) 
				{
					
					SSHKeyListenerEventData eventData(buffer[0], (void *)this);
					listener->onCharacterReceived(eventData);
					
				}
			}
		}
	}
	
};



#if 0
USAGE:

1.add to testApp.h

#include "ConsoleListener.h"

//extend testApp 
class testApp : public ofBaseApp, public SSHKeyListener

2. add required callback definition and instance

void onCharacterReceived(SSHKeyListenerEventData& e);
ConsoleListener consoleListener;

3. add to testApp.cpp

void testApp::onCharacterReceived(SSHKeyListenerEventData& e)
{
	keyPressed((int)e.character);
}

add in testApp::setup()

consoleListener.setup(this);

4. and later
void testApp::keyPressed  (int key){
	
	if (key == 'e') 
	{
		ofLogVerbose() << "e pressed!";
	}
}
#endif