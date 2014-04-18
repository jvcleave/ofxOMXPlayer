#pragma once


#include "DllAvCodec.h"

extern "C" {
	#ifndef HAVE_MMX
#define HAVE_MMX
	#endif
	#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
	#endif
	#ifndef __GNUC__
#pragma warning(disable:4244)
	#endif
#include <libavformat/avformat.h>
}

/* Flag introduced without a version bump */
#ifndef AVSEEK_FORCE
#define AVSEEK_FORCE 0x20000
#endif

typedef int64_t offset_t;
