/*****************************************************************

  cd.c

  Based on code from libcdaudio 0.5.0 (Copyright (C)1998 Tony Arcieri)

  All changes copyright (c) 1998 by Mike Oliphant - oliphant@gtk.org

    http://www.ling.ed.ac.uk/~oliphant/grip

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.

*****************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "grip.h"
#include "config.h"

/* We can check to see if the CD-ROM is mounted if this is available */
#ifdef HAVE_MNTENT_H
#include <mntent.h>
#endif

/* For Linux */
#ifdef HAVE_LINUX_CDROM_H
#include <linux/cdrom.h>
#define NON_BLOCKING
#endif

/* For FreeBSD, OpenBSD, and Solaris */
#ifdef HAVE_SYS_CDIO_H
#include <sys/cdio.h>
#endif

#if defined(__FreeBSD__)
    #define CDIOREADSUBCHANNEL CDIOCREADSUBCHANNEL
#endif

/* For Digital UNIX */
#ifdef HAVE_IO_CAM_CDROM_H
#include <io/cam/cdrom.h>
#endif

/* Initialize the CD-ROM for playing audio CDs */

int CDInitDevice(char *device_name)
{
  int cd_desc;

#ifdef HAVE_MNTENT_H
  FILE *mounts;
  struct mntent *mnt;
  char devname[256];
  struct stat st;

  if(lstat(device_name, &st) < 0)
    return -1;
   
  if(S_ISLNK(st.st_mode))
    readlink(device_name, devname, 256);
  else
    strncpy(devname, device_name, 256);

  if((mounts = setmntent(MOUNTED, "r")) == NULL)
    return -1;
      
  while((mnt = getmntent(mounts)) != NULL) {
    if(strcmp(mnt->mnt_fsname, devname) == 0) {
      endmntent(mounts);
      errno = EBUSY;
      return -1;
    }
  }
  endmntent(mounts);
#endif

#ifdef NON_BLOCKING
  if((cd_desc = open(device_name, O_RDONLY | O_NONBLOCK)) < 0)
#else
    if((cd_desc = open(device_name, O_RDONLY)) < 0)
#endif
      return -1;
   
  return cd_desc;
}

/* Update a CD status structure... because operating system interfaces vary
   so does this function. */

