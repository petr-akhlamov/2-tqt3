#ifndef TQPLATFORMDEFS_H
#define TQPLATFORMDEFS_H

// Get TQt defines/settings

#include "ntqglobal.h"

// Set any POSIX/XOPEN defines at the top of this file to turn on specific APIs

#include <unistd.h>


// We are hot - unistd.h should have turned on the specific APIs we requested


#ifdef TQT_THREAD_SUPPORT
#include <pthread.h>
#endif

#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stat.h>
#include <socket.h>
#include <ioctl.h>
#include <wait.h>
#include <signal.h>
#define TQT_NO_LIBRARY_UNLOAD

// DNS header files are not fully covered by X/Open specifications.
// In particular nothing is said about res_* :/
// On BSDs header files <netinet/in.h> and <arpa/nameser.h> are not
// included by <resolv.h>. Note that <arpa/nameser.h> must be included
// before <resolv.h>.
#include <nameser.h>
#include <netinet/in.h>
#include <resolv.h>


#if !defined(TQT_NO_COMPAT)
#define QT_STATBUF		struct stat
#define QT_STATBUF4TSTAT	struct stat
#define QT_STAT			::stat
#define QT_FSTAT		::fstat
#define QT_STAT_REG		S_IFREG
#define QT_STAT_DIR		S_IFDIR
#define QT_STAT_MASK		S_IFMT
#define QT_STAT_LNK		S_IFLNK
#define QT_FILENO		fileno
#define QT_OPEN			::open
#define QT_CLOSE		::close
#define QT_LSEEK		::lseek
#define QT_READ			::read
#define QT_WRITE		::write
#define QT_ACCESS		::access
#define QT_GETCWD		::getcwd
#define QT_CHDIR		::chdir
#define QT_MKDIR		::mkdir
#define QT_RMDIR		::rmdir
#define QT_OPEN_RDONLY		O_RDONLY
#define QT_OPEN_WRONLY		O_WRONLY
#define QT_OPEN_RDWR		O_RDWR
#define QT_OPEN_CREAT		O_CREAT
#define QT_OPEN_TRUNC		O_TRUNC
#define QT_OPEN_APPEND		O_APPEND
#endif

#define QT_SIGNAL_RETTYPE	void
#define QT_SIGNAL_ARGS		int
#define QT_SIGNAL_IGNORE	(QT_SIGNAL_RETTYPE (*)(QT_SIGNAL_ARGS))1

#define QT_SOCKLEN_T		int

#define QT_SNPRINTF		::snprintf
#define QT_VSNPRINTF		::vsnprintf


#endif // TQPLATFORMDEFS_H
