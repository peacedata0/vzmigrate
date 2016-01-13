/*
 *
 * Copyright (c) 2001 by SWsoft
 * All rights reserved.
 *
 * Common programm undependent routines.
 *
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#include <sys/syslog.h>


#ifdef __cplusplus
extern "C" {
#endif

#define VZMPIPE_DIR	"/var/tmp/"
#define LOCK_FILE	".lck"
#define STREAM_FILE	".str"
#define SUFFIX_MIGRATED	".migrated"
#define NEWTEM_PACKAGES_DIR	"/templates"
#define NEWTEM_PACKAGES_FILE	NEWTEM_PACKAGES_DIR "/vzpackages"
// bug #66819
#define VE_CONF_DIR			"/etc/vz/conf/"
#define VZ_TMP_DIR 			"/vz/tmp/"
#define VE_OLD_CONF_DIR			"/etc/sysconfig/vz-scripts/"
#define PID_FILE			"vzmigrate_ssh_pid"

#define VEID0		((unsigned) 0)
#define SERVICEVEID	((unsigned) 1)

extern int debug_level;
extern int terminated;

void print_log(int level, const char* oformat, ...);
void vprint_log(int level, const char* oformat, va_list pvar);
void open_logger(const char * name);
#define logger(level, fmt, args...) do {	\
	if (debug_level >= (level))		\
		print_log(level, fmt, ##args);	\
} while (0)

#define log_sys(level, format, ...) print_log(level, "%s (%s:%d): " format " error : %m", \
		__func__, __FILE__, __LINE__, ##__VA_ARGS__)

typedef void (*printFunc) (int level, const char *);
extern printFunc print_func;

#define xfree(x) ({ free(x); x = NULL; })

#define isSetBit(m, b) ((m) & (b))
#define setBit(m, b) (m) |= (b)
#define unsetBit(m, b) (m) &= ~(b)

#define xdelete(obj) do { delete obj; obj = NULL; } while(0)

#define do_block(fd) set_block(fd, 1)
#define do_nonblock(fd) set_block(fd, 0)
int set_block(int fd, int state);

#define do_clo(fd) set_clo(fd, 1)
#define do_noclo(fd) set_clo(fd, 0)
int set_clo(int fd, int state);

extern int putErr(int rc, const char * fm, ...);
extern const char * getError();

#ifdef __cplusplus
}
#endif

#define MAX_ARGS		1024
#define MAX_VE_AVAILABLE	512
#if MAX_VE_AVAILABLE >= MAX_ARGS - 20
#error "Max available Containers"
#endif

#define MAX_CMD_SIZE		BUFSIZ
/* 0x7FFFFFFF == 2.147.483.647 */
#define ITOA_BUF_SIZE		11

// Errors
#define MIG_ERR_USAGE		-1
#define MIG_ERR_SYSTEM		-2
#define MIG_ERR_LOCK		-3
#define MIG_ERR_CANT_CONNECT	-4
#define MIG_ERR_IMPROPER	-5
#define MIG_ERR_SPECIALVE	MIG_ERR_IMPROPER
#define MIG_ERR_COPY		-6
#define MIG_ERR_STARTVE		-7
#define MIG_ERR_STOPSOURCE	-8
#define MIG_ERR_EXISTS		-9
#define MIG_ERR_NOEXIST		-10

#define MIG_ERR_PKG_DEPS	-11
#define MIG_ERR_IP_IN_USE	-12
#define MIG_ERR_ACTIONS		-13


#define MIG_ERR_CONN_BROKEN	-21
#define MIG_ERR_CONN_TIMEOUT	-22
/* connection to ssl server refused */
#define MIG_ERR_CONN_REFUSED	-50
#define MIG_ERR_MOUNTVE		MIG_ERR_STARTVE
#define MIG_ERR_TERM		-25
#define MIG_ERR_PROTOCOL	-26
#define MIG_ERR_NOTMPLDIR       -27

#define MIG_ERR_SUSPEND  -29

#define MIG_ERR_LICENSE		-30
#define MIG_ERR_DISKSPACE	-31
#define MIG_ERR_TECHNOLOGIES	-32
#define MIG_ERR_RATE		-33
#define MIG_ERR_EQUALS		-34
#define MIG_ERR_UUID_CONFLICT	-35
#define MIG_ERR_EXT_BINDMOUNT	-36
#define MIG_ERR_LAYOUT		-37
#define MIG_ERR_VEFORMAT	-38
#define MIG_ERR_BAD_VEFORMAT	-39
#define MIG_ERR_VZCACHE2	-40
#define MIG_ERR_SUSPENDED	-41
#define MIG_ERR_VZCTL		-43
#define MIG_ERR_NAME_CONFLICT	-45
#define MIG_ERR_NONSHAREDFS	-46
#define MIG_ERR_UNKNOWN_CMD	-47
#define MIG_ERR_ONLINE_ELDER	-48
#define MIG_ERR_CONN_TOOLONG	-49
#define MIG_ERR_VZSOCK		MIG_ERR_PROTOCOL

