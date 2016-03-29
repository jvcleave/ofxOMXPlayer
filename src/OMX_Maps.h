#ifndef OMX_Maps_h
#define OMX_Maps_h

#include "ofMain.h"

#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>
#include <IL/OMX_Index.h>
#include <IL/OMX_Image.h>
#include <IL/OMX_Video.h>
#include <IL/OMX_Broadcom.h>




class OMX_Maps
{
public:
    static OMX_Maps& getInstance()
    {
        static OMX_Maps    instance;
        return instance;
    }
    
    map<OMX_DYNAMICRANGEEXPANSIONMODETYPE, int> dreTypes;
    
    vector<string> imageFilterNames;
    map<string, OMX_IMAGEFILTERTYPE> imageFilters;
    map<OMX_IMAGEFILTERTYPE, string> imageFilterTypes;
    
    vector<string>& getImageFilterNames()
    {
        return imageFilterNames;
    }
    string getImageFilter(OMX_IMAGEFILTERTYPE type)
    {
        return imageFilterTypes[type];
    }
    
    OMX_IMAGEFILTERTYPE getImageFilter(string name)
    {
        return imageFilters[name];
    }
    map<string, OMX_IMAGEFILTERTYPE>& getImageFilters()
    {
        return imageFilters;
    }
    
    vector<string> whiteBalanceNames;
    map<string, OMX_WHITEBALCONTROLTYPE> whiteBalance;
    map<OMX_WHITEBALCONTROLTYPE, string> whiteBalanceTypes;
    vector<string>& getWhiteBalanceNames()
    {
        return whiteBalanceNames;
    }
    
    string getWhiteBalance(OMX_WHITEBALCONTROLTYPE type)
    {
        return whiteBalanceTypes[type];
    }
    
    OMX_WHITEBALCONTROLTYPE getWhiteBalance(string name)
    {
        return whiteBalance[name];
    }
    
    vector<string> focusNames;
    map<string, OMX_IMAGE_FOCUSCONTROLTYPE> focusControls;
    map<OMX_IMAGE_FOCUSCONTROLTYPE, string> focusControlTypes;
    
    vector<string>& getFocusNames()
    {
        return focusNames;
    }
    
    string getFocus(OMX_IMAGE_FOCUSCONTROLTYPE type)
    {
        return focusControlTypes[type];
    }
    
    OMX_IMAGE_FOCUSCONTROLTYPE getFocus(string name)
    {
        return focusControls[name];
    }
    
    
    
    
    vector<string> meteringNames;
    map<string, OMX_METERINGTYPE> metering;
    map<OMX_METERINGTYPE, string> meteringTypes;
    vector<string>& getMeteringNames()
    {
        return meteringNames;
    }
    string getMetering(OMX_METERINGTYPE type)
    {
        return meteringTypes[type];
    }
    
    OMX_METERINGTYPE getMetering(string name)
    {
        return metering[name];
    }
    
    
    
    vector<string> exposurePresetNames;
    map<string, OMX_EXPOSURECONTROLTYPE> exposurePresets;
    map<OMX_EXPOSURECONTROLTYPE, string> exposurePresetTypes;
    map<string, OMX_EXPOSURECONTROLTYPE>& getExposurePresets()
    {
        return exposurePresets;
    }
    
    vector<string>& getExposurePresetNames()
    {
        return exposurePresetNames;
    }
    string getExposurePreset(OMX_EXPOSURECONTROLTYPE type)
    {
        return exposurePresetTypes[type];
    }
    
    OMX_EXPOSURECONTROLTYPE getExposurePreset(string name)
    {
        return exposurePresets[name];
    }
    
    vector<string> mirrorNames;
    map<string, OMX_MIRRORTYPE> mirrors;
    map<OMX_MIRRORTYPE, string> mirrorTypes;
    
    string getMirror(OMX_MIRRORTYPE type)
    {
        return mirrorTypes[type];
    }
    
    OMX_MIRRORTYPE getMirror(string name)
    {
        return mirrors[name];
    }
    
    
    vector<string> imageCodingNames;
    map<string, OMX_IMAGE_CODINGTYPE> imageCoding;
    map<OMX_IMAGE_CODINGTYPE, string> imageCodingTypes;
    vector<string>& getImageCodingNames()
    {
        return imageCodingNames;
    }
    
    string getImageCoding(OMX_IMAGE_CODINGTYPE type)
    {
        return imageCodingTypes[type];
    }
    
    OMX_IMAGE_CODINGTYPE getImageCoding(string name)
    {
        return imageCoding[name];
    }
    
    vector<string> videoCodingNames;
    map<string, OMX_VIDEO_CODINGTYPE> videoCoding;
    map<OMX_VIDEO_CODINGTYPE, string> videoCodingTypes;
    vector<string>& getVideoCodingNames()
    {
        return videoCodingNames;
    }
    
    string getVideoCoding(OMX_VIDEO_CODINGTYPE type)
    {
        return videoCodingTypes[type];
    }
    
    OMX_VIDEO_CODINGTYPE getVideoCoding(string name)
    {
        return videoCoding[name];
    }
    
    vector<string> colorFormatNames;
    map<string, OMX_COLOR_FORMATTYPE> colorFormats;
    map<OMX_COLOR_FORMATTYPE, string> colorFormatTypes;
    vector<string>& getColorFormatNames()
    {
        return colorFormatNames;
    }
    string getColorFormat(OMX_COLOR_FORMATTYPE type)
    {
        return colorFormatTypes[type];
    }
    
    OMX_COLOR_FORMATTYPE getColorFormat(string name)
    {
        return colorFormats[name];
    }
    
    
    vector<string> workingColorFormatNames;
    map<string, OMX_COLOR_FORMATTYPE> workingColorFormats;
    map<OMX_COLOR_FORMATTYPE, string> workingColorFormatTypes;
    vector<string>& getWorkingColorFormatNames()
    {
        return workingColorFormatNames;
    }
    string getWorkingColorFormat(OMX_COLOR_FORMATTYPE type)
    {
        return workingColorFormatTypes[type];
    }
    
    OMX_COLOR_FORMATTYPE getWorkingColorFormat(string name)
    {
        return workingColorFormats[name];
    }
    
    
    vector<string> algorithmNames;
    map<string, OMX_CAMERADISABLEALGORITHMTYPE> algorithms;
    map<OMX_CAMERADISABLEALGORITHMTYPE, string> algorithmTypes;
    vector<string>& getAlgorithmNames()
    {
        return algorithmNames;
    }
    
    
    vector<string> eventNames;
    map<string, OMX_EVENTTYPE> events;
    map<OMX_EVENTTYPE, string> eventTypes;
    vector<string>& getEventNames()
    {
        return eventNames;
    }
    
