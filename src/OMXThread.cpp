#include "OMXThread.h"


OMXThread::OMXThread()
{
	pthread_mutex_init(&m_lock, NULL);
	pthread_attr_setdetachstate(&m_tattr, PTHREAD_CREATE_JOINABLE);
	pthread_attr_init(&m_tattr);
	m_thread    = 0;
	doStop     = false;
	m_running   = false;
}

OMXThread::~OMXThread()
{
	pthread_mutex_destroy(&m_lock);
	pthread_attr_destroy(&m_tattr);
}

bool OMXThread::StopThread(std::string className)
{
	if(!m_running)
	{
		return false;
	}

	doStop = true;
	pthread_join(m_thread, NULL);
	m_running = false;

	m_thread = 0;

	return true;
}

bool OMXThread::Create()
{
	if(m_running)
	{
		return false;
	}

	doStop    = false;
	m_running = true;

	pthread_create(&m_thread, &m_tattr, &OMXThread::Run, this);

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
	thread->process();

	pthread_exit(NULL);
}

void OMXThread::lock()
{
	if(!m_running)
	{
		return;
	}

	pthread_mutex_lock(&m_lock);
}

void OMXThread::unlock()
{
	if(!m_running)
	{
		return;
	}

	pthread_mutex_unlock(&m_lock);
}