#define MIG_ERR_TASK_FAILED	-52
#define MIG_ERR_TASK_SIGNALED	-53
#define MIG_ERR_TASK_EXITED	-54
//#define MIG_ERR_ISCSI_STORAGE	-55 - not used

#define MIG_ERR_INSUFF_CPU_CAPS	-56
#define MIG_ERR_UNSUPP_CPU_CAPS	-57

#define MIG_ERR_PLOOP		-58
#define MIG_ERR_CONVERT_VZFS	-59
#define MIG_ERR_INCOMPAT_CPT_VER	-60
#define MIG_ERR_NO_IPV6_MODULE_CAPS	-61
#define MIG_ERR_NO_SLM_MODULE_CAPS	-62
#define MIG_ERR_NO_MNT_NAMESPACES_CAPS	-63
#define MIG_ERR_VZFS_ON_PSTORAGE	-64
#define MIG_ERR_PLOOP_FORMAT		-65
#define MIG_ERR_CONVERT_BINDMOUNTS	-66
#define MIG_ERR_CHECKSUM_MIGRATE        -67
#define MIG_ERR_MULTIPLOOP_IS_NOT_SUP	-68
#define MIG_ERR_DST_BUNDLE_EXIST	-69
#define MIG_ERR_EXTERNAL_DISKS_MIXED	-70
#define MIG_ERR_SECOND_LEVEL_BUG        -71
#define MIG_ERR_EXTERNAL_PROCESS        -72

// internal error codes
//#define MIG_ERR_SHARED_NOTFOUND	-101
#define MIG_ERR_DRYRUN		-104
#define MIG_ERR_INVALID_ARG	-105
#define MIG_ERR_TRANSMISSION_FAILED	-106
#define MIG_ERR_DEBUG_OUT	1

#define MIG_ERR_ALREDY_RUNNING	-201

// Errors message

#define VZM_MSG_UNKBIN	"Unknown binary \"%s\" run"

#define VZM_MSG_TRACK	"Can't do tracking, do slow migrate : %s"
#define VZM_MSG_TRACK_FILE	"Can't do stat on track file : %s"
#define VZM_MSG_INIT	"Can't init migrate : %s"
#define VZM_MSG_START	"Can't start migrate"
#define VZM_MSG_TERM	"Migration was terminated"

#define MIG_MSG_PROTOCOL	"migrate protocol error"
#define MIG_MSG_UNKNOWN_CMD	"unknown command: \"%s\", ignored"
#define MIG_MSG_NOTMPLDIR       "failed to access template directory"
#define MIG_MSG_NOSTATUS        "failed to get CT status"

#define MSG_CHECK_VE		", check CT#%d before start"
#define MSG_USE_FORCE		", use '-f' option"
#define MSG_FORCE_USED		"Force option is used"
#define MIG_MSG_GET_CPU_CAPS	"Can't get destination cpu capabilities, seems you need to check versions of packages on destination node"
#define MIG_MSG_CHECK_CAPS	"Can't check source cpu capabilities, seems you need to load CPT modules or check versions of packages on source node"
#define MIG_MSG_CHECK_CPT_VER	"Can't check CPT image version, seems you need to load CPT modules or check versions of packages on source node"
#define MIG_MSG_CHECK_KERNEL_MODULES	"Can't load kernel module %s on target node"
#define MIG_MSG_INSUFF_CPU	"Insufficient cpu capabilities on destination node"
#define MIG_MSG_NO_IPV6_MODULE_CAPS	"The IPv6 module is not loaded on the destination node"
#define MIG_MSG_NO_SLM_MODULE_CAPS	"The SLM module is not loaded on the destination node"
#define MIG_MSG_NO_MNT_NAMESPACES_CAPS	"Mounting namespaces in the container is not allowed by the destination node kernel"
#define MIG_MSG_VECAPS_FORCE	"You can use the '-f' option to force migration,\n"\
"but in this case some applications may fail after migration."
#define MIG_MSG_INCOMPAT_CPT_VER	"CPT image cannot be restored"
#define MIG_MSG_UNSUPPORTED	"CT has unsupported features"
#define MIG_MSG_PAGEIN_EXEC	"Can't execute vzpageind"
#define MIG_MSG_PAGEIN_STATE	"vzpageind is not running"
#define MIG_MSG_PKTDEPS_COM	"Destination node has not got required packages [%s]"
#define MIG_MSG_CACHES_COM	"Destination node has not got required caches [%s]"
#define MIG_MSG_CACHES		MIG_MSG_CACHES_COM MSG_USE_FORCE " or run vzuncache on this CT"
#define MIG_MSG_CACHES_WARN	MIG_MSG_CACHES_COM MSG_CHECK_VE
#define MIG_MSG_IP_IN_USE_COM	"IP addresses [%s] already in use on destination node"
#define MIG_MSG_IP_IN_USE	MIG_MSG_IP_IN_USE_COM MSG_USE_FORCE
#define MIG_MSG_IP_IN_USE_WARN	MIG_MSG_IP_IN_USE_COM MSG_CHECK_VE
#define MIG_MSG_CANTCONVERT	"Can't convert CT config to new format on destination side" MSG_CHECK_VE

