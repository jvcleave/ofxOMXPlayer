#include "OMXThread.h"

#ifdef CLASSNAME
	#undef CLASSNAME
#endif
#define CLASSNAME "OMXThread"

OMXThread::OMXThread()
{
	pthread_mutex_init(&m_lock, NULL);
	pthread_attr_setdetachstate(&m_tattr, PTHREAD_CREATE_JOINABLE);
	pthread_attr_init(&m_tattr);
	m_thread    = 0;
	m_bStop     = false;
	m_running   = false;
}

OMXThread::~OMXThread()
{
	pthread_mutex_destroy(&m_lock);
	pthread_attr_destroy(&m_tattr);
}

bool OMXThread::StopThread()
{
	if(!m_running)
	{
		ofLog(OF_LOG_ERROR, "%s::%s - No thread running ", CLASSNAME, __func__);
		return false;
	}

	m_bStop = true;
	pthread_join(m_thread, NULL);
	m_running = false;

	m_thread = 0;

	ofLog(OF_LOG_VERBOSE, "%s::%s - Thread stopped ", CLASSNAME, __func__);
	return true;
}

bool OMXThread::Create()
{
	if(m_running)
	{
		ofLog(OF_LOG_ERROR, "%s::%s - Thread already running ", CLASSNAME, __func__);
		return false;
	}

	m_bStop    = false;
	m_running = true;

	pthread_create(&m_thread, &m_tattr, &OMXThread::Run, this);

	ofLog(OF_LOG_VERBOSE, "%s::%s - Thread with id %d started ", CLASSNAME, __func__, (int)m_thread);
	return true;
}

bool OMXThread::Running()
{
	return m_running;
}

pthread_t OMXThread::ThreadHandle()
{
	return m_thread;
}

void *OMXThread::Run(void *arg)
{
	OMXThread *thread = static_cast<OMXThread *>(arg);
	thread->Process();

	ofLog(OF_LOG_VERBOSE, "%s::%s - Exited thread with  id %d ", CLASSNAME, __func__, (int)thread->ThreadHandle());
	pthread_exit(NULL);
}

void OMXThread::Lock()
{
	if(!m_running)
	{
		ofLog(OF_LOG_ERROR, "%s::%s - No thread running ", CLASSNAME, __func__);
		return;
	}

	pthread_mutex_lock(&m_lock);
}

void OMXThread::UnLock()
{
	if(!m_running)
	{
		ofLog(OF_LOG_ERROR, "%s::%s - No thread running ", CLASSNAME, __func__);
		return;
	}

	pthread_mutex_unlock(&m_lock);
}

