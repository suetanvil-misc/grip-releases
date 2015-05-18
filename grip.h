/*****************************************************************

  grip.h

  Copyright (c) 1998 by Mike Oliphant - oliphant@ling.ed.ac.uk

    http://www.ling.ed.ac.uk/~oliphant/grip

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.

*****************************************************************/

#ifndef _GRIP_H
#define _GRIP_H

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdarg.h>

#define MP3DB

#define VERSION "2.94"

#ifdef GRIPCD
#define PROGRAM "GCD"
#else
#define PROGRAM "Grip"
#endif

#define WINWIDTH 285
#define WINHEIGHT 250

/* Max number of CPU's supported */

#define MAX_NUM_CPU 16

/* Ripper default info structure */

typedef struct _ripper {
  char name[20];
  char cmdline[256];
} Ripper;

/* Encoder default info structure */

typedef struct _mp3_encoder {
  char name[20];
  char cmdline[256];
} MP3Encoder;

/* Time display modes */

#define TIME_MODE_TRACK 0
#define TIME_MODE_DISC 1
#define TIME_MODE_LEFT_TRACK 2
#define TIME_MODE_LEFT_DISC 3

/* Play mode types */

#define PM_NORMAL 0
#define PM_SHUFFLE 1
#define PM_PLAYLIST 2
#define PM_LASTMODE 3

/* Random value in a range */

#define RRand(range) (random()%(range))


/* Used with disc_info */
#define CDAUDIO_PLAYING				0
#define CDAUDIO_PAUSED				1
#define CDAUDIO_COMPLETED			2
#define CDAUDIO_NOSTATUS			3

#define MAX_TRACKS				100
#define MAX_SLOTS				100 /* For CD changers */

#define CURRENT_CDDBREVISION			2


/* CDDB hello structure */
struct CDDBHello {
   /* Program */
   char hello_program[256];
   /* Program version */
   char hello_version[256];
};

/* Used for keeping track of times */
struct disc_timeval {
   int minutes;
   int seconds;
};

/* Track specific information */
struct track_info {
  struct disc_timeval track_length;
  struct disc_timeval track_pos;
  int track_frames;
  int track_start;
};

/* Disc information such as current track, amount played, etc */
struct disc_info {
   int disc_present;				/* Is disc present? */
   int disc_mode;				/* Current disc mode */
   struct disc_timeval track_time;		/* Current track time */
   struct disc_timeval disc_time;		/* Current disc time */
   struct disc_timeval disc_length;		/* Total disc length */
   int disc_frame;				/* Current frame */
   int disc_track;				/* Current track */
   int disc_totaltracks;			/* Number of tracks on disc */
   struct track_info track[MAX_TRACKS];		/* Track specific information */
};

/* Invisible volume structure */
struct __volume { 
   int left;
   int right;
};

/* Volume structure */
struct disc_volume {
   struct __volume vol_front;
   struct __volume vol_back;
};

/* HTTP proxy server structure */

typedef struct _proxy_server {
  char name[256];
  int port;
} ProxyServer;

/* CDDB server structure */

typedef struct _cddb_server {
  char name[256];
  char cgi_prog[256];
  int port;
  int use_proxy;
  ProxyServer *proxy;
} CDDBServer;

#define GRIP_CDDB_LEVEL "3"  /* Current CDDB protocol level supported */

/* CDDB entry */

typedef struct _cddb_entry {
   unsigned int entry_id;
   int entry_genre;
} CDDBEntry;

/* CDDB hello structure */

typedef struct _cddb_hello {
   /* Program */
   char hello_program[256];
   /* Program version */
   char hello_version[256];
} CDDBHello;

#define MAX_INEXACT_MATCHES			16

/* An entry in the query list */
struct query_list_entry {
   int list_genre;
   int list_id;
   char list_title[64];
   char list_artist[64];
};

/* CDDB query structure */

typedef struct _cddb_query {
   int query_match;
   int query_matches;
   struct query_list_entry query_list[MAX_INEXACT_MATCHES];
} CDDBQuery;

/* Match values returned by a query */