#define MIG_MSG_AREA_USED	"directory '%s' or it descendant already used by CT#%d"
#define MIG_MSG_SEND_PKT	"can't send command to destination side : %m"
#define MIG_MSG_CANT_CONN_SRC	"can't connect to source node (%s) : %s"
#define MIG_MSG_CANT_CONNECT	"can't connect to destination node"
#define MIG_MSG_NOT_INSTALL	"seems you need install migrate package on destination node"
#define MIG_MSG_LOCK		"can't lock CT#%d : %s"
#define MIG_MSG_TEMPL_LOCK	"can't lock template \"%s\""
#define MIG_MSG_NOEXIST		"CT#%d doesn't exist"
#define MIG_MSG_TEMPL_NOEXIST	"Template \"%s\" doesn't exist"
#define MIG_MSG_VEIMPROPER	"CT#%d is improper"
#define MIG_MSG_VEIMPROPER_CONF	"Improper CT/VZ config"
#define MIG_MSG_VELIST		"can't get list of containers"
#define MIG_MSG_TEMIMPROPER	"Template \"%s\" is broken"
#define MIG_MSG_EXISTS		"CT#%d already exists"
#define MIG_MSG_TEMPL_EXISTS	"Later or the same template version \"%s/%s\" already exists on destination"
//TODO: friendly text
#define MIG_MSG_EZTEMPL_EXISTS	"EZ template \"%s\" already exists on destination"
#define MIG_MSG_OS_EZTEMPL_NOT_EXISTS	"EZ ostemplate for \"%s\" does not exists on destination"
#define MIG_MSG_AREA_EXISTS	"destination '%s' already exists"
#define MIG_MSG_CMP_LOCATIONS	"compare old and new CT locations failed : %m"
#define MIG_MSG_SYSTEM		"system error : %m"
#define MIG_MSG_EXEC_TRACK	"can not run tracking"
#define MIG_MSG_TRACK_FAILED	"tracking failed : exit code [%d]"
#define MIG_MSG_TRACK_TREE	"can not create track tree"
#define MIG_MSG_COPY		"copy (using : [%s]) failed : %s"
#define MIG_MSG_MOVE		"can not move '%s' -> '%s' : %m"
#define MIG_MSG_COPY_FILE	"can not copy '%s' -> '%s' : %s"
#define MIG_MSG_EXEC_BIN	"can not exec '%s' : %m"
#define MIG_MSG_STOP		"can not stop/umount CT#%d : %s"
#define MIG_MSG_START		"can not start/mount CT#%d : %s"
#define MIG_MSG_REALTIME	"Can not perform online migration with low delay"
#define MIG_MSG_ITER_MIG	"Iterative migration is not available"
#define MIG_MSG_LAZY		"Can not perform lazy online migration"
#define MIG_MSG_ONLINE_ELDER	"Can not perform online migration to/from elder version"
#define MIG_MSG_VM_PREP		"can not prepare vm for CT#%d : %s"
#define MIG_MSG_SUSPEND		"can not suspend CT#%d : %s"
#define MIG_MSG_LAZY_VM		"can not set lazy_vm flag for CT#%d : %s"
#define MIG_MSG_DUMP		"can not dump CT#%d : %s" MIG_MSG_TRYOFFLINE
#define MIG_MSG_PAGEOUT		"can not start pageout for CT#%d : %s"
#define MIG_MSG_UNDUMP		"can not undump CT#%d : %s" MIG_MSG_TRYOFFLINE
#define MIG_MSG_TRYOFFLINE	"\nThis CT can't be migrated online at the moment. Try offline migration."
#define MIG_MSG_ITER		"can not prepare destination CT#%d for iterative migration : %s"
#define MIG_MSG_ROLLBACK	"can not rollback to simple online migration on destination CT#%d : %s"
#define MIG_MSG_DST_RESUME	"can not resume destination CT#%d : %s"
#define MIG_MSG_RESUME		"can not resume CT#%d : %s"
#define MIG_MSG_SPECIAL		"CT#%d is special CT type '%s'"
#define MIG_MSG_DELETE		"can not delete '%s' : %s"
#define MIG_MSG_CREATE_DIR	"can not create dir '%s' : %m "
#define MIG_MSG_SET_MODE	"chmod '%s' : %m"
#define MIG_MSG_REPLY		"can not read reply from destination node"
#define MIG_MSG_KEEPER_RUN	"keeper CT#%d is not running"
#define MIG_MSG_OFFLINE_MAN	"can't turn off/on offline management for CT#%d : %s"
#define MIG_MSG_NEWTEM_REPAIR	"can't repair CT#%d packages : %s"
#define MIG_MSG_NEWTEM_CONFPATH	"can't get template config path : %d"

