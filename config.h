/* Edit the following #defines to correspond to your system */

/* MAILER is 'sendmail -t' since it supports the mailfile format we use */

/* Mike's email address */
#define MAINTAINER "oliphant@gtk.org"

#if defined(__linux__) || defined(__FreeBSD__) || defined(__osf__)  /* __osf__ ?? */

#define MAILER "/usr/sbin/sendmail -t"

#elif defined(__sparc__)

#define MAILER "/usr/lib/sendmail -t"

#endif


#if defined(__linux__)

/* Define if you have the <linux/cdrom.h> header file. (Linux) */
#define HAVE_LINUX_CDROM_H 1

/* Define if you have the <linux/ucdrom.h> header file. (Linux) */
#define HAVE_LINUX_UCDROM_H 1

/* Define if you have the <mntent.h> header file. (to check mounting). */
#define HAVE_MNTENT_H 1


#elif defined(__FreeBSD__)

/* Define if you have the <io/cam/cdrom.h> header file. (FreeBSD, OpenBSD ,
   Solaris*/
#define HAVE_SYS_CDIO_H 1


#elif defined(__osf__) /* ?? */

/* Define if you have the <sys/cdio.h> header file. (Digital Unix) */
#define HAVE_IO_CAM_CDROM_H 1

#endif