    string getEvent(OMX_EVENTTYPE type)
    {
        return eventTypes[type];
    }
    
    OMX_EVENTTYPE getEvent(string name)
    {
        return events[name];
    }
    
    
    vector<string> omxErrorNames;
    map<string, OMX_ERRORTYPE> omxErrors;
    map<OMX_ERRORTYPE, string> omxErrorTypes;
    vector<string>& getOMXErrorNames()
    {
        return omxErrorNames;
    }
    
    string getOMXError(OMX_ERRORTYPE type)
    {
        return omxErrorTypes[type];
    }
    
    OMX_ERRORTYPE getOMXError(string name)
    {
        return omxErrors[name];
    }
    
    vector<string> commandNames;
    map<string, OMX_COMMANDTYPE> commands;
    map<OMX_COMMANDTYPE, string> commandTypes;
    
    vector<string> omxStateNames;
    map<string, OMX_STATETYPE> omxStates;
    map<OMX_STATETYPE, string> omxStateTypes;
    
    map<EGLint, string> eglErrors;
    
private:	
    OMX_Maps()
    {
        
        commands["OMX_CommandVendorStartUnused"]= OMX_CommandVendorStartUnused;
        commands["OMX_CommandMax"]= OMX_CommandMax;
        commands["OMX_CommandStateSet"]= OMX_CommandStateSet;
        commands["OMX_CommandFlush"]= OMX_CommandFlush;
        commands["OMX_CommandPortDisable"]= OMX_CommandPortDisable;
        commands["OMX_CommandPortEnable"]= OMX_CommandPortEnable;
        commands["OMX_CommandMarkBuffer"]= OMX_CommandMarkBuffer;
        commands["OMX_CommandKhronosExtensions"]= OMX_CommandKhronosExtensions;
        
        collectNames<OMX_COMMANDTYPE>(commands, commandNames, commandTypes);
        
        focusControls["On"] = OMX_IMAGE_FocusControlOn;
        focusControls["Off"] = OMX_IMAGE_FocusControlOff;
        focusControls["Auto"] = OMX_IMAGE_FocusControlAuto;
        focusControls["AutoLock"] = OMX_IMAGE_FocusControlAutoLock;
        focusControls["Hyperfocal"] = OMX_IMAGE_FocusControlHyperfocal;
        focusControls["AutoMacro"] = OMX_IMAGE_FocusControlAutoMacro;
        focusControls["AutoInfinity"] = OMX_IMAGE_FocusControlAutoInfinity;
        focusControls["AutoLockMacro"] = OMX_IMAGE_FocusControlAutoLockMacro;
        focusControls["AutoLockInfinity"] = OMX_IMAGE_FocusControlAutoLockInfinity;
        focusControls["NearFixed"] = OMX_IMAGE_FocusControlNearFixed;
        focusControls["AutoNear"] = OMX_IMAGE_FocusControlAutoNear;
        focusControls["AutoLockNear"] = OMX_IMAGE_FocusControlAutoLockNear;
        focusControls["InfinityFixed"] = OMX_IMAGE_FocusControlInfinityFixed;
        focusControls["MacroFixed"] = OMX_IMAGE_FocusControlMacroFixed;
        focusControls["AutoFast"] = OMX_IMAGE_FocusControlAutoFast;
        focusControls["AutoMacroFast"] = OMX_IMAGE_FocusControlAutoMacroFast;
        focusControls["AutoNearFast"] = OMX_IMAGE_FocusControlAutoNearFast;
        focusControls["AutoInfinityFast"] = OMX_IMAGE_FocusControlAutoInfinityFast;
        focusControls["CurrentFixed"] = OMX_IMAGE_FocusControlCurrentFixed;
        
        collectNames<OMX_IMAGE_FOCUSCONTROLTYPE>(focusControls, focusNames, focusControlTypes);
        
        dreTypes[OMX_DynRangeExpOff] = 0;
        dreTypes[OMX_DynRangeExpLow] = 1;
        dreTypes[OMX_DynRangeExpMedium] = 2;
        dreTypes[OMX_DynRangeExpHigh] = 3;
        
        
        
        mirrors["None"] = OMX_MirrorNone;
        mirrors["Vertical"] = OMX_MirrorVertical;
        mirrors["Horizontal"] = OMX_MirrorHorizontal;
        mirrors["Both"] = OMX_MirrorBoth;
        collectNames<OMX_MIRRORTYPE>(mirrors, mirrorNames, mirrorTypes);
        
        
        imageFilters["None"] = OMX_ImageFilterNone;
        imageFilters["Noise"] = OMX_ImageFilterNoise;
        imageFilters["Emboss"] = OMX_ImageFilterEmboss;
        imageFilters["Negative"] = OMX_ImageFilterNegative;
        imageFilters["Sketch"] = OMX_ImageFilterSketch;
        imageFilters["OilPaint"] = OMX_ImageFilterOilPaint;
        imageFilters["Hatch"] = OMX_ImageFilterHatch;
        imageFilters["Gpen"] = OMX_ImageFilterGpen;
        imageFilters["Antialias"] = OMX_ImageFilterAntialias;
        imageFilters["DeRing"] = OMX_ImageFilterDeRing;
        imageFilters["Solarize"] = OMX_ImageFilterSolarize;
        imageFilters["Watercolor"] = OMX_ImageFilterWatercolor;
        imageFilters["Pastel"] = OMX_ImageFilterPastel;
        imageFilters["Sharpen"] = OMX_ImageFilterSharpen;
        imageFilters["Film"] = OMX_ImageFilterFilm;
        imageFilters["Blur"] = OMX_ImageFilterBlur;
        imageFilters["Saturation"] = OMX_ImageFilterSaturation;
        imageFilters["DeInterlaceLineDouble"] = OMX_ImageFilterDeInterlaceLineDouble;
        imageFilters["DeInterlaceAdvanced"] = OMX_ImageFilterDeInterlaceAdvanced;
        imageFilters["ColourSwap"] = OMX_ImageFilterColourSwap;
        imageFilters["WashedOut"] = OMX_ImageFilterWashedOut;
        imageFilters["ColourPoint"] = OMX_ImageFilterColourPoint;
        imageFilters["Posterise"] = OMX_ImageFilterPosterise;
        imageFilters["ColourBalance"] = OMX_ImageFilterColourBalance;
        imageFilters["Cartoon"] = OMX_ImageFilterCartoon;
        
        collectNames<OMX_IMAGEFILTERTYPE>(imageFilters, imageFilterNames, imageFilterTypes);
        
        
        
        
        whiteBalance["Off"] = OMX_WhiteBalControlOff;
        whiteBalance["Auto"] = OMX_WhiteBalControlAuto;
        whiteBalance["SunLight"] = OMX_WhiteBalControlSunLight;
        whiteBalance["Cloudy"] = OMX_WhiteBalControlCloudy;
        whiteBalance["Shade"] = OMX_WhiteBalControlShade;
        whiteBalance["Tungsten"] = OMX_WhiteBalControlTungsten;
        whiteBalance["Fluorescent"] = OMX_WhiteBalControlFluorescent;
        whiteBalance["Incandescent"] = OMX_WhiteBalControlIncandescent;
        whiteBalance["Flash"] = OMX_WhiteBalControlFlash;
        whiteBalance["Horizon"] = OMX_WhiteBalControlHorizon;
        
        collectNames<OMX_WHITEBALCONTROLTYPE>(whiteBalance, whiteBalanceNames, whiteBalanceTypes);
        
        metering["Average"] = OMX_MeteringModeAverage;
        metering["Spot"] = OMX_MeteringModeSpot;
        metering["Matrix"] = OMX_MeteringModeMatrix;
        metering["Backlit"] = OMX_MeteringModeBacklit;
        
        collectNames<OMX_METERINGTYPE>(metering, meteringNames, meteringTypes);
        
        
        exposurePresets["Off"] = OMX_ExposureControlOff;
        exposurePresets["Auto"] = OMX_ExposureControlAuto;
        exposurePresets["Night"] = OMX_ExposureControlNight;
        exposurePresets["BackLight"] = OMX_ExposureControlBackLight;
        exposurePresets["SpotLight"] = OMX_ExposureControlSpotLight;
        exposurePresets["Sports"] = OMX_ExposureControlSports;
        exposurePresets["Snow"] = OMX_ExposureControlSnow;
        exposurePresets["Beach"] = OMX_ExposureControlBeach;
        exposurePresets["LargeAperture"] = OMX_ExposureControlLargeAperture;
        exposurePresets["SmallAperture"] = OMX_ExposureControlSmallAperture;
        exposurePresets["VeryLong"] = OMX_ExposureControlVeryLong;
        exposurePresets["FixedFps"] = OMX_ExposureControlFixedFps;
        exposurePresets["NightWithPreview"] = OMX_ExposureControlNightWithPreview;
        exposurePresets["Antishake"] = OMX_ExposureControlAntishake;
        exposurePresets["Fireworks"] = OMX_ExposureControlFireworks;
        
        collectNames<OMX_EXPOSURECONTROLTYPE>(exposurePresets, exposurePresetNames, exposurePresetTypes);
        
        
        
        colorFormats["Unused"] = OMX_COLOR_FormatUnused;
        colorFormats["Monochrome"] = OMX_COLOR_FormatMonochrome;
        colorFormats["8bitRGB332"] = OMX_COLOR_Format8bitRGB332;
        colorFormats["12bitRGB444"] = OMX_COLOR_Format12bitRGB444;
        colorFormats["16bitARGB4444"] = OMX_COLOR_Format16bitARGB4444;
        colorFormats["16bitARGB1555"] = OMX_COLOR_Format16bitARGB1555;
        colorFormats["16bitRGB565"] = OMX_COLOR_Format16bitRGB565;
        colorFormats["16bitRGB565"] = OMX_COLOR_Format16bitBGR565;
        colorFormats["16bitRGB565"] = OMX_COLOR_Format18bitRGB666;
        colorFormats["18bitARGB1665"] = OMX_COLOR_Format18bitARGB1665;
        colorFormats["19bitARGB1666"] = OMX_COLOR_Format19bitARGB1666; 
        colorFormats["24bitRGB888"] = OMX_COLOR_Format24bitRGB888;
        colorFormats["24bitBGR888"] = OMX_COLOR_Format24bitBGR888;
        colorFormats["24bitARGB1887"] = OMX_COLOR_Format24bitARGB1887;
        colorFormats["25bitARGB1888"] = OMX_COLOR_Format25bitARGB1888;
        colorFormats["32bitBGRA8888"] = OMX_COLOR_Format32bitBGRA8888;
        colorFormats["32bitARGB8888"] = OMX_COLOR_Format32bitARGB8888;
        colorFormats["YUV411Planar"] = OMX_COLOR_FormatYUV411Planar;
        colorFormats["YUV411PackedPlanar"] = OMX_COLOR_FormatYUV411PackedPlanar;
        colorFormats["YUV420Planar"] = OMX_COLOR_FormatYUV420Planar;
        colorFormats["YUV420PackedPlanar"] = OMX_COLOR_FormatYUV420PackedPlanar;
        colorFormats["YUV420SemiPlanar"] = OMX_COLOR_FormatYUV420SemiPlanar;
        colorFormats["YUV422Planar"] = OMX_COLOR_FormatYUV422Planar;
        colorFormats["YUV422PackedPlanar"] = OMX_COLOR_FormatYUV422PackedPlanar;
        colorFormats["YUV422SemiPlanar"] = OMX_COLOR_FormatYUV422SemiPlanar;
        colorFormats["YCbYCr"] = OMX_COLOR_FormatYCbYCr;
        colorFormats["YCrYCb"] = OMX_COLOR_FormatYCrYCb;
        colorFormats["CbYCrY"] = OMX_COLOR_FormatCbYCrY;
        colorFormats["CrYCbY"] = OMX_COLOR_FormatCrYCbY;
        colorFormats["YUV444Interleaved"] = OMX_COLOR_FormatYUV444Interleaved;
        colorFormats["RawBayer8bit"] = OMX_COLOR_FormatRawBayer8bit;
        colorFormats["RawBayer10bit"] = OMX_COLOR_FormatRawBayer10bit;
        colorFormats["RawBayer8bitcompressed"] = OMX_COLOR_FormatRawBayer8bitcompressed;
        colorFormats["L2"] = OMX_COLOR_FormatL2; 
        colorFormats["L4"] = OMX_COLOR_FormatL4; 
        colorFormats["L8"] = OMX_COLOR_FormatL8; 
        colorFormats["L16"] = OMX_COLOR_FormatL16; 
        colorFormats["L24"] = OMX_COLOR_FormatL24; 
        colorFormats["L32"] = OMX_COLOR_FormatL32;
        colorFormats["YUV420PackedSemiPlanar"] = OMX_COLOR_FormatYUV420PackedSemiPlanar;
        colorFormats["YUV422PackedSemiPlanar"] = OMX_COLOR_FormatYUV422PackedSemiPlanar;
        colorFormats["18BitBGR666"] = OMX_COLOR_Format18BitBGR666;
        colorFormats["24BitARGB6666"] = OMX_COLOR_Format24BitARGB6666;
        colorFormats["24BitABGR6666"] = OMX_COLOR_Format24BitABGR6666;
        colorFormats["32bitABGR8888"] = OMX_COLOR_Format32bitABGR8888;
        colorFormats["8bitPalette"] = OMX_COLOR_Format8bitPalette;
        colorFormats["YUVUV128"] = OMX_COLOR_FormatYUVUV128;
        colorFormats["RawBayer12bit"] = OMX_COLOR_FormatRawBayer12bit;
        colorFormats["BRCMEGL"] = OMX_COLOR_FormatBRCMEGL;
        colorFormats["BRCMOpaque"] = OMX_COLOR_FormatBRCMOpaque;
        colorFormats["YVU420PackedPlanar"] = OMX_COLOR_FormatYVU420PackedPlanar;
        colorFormats["YVU420PackedSemiPlanar"] = OMX_COLOR_FormatYVU420PackedSemiPlanar;
        
        
        collectNames<OMX_COLOR_FORMATTYPE>(colorFormats, colorFormatNames, colorFormatTypes);
        
        workingColorFormats["Unused"] = OMX_COLOR_FormatUnused;
        workingColorFormats["YUV420PackedPlanar"] = OMX_COLOR_FormatYUV420PackedPlanar;
        workingColorFormats["YUV420PackedSemiPlanar"] = OMX_COLOR_FormatYUV420PackedSemiPlanar;
        workingColorFormats["YUV422PackedPlanar"] = OMX_COLOR_FormatYUV422PackedPlanar;
        workingColorFormats["YVU420PackedPlanar"] = OMX_COLOR_FormatYVU420PackedPlanar;
        workingColorFormats["YVU420PackedSemiPlanar"] = OMX_COLOR_FormatYVU420PackedSemiPlanar;
        
        collectNames<OMX_COLOR_FORMATTYPE>(workingColorFormats, workingColorFormatNames, workingColorFormatTypes);
        
        
        videoCoding["Unused"] = OMX_VIDEO_CodingUnused;
        videoCoding["AutoDetect"] = OMX_VIDEO_CodingAutoDetect;
        videoCoding["MPEG2"] = OMX_VIDEO_CodingMPEG2;
        videoCoding["H263"] = OMX_VIDEO_CodingH263;
        videoCoding["MPEG4"] = OMX_VIDEO_CodingMPEG4;
        videoCoding["WMV"] = OMX_VIDEO_CodingWMV;
        videoCoding["RV"] = OMX_VIDEO_CodingRV;
        videoCoding["AVC"] = OMX_VIDEO_CodingAVC;
        videoCoding["MJPEG"] = OMX_VIDEO_CodingMJPEG;
        videoCoding["VP6"] = OMX_VIDEO_CodingVP6;
        videoCoding["VP7"] = OMX_VIDEO_CodingVP7;
        videoCoding["VP8"] = OMX_VIDEO_CodingVP8;
        videoCoding["YUV"] = OMX_VIDEO_CodingYUV;
        videoCoding["Sorenson"] = OMX_VIDEO_CodingSorenson;
        videoCoding["Theora"] = OMX_VIDEO_CodingTheora;
        videoCoding["MVC"] = OMX_VIDEO_CodingMVC; 	
        
        
        collectNames<OMX_VIDEO_CODINGTYPE>(videoCoding, videoCodingNames, videoCodingTypes);
        
        
        imageCoding["Unused"] = OMX_IMAGE_CodingUnused;
        imageCoding["AutoDetect"] = OMX_IMAGE_CodingAutoDetect;
        imageCoding["JPEG"] = OMX_IMAGE_CodingJPEG;
        imageCoding["JPEG2K"] = OMX_IMAGE_CodingJPEG2K;
        imageCoding["EXIF"] = OMX_IMAGE_CodingEXIF;
        imageCoding["TIFF"] = OMX_IMAGE_CodingTIFF;
        imageCoding["GIF"] = OMX_IMAGE_CodingGIF;
        imageCoding["PNG"] = OMX_IMAGE_CodingPNG;
        imageCoding["LZW"] = OMX_IMAGE_CodingLZW;
        imageCoding["BMP"] = OMX_IMAGE_CodingBMP; 
        imageCoding["TGA"] = OMX_IMAGE_CodingTGA;
        imageCoding["PPM"] = OMX_IMAGE_CodingPPM;
        
        collectNames<OMX_IMAGE_CODINGTYPE>(imageCoding, imageCodingNames, imageCodingTypes);
        
        algorithms["Facetracking"] = OMX_CameraDisableAlgorithmFacetracking;
        algorithms["RedEyeReduction"] = OMX_CameraDisableAlgorithmRedEyeReduction;
        algorithms["VideoStabilisation"] = OMX_CameraDisableAlgorithmVideoStabilisation;
        algorithms["WriteRaw"] = OMX_CameraDisableAlgorithmWriteRaw;
        algorithms["VideoDenoise"] = OMX_CameraDisableAlgorithmVideoDenoise;
        algorithms["StillsDenoise"] = OMX_CameraDisableAlgorithmStillsDenoise;
        algorithms["AntiShake"] = OMX_CameraDisableAlgorithmAntiShake;
        algorithms["ImageEffects"] = OMX_CameraDisableAlgorithmImageEffects;
        algorithms["DarkSubtract"] = OMX_CameraDisableAlgorithmDarkSubtract;
        algorithms["DynamicRangeExpansion"] = OMX_CameraDisableAlgorithmDynamicRangeExpansion;
        algorithms["FaceRecognition"] = OMX_CameraDisableAlgorithmFaceRecognition;
        algorithms["FaceBeautification"] = OMX_CameraDisableAlgorithmFaceBeautification;
        algorithms["SceneDetection"] = OMX_CameraDisableAlgorithmSceneDetection;
        algorithms["HighDynamicRange"] = OMX_CameraDisableAlgorithmHighDynamicRange;
        
        
        collectNames<OMX_CAMERADISABLEALGORITHMTYPE>(algorithms, algorithmNames, algorithmTypes);
        
        
        
        omxStateTypes[OMX_StateInvalid] = "OMX_StateInvalid";
        omxStateTypes[OMX_StateLoaded] = "OMX_StateLoaded";
        omxStateTypes[OMX_StateIdle] = "OMX_StateIdle";
        omxStateTypes[OMX_StateExecuting] = "OMX_StateExecuting";
        omxStateTypes[OMX_StatePause] = "OMX_StatePause";
        omxStateTypes[OMX_StateWaitForResources] = "OMX_StateWaitForResources";
        
        
        collectNames<OMX_STATETYPE>(omxStates, omxStateNames, omxStateTypes);
        
        omxErrorTypes[OMX_ErrorNone] =  "OMX_ErrorNone";
        omxErrorTypes[OMX_ErrorInsufficientResources] =  "OMX_ErrorInsufficientResources";
        omxErrorTypes[OMX_ErrorUndefined] =  "OMX_ErrorUndefined";
        omxErrorTypes[OMX_ErrorInvalidComponentName] =  "OMX_ErrorInvalidComponentName";
        omxErrorTypes[OMX_ErrorComponentNotFound] =  "OMX_ErrorComponentNotFound";
        omxErrorTypes[OMX_ErrorInvalidComponent] =  "OMX_ErrorInvalidComponent";
        omxErrorTypes[OMX_ErrorBadParameter] =  "OMX_ErrorBadParameter";
        omxErrorTypes[OMX_ErrorNotImplemented] =  "OMX_ErrorNotImplemented";
        omxErrorTypes[OMX_ErrorUnderflow] =  "OMX_ErrorUnderflow";
        omxErrorTypes[OMX_ErrorOverflow] =  "OMX_ErrorOverflow";
        omxErrorTypes[OMX_ErrorHardware] =  "OMX_ErrorHardware";
        omxErrorTypes[OMX_ErrorInvalidState] =  "OMX_ErrorInvalidState";
        omxErrorTypes[OMX_ErrorStreamCorrupt] =  "OMX_ErrorStreamCorrupt";
        omxErrorTypes[OMX_ErrorPortsNotCompatible] =  "OMX_ErrorPortsNotCompatible";
        omxErrorTypes[OMX_ErrorResourcesLost] =  "OMX_ErrorResourcesLost";
        omxErrorTypes[OMX_ErrorNoMore] =  "OMX_ErrorNoMore";
        omxErrorTypes[OMX_ErrorVersionMismatch] =  "OMX_ErrorVersionMismatch";
        omxErrorTypes[OMX_ErrorNotReady] =  "OMX_ErrorNotReady";
        omxErrorTypes[OMX_ErrorTimeout] =  "OMX_ErrorTimeout";
        omxErrorTypes[OMX_ErrorSameState] =  "OMX_ErrorSameState";
        omxErrorTypes[OMX_ErrorResourcesPreempted] =  "OMX_ErrorResourcesPreempted";
        omxErrorTypes[OMX_ErrorPortUnresponsiveDuringAllocation] =  "OMX_ErrorPortUnresponsiveDuringAllocation";
        omxErrorTypes[OMX_ErrorPortUnresponsiveDuringDeallocation] =  "OMX_ErrorPortUnresponsiveDuringDeallocation";
        omxErrorTypes[OMX_ErrorPortUnresponsiveDuringStop] =  "OMX_ErrorPortUnresponsiveDuringStop";
        omxErrorTypes[OMX_ErrorIncorrectStateTransition] =  "OMX_ErrorIncorrectStateTransition";
        omxErrorTypes[OMX_ErrorIncorrectStateOperation] =  "OMX_ErrorIncorrectStateOperation";
        omxErrorTypes[OMX_ErrorUnsupportedSetting] =  "OMX_ErrorUnsupportedSetting";
        omxErrorTypes[OMX_ErrorUnsupportedIndex] =  "OMX_ErrorUnsupportedIndex";
        omxErrorTypes[OMX_ErrorBadPortIndex] =  "OMX_ErrorBadPortIndex";
        omxErrorTypes[OMX_ErrorPortUnpopulated] =  "OMX_ErrorPortUnpopulated";
        omxErrorTypes[OMX_ErrorComponentSuspended] =  "OMX_ErrorComponentSuspended";
        omxErrorTypes[OMX_ErrorDynamicResourcesUnavailable] =  "OMX_ErrorDynamicResourcesUnavailable";
        omxErrorTypes[OMX_ErrorMbErrorsInFrame] =  "OMX_ErrorMbErrorsInFrame";
        omxErrorTypes[OMX_ErrorFormatNotDetected] =  "OMX_ErrorFormatNotDetected";
        omxErrorTypes[OMX_ErrorContentPipeOpenFailed] =  "OMX_ErrorContentPipeOpenFailed";
        omxErrorTypes[OMX_ErrorContentPipeCreationFailed] =  "OMX_ErrorContentPipeCreationFailed";
        omxErrorTypes[OMX_ErrorSeperateTablesUsed] =  "OMX_ErrorSeperateTablesUsed";
        omxErrorTypes[OMX_ErrorTunnelingUnsupported] =  "OMX_ErrorTunnelingUnsupported";
        omxErrorTypes[OMX_ErrorKhronosExtensions] =  "OMX_ErrorKhronosExtensions";
        omxErrorTypes[OMX_ErrorVendorStartUnused] =  "OMX_ErrorVendorStartUnused";
        omxErrorTypes[OMX_ErrorDiskFull] =  "OMX_ErrorDiskFull";
        omxErrorTypes[OMX_ErrorMaxFileSize] =  "OMX_ErrorMaxFileSize";
        omxErrorTypes[OMX_ErrorDrmUnauthorised] =  "OMX_ErrorDrmUnauthorised";
        omxErrorTypes[OMX_ErrorDrmExpired] =  "OMX_ErrorDrmExpired";
        omxErrorTypes[OMX_ErrorDrmGeneral] =  "OMX_ErrorDrmGeneral";
        
        
        collectNames<OMX_ERRORTYPE>(omxErrors, omxErrorNames, omxErrorTypes);
        
        eventTypes[OMX_EventError] = "OMX_EventError";
        eventTypes[OMX_EventCmdComplete] = "OMX_EventCmdComplete";
        eventTypes[OMX_EventMark] = "OMX_EventMark";
        eventTypes[OMX_EventPortSettingsChanged] = "OMX_EventPortSettingsChanged";
        eventTypes[OMX_EventBufferFlag] = "OMX_EventBufferFlag";
        eventTypes[OMX_EventResourcesAcquired] = "OMX_EventResourcesAcquired";
        eventTypes[OMX_EventComponentResumed] = "OMX_EventComponentResumed";
        eventTypes[OMX_EventDynamicResourcesAvailable] = "OMX_EventDynamicResourcesAvailable";
        eventTypes[OMX_EventKhronosExtensions] = "OMX_EventKhronosExtensions";
        eventTypes[OMX_EventVendorStartUnused] = "OMX_EventVendorStartUnused";
        eventTypes[OMX_EventParamOrConfigChanged] = "OMX_EventParamOrConfigChanged";
        
        collectNames<OMX_EVENTTYPE>(events, eventNames, eventTypes);
        
        eglErrors[EGL_SUCCESS]="EGL_SUCCESS";
        eglErrors[EGL_NOT_INITIALIZED]="EGL_NOT_INITIALIZED";
        eglErrors[EGL_BAD_ACCESS]="EGL_BAD_ACCESS";
        eglErrors[EGL_BAD_ALLOC]="EGL_BAD_ALLOC";
        eglErrors[EGL_BAD_ATTRIBUTE]="EGL_BAD_ATTRIBUTE";
        eglErrors[EGL_BAD_CONFIG]="EGL_BAD_CONFIG";
        eglErrors[EGL_BAD_CONTEXT]="EGL_BAD_CONTEXT";
        eglErrors[EGL_BAD_CURRENT_SURFACE]="EGL_BAD_CURRENT_SURFACE";
        eglErrors[EGL_BAD_DISPLAY]="EGL_BAD_DISPLAY";
        eglErrors[EGL_BAD_MATCH]="EGL_BAD_MATCH";
        eglErrors[EGL_BAD_NATIVE_PIXMAP]="EGL_BAD_NATIVE_PIXMAP";
        eglErrors[EGL_BAD_NATIVE_WINDOW]="EGL_BAD_NATIVE_WINDOW";
        eglErrors[EGL_BAD_PARAMETER]="EGL_BAD_PARAMETER";
        eglErrors[EGL_BAD_SURFACE]="EGL_BAD_SURFACE";
        eglErrors[EGL_CONTEXT_LOST]="EGL_CONTEXT_LOST";
        
        //~3MS
        
    }
    ~OMX_Maps(){};
    OMX_Maps(OMX_Maps const&);
    void operator=(OMX_Maps const&);
    
    
    template<typename OMXEnum>
    void collectNames(map<string, OMXEnum>& sourceMap, vector<string>& names, map<OMXEnum, string>& reverseMap)
    {
        typename map<string, OMXEnum>::iterator mapIterator = sourceMap.begin();
        while (mapIterator != sourceMap.end()) 
        {
            names.push_back((*mapIterator).first);
            reverseMap[(*mapIterator).second] = (*mapIterator).first;
            ++mapIterator;
        }
    }
    
