# All variables and this file are optional, if they are not present the PG and the
# makefiles will try to parse the correct values from the file system.
#
# Variables that specify exclusions can use % as a wildcard to specify that anything in
# that position will match. A partial path can also be specified to, for example, exclude
# a whole folder from the parsed paths from the file system
#
# Variables can be specified using = or +=
# = will clear the contents of that variable both specified from the file or the ones parsed
# from the file system
# += will add the values to the previous ones in the file or the ones parsed from the file 
# system
# 
# The PG can be used to detect errors in this file, just create a new project with this addon 
# and the PG will write to the console the kind of error and in which line it is

meta:
	ADDON_NAME = ofxOMXPlayer
	ADDON_DESCRIPTION = Hardware accelerated video player for the Raspberry Pi
	ADDON_AUTHOR = Jason Van Cleave
	ADDON_TAGS = "raspberry pi, video player"
	ADDON_URL = https://github.com/jvcleave/ofxOMXPlayer

common:
	# dependencies with other addons, a list of them separated by spaces 
	# or use += in several lines
	# ADDON_DEPENDENCIES =
	
	OFXOMXPLAYER_ROOT = $(OF_ROOT)/addons/ofxOMXPlayer
	
	# include search paths, this will be usually parsed from the file system
	# but if the addon or addon libraries need special search paths they can be
	# specified here separated by spaces or one per line using +=
	ADDON_INCLUDES = $(OFXOMXPLAYER_ROOT)/src $(OFXOMXPLAYER_ROOT)/libs/ffmpeg/include 
		
	# any special flag that should be passed to the compiler when using this
	# addon
	ADDON_CFLAGS = -I$(OFXOMXPLAYER_ROOT)/src -I$(OFXOMXPLAYER_ROOT)/libs/ffmpeg/include -fPIC -U_FORTIFY_SOURCE -Wall -ftree-vectorize -ftree-vectorize -Wno-deprecated-declarations -Wno-sign-compare -Wno-unknown-pragmas

	# any special flag that should be passed to the linker when using this
	# addon, also used for system libraries with -lname
	
	FFMPEG_LIBS = $(OFXOMXPLAYER_ROOT)/libs/ffmpeg/lib
	FORMAT_STATIC=$(FFMPEG_LIBS)/libavformat.a
	CODEC_STATIC=$(FFMPEG_LIBS)/libavcodec.a
	SCALE_STATIC=$(FFMPEG_LIBS)/libswscale.a
	UTIL_STATIC=$(FFMPEG_LIBS)/libavutil.a

	#unused but available
	FILTER_STATIC=$(FFMPEG_LIBS)/libavfilter.a
	POSTPROC_STATIC=$(FFMPEG_LIBS)/libpostproc.a
	DEVICE_STATIC=$(FFMPEG_LIBS)/libavdevice.a
	RESAMPLE_STATIC=$(FFMPEG_LIBS)/libswresample.a

	ADDON_LDFLAGS=-L$(FFMPEG_LIBS) $(FORMAT_STATIC) $(CODEC_STATIC) $(SCALE_STATIC) $(UTIL_STATIC) $(RESAMPLE_STATIC) $(FILTER_STATIC) -lm
	
	
	# linux only, any library that should be included in the project using
	# pkg-config
	# ADDON_PKG_CONFIG_LIBRARIES =
	
	# osx/iOS only, any framework that should be included in the project
	# ADDON_FRAMEWORKS =
	
	# source files, these will be usually parsed from the file system looking
	# in the src folders in libs and the root of the addon. if your addon needs
	# to include files in different places or a different set of files per platform
	# they can be specified here
	# ADDON_SOURCES =
	
	# some addons need resources to be copied to the bin/data folder of the project
	# specify here any files that need to be copied, you can use wildcards like * and ?
	# ADDON_DATA = 
	
	# when parsing the file system looking for libraries exclude this for all or
	# a specific platform
	#ADDON_LIBS_EXCLUDE = $(OFXOMXPLAYER_ROOT)/libs/ffmpeg/include%
	
