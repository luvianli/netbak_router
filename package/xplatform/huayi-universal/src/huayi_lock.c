/*
This file implement huayi file lock 
*/ 

#include <sys/file.h> /* for flock(2) */
#include <sys/stat.h> /* for S_* constants */
#include <errno.h> /* for errno prototype */
#include <unistd.h> /* for close(2) prototypes */

#include "include/huayi_lock.h"

#define ALOCKLOG(level, fmt, arg...) do { \
		}while (0)

int A_FLOCK_INIT(char* filepath) {
    int fd;

    ALOCKLOG(HUAYILOG_DEBUG, "A_FLOCK_INIT\n");
    fd = open(filepath, O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
    if (fd==-1) {
        ALOCKLOG(HUAYILOG_ERR,
                 "A_FLOCK_INIT open(\"%s\",O_RDWR,S_IRUSR|S_IWUSR): %s (%i)\n",
                filepath, strerror(errno), errno);
        return 0;
    }
    return fd;
}
int A_FLOCK_DESTROY(int fd) {
    ALOCKLOG(HUAYILOG_DEBUG, "A_FLOCK_DESTROY\n");
    if (close(fd)==-1) {
        ALOCKLOG(HUAYILOG_ERR, "A_FLOCK_DESTROY close(fd): %s (%i)\n",
                 strerror(errno), errno);
        return 0;
    }
    return 1;
}

int A_FLOCK_LOCK_R(int fd) {
    ALOCKLOG(HUAYILOG_DEBUG, "A_FLOCK_LOCK_R\n");
    if (flock(fd, LOCK_SH) == -1) {
        ALOCKLOG(HUAYILOG_ERR,
                 "A_FLOCK_LOCK_R flock(fd,LOCK_EX): %s (%i)\n",
                 strerror(errno), errno);
        return 0;
    }
    return 1;
}

int A_FLOCK_LOCK_RW(int fd) {
    ALOCKLOG(HUAYILOG_DEBUG, "A_FLOCK_LOCK_RW\n");
    if (flock(fd, LOCK_EX) == -1) {
        ALOCKLOG(HUAYILOG_ERR, "A_FLOCK_LOCK_RW flock(fd,LOCK_EX): %s (%i)\n",
                 strerror(errno), errno);
        return 0;
    }
    return 1;
}

int A_FLOCK_LOCK_R_NB(int fd) {
    ALOCKLOG(HUAYILOG_DEBUG, "A_FLOCK_LOCK_R_NB\n");
    if (flock(fd, LOCK_SH|LOCK_NB) == -1) {
        ALOCKLOG(HUAYILOG_ERR, "A_FLOCK_LOCK_R_NB flock(fd,LOCK_EX): %s (%i)\n",
                 strerror(errno), errno);
        return 0;
    }
    return 1;
}

int A_FLOCK_LOCK_RW_NB(int fd) {
    ALOCKLOG(HUAYILOG_DEBUG, "A_FLOCK_LOCK_RW_NB\n");
    if (flock(fd, LOCK_EX|LOCK_NB) == -1) {
        ALOCKLOG(HUAYILOG_ERR, "A_FLOCK_LOCK_RW_NB flock(fd,LOCK_EX): %s (%i)\n",
                 strerror(errno), errno);
        return 0;
    }
    return 1;
}

int A_FLOCK_UNLOCK(int fd) {
    ALOCKLOG(HUAYILOG_DEBUG, "A_FLOCK_UNLOCK\n");
    if (flock(fd,LOCK_UN)==-1) {
        ALOCKLOG(HUAYILOG_ERR, "A_FLOCK_UNLOCK flock(fd,LOCK_UN): %s (%i)\n",
                 strerror(errno), errno);
        return 0;
    }
    return 1;
}

/* huayi locks */
int A_LOCK_INIT(struct huayi_lock *lock, int type) {
    lock->fd = A_FLOCK_INIT(HUAYI_LOCK_TYPE_TOPATH(type));
    if (lock->fd == 0) {
        return 0;
    }
    return lock->fd;
}
int A_LOCK_DESTROY(struct huayi_lock *lock) {
    return A_FLOCK_DESTROY(lock->fd);
}
int A_LOCK_LOCK(struct huayi_lock *lock, int flag) {
    switch (flag) {
    case ALOCK_FLAG_R:
        return A_FLOCK_LOCK_R(lock->fd);
    case ALOCK_FLAG_RW:
        return A_FLOCK_LOCK_RW(lock->fd);
    case ALOCK_FLAG_R_NB:
        return A_FLOCK_LOCK_R_NB(lock->fd);
    case ALOCK_FLAG_RW_NB:
        return A_FLOCK_LOCK_RW_NB(lock->fd);
    default:
        ALOCKLOG(HUAYILOG_ERR, "A_LOCK_LOCK invalid flag(%i)\n", flag);
        return 0;
    }
    return 0;
}
int A_LOCK_UNLOCK(struct huayi_lock *lock) {
    return A_FLOCK_UNLOCK(lock->fd);
}


