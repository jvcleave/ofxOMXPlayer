### REQUIREMENTS:   
- GPU split set to 128 (may need to be higher for larger videos)
- openFrameworks 10, Raspberry Pi 0-3 http://www.openframeworks.cc/setup/raspberrypi/
- run install_depends.sh for necessary libav dependencies

### CURRENT KNOWN MAJOR ISSUES:  
Videos with and audio track (even silent) work best and allow more accurate looping/frame tracking

### DESCRIPTION:   
This is an openFrameworks video player addon for the Raspberry Pi. The video player is hardware accelerated and can provide textures to openFrameworks to be used for shaders, etc.

### USAGE:   

Use [Releases](https://github.com/jvcleave/ofxOMXPlayer/releases) unless you are contributing or looking for bleeding edge. Releases contain tested versions that work for specific versions of openFrameworks.


Clone into openFrameworks/addons

There 2 modes of Playback, Direct (aka Non-textured) or Textured. 

#### Direct:
- Renders directly to the screen, no texture access or pixels. This is most similar to what you will see in omxplayer
- By default is played fullscreen video
- OF is still able to run other processes in the background. 
- 1080p video playback is typically good

#### Textured:   
Allows the use of
- shaders
- pixel access
- drawing overlays
- 720p video works best here

#### Audio:   
- Yes


Headphone jack may require this tweak:   
https://gist.github.com/jvcleave/4972661

To use HDMI Audio you may need to add the below line to /boot/config.txt and reboot. See http://elinux.org/RPiconfig for more details

hdmi_drive=2

### EXAMPLES:   
example-basic:   
Playback of video in texture mode (default)

#### example-texture-mode:
Playback of video in texture mode (default)

#### example-direct-mode:
Playback of video in direct mode (no textures/pixel access)

#### example-multiple-players:   
Simultaneous playback 2 videos in non-texture mode

#### example-playlist:   
Playback of a folder of videos in texture mode

#### example-pixels:   
Example of pixel access that is needed for OpenCv operations/Saving images, etc

#### example-http-stream:   
plays network streamed video

#### example-shader:   
Use of shaders

#### example-playback-controls:   
tried to keep these close to omxplayer

#### example-wrapper:   
ofRPIVideoPlayer extends ofVideoPlayer in hopes to be  a drop in replacement for ofVideoPlayer, 

There may be other examples in Master branch but the above are most likely to remain

### COMPRESSION RECOMMENDATIONS:   
- Use MPEGStreamclip (http://www.squared5.com/)
- H.264
- Quality 50%
- PCM Audio (more compatible with HDMI)

### CREDITS:   
Majority of the code is based off of 
https://github.com/popcornmix/omxplayer

See  COPYING for license details inherited from omxplayer/xbmc

Test video files from:
http://www.bigbuckbunny.org/index.php/download/

## REHAUL NOTES:
I have basically rewritten the underlying engine in order to keep as close to omxplayer as possible.
It seems to be about the same performance-wise. Noticeable changes are better looping and audio. This addon *should* play most files the same way as omxplayer. ofxOMXPlayerEngine is basically omxplayer.cpp with additions to support the openFrameworks model and textures.

This addon no longer has static/custom compiled versions of libav.

I closed all the previous issues as I won't be doing anything to the old code.

TODO:
I still need to port back some of the direct mode display options and some stuff around the filters. There is also some functionality missing around switching the audio settings at runtime. 








