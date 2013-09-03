#pragma once

extern "C" {
#include <bcm_host.h>
}

#include "DynamicDll.h"


////////////////////////////////////////////////////////////////////////////////////////////

class DllBcmHostInterface
{
public:
  virtual ~DllBcmHostInterface() {}

  virtual void bcm_host_init() = 0;
  virtual void bcm_host_deinit() = 0;
  virtual int32_t graphics_get_display_size( const uint16_t display_number, uint32_t *width, uint32_t *height) = 0;
  virtual int vc_tv_hdmi_get_supported_modes_new(HDMI_RES_GROUP_T group, TV_SUPPORTED_MODE_NEW_T *supported_modes,
                                             uint32_t max_supported_modes, HDMI_RES_GROUP_T *preferred_group,
                                             uint32_t *preferred_mode) = 0;
  virtual int vc_tv_hdmi_power_on_explicit_new(HDMI_MODE_T mode, HDMI_RES_GROUP_T group, uint32_t code) = 0;
  virtual int vc_tv_hdmi_set_property(const HDMI_PROPERTY_PARAM_T *property) = 0;
  virtual int vc_tv_get_display_state(TV_DISPLAY_STATE_T *tvstate) = 0;
  virtual int vc_tv_show_info(uint32_t show) = 0;
  virtual int vc_gencmd(char *response, int maxlen, const char *string) = 0;
  virtual void vc_tv_register_callback(TVSERVICE_CALLBACK_T callback, void *callback_data) = 0;
  virtual void vc_tv_unregister_callback(TVSERVICE_CALLBACK_T callback) = 0;
  virtual void vc_cec_register_callback(CECSERVICE_CALLBACK_T callback, void *callback_data) = 0;
  //virtual void vc_cec_unregister_callback(CECSERVICE_CALLBACK_T callback) = 0;
  virtual DISPMANX_DISPLAY_HANDLE_T vc_dispmanx_display_open( uint32_t device ) = 0;
  virtual DISPMANX_UPDATE_HANDLE_T vc_dispmanx_update_start( int32_t priority ) = 0;
  virtual DISPMANX_ELEMENT_HANDLE_T vc_dispmanx_element_add ( DISPMANX_UPDATE_HANDLE_T update, DISPMANX_DISPLAY_HANDLE_T display,
                                                              int32_t layer, const VC_RECT_T *dest_rect, DISPMANX_RESOURCE_HANDLE_T src,
                                                              const VC_RECT_T *src_rect, DISPMANX_PROTECTION_T protection,
                                                              VC_DISPMANX_ALPHA_T *alpha,
                                                              DISPMANX_CLAMP_T *clamp, DISPMANX_TRANSFORM_T transform ) = 0;
  virtual int vc_dispmanx_update_submit_sync( DISPMANX_UPDATE_HANDLE_T update ) = 0;
  virtual int vc_dispmanx_element_remove( DISPMANX_UPDATE_HANDLE_T update, DISPMANX_ELEMENT_HANDLE_T element ) = 0;
  virtual int vc_dispmanx_display_close( DISPMANX_DISPLAY_HANDLE_T display ) = 0;
  virtual int vc_dispmanx_display_get_info( DISPMANX_DISPLAY_HANDLE_T display, DISPMANX_MODEINFO_T * pinfo ) = 0;
  virtual int vc_dispmanx_display_set_background( DISPMANX_UPDATE_HANDLE_T update, DISPMANX_DISPLAY_HANDLE_T display,
                                                  uint8_t red, uint8_t green, uint8_t blue ) = 0;
  virtual int vc_tv_hdmi_audio_supported(uint32_t audio_format, uint32_t num_channels,
                                                  EDID_AudioSampleRate fs, uint32_t bitrate) = 0;
};

