#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdarg.h>
#include <assert.h>
#ifdef WIN32
#	include <Winsock2.h>
#	include <process.h>
#	include <time.h>
#	include <socket.h>
#	include <ws2tcpip.h>
#	define MSG_NOSIGNAL 0
typedef int socklen_t;
typedef int pid_t;
#	define PRId64 "I64d"
#	define PRIx64 "I64X"
	typedef unsigned int __uint32_t;
	typedef __int64 __int64_t;
	typedef unsigned __int64 __uint64_t;
#	define strcasecmp stricmp
#else
#	define LINUX
#	include <unistd.h>
#	include <fcntl.h>
#	include <pthread.h>
#	include <sys/socket.h>
#	include <arpa/inet.h>
#	include <sys/stat.h>
#	include <errno.h> 
#	include <sys/types.h> 
#	include <sys/stat.h>
#	include <semaphore.h>
#	include <unistd.h>
#	include <netdb.h>
#	include <sys/time.h> 
#	include <sys/times.h>
#	define __STDC_FORMAT_MACROS
#	include <inttypes.h>
#	include<ctype.h>
#	include <signal.h>
#	include <dirent.h>
#	include <netinet/in.h>
#	include <net/if.h>
# 	include <sys/ioctl.h>
#	define TCP_NODELAY         0x0001
typedef int SOCKET;
#	define INVALID_SOCKET  (SOCKET)(~0)
#	define SOCKET_ERROR            (-1)
#	define closesocket close
#	define override
#	define Sleep(t) usleep(1000*(t))
	typedef long SSIZE_T;
	typedef __int64_t __int64;
#	define nullptr NULL
#	define GetCurrentThreadId pthread_self
#	define _atoi64(val)     strtoll(val, NULL, 10)
#endif

typedef unsigned char byte;
typedef unsigned int uint;

#define Base64SizeOf(cls) ((sizeof(cls)+2)/3*4)

// 获取当前时间，精确到毫秒
inline __int64 gettimems()
{
#ifdef WIN32
	SYSTEMTIME systime;
	FILETIME ftime;
	GetLocalTime(&systime);
	SystemTimeToFileTime(&systime, &ftime);
	return (*(__int64*)&ftime)/10000;
#else
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (__int64)tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

// format example: %d年%d月%d日 %d:%d:%d
inline std::string timestr(time_t t, const char * formate)
{
	char temp[256];
	struct tm m = *localtime(&t);
	sprintf(temp, formate, 1900 + m.tm_year, 1 + m.tm_mon, m.tm_mday, m.tm_hour, m.tm_min, m.tm_sec);
	return temp;
}

inline std::string nowtimestr(const char * formate)
{
	time_t t = time(NULL);
	return timestr(t, formate);
}
inline std::string timestr(time_t t)
{
	char temp[256];
	struct tm m = *localtime(&t);
	sprintf(temp, "%d年%d月%d日 %d:%d:%d", 1900 + m.tm_year, 1 + m.tm_mon, m.tm_mday, m.tm_hour, m.tm_min, m.tm_sec);
	return temp;
}
inline std::string datestr(time_t t)
{
	char temp[256];
	struct tm m = *localtime(&t);
	sprintf(temp, "%d-%d-%d", 1900 + m.tm_year, 1 + m.tm_mon, m.tm_mday);
	return temp;
}

/******************************************************************************
 * CEvent事件，CLock锁，CLockGuard自动锁
 * 线程，继承CThread，如果重载Work，则该函数返回时线程结束，
 * 也可以重载DoWork，线程会自动循环调用该函数。
 * class CMyThread : public CThread
 * {
 * protected:
 *		void Work() 
 *		{
 *			while(Runing)
 *			{
 *				//.......
 *			}
 *		}
 * };
 *
 * CMyThread th;
 * th.Start();
 *
 ********************************************************************************/
class CThread
{
#ifdef WIN32
	HANDLE thread_handle;
	DWORD thread_id;
#else
	pthread_t thread_handle;
#endif
#ifdef WIN32
	static DWORD WINAPI __threadproc(void *ctx)
#else
	static void *  __threadproc(void *ctx)
#endif
	{
		CThread *th = static_cast<CThread*>(ctx);
		
		th->Work();
		return 0;
	}
public:
	CThread()
	{
		thread_handle = 0;
#ifdef WIN32
		thread_id = 0;
#endif
		Runing = false;
	}

	virtual~CThread()
	{
#ifdef WIN32
		CloseHandle(thread_handle);
#endif
	}
	// 启动线程
	bool Start(bool hightPriority=true)
	{
#ifdef WIN32
		if (!thread_handle || (WAIT_OBJECT_0 == WaitForSingleObject(thread_handle, 0)))
#else
		if (!thread_handle || ESRCH == pthread_kill(thread_handle, 0)) // 判断线程是否已经终止
#endif
		{
			Runing = false;
			thread_handle = 0;
		}
		if (!Runing)
		{
			Runing = true;
#ifdef WIN32
			if ((thread_handle = CreateThread(NULL, 0, __threadproc, this, 0, &thread_id)) != NULL)
			{
				if (hightPriority && !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST))
				{
                    //CLog::Info("无法设置线程优先级");
				}
				return true;
			}
			else
			{
				return false;
			}
#else
			if (hightPriority)
			{
				pthread_attr_t attr;
				pthread_attr_init(&attr);
				pthread_attr_setschedpolicy( &attr, SCHED_FIFO );
				struct sched_param param;
				param.sched_priority = 99;
				pthread_attr_setschedparam( &attr, &param );
				pthread_attr_setdetachstate(&attr,/*PTHREAD_CREATE_DETACHED*/PTHREAD_CREATE_JOINABLE);
				return 0==pthread_create(&thread_handle, &attr, __threadproc, this);
			}
			else
			{
				return 0==pthread_create(&thread_handle, nullptr, __threadproc, this);
			}
#endif
		}
		return true;
	}
	// 停止线程，该函数只设置停止标志位需要在Work重载中while(Runing)
	void Stop()
	{
		Runing = false;
	}
	// 等待线程结束
	void Join()
	{
		if (!thread_handle) return;
#ifdef WIN32
		if (thread_id == GetCurrentThreadId()) return;
		WaitForSingleObject(thread_handle, -1);
#else
		if (thread_handle == pthread_self()) return;
        //CLog::Info("Join thread %d", thread_handle);
		pthread_join(thread_handle,nullptr);
#endif
	}
	void StartInCurrent()
	{
		if (!Runing)
		{
#ifdef WIN32
			thread_id = GetCurrentThreadId();
#else
			thread_handle = pthread_self();
#endif
			Runing = true;
			Work();
		}
		Runing = false;
	}
protected:

	volatile bool Runing;
	// 线程主函数，请使用while(Runing)执行循环任务
	virtual void Work()
	{
        //CLog::Warn("默认线程处理函数");
		while(Runing)
		{
			DoWork();
		}
	}
	// 线程单词任务，Work的默认实现会自动循环调用该函数
	virtual void DoWork()
	{
        //CLog::Warn("空线程处理函数");
	}
};

