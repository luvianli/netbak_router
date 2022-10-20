/**
 * @file huayi_lock.h
 *
 * Define API for using mutex/spin locks.
 * The API defined can be used in both User Space and Kernel Space.
 *
 * Guideline to Spin Locks Versus Mutex
 *
 * Requirement                  Recommended Lock
 * ---------------------------------------------------
 * Low overhead                 Spin lock is preferred
 * Short lock hold time         Spin lock is preferred
 * Long lock hold time          Mutex is preferred
 * Need to lock from interrupt  Spin lock is required
 * Need to sleep while locking  Mutex is required
 *
 */

/**
 * TODO: define rw_lock (read/write lock) api 
 */

#ifndef __HUAYI_LOCK_H__
#define __HUAYI_LOCK_H__

#if defined(__linux__) || defined(__linux)


#ifdef __KERNEL__
/**
 * Mutex (can sleep, can't be used in interrupt)
 */
#include <linux/mutex.h>
//Mutex data type
#define a_mutex_t	struct mutex
//Statically define and init a mutex
#define HUAYI_DEFINE_MUTEX(amutex)      DEFINE_MUTEX(amutex)
//dynamically initialize a mutex
#define HUAYI_MUTEX_INIT(pmutex)        mutex_init(pmutex)
//Locks the given mutex; sleeps if the lock is unavailable
#define HUAYI_MUTEX_LOCK(pmutex)        mutex_lock(pmutex)
//Unlocks the given mutex
#define HUAYI_MUTEX_UNLOCK(pmutex)      mutex_unlock(pmutex)
//Returns one if the lock is locked and zero otherwise
#define HUAYI_MUTEX_IS_LOCKED(pmutex)   mutex_is_locked(pmutex)
//Cleanup mutex
#define HUAYI_MUTEX_DESTROY(pmutex)     


/**
 * Spin locks (can be used in interrupt)
 */
#include <linux/spinlock.h>
//Spinlock type
#define a_spinlock_t	 spinlock_t
//Statically define and init a spin lock
#define HUAYI_DEFINE_SPINLOCK(_Lock)            DEFINE_SPINLOCK(_Lock)
//Dynamically define and init a spin lock
#define HUAYI_SPINLOCK_INIT(_pLock)             spin_lock_init(_pLock)
//Lock a spin lock
#define HUAYI_SPINLOCK_LOCK(_pLock)             spin_lock(_pLock)
#define HUAYI_SPINLOCK_LOCK_IRQSAVE(_pLock, _flags)     spin_lock_irqsave(_pLock, _flags)
//Unlock a spin lock 
#define HUAYI_SPINLOCK_UNLOCK(_pLock)           spin_unlock(_pLock)
#define HUAYI_SPINLOCK_UNLOCK_IRQRESTORE(_pLock, _flags)   spin_unlock_irqrestore(_pLock, _flags)
//Returns one if the lock is locked and zero otherwise
#define HUAYI_SPINLOCK_IS_LOCKED(_pLock)        spin_is_locked(_pLock)
//Cleanup spinlock
#define HUAYI_SPINLOCK_DESTROY(_pLock)          


extern rwlock_t app_config_file_lock;


#define app_config_read_lock() 		    read_lock_bh(&app_config_file_lock);
#define app_config_read_unlock() 		read_unlock_bh(&app_config_file_lock);
#define app_config_write_lock() 		write_lock_bh(&app_config_file_lock);
#define app_config_write_unlock()		write_unlock_bh(&app_config_file_lock);



#else //__KERNEL__

/**
 * User space Mutex/Spinlock
 */
#include <pthread.h>
//Mutex data type
#define a_mutex_t	pthread_mutex_t
//dynamically initialize a mutex
#define HUAYI_MUTEX_INIT(pmutex)        pthread_mutex_init(pmutex,NULL)
//Locks the given mutex; sleeps if the lock is unavailable
#define HUAYI_MUTEX_LOCK(pmutex)        pthread_mutex_lock(pmutex)
//Unlocks the given mutex
#define HUAYI_MUTEX_UNLOCK(pmutex)      pthread_mutex_unlock(pmutex)
//Returns one if the lock is locked and zero otherwise
//#define A_MUTEX_IS_LOCKED(pmutex)   mutex_is_locked(pmutex)
//Cleanup spinlock
#define HUAYI_MUTEX_DESTROY(pmutex)     pthread_mutex_destroy(pmutex)


/**
 * Spin locks 
 */
//Spinlock type
#define a_spinlock_t	pthread_spinlock_t
//Init a spin lock
#define HUAYI_SPINLOCK_INIT(_pLock)             pthread_spin_init(_pLock,0)
//Lock a spin lock
#define HUAYI_SPINLOCK_LOCK(_pLock, _flags)     pthread_spin_lock(_pLock)
//Unlock a spin lock 
#define HUAYI_SPINLOCK_UNLOCK(_pLock, _flags)   pthread_spin_unlock(_pLock)
//Returns one if the lock is locked and zero otherwise
//#define A_SPINLOCK_IS_LOCKED(_pLock)        spin_is_locked(_pLock)
//Cleanup spinlock
#define HUAYI_SPINLOCK_DESTROY(_pLock)          pthread_spin_destroy(_pLock)

/**
 * File Locks
 */
//Init and Destroy a file lock
int A_FLOCK_INIT(char* filepath);
int A_FLOCK_DESTROY(int fd);

//Lock and unlock a file lock
int A_FLOCK_LOCK_R(int fd);
int A_FLOCK_LOCK_RW(int fd);
int A_FLOCK_LOCK_R_NB(int fd);
int A_FLOCK_LOCK_RW_NB(int fd);
int A_FLOCK_UNLOCK(int fd);

/* huayi lock */
typedef enum {
    ALOCK_CONFIG = 0,   // configuration
} HUAYI_LOCK_TYPE;

#define HUAYI_LOCK_TYPE_TOPATH(t) \
  ((t == ALOCK_CONFIG) ? "/var/run/huayi-status" : "unknown")

typedef enum {
    ALOCK_FLAG_R = 0,
    ALOCK_FLAG_RW,
    ALOCK_FLAG_R_NB,
    ALOCK_FLAG_RW_NB,
} HUAYI_LOCK_FLAG;

typedef struct huayi_lock {
    int fd;   // fd handle for file lock
} HUAYI_LOCK;

int A_LOCK_INIT(struct huayi_lock *lock, int type);
int A_LOCK_DESTROY(struct huayi_lock *lock);
int A_LOCK_LOCK(struct huayi_lock *lock, int flag);
int A_LOCK_UNLOCK(struct huayi_lock *lock);

#endif //__KERNEL__

#else //(__linux__) || (__linux)
#error "TODO: porting for new OS!"
#endif //(__linux__) || (__linux)

#endif /* __HUAYI_LOCK_H__ */
