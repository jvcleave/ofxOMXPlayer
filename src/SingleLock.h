// SingleLock.h: interface for the SingleLock class.
//
//////////////////////////////////////////////////////////////////////

/*
 * XBMC Media Center
 * Copyright (c) 2002 Frodo
 * Portions Copyright (c) by the authors of ffmpeg and xvid
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#pragma once

#include <pthread.h>

class CriticalSection
{
public:
    inline CriticalSection()
    {
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m_lock, &mta);
    }
    inline ~CriticalSection()
    {
        pthread_mutex_destroy(&m_lock);
    }
    inline void lock()
    {
        pthread_mutex_lock(&m_lock);
    }
    inline void unlock()
    {
        pthread_mutex_unlock(&m_lock);
    }
    
protected:
    pthread_mutex_t m_lock;
};


class SingleLock
{
public:
    inline SingleLock(CriticalSection& cs)
    {
        section = cs;
        section.lock();
    }
    inline ~SingleLock()
    {
        section.unlock();
    }
    
protected:
    CriticalSection section;
};


