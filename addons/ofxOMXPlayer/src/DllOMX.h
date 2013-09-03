#pragma once


#include "DynamicDll.h"


#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>
#include <IL/OMX_Index.h>
#include <IL/OMX_Image.h>
#include <IL/OMX_Video.h>
#include <IL/OMX_Broadcom.h>

////////////////////////////////////////////////////////////////////////////////////////////

class DllOMXInterface
{
public:
  virtual ~DllOMXInterface() {}

  virtual OMX_ERRORTYPE OMX_Init(void) = 0;
  virtual OMX_ERRORTYPE OMX_Deinit(void) = 0;
  virtual OMX_ERRORTYPE OMX_GetHandle(OMX_HANDLETYPE *pHandle, OMX_STRING cComponentName, OMX_PTR pAppData, OMX_CALLBACKTYPE *pCallBacks) = 0;
  virtual OMX_ERRORTYPE OMX_FreeHandle(OMX_HANDLETYPE hComponent) = 0;
  virtual OMX_ERRORTYPE OMX_GetComponentsOfRole(OMX_STRING role, OMX_U32 *pNumComps, OMX_U8 **compNames) = 0;
  virtual OMX_ERRORTYPE OMX_GetRolesOfComponent(OMX_STRING compName, OMX_U32 *pNumRoles, OMX_U8 **roles) = 0;
  virtual OMX_ERRORTYPE OMX_ComponentNameEnum(OMX_STRING cComponentName, OMX_U32 nNameLength, OMX_U32 nIndex) = 0;
  virtual OMX_ERRORTYPE OMX_SetupTunnel(OMX_HANDLETYPE hOutput, OMX_U32 nPortOutput, OMX_HANDLETYPE hInput, OMX_U32 nPortInput) = 0;

};

class DllOMX : public DllDynamic, DllOMXInterface
{
public:
  virtual OMX_ERRORTYPE OMX_Init(void) 
    { return ::OMX_Init(); };
  virtual OMX_ERRORTYPE OMX_Deinit(void) 
    { return ::OMX_Deinit(); };
  virtual OMX_ERRORTYPE OMX_GetHandle(OMX_HANDLETYPE *pHandle, OMX_STRING cComponentName, OMX_PTR pAppData, OMX_CALLBACKTYPE *pCallBacks)
    { return ::OMX_GetHandle(pHandle, cComponentName, pAppData, pCallBacks); };
  virtual OMX_ERRORTYPE OMX_FreeHandle(OMX_HANDLETYPE hComponent)
    { return ::OMX_FreeHandle(hComponent); };
  virtual OMX_ERRORTYPE OMX_GetComponentsOfRole(OMX_STRING role, OMX_U32 *pNumComps, OMX_U8 **compNames) 
    { return ::OMX_GetComponentsOfRole(role, pNumComps, compNames); };
  virtual OMX_ERRORTYPE OMX_GetRolesOfComponent(OMX_STRING compName, OMX_U32 *pNumRoles, OMX_U8 **roles)
    { return ::OMX_GetRolesOfComponent(compName, pNumRoles, roles); };
  virtual OMX_ERRORTYPE OMX_ComponentNameEnum(OMX_STRING cComponentName, OMX_U32 nNameLength, OMX_U32 nIndex)
    { return ::OMX_ComponentNameEnum(cComponentName, nNameLength, nIndex); };
  virtual OMX_ERRORTYPE OMX_SetupTunnel(OMX_HANDLETYPE hOutput, OMX_U32 nPortOutput, OMX_HANDLETYPE hInput, OMX_U32 nPortInput)
    { return ::OMX_SetupTunnel(hOutput, nPortOutput, hInput, nPortInput); };
  virtual bool ResolveExports() 
    { return true; }
  //virtual bool Load() 
//  {
//    printf("\nDllOMX: Using omx system library \n");
//    return true;
//  }
  virtual void Unload() {}
};