#define MATCH_NOMATCH	 0
#define MATCH_EXACT	 1
#define MATCH_INEXACT	 2

/* Track database structure */

typedef struct _track_data {
  char track_name[256];	              /* Track name */
  char track_artist[256];	      /* Track artist */
  char track_extended[4096];	      /* Extended information */
} TrackData;

/* Disc database structure */

typedef struct _disc_data {
  unsigned int data_id;	      /* CD id */
  char data_title[256];	              /* Disc title */
  char data_artist[256];	      /* We may be able to extract this */
  char data_extended[4096];	      /* Extended information */
  int data_genre;		      /* Disc genre */
  int data_year;                      /* Disc year */
  char data_playlist[256];            /* Playlist info */
  gboolean data_multi_artist;         /* Is CD multi-artist? */
  TrackData data_track[MAX_TRACKS];   /* Track names */
} DiscData;


/* Encode list structure */

typedef struct _encode_track {
  int track_num;
  int start_frame;
  int end_frame;
  char song_name[80];
  char song_artist[80];
  char disc_name[80];
  char disc_artist[80];
  int song_year;
  int id3_genre;
  int mins;
  int secs;
  int discid;
} EncodeTrack;


/* CDDB lookup routines -- found in cddb.c */

unsigned int CDDBDiscid(int cd_desc);
char *CDDBGenre(int genre);
int CDDBGenreValue(char *genre);
gboolean CDDBDoQuery(int cd_desc,CDDBServer *server,
		     CDDBHello *hello,CDDBQuery *query);
gboolean CDDBRead(int cd_desc,CDDBServer *server,
		  CDDBHello *hello,CDDBEntry *entry,
		  DiscData *data);
gboolean CDDBRead(int cd_desc,CDDBServer *server,
		  CDDBHello *hello,CDDBEntry *entry,
		  DiscData *data);
gboolean CDDBStatDiscData(int cd_desc);
int CDDBReadDiscData(int cd_desc, DiscData *outdata);
int CDDBWriteDiscData(int cd_desc,DiscData *ddata,FILE *outfile,
		      gboolean gripext);
void CDDBParseTitle(char *buf,char *title,char *artist,char *sep);
char *ChopWhite(char *buf);


/* Low-level cd control routines -- found in cd.c */

int CDInitDevice(char *device_name);
int CDStat(int cd_desc,struct disc_info *disc,gboolean read_toc);
int CDPlayFrames(int cd_desc,int startframe,int endframe);
int CDPlayTrackPos(int cd_desc,struct disc_info *disc,int starttrack,
		   int endtrack,int startpos);
int CDPlayTrack(int cd_desc,struct disc_info *disc,int starttrack,
		int endtrack);
int CDAdvance(int cd_desc,struct disc_info *dics,struct disc_timeval *time);
int CDStop(int cd_desc);
int CDPause(int cd_desc);
int CDResume(int cd_desc);
int TrayOpen(int cd_desc);
int CDEject(int cd_desc);
int CDClose(int cd_desc);
int CDGetVolume(int cd_desc, struct disc_volume *vol);
int CDSetVolume(int cd_desc, struct disc_volume *vol);
int CDChangerSelectDisc(int cd_desc,int disc);
int CDChangerSlots(int cd_desc);

/* from bug.c */

void BugReport(void);

/* from id3.c */
gboolean ID3TagFile(char *filename,char *title,char *artist,char *album,
		    char *year,char *comment,unsigned char genre,
		    unsigned char tracknum);


/* from cdpar.c */

#ifdef CDPAR
typedef struct _cdp_args {
  char *device;
  int track;
  long first_sector;
  long last_sector;
  char *outfile;
  int paranoia_mode;
} CDPArgs;

gboolean CDPRip(char *device,int track,long first_sector,long last_sector,
		char *outfile,int paranoia_mode);
#endif

/* functions in grip.c used in other files */

void Debug(char *fmt,...);
void BoolDialog(char *question,char *yes,GtkSignalFunc yesfunc,
		char *no,GtkSignalFunc nofunc);

#endif