int CDStat(int cd_desc,struct disc_info *disc,gboolean read_toc)
{
  /* Since every platform does this a little bit differently this gets pretty
     complicated... */
#ifdef CDIOREADSUBCHANNEL
  struct ioc_read_subchannel cdsc;
  struct cd_sub_channel_info data;
#endif
#ifdef CDIOREADTOCENTRYS
  struct cd_toc_entry toc_buffer[MAX_TRACKS];
  struct ioc_read_toc_entry cdte;
#endif
#ifdef CDROMSUBCHNL
  struct cdrom_subchnl cdsc;
#endif
#ifdef CDROM_READ_SUBCHANNEL
  struct cd_sub_channel sch;
#endif
#ifdef CDROMREADTOCHDR
  struct cdrom_tochdr cdth;
#endif
#ifdef CDROMREADTOCENTRY
  struct cdrom_tocentry cdte;
#endif
#ifdef CDIOREADTOCHEADER
  struct ioc_toc_header cdth;
#endif

  int readtracks,frame[MAX_TRACKS],pos;

#ifdef CDIOREADSUBCHANNEL
  bzero(&cdsc, sizeof(cdsc));
  cdsc.data = &data;
  cdsc.data_len = sizeof(data);
  cdsc.data_format = CD_CURRENT_POSITION;
  cdsc.address_format = CD_MSF_FORMAT;
   
  if(ioctl(cd_desc, CDIOCREADSUBCHANNEL, (char *)&cdsc) < 0)
#endif
#ifdef CDROM_READ_SUBCHANNEL
    sch.sch_data_format = CDROM_CURRENT_POSITION;

  sch.sch_address_format = CDROM_MSF_FORMAT;
   
  if(ioctl(cd_desc, CDROM_READ_SUBCHANNEL, &sch) < 0)
#endif
#ifdef CDROMSUBCHNL
    cdsc.cdsc_format = CDROM_MSF;

    if(ioctl(cd_desc, CDROMSUBCHNL, &cdsc) < 0)
#endif
      {
	disc->disc_present = 0;

	return 0;
      }

#ifdef CDROMSUBCHNL
  Debug("audio status is %d\n",cdsc.cdsc_audiostatus);

  if(cdsc.cdsc_audiostatus&&
     (cdsc.cdsc_audiostatus<0x11||cdsc.cdsc_audiostatus>0x15))
    {
      disc->disc_present = 0;

      return 0;
    }
#endif


  disc->disc_present = 1;

#ifdef CDIOREADSUBCHANNEL

  disc->disc_time.minutes = data.what.position.absaddr.msf.minute;
  disc->disc_time.seconds = data.what.position.absaddr.msf.second;   
  disc->disc_frame = (data.what.position.absaddr.msf.minute * 60 +
		      data.what.position.absaddr.msf.second) * 75 +
    data.what.position.absaddr.msf.frame;
   
  switch(data.header.audio_status) {
  case CD_AS_AUDIO_INVALID:
    disc->disc_mode = CDAUDIO_NOSTATUS;
    break;
  case CD_AS_PLAY_IN_PROGRESS:
    disc->disc_mode = CDAUDIO_PLAYING;
    break;
  case CD_AS_PLAY_PAUSED:
    disc->disc_mode = CDAUDIO_PAUSED;
    break;
  case CD_AS_PLAY_COMPLETED:
    disc->disc_mode = CDAUDIO_COMPLETED;
    break;
  case CD_AS_PLAY_ERROR:
    disc->disc_mode = CDAUDIO_NOSTATUS;
    break;
  case CD_AS_NO_STATUS:
    disc->disc_mode = CDAUDIO_NOSTATUS;
  }
#endif
#ifdef CDROMSUBCHNL
  disc->disc_time.minutes = cdsc.cdsc_absaddr.msf.minute;
  disc->disc_time.seconds = cdsc.cdsc_absaddr.msf.second;
  disc->disc_frame = (cdsc.cdsc_absaddr.msf.minute * 60 +
		      cdsc.cdsc_absaddr.msf.second) * 75 +
    cdsc.cdsc_absaddr.msf.frame;

  switch(cdsc.cdsc_audiostatus) {
  case CDROM_AUDIO_PLAY:
    disc->disc_mode = CDAUDIO_PLAYING;
    break;
  case CDROM_AUDIO_PAUSED:
    disc->disc_mode = CDAUDIO_PAUSED;
    break;
  case CDROM_AUDIO_NO_STATUS:
    disc->disc_mode = CDAUDIO_NOSTATUS;
    break;
  case CDROM_AUDIO_COMPLETED:
    disc->disc_mode = CDAUDIO_COMPLETED;
    break;
  }
#endif

  if(read_toc) {

    /* Read the Table Of Contents header */

#ifdef CDIOREADTOCHEADER
    if(ioctl(cd_desc, CDIOREADTOCHEADER, (char *)&cdth) < 0) {
      printf("Error: Failed to read disc contents\n");

      return -1;
    }
    
    disc->disc_totaltracks = cdth.ending_track;
#endif
#ifdef CDROMREADTOCHDR
    if(ioctl(cd_desc, CDROMREADTOCHDR, &cdth) < 0) {
      printf("Error: Failed to read disc contents\n");

      return -1;
    }
    
    disc->disc_totaltracks = cdth.cdth_trk1;
#endif
    
    /* Read the table of contents */
    
#ifdef CDIOREADTOCENTRYS
    cdte.address_format = CD_MSF_FORMAT;
    cdte.starting_track = 0;
    cdte.data = toc_buffer;
    cdte.data_len = sizeof(toc_buffer);
    
    if(ioctl(cd_desc, CDIOREADTOCENTRYS, (char *)&cdte) < 0) {
      printf("Error: Failed to read disc contents\n");

      return -1;
    }
    
    for(readtracks = 0; readtracks <= disc->disc_totaltracks; readtracks++) {
      disc->track[readtracks].track_pos.minutes =
	cdte.data[readtracks].addr.msf.minute;
      disc->track[readtracks].track_pos.seconds =
	cdte.data[readtracks].addr.msf.second;
      frame[readtracks] = cdte.data[readtracks].addr.msf.frame;
    }
#endif
#ifdef CDROMREADTOCENTRY
    for(readtracks = 0; readtracks <= disc->disc_totaltracks; readtracks++) {
      if(readtracks == disc->disc_totaltracks)	
	cdte.cdte_track = CDROM_LEADOUT;
      else
	cdte.cdte_track = readtracks + 1;
      
      cdte.cdte_format = CDROM_MSF;
      if(ioctl(cd_desc, CDROMREADTOCENTRY, &cdte) < 0) {
	printf("Error: Failed to read disc contents\n");

	return -1;
      }
      
      disc->track[readtracks].track_pos.minutes = cdte.cdte_addr.msf.minute;
      disc->track[readtracks].track_pos.seconds = cdte.cdte_addr.msf.second;
      frame[readtracks] = cdte.cdte_addr.msf.frame;
    }
#endif
    
    for(readtracks = 0; readtracks <= disc->disc_totaltracks; readtracks++) {
      disc->track[readtracks].track_start=
	(disc->track[readtracks].track_pos.minutes * 60 +
	 disc->track[readtracks].track_pos.seconds) * 75 + frame[readtracks];
      
      if(readtracks > 0) {
	pos = (disc->track[readtracks].track_pos.minutes * 60 +
	       disc->track[readtracks].track_pos.seconds) -
	  (disc->track[readtracks - 1].track_pos.minutes * 60 +
	   disc->track[readtracks -1].track_pos.seconds);
	disc->track[readtracks - 1].track_length.minutes = pos / 60;
	disc->track[readtracks - 1].track_length.seconds = pos % 60;
      }
    }
    
    disc->disc_length.minutes=
      disc->track[disc->disc_totaltracks].track_pos.minutes;
    
    disc->disc_length.seconds=
      disc->track[disc->disc_totaltracks].track_pos.seconds;
  }
   
  disc->disc_track = 0;

  while(disc->disc_track < disc->disc_totaltracks &&
	disc->disc_frame >= disc->track[disc->disc_track].track_start)
    disc->disc_track++;

  pos=(disc->disc_frame - disc->track[disc->disc_track - 1].track_start) / 75;

  disc->track_time.minutes = pos / 60;
  disc->track_time.seconds = pos % 60;

  return 0;
}