    template<typename OMXEnum>
    void collectNames(map<string, OMXEnum>& sourceMap, vector<string>& names)
    {
        typename map<string, OMXEnum>::iterator mapIterator = sourceMap.begin();
        while (mapIterator != sourceMap.end()) 
        {
            names.push_back((*mapIterator).first);
            ++mapIterator;
        }
    }
    
    
    
};



static
string  GetOMXStateString(OMX_STATETYPE type)
{
    return OMX_Maps::getInstance().omxStateTypes[type];
};

static 
string PrintOMXState(OMX_HANDLETYPE handle)
{
    OMX_ERRORTYPE error = OMX_ErrorNone;
    OMX_STATETYPE currentState;
    error = OMX_GetState(handle, &currentState);
    return GetOMXStateString(currentState);
};

static
OMX_STATETYPE  GetOMXState(string name)
{
    return OMX_Maps::getInstance().omxStates[name];
};

static
string  GetWhiteBalanceString(OMX_WHITEBALCONTROLTYPE type)
{
    return OMX_Maps::getInstance().whiteBalanceTypes[type];
};


static
OMX_WHITEBALCONTROLTYPE  GetWhiteBalance(string name)
{
    return OMX_Maps::getInstance().whiteBalance[name];
};


static
string  GetImageFilterString(OMX_IMAGEFILTERTYPE type)
{
    return OMX_Maps::getInstance().imageFilterTypes[type];
};


