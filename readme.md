STATUS: PERPETUAL DEVELOPMENT

DESCRIPTION:
This is an ofxAddon that allows textures to be read by openFrameworks on the Raspberry Pi via OpenMax based video. Textures can then be manipulated and used for shaders, etc.

REQUIREMENTS:
GPU split set to 128 (may need to be higher for larger videos)
openFrameworks version 0.8 or higher http://www.openframeworks.cc/setup/raspberrypi/

USAGE: (NEW) Clone into openFrameworks/addons

There 2 modes of Playback, textured or non-textured. 

Non-Textured:
 - By default is played fullscreen video
 - OF is still able to run other processes in the background. 
 - 1080p video playback is typically good

Textured:
Allows the use of
 - shaders
 - pixel access
 - drawing overlays
720p video works best here

Audio:
Audio can be played back through HDMI or headphone jack
May be disabled to save resources

Headphone jack may require this tweak:
https://gist.github.com/jvcleave/4972661

To use HDMI Audio you may need to add the below line to /boot/config.txt and reboot. See http://elinux.org/RPiconfig for more details

hdmi_drive=2

EXAMPLES:
example-basic:
Playback of video in texture mode (default)

example-playlist:
Playback of a folder of videos in texture mode

example-pixels:
Example of pixel access that is needed for OpenCv operations/Saving images, etc

example-shader:
Use of shaders, fbos and video

example-multiple-players:
EXPERIMENTAL: shows some of the more "alpha" features
Multiple players
Usage of a display area to play non-textured video non-fullscreen

COMPRESSION RECOMMENDATIONS:
Use MPEGStreamclip (http://www.squared5.com/)
H.264
Quality 50%
PCM Audio (more compatible with HDMI)

TODO:
Multiple video support for textured Player
Implement better Seeking
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

Test video files from:
http://www.bigbuckbunny.org/index.php/download/



