extern inline  
string omxErrorToString(OMX_ERRORTYPE error)
{
    return OMX_Maps::getInstance().getOMXError(error);
}


#define OMX_LOG_LEVEL_DEV 1
#define OMX_LOG_LEVEL_ERROR_ONLY 2
#define OMX_LOG_LEVEL_VERBOSE 3
#define OMX_LOG_LEVEL_SILENT 9

#ifndef OMX_LOG_LEVEL
#define OMX_LOG_LEVEL OMX_LOG_LEVEL_ERROR_ONLY
#endif

extern inline  
void logOMXError(OMX_ERRORTYPE error, string comments="", string functionName="", int lineNumber=0)
{
    string commentLine = " ";
    if(!comments.empty())
    {
        commentLine = " " + comments + " ";
    }
    
    switch(OMX_LOG_LEVEL)
    {
        case OMX_LOG_LEVEL_DEV:
        {
            if(error != OMX_ErrorNone)
            {
                ofLogError(functionName) << lineNumber << commentLine << omxErrorToString(error);
            }else
            {
                ofLogVerbose(functionName) << lineNumber << commentLine << omxErrorToString(error);
            }
            break;
        }
        case OMX_LOG_LEVEL_ERROR_ONLY:
        {
            
            if(error != OMX_ErrorNone)
            {
                ofLogError(functionName) << lineNumber << commentLine << omxErrorToString(error);
            }
            break;
        }
        case OMX_LOG_LEVEL_VERBOSE:
        {
            ofLogError(functionName)  << commentLine << omxErrorToString(error);
            break;
        }
        default:
        {
            break;
        }
    }
    
}



#define OMX_TRACE_1_ARGS(error)                      logOMXError(error, "", __func__, __LINE__);
#define OMX_TRACE_2_ARGS(error, comments)            logOMXError(error, comments, __func__, __LINE__);
#define OMX_TRACE_3_ARGS(error, comments, whatever)  logOMXError(error, comments, __func__, __LINE__);

#define GET_OMX_TRACE_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4
#define OMX_TRACE_MACRO_CHOOSER(...) GET_OMX_TRACE_4TH_ARG(__VA_ARGS__, OMX_TRACE_3_ARGS, OMX_TRACE_2_ARGS, OMX_TRACE_1_ARGS, )

#define ENABLE_OMX_TRACE 1

#if defined (ENABLE_OMX_TRACE)
//#warning enabling OMX_TRACE
#define OMX_TRACE(...) OMX_TRACE_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)
#else
#warning  disabling OMX_TRACE
#define OMX_TRACE(...)
#endif


//#define DEBUG_AUDIO 1
//#define DEBUG_EVENTS 1
//#define DEBUG_STATES 1
//#define DEBUG_COMMANDS 1
//#define DEBUG_PORTS 1
//#define DEBUG_VIDEO_DISPLAY 1


