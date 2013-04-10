STATUS: IN DEVELOPMENT

USAGE: This is not currently setup to run in the addons folder. Clone into openFrameworks/apps/myApps/ofxOMXPlayer

keep open a second terminal and run 

$ killall -9 ofxOMXPlayer

if it hangs (usually on exit)

Now with Audio!
This was only tested with the audio via the headphone jack with this tweak:
https://gist.github.com/jvcleave/4972661

test videos here:
http://www.jvcref.com/files/PI/video/

DESCRIPTION:
This is an ofxAddon that allows textures to be read by openFrameworks on the Raspberry Pi via OpenMax based video. Textures can then be manipulated and used for shaders, etc.

REQUIREMENTS:
GPU split set to 128 (may need to be higher for larger videos)
develop-raspberrypi branch of https://github.com/openFrameworks-RaspberryPi/openFrameworks


CREDITS:
Majority of the code is based off of 
https://github.com/huceke/omxplayer

with some modifications by xbmc:
https://github.com/xbmc/xbmc

See addons/ofxOMXPlayer/libs/omxplayer/COPYING for license details inherited from omxplayer/xbmc

Thanks to OtherCrashOverride for helping me figure out the linking issue to enable callbacks
https://github.com/OtherCrashOverride

Included shader from
http://www.iquilezles.org/apps/shadertoy/?p=Postpro