/* Play frames from CD */

int CDPlayFrames(int cd_desc,int startframe,int endframe)
{
#ifdef CDIOCPLAYMSF
  struct ioc_play_msf cdmsf;
#endif
#ifdef CDROMPLAYMSF
  struct cdrom_msf cdmsf;
#endif

#ifdef CDIOCPLAYMSF
  cdmsf.start_m = startframe / (60 * 75);
  cdmsf.start_s = (startframe % (60 * 75)) / 75;
  cdmsf.start_f = startframe % 75;
  cdmsf.end_m = endframe / (60 * 75);
  cdmsf.end_s = (endframe % (60 * 75)) / 75;
  cdmsf.end_f = endframe % 75;
#endif
#ifdef CDROMPLAYMSF
  cdmsf.cdmsf_min0 = startframe / (60 * 75);
  cdmsf.cdmsf_sec0 = (startframe % (60 * 75)) / 75;
  cdmsf.cdmsf_frame0 = startframe % 75;
  cdmsf.cdmsf_min1 = endframe / (60 * 75);
  cdmsf.cdmsf_sec1 = (endframe % (60 * 75)) / 75;
  cdmsf.cdmsf_frame1 = endframe % 75;
#endif

#ifdef CDIOCSTART
  if(ioctl(cd_desc,CDIOCSTART) < 0)
    return -1;
#endif
#ifdef CDROMSTART
  if(ioctl(cd_desc,CDROMSTART) < 0)
    return -1;
#endif
#ifdef CDIOCPLAYMSF
  if(ioctl(cd_desc,CDIOCPLAYMSF, (char *)&cdmsf) < 0)
    return -1;
#endif
#ifdef CDROMPLAYMSF
  if(ioctl(cd_desc,CDROMPLAYMSF, &cdmsf) < 0)
    return -1;
#endif
   
  return 0;
}

/* Play starttrack at position pos to endtrack */

