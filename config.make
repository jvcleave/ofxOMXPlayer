################################################################################
# CONFIGURE PROJECT MAKEFILE (optional)
#   This file is where we make project specific configurations.
################################################################################

################################################################################
# OF ROOT
#   The location of your root openFrameworks installation
#       (default) OF_ROOT = ../../.. 
################################################################################
# OF_ROOT = ../../..

################################################################################
# App Name
# 	Custom Application Name
#       (default) APPNAME = (this project's folder name)
# App Name Suffix
#       (default) APPNAME_SUFFIX = .app
#
################################################################################
# APPNAME = customAppName
# APPNAME_SUFFIX = .app

################################################################################
# PROJECT ROOT
#   The location of the project - a starting place for searching for files
#       (default) PROJECT_ROOT = . (this directory)
#    
################################################################################
# PROJECT_ROOT = .

################################################################################
# PROJECT SPECIFIC CHECKS
#   This is a project defined section to create internal makefile flags to 
#   conditionally enable or disable the addition of various features within 
#   this makefile.  For instance, if you want to make changes based on whether
#   GTK is installed, one might test that here and create a variable to check. 
################################################################################
# None

################################################################################
# PROJECT EXTERNAL SOURCE PATHS
#   These are fully qualified paths that are not within the PROJECT_ROOT folder.
#   Like source folders in the PROJECT_ROOT, these paths are subject to 
#   exlclusion via the PROJECT_EXLCUSIONS list.
#
#     (default) PROJECT_EXTERNAL_SOURCE_PATHS = (blank) 
#
#   Note: Leave a leading space when adding list items with the += operator
################################################################################
# PROJECT_EXTERNAL_SOURCE_PATHS = 

################################################################################
# PROJECT EXCLUSIONS
#   These makefiles assume that all folders in your current project directory 
#   and any listed in the PROJECT_EXTERNAL_SOURCH_PATHS are are valid locations
#   to look for source code. The any folders or files that match any of the 
#   items in the PROJECT_EXCLUSIONS list below will be ignored.
#
#   Each item in the PROJECT_EXCLUSIONS list will be treated as a complete 
#   string unless teh user adds a wildcard (%) operator to match subdirectories.
#   GNU make only allows one wildcard for matching.  The second wildcard (%) is
#   treated literally.
#
#      (default) PROJECT_EXCLUSIONS = (blank)
#
#		Will automatically exclude the following:
#
#			$(PROJECT_ROOT)/bin%
#			$(PROJECT_ROOT)/obj%
#			$(PROJECT_ROOT)/%.xcodeproj
#
#   Note: Leave a leading space when adding list items with the += operator
################################################################################
PROJECT_EXCLUSIONS =$(PROJECT_ROOT)/addons/ofxOMXPlayer/libs/ffmpeg/include%

################################################################################
# PROJECT LINKER FLAGS
#	These flags will be sent to the linker when compiling the executable.
#
#		(default) PROJECT_LDFLAGS = -Wl,-rpath=./libs
#
#   Note: Leave a leading space when adding list items with the += operator
################################################################################

# Currently, shared libraries that are needed are copied to the 
# $(PROJECT_ROOT)/bin/libs directory.  The following LDFLAGS tell the linker to
# add a runtime path to search for those shared libraries, since they aren't 
# incorporated directly into the final executable application binary.


FFMPEG_LIBS = $(PROJECT_ROOT)/addons/ofxOMXPlayer/libs/ffmpeg/lib
FORMAT_STATIC=$(FFMPEG_LIBS)/libavformat.a
CODEC_STATIC=$(FFMPEG_LIBS)/libavcodec.a
SCALE_STATIC=$(FFMPEG_LIBS)/libswscale.a
UTIL_STATIC=$(FFMPEG_LIBS)/libavutil.a

#unused but available
FILTER_STATIC=$(FFMPEG_LIBS)/libavfilter.a
POSTPROC_STATIC=$(FFMPEG_LIBS)/libpostproc.a
DEVICE_STATIC=$(FFMPEG_LIBS)/libavdevice.a
RESAMPLE_STATIC=$(FFMPEG_LIBS)/libswresample.a

