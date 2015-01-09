#ifndef _LINUX_FANOTIFY_H
#define _LINUX_FANOTIFY_H

/* the following events that user-space can register for */
#define FAN_ACCESS		0x00000001	/* File was accessed */
#define FAN_MODIFY		0x00000002	/* File was modified */
#define FAN_CLOSE_WRITE		0x00000008	/* Writtable file closed */
#define FAN_CLOSE_NOWRITE	0x00000010	/* Unwrittable file closed */
#define FAN_OPEN		0x00000020	/* File was opened */

#define FAN_Q_OVERFLOW		0x00004000	/* Event queued overflowed */

#define FAN_OPEN_PERM		0x00010000	/* File open in perm check */
#define FAN_ACCESS_PERM		0x00020000	/* File accessed in perm check */

#define FAN_ONDIR		0x40000000	/* event occurred against dir */

#define FAN_EVENT_ON_CHILD	0x08000000	/* interested in child events */

/* helper events */
#define FAN_CLOSE		(FAN_CLOSE_WRITE | FAN_CLOSE_NOWRITE) /* close */

/* flags used for fanotify_init() */
#define FAN_CLOEXEC		0x00000001
#define FAN_NONBLOCK		0x00000002

/* These are NOT bitwise flags.  Both bits are used togther.  */
#define FAN_CLASS_NOTIF		0x00000000
#define FAN_CLASS_CONTENT	0x00000004
#define FAN_CLASS_PRE_CONTENT	0x00000008
#define FAN_ALL_CLASS_BITS	(FAN_CLASS_NOTIF | FAN_CLASS_CONTENT | \
				 FAN_CLASS_PRE_CONTENT)

#define FAN_UNLIMITED_QUEUE	0x00000010
#define FAN_UNLIMITED_MARKS	0x00000020

#define FAN_ALL_INIT_FLAGS	(FAN_CLOEXEC | FAN_NONBLOCK | \
				 FAN_ALL_CLASS_BITS | FAN_UNLIMITED_QUEUE |\
				 FAN_UNLIMITED_MARKS)

/* flags used for fanotify_modify_mark() */
#define FAN_MARK_ADD		0x00000001
#define FAN_MARK_REMOVE		0x00000002
#define FAN_MARK_DONT_FOLLOW	0x00000004
#define FAN_MARK_ONLYDIR	0x00000008
#define FAN_MARK_MOUNT		0x00000010
#define FAN_MARK_IGNORED_MASK	0x00000020
#define FAN_MARK_IGNORED_SURV_MODIFY	0x00000040
#define FAN_MARK_FLUSH		0x00000080

/* No fd set in event */
#define FAN_NOFD	-1

#endif /* _LINUX_FANOTIFY_H */
