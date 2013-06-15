#pragma once

#include "ofMain.h"

class SSHKeyListenerEventData
{
public:
	SSHKeyListenerEventData(char character_)
	{
		character = character_;
	}
	char character;
};

class SSHKeyListener
{
public:
	virtual void onCharacterReceived(SSHKeyListenerEventData& e) = 0;
};

class ConsoleListener: public ofThread
{
public:
	SSHKeyListener* listener;
	ConsoleListener()
	{
		listener = NULL;
	}
	void setup(SSHKeyListener* listener_)
	{
		listener = listener_;
		startThread(false, false);
	}
	void threadedFunction()
	{
		while (isThreadRunning()) 
		{
			char buffer[10];
			while(fgets(buffer, 10 , stdin) != NULL)
			{
				//ofLogVerbose() << buffer;
				SSHKeyListenerEventData eventData(buffer[0]);
				listener->onCharacterReceived(eventData);
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