template<typename T>
class CThreadT : public CThread
{
	typedef void(T::*WorkFun)();
	T *obj;
	WorkFun fun;
public:
	CThreadT(T *obj, WorkFun fun) : obj(obj), fun(fun) {}
protected:
	void Work() override
	{
		(obj->*fun)();
	}
};

template<typename T>
inline CThread * CreateThreadAndStart(T *obj, void(T::*WorkFun)(), bool hightPriority = true)
{
	CThreadT<T> *th = new CThreadT<T>(obj, WorkFun);
	if (th->Start(hightPriority)) return th;
	else
	{
		delete th;
		return NULL;
	}
}

class CThreadS : public CThread
{
	typedef void(*WorkFun)();
	WorkFun fun;
public:
	CThreadS(WorkFun fun) : fun(fun) {}
protected:
	void Work() override
	{
		(*fun)();
	}
};

inline CThread * CreateThreadAndStart(void(*WorkFun)(), bool hightPriority = true)
{
	CThreadS *th = new CThreadS(WorkFun);
	if (th->Start(hightPriority)) return th;
	else
	{
		delete th;
		return NULL;
	}
}

#ifdef WIN32
class CEvent
{
	HANDLE sem;
public:
	CEvent(bool eventset=true)
	{
		sem = CreateSemaphore(NULL, eventset ? 1 : 0, 1, NULL);
	}

	void Set()
	{
		ReleaseSemaphore(sem, 1, NULL);
	}

	bool Reset()
	{
		return WAIT_OBJECT_0 == WaitForSingleObject(sem, 0);
	}

	bool TryWait()
	{
		return WAIT_OBJECT_0 == WaitForSingleObject(sem, 0);
	}

	bool WaitOne(int dwMilliseconds = -1)
	{
		return WAIT_OBJECT_0 == WaitForSingleObject(sem, dwMilliseconds);
	}

	virtual ~CEvent()
	{
		CloseHandle(sem);
	}
};
class CLock
{
	HANDLE sem;
public:
	CLock()
	{
		sem = CreateSemaphore(NULL, 1, 1, NULL);
	}

	bool Lock()
	{
		return WAIT_OBJECT_0 == WaitForSingleObject(sem, INFINITE);
	}

	bool TryLock()
	{
		return WAIT_OBJECT_0 == WaitForSingleObject(sem, 0);
	}

	void UnLock()
	{
		ReleaseSemaphore(sem, 1, NULL);
	}

	virtual ~CLock()
	{
		CloseHandle(sem);
	}
};
#else
class CEvent
{
	sem_t sem;
public:
	CEvent(bool eventset=true)
	{
		sem_init(&sem,0,eventset ? 1 : 0);
	}

	void Set()
	{
		sem_post(&sem);
	}

	bool Reset()
	{
		return -1 != sem_trywait(&sem);
	}

	bool TryWait()
	{
		return -1 != sem_trywait(&sem);
	}

	bool WaitOne(int dwMilliseconds = -1)
	{
		if(dwMilliseconds==-1)
		{
			sem_wait(&sem);
			return true;
		}
		else if (dwMilliseconds == 0)
		{
			return -1 != sem_trywait(&sem);
		}
		struct timespec ts;
		if (clock_gettime(CLOCK_REALTIME, &ts) == -1) 
		{
        	perror("clock_gettime\n");
			return false;
		}
		ts.tv_sec += dwMilliseconds/1000;
		ts.tv_nsec += (dwMilliseconds%1000)*1000*1000;
		int i;
		i= sem_timedwait(&sem,&ts);
		return i!=-1;
	}

	virtual~CEvent()
	{
		sem_destroy(&sem);
	}
};
class CLock
{
	sem_t sem;
public:
	CLock()
	{
		sem_init(&sem,0,1);
	}

	bool Lock()
	{
		return -1 != sem_wait(&sem);
	}

	bool TryLock()
	{
		return -1 != sem_trywait(&sem);
	}

	void UnLock()
	{
		sem_post(&sem);
	}

	virtual~CLock()
	{
		sem_destroy(&sem);
	}
};
#endif

struct CLockGuard
{
	CLock &m;
	CLockGuard(CLock &m) : m(m) { m.Lock(); } 
	~CLockGuard() { m.UnLock(); }
};

