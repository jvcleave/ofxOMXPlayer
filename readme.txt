STATUS: PERPETUAL DEVELOPMENT

DESCRIPTION:
This is an ofxAddon that allows textures to be read by openFrameworks on the Raspberry Pi via OpenMax based video. Textures can then be manipulated and used for shaders, etc.

REQUIREMENTS:
GPU split set to 128 (may need to be higher for larger videos)
openFrameworks version 0.8 or higher http://www.openframeworks.cc/setup/raspberrypi/

USAGE: This is not currently setup to run in the openFrameworks/addons folder. Clone into openFrameworks/apps/myApps/ofxOMXPlayer

There are now 2 modes of Playback, textured or non-textured. 

Non-Textured:
Is limited to fullscreen video but OF is still able to run other processes in the background. No other drawing can take place (overlays, etc). 1080p video plays back fine in this mode

Textured:
Allows the use of shaders, pixel manipulation, drawing overlays, etc, 720p video works best here

MISC:
Audio can be played back through HDMI or headphone jack
Headphone jack may require this tweak:
https://gist.github.com/jvcleave/4972661

HDMI Audio may require this line added to /boot/config.txt
To use HDMI Audio you may need to add the below line to /boot/config.txt and reboot. See http://elinux.org/RPiconfig for more details
	
hdmi_drive=2


There are 3 apps in this repo. You can switch which one runs in main.cpp

testApp
The most "stable" of the 3 and demos the most tested functionality. 
This is typically the default when you clone/download but I sometimes forget to set it back before pushing

developApp
I use the developApp to test features. It uses a ConsoleListener class that allows you to input
keys via an SSH terminal which is the way I develop/test

playlistApp
The newest of the 3 and demonstrates the ability to play multiple videos in non-texture mode. There is currently a bit of glitching while as files switch.
This also demonstrates the ofxOMXPlayerListener pattern available. If your app extends ofxOMXPlayerListener you will receive an event when the video ends


TODO:
Multiple video support for textured Player
Implement better Seeking
Get into ofxAddons structure
Re-introduce support for test.h264 like files with no metadata
General cleanup (many properties are public)
Possibly have it extend ofBaseVideoPlayer

CREDITS:
Majority of the code is based off of 
https://github.com/popcornmix/omxplayer

with some modifications by xbmc:
https://github.com/xbmc/xbmc

See addons/ofxOMXPlayer/libs/omxplayer/COPYING for license details inherited from omxplayer/xbmc

Thanks to OtherCrashOverride for helping me figure out the linking issue to enable callbacks
https://github.com/OtherCrashOverride

Thanks to @bendenoz for help with the looping
https://github.com/bendenoz

Included shader from
http://www.iquilezles.org/apps/shadertoy/?p=Postpro

Test Video file from:
http://www.bigbuckbunny.org/index.php/download/

Recompressed with MPEGStreamclip (http://www.squared5.com/) to:
Duration: 0:01:00
Data Size: 14.69 MB
Bit Rate: 2.05 Mbps

Video Tracks:
H.264, 1280 × 720, 24 fps, 1.79 Mbps

Audio Tracks:
MPEG-4 Audio stereo, 48 kHz, 256 kbps



