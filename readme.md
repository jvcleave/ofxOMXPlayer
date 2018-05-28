STATUS: PERPETUAL DEVELOPMENT    
Use [Releases] (https://github.com/jvcleave/ofxOMXPlayer/releases) unless you are contributing or looking for bleeding edge (requires blood). Releases contain tested versions that work for specific versions of openFrameworks.

CURRENT KNOWN MAJOR ISSUES:   
Videos with and audio track (even silent) work best and allow more accurate looping/frame tracking

DESCRIPTION:   
This is an openFrameworks video player addon for the Raspberry Pi. The video player is hardware accelerated and can provide textures to openFrameworks to be used for shaders, etc.

REQUIREMENTS:   
GPU split set to 128 (may need to be higher for larger videos)
openFrameworks version 0.8 or higher http://www.openframeworks.cc/setup/raspberrypi/
libavfilter-dev, install it with `sudo apt-get install libavfilter-dev`

USAGE:   
(NEW) Clone into openFrameworks/addons

There 2 modes of Playback, Direct (aka Non-textured) or Textured. 

Direct:
 - Renders directly to the screen, no texture access or pixels. This is most similar to what you will see in omxplayer
 - By default is played fullscreen video
 - OF is still able to run other processes in the background. 
 - 1080p video playback is typically good

Textured:   
Allows the use of
 - shaders
 - pixel access
 - drawing overlays
 - 720p video works best here

Audio:   
 - Audio can be played back through HDMI or headphone jack
 - May be disabled to save resources
 - initialVolume can be passed


Headphone jack may require this tweak:   
https://gist.github.com/jvcleave/4972661

To use HDMI Audio you may need to add the below line to /boot/config.txt and reboot. See http://elinux.org/RPiconfig for more details

hdmi_drive=2

EXAMPLES:   
example-basic:   
Playback of video in texture mode (default)

example-multiple-players:   
Simultaneous playback 2 videos in non-texture mode

example-playlist:   
Playback of a folder of videos in texture mode

example-pixels:   
Example of pixel access that is needed for OpenCv operations/Saving images, etc

example-restartMovie:   
restart a movie by pressing "r" + ENTER in ssh session (or "r" on attached keyboard)

example-shader:   
Use of shaders, fbos and video

example-multiple-players:   
 - Multiple players
 - Usage of a display area to play non-textured video non-fullscreen

There may be other examples in Master branch but the above are most likely to remain

COMPRESSION RECOMMENDATIONS:   
 - Use MPEGStreamclip (http://www.squared5.com/)
 - H.264
 - Quality 50%
 - PCM Audio (more compatible with HDMI)

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