#define MIG_MSG_LICENSE		"License check failed: %s"
#define MIG_MSG_DISKSPACE	"Disk space check failed: %s"
#define MIG_MSG_TECHNOLOGIES	"The next techologies are not supported on "\
				"destination node:%s"
#define MIG_MSG_RATE		"failed check rate: %s"
#define MIG_MSG_EQUALS		"Source and destination CT equals"
#define MIG_MSG_PRIV_EQUALS	"Source and destination CTs have the same private area %s"
#define MIG_MSG_ROOT_EQUALS	"Root of source and destination CTs are equal"

#define MIG_MSG_LAYOUT		"CT layout (%d) is not supported on destination node"
#define MIG_MSG_VEFORMAT	"CT format (%s) is not supported on destination node"
#define MIG_MSG_BAD_VEFORMAT	"can't get source CT format"
#define MIG_MSG_VZCACHE2	"can't migrate vzcache2 area %s (old protocol)"
#define MIG_MSG_SUSPENDED	"can't migrate suspended CT (old protocol)"
#define MIG_MSG_NAME_CONFLICT	"name %s already used by CT %d"
#define MIG_MSG_PLOOP_FORMAT	"The following ploop format is not supported "\
				"on the destination node: %d"
#define MIG_MSG_CONVERT_BINDMOUNTS	"Migration CT with bindmounts does "\
					"not supported on destination node"
#define MIG_MSG_MULTIPLOOP_IS_NOT_SUP	"No secondary ploop support on destination"

#define MIG_MSG_CLONE_FORBIDDEN_FOR_SUSPENDED        "Clone is forbidden for suspended CT"
#define MIG_MSG_CHECKSUM_MIGRATE        "can't migrate on vzfs traker failure. Update vzmigrate package to 6.0.7 on destination"
#define MIG_MSG_SECOND_LEVEL_BUG	"Update vmigrate package to 6.0.8 on destination to "\
					"migrate online with vzfs to ploop conversion."

//TODO: friendly message
#define MIG_MSG_UUID_CONFLICT 	"UUID-collision, directory %s exists"
#define MIG_MSG_UNCOMPATIBILITY_SRC 	"Migration is not supported, please, upgrade source part to VZ >= 3.0"
#define MIG_MSG_UNCOMPATIBILITY_DST 	"Migration is not supported, please, upgrade destination part to VZ >= 4.0"
#define MIG_MSG_GETSIZE_DIR 	"can't get size for directory %s: %s"
#define MIG_MSG_EMPTY_OUTPUT 	"program return empty output"
#define MIG_MSG_EXT_BINDMOUNT 	"config ve contains ext bindmount: %s"

// restore state debug messages
#define MIG_MSG_RST	"cleaning : "
#define MIG_MSG_RST_RM_FILE	"cleaning : 'rm' file : %s"
#define MIG_MSG_RST_RMDIR	"cleaning : 'rmdir' dir : %s"
#define MIG_MSG_RST_RM_DIR	"cleaning : 'rm' dir : %s"
#define MIG_MSG_RST_RENAME	"cleaning : rename : %s -> %s"
#define MIG_MSG_RST_START	"cleaning : start CT#%d"
#define MIG_MSG_RST_RESUME	"cleaning : resume CT#%d"
#define MIG_MSG_RST_CHANNEL	"cleaning : close ssh channel"
#define MIG_MSG_RST_RESTORE	"cleaning : restore CT#%d"
#define MIG_MSG_RST_MOUNT	"cleaning : mount CT#%d"
#define MIG_MSG_RST_OFFLINE	"cleaning : restore offline management CT#%d"
#define MIG_MSG_STOP_TRACK	"cleaning : stop tracking CT#%d"

/* shared FS support */
#define MIG_MSG_THESAME_CLUSTER	"Source and target %s resides "\
	"on the same cluster %s"
#define MIG_MSG_THESAME_SHARED	"Source and target %s resides "\
	"on the same shared partition"

#define MSG_FUNC_COPY_DUMP		"Syncing dump file..."

#endif