int CDPlayTrackPos(int cd_desc,struct disc_info *disc,int starttrack,
		   int endtrack,int startpos)
{
  return CDPlayFrames(cd_desc, disc->track[starttrack - 1].track_start +
		      startpos * 75, endtrack >= disc->disc_totaltracks ?
		      (disc->disc_length.minutes * 60 +
		       disc->disc_length.seconds) * 75 :
		      disc->track[endtrack].track_start - 1);
}

/* Play starttrack to endtrack */

int CDPlayTrack(int cd_desc,struct disc_info *disc,int starttrack,int endtrack)
{
  return CDPlayTrackPos(cd_desc,disc,starttrack,endtrack,0);
}

/* Advance (fastfwd) */

int CDAdvance(int cd_desc,struct disc_info *disc,struct disc_timeval *time)
{
  disc->track_time.minutes += time->minutes;
  disc->track_time.seconds += time->seconds;

  if(disc->track_time.seconds > 60) {
    disc->track_time.seconds -= 60;
    disc->track_time.minutes++;
  }

  if(disc->track_time.seconds < 0) {
    disc->track_time.seconds = 60 + disc->track_time.seconds;
    disc->track_time.minutes--;
  }
  
  /*  If we skip back past the beginning of a track, go to the end of
      the last track - DCV */
  if(disc->track_time.minutes < 0) {
    disc->disc_track--;
    
    /*  Tried to skip past first track so go to the beginning  */
    if(disc->disc_track == 0) {
      disc->disc_track = 1;
      return CDPlayTrack(cd_desc,disc,disc->disc_track,disc->disc_track);
    }
    
    /*  Go to the end of the last track  */
    disc->track_time.minutes = disc->track[(disc->disc_track)-1].
      track_length.minutes;
    disc->track_time.seconds = disc->track[(disc->disc_track)-1].
      track_length.seconds;

    /*  Try again  */
    return CDAdvance(cd_desc,disc,time);
  }
   
  if((disc->track_time.minutes ==
      disc->track[disc->disc_track].track_pos.minutes &&
      disc->track_time.seconds >
      disc->track[disc->disc_track].track_pos.seconds)
     || disc->track_time.minutes >
     disc->track[disc->disc_track].track_pos.minutes) {
    disc->disc_track++;

    if(disc->disc_track > disc->disc_totaltracks)
      disc->disc_track=disc->disc_totaltracks;
      
    return CDPlayTrack(cd_desc,disc,disc->disc_track,disc->disc_track);
  }
   
  return CDPlayTrackPos(cd_desc,disc,disc->disc_track,disc->disc_track,
			disc->track_time.minutes * 60 +
			disc->track_time.seconds);
}

/* Stop the CD, if it is playing */

int CDStop(int cd_desc)
{
#ifdef CDIOCSTOP
  if(ioctl(cd_desc, CDIOCSTOP) < 0)
    return -1;
#endif
#ifdef CDROMSTOP
  if(ioctl(cd_desc, CDROMSTOP) < 0)
    return -1;
#endif
   
  return 0;
}

/* Pause the CD */

int CDPause(int cd_desc)
{
#ifdef CDIOCPAUSE
  if(ioctl(cd_desc, CDIOCPAUSE) < 0)
    return -1;
#endif
#ifdef CDROMPAUSE
  if(ioctl(cd_desc, CDROMPAUSE) < 0)
    return -1;
#endif
   
  return 0;
}

/* Resume playing */

int CDResume(int cd_desc)
{
#ifdef CDIOCRESUME
  if(ioctl(cd_desc, CDIOCRESUME) < 0)
    return -1;
#endif
#ifdef CDROMRESUME
  if(ioctl(cd_desc, CDROMRESUME) < 0)
    return -1;
#endif
   
  return 0;
}

/* Check the tray status */

int TrayOpen(int cd_desc)
{
#ifdef CDROM_DRIVE_STATUS
  int status;

  if(ioctl(cd_desc,CDROM_DRIVE_STATUS,&status) < 0) {
    Debug("Drive doesn't support drive status check\n");
    return -1;
  }

  return status==CDS_TRAY_OPEN;
#endif

  return -1;
}