static
OMX_IMAGEFILTERTYPE  GetImageFilter(string name)
{
    return OMX_Maps::getInstance().imageFilters[name];
};


static
string  GetFocusString(OMX_IMAGE_FOCUSCONTROLTYPE type)
{
    return OMX_Maps::getInstance().focusControlTypes[type];
};


static
OMX_IMAGE_FOCUSCONTROLTYPE  GetFocus(string name)
{
    return OMX_Maps::getInstance().focusControls[name];
};


static
string  GetMeteringString(OMX_METERINGTYPE type)
{
    return OMX_Maps::getInstance().meteringTypes[type];
};


static
OMX_METERINGTYPE  GetMetering(string name)
{
    return OMX_Maps::getInstance().metering[name];
};


static
string  GetExposurePresetString(OMX_EXPOSURECONTROLTYPE type)
{
    return OMX_Maps::getInstance().exposurePresetTypes[type];
};


static
OMX_EXPOSURECONTROLTYPE  GetExposurePreset(string name)
{
    return OMX_Maps::getInstance().exposurePresets[name];
};


static
string  GetMirrorString(OMX_MIRRORTYPE type)
{
    return OMX_Maps::getInstance().mirrorTypes[type];
};


static
OMX_MIRRORTYPE  GetMirror(string name)
{
    return OMX_Maps::getInstance().mirrors[name];
};