class DllBcmHost : public DllDynamic, DllBcmHostInterface
{
public:
  virtual void bcm_host_init()
    { return ::bcm_host_init(); };
  virtual void bcm_host_deinit()
    { return ::bcm_host_deinit(); };
  virtual int32_t graphics_get_display_size( const uint16_t display_number, uint32_t *width, uint32_t *height)
    { return ::graphics_get_display_size(display_number, width, height); };
  virtual int vc_tv_hdmi_get_supported_modes_new(HDMI_RES_GROUP_T group, TV_SUPPORTED_MODE_NEW_T *supported_modes,
                                             uint32_t max_supported_modes, HDMI_RES_GROUP_T *preferred_group,
                                             uint32_t *preferred_mode)
    { return ::vc_tv_hdmi_get_supported_modes_new(group, supported_modes, max_supported_modes, preferred_group, preferred_mode); };
  virtual int vc_tv_hdmi_power_on_explicit_new(HDMI_MODE_T mode, HDMI_RES_GROUP_T group, uint32_t code)
    { return ::vc_tv_hdmi_power_on_explicit_new(mode, group, code); };
  virtual int vc_tv_hdmi_set_property(const HDMI_PROPERTY_PARAM_T *property)
    { return ::vc_tv_hdmi_set_property(property); }
  virtual int vc_tv_get_display_state(TV_DISPLAY_STATE_T *tvstate)
    { return ::vc_tv_get_display_state(tvstate); };
  virtual int vc_tv_show_info(uint32_t show)
    { return ::vc_tv_show_info(show); };
  virtual int vc_gencmd(char *response, int maxlen, const char *string)
    { return ::vc_gencmd(response, maxlen, string); };
  virtual void vc_tv_register_callback(TVSERVICE_CALLBACK_T callback, void *callback_data)
    { ::vc_tv_register_callback(callback, callback_data); };
  virtual void vc_tv_unregister_callback(TVSERVICE_CALLBACK_T callback)
    { ::vc_tv_unregister_callback(callback); };
  virtual void vc_cec_register_callback(CECSERVICE_CALLBACK_T callback, void *callback_data)
    { ::vc_cec_register_callback(callback, callback_data); };
  //virtual void vc_cec_unregister_callback(CECSERVICE_CALLBACK_T callback)
  //  { ::vc_cec_unregister_callback(callback); };
  virtual DISPMANX_DISPLAY_HANDLE_T vc_dispmanx_display_open( uint32_t device )
     { return ::vc_dispmanx_display_open(device); };
  virtual DISPMANX_UPDATE_HANDLE_T vc_dispmanx_update_start( int32_t priority )
    { return ::vc_dispmanx_update_start(priority); };
  virtual DISPMANX_ELEMENT_HANDLE_T vc_dispmanx_element_add ( DISPMANX_UPDATE_HANDLE_T update, DISPMANX_DISPLAY_HANDLE_T display,
                                                                              int32_t layer, const VC_RECT_T *dest_rect, DISPMANX_RESOURCE_HANDLE_T src,
                                                                              const VC_RECT_T *src_rect, DISPMANX_PROTECTION_T protection,
                                                                              VC_DISPMANX_ALPHA_T *alpha,
                                                                              DISPMANX_CLAMP_T *clamp, DISPMANX_TRANSFORM_T transform )
    { return ::vc_dispmanx_element_add(update, display, layer, dest_rect, src, src_rect, protection, alpha, clamp, transform); };
  virtual int vc_dispmanx_update_submit_sync( DISPMANX_UPDATE_HANDLE_T update )
    { return ::vc_dispmanx_update_submit_sync(update); };
  virtual int vc_dispmanx_element_remove( DISPMANX_UPDATE_HANDLE_T update, DISPMANX_ELEMENT_HANDLE_T element )
    { return ::vc_dispmanx_element_remove(update, element); };
  virtual int vc_dispmanx_display_close( DISPMANX_DISPLAY_HANDLE_T display )
    { return ::vc_dispmanx_display_close(display); };
  virtual int vc_dispmanx_display_get_info( DISPMANX_DISPLAY_HANDLE_T display, DISPMANX_MODEINFO_T *pinfo )
    { return ::vc_dispmanx_display_get_info(display, pinfo); };
  virtual int vc_dispmanx_display_set_background( DISPMANX_UPDATE_HANDLE_T update, DISPMANX_DISPLAY_HANDLE_T display,
                                                  uint8_t red, uint8_t green, uint8_t blue )
    { return ::vc_dispmanx_display_set_background(update, display, red, green, blue); };
  virtual int vc_tv_hdmi_audio_supported(uint32_t audio_format, uint32_t num_channels,
                                                  EDID_AudioSampleRate fs, uint32_t bitrate)
  { return ::vc_tv_hdmi_audio_supported(audio_format, num_channels, fs, bitrate); };
  virtual bool ResolveExports() 
    { return true; }
  virtual bool Load() 
  {
    printf("DllBcm: Using omx system library \n");
    return true;
  }
  virtual void Unload() {}
};