PROJECT_LDFLAGS=-L$(FFMPEG_LIBS) $(FORMAT_STATIC) $(CODEC_STATIC) $(SCALE_STATIC) $(UTIL_STATIC) $(RESAMPLE_STATIC) $(FILTER_STATIC) -lm

################################################################################
# PROJECT DEFINES
#   Create a space-delimited list of DEFINES. The list will be converted into 
#   CFLAGS with the "-D" flag later in the makefile.
#
#		(default) PROJECT_DEFINES = (blank)
#
#   Note: Leave a leading space when adding list items with the += operator
################################################################################
PROJECT_DEFINES += __STDC_CONSTANT_MACROS 
PROJECT_DEFINES += __STDC_LIMIT_MACROS 
PROJECT_DEFINES += TARGET_POSIX 
PROJECT_DEFINES += _LINUX
#http://en.wikipedia.org/wiki/Position-independent_code 
PROJECT_DEFINES += PIC 
#Defining _REENTRANT causes the compiler to use thread safe (i.e. re-entrant) versions of several functions in the C library.
PROJECT_DEFINES += _REENTRANT 
PROJECT_DEFINES += OMX 
PROJECT_DEFINES += OMX_SKIP64BIT 
#PROJECT_DEFINES += FF_API_CODEC_ID 

################################################################################
# PROJECT CFLAGS
#   This is a list of fully qualified CFLAGS required when compiling for this 
#   project.  These CFLAGS will be used IN ADDITION TO the PLATFORM_CFLAGS 
#   defined in your platform specific core configuration files. These flags are
#   presented to the compiler BEFORE the PROJECT_OPTIMIZATION_CFLAGS below. 
#
#		(default) PROJECT_CFLAGS = (blank)
#
#   Note: Before adding PROJECT_CFLAGS, note that the PLATFORM_CFLAGS defined in 
#   your platform specific configuration file will be applied by default and 
#   further flags here may not be needed.
#
#   Note: Leave a leading space when adding list items with the += operator
################################################################################
PROJECT_CFLAGS = -I$(PROJECT_ROOT)/addons/ofxOMXPlayer/libs/ffmpeg/include -fPIC -U_FORTIFY_SOURCE -Wall -ftree-vectorize -ftree-vectorize -Wno-deprecated-declarations -Wno-sign-compare
 

################################################################################
# PROJECT OPTIMIZATION CFLAGS
#   These are lists of CFLAGS that are target-specific.  While any flags could 
#   be conditionally added, they are usually limited to optimization flags. 
#   These flags are added BEFORE the PROJECT_CFLAGS.
#
#   PROJECT_OPTIMIZATION_CFLAGS_RELEASE flags are only applied to RELEASE targets.
#
#		(default) PROJECT_OPTIMIZATION_CFLAGS_RELEASE = (blank)
#
#   PROJECT_OPTIMIZATION_CFLAGS_DEBUG flags are only applied to DEBUG targets.
#
#		(default) PROJECT_OPTIMIZATION_CFLAGS_DEBUG = (blank)
#
#   Note: Before adding PROJECT_OPTIMIZATION_CFLAGS, please note that the 
#   PLATFORM_OPTIMIZATION_CFLAGS defined in your platform specific configuration 
#   file will be applied by default and further optimization flags here may not 
#   be needed.
#
#   Note: Leave a leading space when adding list items with the += operator
################################################################################
# PROJECT_OPTIMIZATION_CFLAGS_RELEASE = 
# PROJECT_OPTIMIZATION_CFLAGS_DEBUG = 

################################################################################
# PROJECT COMPILERS
#   Custom compilers can be set for CC and CXX
#		(default) PROJECT_CXX = (blank)
#		(default) PROJECT_CC = (blank)
#   Note: Leave a leading space when adding list items with the += operator
################################################################################
# PROJECT_CXX = 
# PROJECT_CC = 