static
string  GetImageCodingString(OMX_IMAGE_CODINGTYPE type)
{
    return OMX_Maps::getInstance().imageCodingTypes[type];
};


static
OMX_IMAGE_CODINGTYPE  GetImageCoding(string name)
{
    return OMX_Maps::getInstance().imageCoding[name];
};


static
string  GetVideoCodingString(OMX_VIDEO_CODINGTYPE type)
{
    return OMX_Maps::getInstance().videoCodingTypes[type];
};


static
OMX_VIDEO_CODINGTYPE  GetVideoCoding(string name)
{
    return OMX_Maps::getInstance().videoCoding[name];
};


static
string  GetColorFormatString(OMX_COLOR_FORMATTYPE type)
{
    return OMX_Maps::getInstance().colorFormatTypes[type];
};


static
OMX_COLOR_FORMATTYPE  GetColorFormat(string name)
{
    return OMX_Maps::getInstance().colorFormats[name];
};


static
string  GetWorkingColorFormatString(OMX_COLOR_FORMATTYPE type)
{
    return OMX_Maps::getInstance().workingColorFormatTypes[type];
};


static
OMX_COLOR_FORMATTYPE  GetWorkingColorFormat(string name)
{
    return OMX_Maps::getInstance().workingColorFormats[name];
};


