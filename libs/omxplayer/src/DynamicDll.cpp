/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "DynamicDll.h"


DllDynamic::DllDynamic()
{
  m_dll=NULL;
  m_DelayUnload=true;
}

DllDynamic::DllDynamic(const CStdString& strDllName)
{
  m_strDllName=strDllName;
  m_dll=NULL;
  m_DelayUnload=true;
}

DllDynamic::~DllDynamic()
{
  Unload();
}

bool DllDynamic::Load()
{
  if (m_dll)
    return true;


  return true;
}

void DllDynamic::Unload()
{

  m_dll=NULL;
}

bool DllDynamic::CanLoad()
{
  return true;
}

bool DllDynamic::EnableDelayedUnload(bool bOnOff)
{
  if (m_dll)
    return false;

  m_DelayUnload=bOnOff;

  return true;
}

bool DllDynamic::SetFile(const CStdString& strDllName)
{
  if (m_dll)
    return false;

  m_strDllName=strDllName;
  return true;
}