/* Eject the tray */
int CDEject(int cd_desc)
{  
#ifdef CDIOCEJECT
  /*  always unlock door before an eject in case something else locked it  */
#if defined(CDROM_LOCKDOOR)
  if(ioctl(cd_desc, CDROM_LOCKDOOR, 0) < 0)
    printf("Unlock failed: %d", errno);
#endif
#ifdef CDIOCALLOW
  if(ioctl(cd_desc, CDIOCALLOW) < 0)
    printf("Unlock failed: %d", errno);
#endif

  if(ioctl(cd_desc, CDIOCEJECT) < 0) {
    perror("CDIOCEJECT");
    return -1;
  }
#endif
#ifdef CDROMEJECT
  if(ioctl(cd_desc, CDROMEJECT) < 0)
    return -1;
#endif
   
  return 0;
}

/* Close the tray */

int CDClose(int cd_desc)
{
#ifdef CDIOCCLOSE
  if(ioctl(cd_desc, CDIOCCLOSE) < 0)
    return -1;
#endif
#ifdef CDROMCLOSETRAY
  if(ioctl(cd_desc, CDROMCLOSETRAY) < 0)
    return -1;
#endif
   
  return 0;
}

int CDGetVolume(int cd_desc, struct disc_volume *vol)
{
#ifdef CDIOCGETVOL
  struct ioc_vol volume;
#endif
#ifdef CDROMVOLREAD
  struct cdrom_volctrl volume;
#endif
   
#ifdef CDIOCGETVOL
  if(ioctl(cd_desc, CDIOCGETVOL, &volume) < 0)
    return -1;
   
  vol->vol_front.left = volume.vol[0];
  vol->vol_front.right = volume.vol[1];
  vol->vol_back.left = volume.vol[2];
  vol->vol_back.right = volume.vol[3];
#endif
#ifdef CDROMVOLREAD
  if(ioctl(cd_desc, CDROMVOLREAD, &volume) < 0)
    return -1;
      
  vol->vol_front.left = volume.channel0;
  vol->vol_front.right = volume.channel1;
  vol->vol_back.left = volume.channel2;
  vol->vol_back.right = volume.channel3;
#endif
   
  return 0;
}

int CDSetVolume(int cd_desc, struct disc_volume *vol)
{
#ifdef CDIOCSETVOL
  struct ioc_vol volume;
#endif
#ifdef CDROMVOLCTRL
  struct cdrom_volctrl volume;
#endif
   
  if(vol->vol_front.left > 255 || vol->vol_front.left < 0 ||
     vol->vol_front.right > 255 || vol->vol_front.right < 0 ||
     vol->vol_back.left > 255 || vol->vol_back.left < 0 ||
     vol->vol_back.right > 255 || vol->vol_back.right < 0)
    return -1;

#ifdef CDIOCSETVOL
  volume.vol[0] = vol->vol_front.left;
  volume.vol[1] = vol->vol_front.right;
  volume.vol[2] = vol->vol_back.left;
  volume.vol[3] = vol->vol_back.right;
   
  if(ioctl(cd_desc, CDIOCSETVOL, &volume) < 0)
    return -1;
#endif
#ifdef CDROMVOLCTRL
  volume.channel0 = vol->vol_front.left;
  volume.channel1 = vol->vol_front.right;
  volume.channel2 = vol->vol_back.left;
  volume.channel3 = vol->vol_back.right;
   
  if(ioctl(cd_desc, CDROMVOLCTRL, &volume) < 0)
    return -1;
#endif
   
  return 0;
}

/* CD Changer routines */

/* Choose a particular disc from the CD changer */

int CDChangerSelectDisc(int cd_desc,int disc)
{
#ifdef CDROM_SELECT_DISC
  if(ioctl(cd_desc, CDROM_SELECT_DISC, disc) < 0)
    return -1;
   
  return 0;
#else
  errno = ENOSYS;
   
  return -1;
#endif
}

/* Identify how many CD-ROMs the changer can hold */

int CDChangerSlots(int cd_desc)
{
#ifdef CDROM_CHANGER_NSLOTS
  int slots;

  if((slots = ioctl(cd_desc, CDROM_CHANGER_NSLOTS)) < 0)
    slots = 1;
   
  if(slots == 0)
    return 1;
   
  return slots;
#else
  return 1;
#endif
}