static
string  GetEventString(OMX_EVENTTYPE type)
{
    return OMX_Maps::getInstance().eventTypes[type];
};


static
OMX_EVENTTYPE  GetEvent(string name)
{
    return OMX_Maps::getInstance().events[name];
};


static
string  GetOMXErrorString(OMX_ERRORTYPE type)
{
    return OMX_Maps::getInstance().omxErrorTypes[type];
};


static
OMX_ERRORTYPE  GetOMXError(string name)
{
    return OMX_Maps::getInstance().omxErrors[name];
};


static
string  GetOMXCommandString(OMX_COMMANDTYPE type)
{
    return OMX_Maps::getInstance().commandTypes[type];
};


static
OMX_COMMANDTYPE  GetOMXCommand(string name)
{
    return OMX_Maps::getInstance().commands[name];
};

static
string GetPortDefinitionString(OMX_PARAM_PORTDEFINITIONTYPE def)
{
    
    stringstream info;
    info << endl;
    info << "nPortIndex: " << def.nPortIndex << endl;
    info << "eDir: " << (def.eDir == OMX_DirInput ? "in" : "out") << endl;
    info << "nBufferCountActual: " << def.nBufferCountActual << endl;
    info << "nBufferCountMin: " << def.nBufferCountMin << endl;
    info << "nBufferSize: " << (def.nBufferSize >> 16) << endl;
    info << "nBufferAlignment: " << def.nBufferAlignment << endl;
    info << "bEnabled: " << def.bEnabled << endl;
    info << "bPopulated: " << def.bPopulated << endl;
    info << "bBuffersContiguous: " << def.bBuffersContiguous << endl;
    info << "nFrameWidth: " << def.format.video.nFrameWidth << endl;
    info << "nFrameHeight: " << def.format.video.nFrameHeight << endl;
    info << "nStride: " << def.format.video.nStride << endl;
    info << "nSliceHeight: " << def.format.video.nSliceHeight << endl;
    info << "xFramerate: " << (def.format.video.xFramerate >> 16) << endl;
    info << "eColorFormat: " << GetColorFormatString(def.format.video.eColorFormat) << endl;
    return info.str();
}
static 
string GetBufferHeaderString(OMX_BUFFERHEADERTYPE* omxBuffer)
{
    stringstream info;
    info << endl;
    if (omxBuffer) 
    {
        
        info << "nSize: " << omxBuffer->nSize  << endl;
        
        info << "nAllocLen: " << omxBuffer->nAllocLen  << endl;
        info << "nAllocLen Q16: " << (omxBuffer->nAllocLen >> 16)  << endl;
        
        info << "nFilledLen: "      << omxBuffer->nFilledLen       << endl;
        info << "nFilledLen Q16: "      << (omxBuffer->nFilledLen >> 16)       << endl;
        
        info << "nOffset: "         << omxBuffer->nOffset          << endl;
        //info << "nTickCount: "         << omxBuffer->nTickCount          << endl;
        info << "nFlags: "         << omxBuffer->nFlags          << endl;
        info << "nInputPortIndex: " << omxBuffer->nInputPortIndex  << endl;
        info << "nOutputPortIndex: " << omxBuffer->nOutputPortIndex  << endl;
        
    }
    return info.str();
}

#define MEGABYTE_IN_BITS 8388608


#define OMX_INIT_STRUCTURE(a) \
memset(&(a), 0, sizeof(a)); \
(a).nSize = sizeof(a); \
(a).nVersion.s.nVersionMajor = OMX_VERSION_MAJOR; \
(a).nVersion.s.nVersionMinor = OMX_VERSION_MINOR; \
(a).nVersion.s.nRevision = OMX_VERSION_REVISION; \
(a).nVersion.s.nStep = OMX_VERSION_STEP



#define OMX_CAMERA (OMX_STRING)"OMX.broadcom.camera"
#define CAMERA_PREVIEW_PORT		70
#define CAMERA_OUTPUT_PORT		71
#define CAMERA_STILL_OUTPUT_PORT 72

#define OMX_IMAGE_ENCODER (OMX_STRING)"OMX.broadcom.image_encode"
#define IMAGE_ENCODER_INPUT_PORT 340
#define IMAGE_ENCODER_OUTPUT_PORT 341

#define OMX_VIDEO_ENCODER (OMX_STRING)"OMX.broadcom.video_encode"
#define VIDEO_ENCODE_INPUT_PORT 200
#define VIDEO_ENCODE_OUTPUT_PORT 201

#define OMX_VIDEO_DECODER (OMX_STRING)"OMX.broadcom.video_decode"
#define VIDEO_DECODE_INPUT_PORT 130
#define VIDEO_DECODE_OUTPUT_PORT 131


#define OMX_VIDEO_SPLITTER (OMX_STRING)"OMX.broadcom.video_splitter"
#define VIDEO_SPLITTER_INPUT_PORT 250

#define VIDEO_SPLITTER_OUTPUT_PORT1 251
#define VIDEO_SPLITTER_OUTPUT_PORT2 252
#define VIDEO_SPLITTER_OUTPUT_PORT3 253
#define VIDEO_SPLITTER_OUTPUT_PORT4 254

#define OMX_VIDEO_RENDER (OMX_STRING)"OMX.broadcom.video_render"
#define VIDEO_RENDER_INPUT_PORT	90

#define OMX_EGL_RENDER (OMX_STRING)"OMX.broadcom.egl_render"
#define EGL_RENDER_INPUT_PORT	220
#define EGL_RENDER_OUTPUT_PORT	221

#define OMX_NULL_SINK (OMX_STRING)"OMX.broadcom.null_sink"
#define NULL_SINK_INPUT_PORT 240

//really a guess - higher values didn't seem to make any difference

#define MAX_SHUTTER_SPEED_MICROSECONDS 51200 
#define __func__ __PRETTY_FUNCTION__
#define FORCEDINLINE __attribute__((always_inline))

//#define OMX_TRACE(error) ofLogVerbose(__func__) << __LINE__ << " " << omxErrorToString(error);

#define DVD_TIME_BASE 1000000
#define DVD_NOPTS_VALUE    (-1LL<<52) // should be possible to represent in both double and __int64

#define DVD_TIME_TO_SEC(x)  ((int)((double)(x) / DVD_TIME_BASE))
#define DVD_TIME_TO_MSEC(x) ((int)((double)(x) * 1000 / DVD_TIME_BASE))
#define DVD_SEC_TO_TIME(x)  ((double)(x) * DVD_TIME_BASE)
#define DVD_MSEC_TO_TIME(x) ((double)(x) * DVD_TIME_BASE / 1000)

#define DVD_PLAYSPEED_PAUSE       0       // frame stepping
#define DVD_PLAYSPEED_NORMAL      1000


extern inline  
string omxErrorToString(OMX_ERRORTYPE error)
{
    return OMX_Maps::getInstance().getOMXError(error);
}

#define ENABLE_OMX_TRACE

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

extern inline
void PrintPortDef(OMX_PARAM_PORTDEFINITIONTYPE portDefinition)
{
    stringstream info;
    
    //info << "cMIMEType: "		<< portDefinition.format.video.cMIMEType << endl;
    //info << "pNativeRender: "   << portDefinition.format.video.pNativeRender << endl;
    
    
    
    
    //info << "nVersion: "		<< portDefinition.nVersion << endl;
    info << "nPortIndex: "		<< portDefinition.nPortIndex << endl;
    info << "eDir: "            << portDefinition.eDir << endl;
    info << "nBufferCountActual: "  << portDefinition.nBufferCountActual << endl;
    info << "nBufferCountMin: "     << portDefinition.nBufferCountMin << endl;
    info << "nBufferSize: "         << portDefinition.nBufferSize << endl;
    info << "bEnabled: "            << portDefinition.bEnabled << endl;
    info << "bPopulated: "          << portDefinition.bPopulated << endl;
    
    
    info << "nFrameWidth: "		<< portDefinition.format.video.nFrameWidth << endl;
    info << "nFrameHeight: "	<< portDefinition.format.video.nFrameHeight << endl;
    info << "nStride: "			<< portDefinition.format.video.nStride << endl;
    info << "nSliceHeight: "    << portDefinition.format.video.nSliceHeight << endl;
    info << "nBitrate: "        << portDefinition.format.video.nBitrate << endl;
    info << "xFramerate: "		<< (portDefinition.format.video.xFramerate >> 16) << endl;
    info << "bFlagErrorConcealment: "   << portDefinition.format.video.bFlagErrorConcealment << endl;
    info << "eCompressionFormat: "      << GetVideoCodingString(portDefinition.format.video.eCompressionFormat) << endl;
    info << "eColorFormat: "            << GetColorFormatString(portDefinition.format.video.eColorFormat) << endl;
    info << "pNativeWindow: "           << portDefinition.format.video.pNativeWindow << endl;
    
    ofLogVerbose(__func__) << info.str();
    
    /*
     
     OMX_STRING cMIMEType;
     OMX_NATIVE_DEVICETYPE pNativeRender;
     OMX_U32 nFrameWidth;
     OMX_U32 nFrameHeight;
     OMX_S32 nStride;
     OMX_U32 nSliceHeight;
     OMX_U32 nBitrate;
     OMX_U32 xFramerate;
     OMX_BOOL bFlagErrorConcealment;
     OMX_VIDEO_CODINGTYPE eCompressionFormat;
     OMX_COLOR_FORMATTYPE eColorFormat;
     OMX_NATIVE_WINDOWTYPE pNativeWindow;
     */
}


extern inline  
string eglErrorToString(EGLint error)
{
    return OMX_Maps::getInstance().eglErrors[error];
}

extern inline  
void logEGLError(EGLint error, string comments="", string functionName="", int lineNumber=0)
{
    string commentLine = " ";
    if(!comments.empty())
    {
        commentLine = " " + comments + " ";
    }
    
    
    /*if(error != EGL_SUCCESS)
     {
     ofLogError(functionName) << lineNumber << commentLine << eglErrorToString(error);
     }else
     {
     ofLogVerbose(functionName) << lineNumber << commentLine << eglErrorToString(error);
     }*/
    ofLogVerbose(functionName) << lineNumber << commentLine << eglErrorToString(error);
    
}




#define OMX_TRACE_1_ARGS(error)                      logOMXError(error, "", __func__, __LINE__);
#define OMX_TRACE_2_ARGS(error, comments)            logOMXError(error, comments, __func__, __LINE__);
#define OMX_TRACE_3_ARGS(error, comments, whatever)  logOMXError(error, comments, __func__, __LINE__);

#define GET_OMX_TRACE_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4
#define OMX_TRACE_MACRO_CHOOSER(...) GET_OMX_TRACE_4TH_ARG(__VA_ARGS__, OMX_TRACE_3_ARGS, OMX_TRACE_2_ARGS, OMX_TRACE_1_ARGS, )

#if defined (ENABLE_OMX_TRACE)
//#warning enabling OMX_TRACE
#define OMX_TRACE(...) OMX_TRACE_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)
#else
#warning  disabling OMX_TRACE
#warning  disabling -Wunused-but-set-variable -Wunused-variable
#define OMX_TRACE(...)
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif


#define EGL_TRACE_1_ARGS(error)                      logEGLError(error, "", __func__, __LINE__);
#define EGL_TRACE_2_ARGS(error, comments)            logEGLError(error, comments, __func__, __LINE__);
#define EGL_TRACE_3_ARGS(error, comments, whatever)  logEGLError(error, comments, __func__, __LINE__);

#define GET_EGL_TRACE_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4
#define EGL_TRACE_MACRO_CHOOSER(...) GET_EGL_TRACE_4TH_ARG(__VA_ARGS__, EGL_TRACE_3_ARGS, EGL_TRACE_2_ARGS, EGL_TRACE_1_ARGS, )


#define ENABLE_EGL_TRACE

#if defined (ENABLE_EGL_TRACE)
//#warning enabling EGL_TRACE
#define EGL_TRACE(...) EGL_TRACE_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)
#else
#warning  disabling EGL_TRACE
#warning  disabling -Wunused-but-set-variable -Wunused-variable
#define EGL_TRACE(...)
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

extern inline 
const char* omxErrorToCString(OMX_ERRORTYPE error)
{
    return OMX_Maps::getInstance().getOMXError(error).c_str();
}

extern inline 
OMX_BOOL toOMXBool(bool boolean)
{
    if(boolean) { return OMX_TRUE; } else { return OMX_FALSE; }
}

extern inline  
bool fromOMXBool(OMX_BOOL omxBool)
{
    if(omxBool == OMX_TRUE) { return true; } else { return false; } 
}

extern inline 
int toQ16(float n) 
{
    return (int)(n* 65536); 
}

extern inline 
float fromQ16(float n) 
{ 
    return n*(1/65536.0); 
}

extern inline
OMX_ERRORTYPE DisableAllPortsForComponent(OMX_HANDLETYPE* handle, string componentName="")
{
    
    OMX_ERRORTYPE error = OMX_ErrorNone;
    
    
    OMX_INDEXTYPE indexTypes[] = 
    {
        OMX_IndexParamAudioInit,
        OMX_IndexParamImageInit,
        OMX_IndexParamVideoInit, 
        OMX_IndexParamOtherInit
    };
    
    OMX_PORT_PARAM_TYPE ports;
    OMX_INIT_STRUCTURE(ports);
    
    for(int i=0; i < 4; i++)
    {
        error = OMX_GetParameter(*handle, indexTypes[i], &ports);
        OMX_TRACE(error);
        if(error == OMX_ErrorNone) 
        {
            
            uint32_t j;
            for(j=0; j<ports.nPorts; j++)
            {
                
                
                OMX_PARAM_PORTDEFINITIONTYPE portFormat;
                OMX_INIT_STRUCTURE(portFormat);
                portFormat.nPortIndex = ports.nStartPortNumber+j;
                
                error = OMX_GetParameter(*handle, OMX_IndexParamPortDefinition, &portFormat);
                OMX_TRACE(error);
                if(error != OMX_ErrorNone)
                {
                    if(portFormat.bEnabled == OMX_FALSE)
                    {
                        continue;
                    }
                }
                
                error = OMX_SendCommand(*handle, OMX_CommandPortDisable, ports.nStartPortNumber+j, NULL);
                OMX_TRACE(error, componentName + " PORT # " + ofToString(ports.nStartPortNumber+j) + " DISABLED")
                
                
            }
            
        }
    }
    
    return error;
}
#endif
