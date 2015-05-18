/*****************************************************************

  grip.c

  Copyright (c) 1998, 1999 by Mike Oliphant - oliphant@gtk.org

    http://www.nostatic.org/grip

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.

*****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef SOLARIS
#include <strings.h>
#endif
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#ifdef SOLARIS
#include <sys/statvfs.h>
#endif
#include <sys/time.h>
#if defined(__linux__)
    #include <sys/vfs.h>
#endif
#if defined(__FreeBSD__)
    #include <sys/param.h>
    #include <sys/mount.h>
#endif
#include <signal.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <pthread.h>
#include <X11/Xlib.h>
#include "grip.h"
#include "xpm.h"
#include "config.h"
#include "parsecfg.h"
#include "dialog/dialog.h"

#ifdef CDPAR
#define size16 short
#define size32 int
#include "cdparanoia/interface/cdda_interface.h"
#include "cdparanoia/paranoia/cdda_paranoia.h"
#endif

void ShutDownCB(void);
void ShutDown(void);
gboolean FileExists(char *filename);
gboolean IsDir(char *path);
int BytesLeftInFS(char *path);
void UpdateGTK(void);
void CheckWindowPosition(void);
void Busy(void);
void UnBusy(void);
void MinMax(void);
void ToggleControlButtons(void);
void ToggleVol(void);
void ToggleProg(void);
void PlaylistChanged(void);
void UpdateMultiArtist(void);
void ToggleTrackEdit(void);
void SetTitle(char *title);
void SetArtist(char *artist);
void SetCDDBGenre(int genre);
void SetYear(int year);
void SaveDiscInfo(void);
void TitleEditChanged(void);
void ArtistEditChanged(void);
void YearEditChanged(void);
void TrackEditChanged(void);
void CDDBGenreChanged(void);
void EditNextTrack(void);
gint TimeOut(gpointer data);
char *MungeString(char *str);
void SeparateFields(char *buf,char *title,char *artist,char *sep);
void SplitTitleArtist(GtkWidget *widget,gpointer data);
void ParseFileFmt(char *instr,char *outstr,int track,int startsec,int endsec);
void ParseWavCmd(char *instr,char *outstr,char *ripfile);
void KillChild(void);
void SetVolume(GtkWidget *widget,gpointer data);
void NextTrack(void);
void PrevTrack(void);
void InitProgram(int num_tracks,int mode);
void ToggleLoop(void);
void ChangePlayMode(void);
void FastFwdCB(GtkWidget *widget,gpointer data);
void FastFwd(void);
void RewindCB(GtkWidget *widget,gpointer data);
void Rewind(void);
void NextDisc(void);
void StopPlay(void);
void EjectDisc(void);
void PlayTrackCB(GtkWidget *widget,gpointer data);
void PlayTrack(int track);
void ChangeTimeMode(void);
void CheckNewDisc(void);
void UpdateDisplay(void);
gboolean CheckTracks(void);
void SetCurrentTrack(int track);
gboolean TrackIsChecked(int track);
void ToggleChecked(int track);
void SetChecked(int track,gboolean checked);
void ClickColumn(GtkWidget *widget,gint column,gpointer data);
void CListButtonPressed(GtkWidget *widget,GdkEventButton *event,gpointer data);
void UnSelectRow(GtkWidget *widget,gint row,gint column,
		 GdkEventButton *event,gpointer data);
void SelectRow(GtkWidget *widget,gint row,gint column,
	       GdkEventButton *event,gpointer data);
void CDDBToggle(void);
void LookupDisc(gboolean manual);
void DoLookup(void);
gboolean CDDBLookupDisc(CDDBServer *server);
void SubmitEntryCB(void);
void SubmitEntry(void);
void UpdateTracks(void);
void UseProxyChanged(void);
GtkWidget *MakeNewPage(char *name);
void MakeTrackPage(void);
void ChopCR(char *str);
void DoLoadConfig(void);
void DoSaveConfig(void);
char *FindExe(char *exename,char **paths);
void MakeConfigPage(void);
void Homepage(void);
void MakeHelpPage(void);
void MakeAboutPage(void);
GtkTooltips *MakeToolTip(void);
GtkWidget *MakeEditBox(void);
GtkWidget *MakePlayOpts(void);
GtkWidget *MakeControls(void);
GtkWidget *ImageButton(char **xpm);
GtkWidget *ImageToggleButton(char **xpm, gint state);
GtkWidget *Loadxpm(char **xpm);
GtkWidget *NewBlankPixmap(void);
void CopyPixmap(GtkPixmap *src,GtkPixmap *dest);
GdkColor *MakeColor(int red,int green,int blue);
GtkStyle *MakeStyle(GdkColor *fg,GdkColor *bg,gboolean do_grade);
void Usage(void);
void ParseArgs(int numargs,char *args[]);
void ShuffleTracks(int num_tracks);
void MakeStyles(void);

/* Rip-related functions */

#ifndef GRIPCD

char *MakeRelative(char *file1,char *file2);
void AddM3U(void);
#ifdef MP3DB
void DBScan(void);
void AddSQLEntry(EncodeTrack *enc_track);
#endif

void RipPartialChanged(void);
void ParseEncFileFmt(char *instr,char *outstr,EncodeTrack *enc_track);
void CheckDupNames(void);
void UpdateRipProgress(void);
char *FindRoot(char *str);
void MakeArgs(char *str,char **args,int maxargs);
void ParseMP3Cmd(char *instr,char *outstr,char *ripfile,char *mp3,int cpu);
void FillInEncode(int track,EncodeTrack *new_track);
void AddToEncode(int track);
gboolean MP3Encode(void);

void RedirectIO(gboolean redirect_output);
void ID3Add(char *file,EncodeTrack *enc_track);
void RipIsFinished(void);
void DoRip(GtkWidget *widget,gpointer data);
void MakeDirs(char *path);
char *MakePath(char *str);
int NextTrackToRip(void);
gboolean RipNextTrack(void);
void PlaySegmentCB(GtkWidget *widget,gpointer data);
void PlaySegment(int track);
void UpdateID3Genre(void);
void ID3GenreChanged(GtkWidget *widget,gint number);
GtkWidget *MakeRangeSelects(void);
void MakeRipPage(void);
void RipperSelected(GtkWidget *widget,Ripper *rip);
void EncoderSelected(GtkWidget *widget,MP3Encoder *enc);
#endif /* ifndef GRIPCD */

char *Program=PROGRAM;
char *Version=VERSION;

GdkColor gdkblack;
GdkColor gdkwhite;
GdkColor *color_LCD;
GdkColor *color_dark_grey;

GtkStyle *style_wb;
GtkStyle *style_LCD;
GtkStyle *style_dark_grey;

GdkCursor *wait_cursor;

int cd_desc;
int changer_slots;
int current_disc=0;
int volume=255;
gboolean local_mode=FALSE;
struct disc_info info;
DiscData ddata;
ProxyServer proxy_server={"",8000};
CDDBServer dbserver={"freedb.freedb.org","~cddb/cddb.cgi",
		     80,0,&proxy_server};
CDDBServer dbserver2={"","~cddb/cddb.cgi",
		      80,0,&proxy_server};
#ifdef SOLARIS
char *cddevice = NULL;
#else
char cddevice[80]="/dev/cdrom";
#endif
int current_discid;
int first_time=1;
gboolean have_working_device=FALSE;
gboolean do_debug=FALSE;
gboolean stopped=FALSE;
gboolean playing=FALSE;
gboolean looking_up=FALSE;
gboolean ask_submit=FALSE;
gboolean update_required=TRUE;
gboolean is_new_disc=FALSE;
gboolean have_disc=FALSE;
#ifndef GRIPCD
gboolean minimized=FALSE;
#else
gboolean minimized=TRUE;
#endif
gboolean control_buttons_visible=TRUE;
gboolean volvis=FALSE;
gboolean track_edit_visible=FALSE;
gboolean track_prog_visible=FALSE;
gboolean ffwding=FALSE;
gboolean rewinding=FALSE;
int time_display_mode=TIME_MODE_TRACK;

/* The play mode */
unsigned long int play_mode = PM_NORMAL;
gboolean playloop=TRUE;

/* Related to program management */
int tracks_prog[MAX_TRACKS];
int prog_totaltracks    = 0;
int current_track_index = 0;
#define PREV_TRACK    (tracks_prog[current_track_index - 1])
#define CURRENT_TRACK (tracks_prog[current_track_index])
#define NEXT_TRACK    (tracks_prog[current_track_index + 1])

GtkWidget *window;
GtkWidget *winbox;
GtkWidget *controls;
GtkWidget *control_button_box;
GtkWidget *rip_exe_box;
GtkWidget *rip_builtin_box;
GtkWidget *playopts;
GtkWidget *notebook;
GtkWidget *trackpage;
GtkWidget *helppage;
GtkWidget *configpage;
GtkWidget *config_notebook;
GtkWidget *aboutpage;
GtkWidget *trackclist;
#ifndef GRIPCD
gchar *titles[3]={"Track","Length","Rip"};
#else
gchar *titles[3]={"Track","Length","PL"};
#endif
GtkWidget *disc_name_label;
GtkWidget *disc_artist_label;
GtkWidget *current_track_label;
GtkWidget *play_time_label;
GtkWidget *play_sector_label;
GtkWidget *volume_control;
GtkWidget *track_edit_entry;
GtkWidget *track_artist_edit_entry;
GtkWidget *multi_artist_box;
GtkWidget *title_edit_entry;
GtkWidget *year_spin_button;
GtkWidget *artist_edit_entry;
GtkWidget *track_edit_box;
GtkWidget *playlist_entry;
GtkWidget *check_image;
GtkWidget *empty_image;
GtkWidget *cddb_indicator;
GtkWidget *cddb_pix[2];
GtkWidget *play_indicator;
GtkWidget *play_pix[3];
GtkWidget *loop_indicator;
GtkWidget *loop_pic;
GtkWidget *noloop_pic;

char cdupdate[256]="";
char user_email[256]=""; /* User's email address as defined in Misc page */

gboolean use_proxy=FALSE;
gboolean use_proxy_env=FALSE;

char *bin_search_paths[]={"/cpd/misc/bin","/usr/bin","/usr/local/bin",NULL};
Ripper ripper_defaults[]={
#ifdef CDPAR
  {"grip (cdparanoia)",""},
#endif
  {"cdparanoia","-d %c %t:[.%b]-%t:[.%e] %f"},
#ifdef SOLARIS
  {"cdda2wav","-x -H -t %t -O wav %f"},
#else
  {"cdda2wav","-D %c -x -H -t %t -O wav %f"},
#endif
  {"other",""},
  {"",""}};
char ripexename[256]="/usr/bin/cdparanoia";
char ripcmdline[256]="-d %c %t:[.%b]-%t:[.%e] %f";
int selected_ripper=0;
char outputdir[256];
char ripfileformat[256]="~/mp3/%a/%d/%n.wav";
char wav_filter_cmd[256]="";
MP3Encoder encoder_defaults[]={{"bladeenc","-%b -QUIT %f"},
			       {"lame","-p -b %b %f %o"},
			       {"l3enc","-br %b %f %o"},
			       {"xingmp3enc","-B %b -Q %f"},
			       {"mp3encode","-p 2 -l 3 -b %b %f %o"},
			       {"gogo","-b %b %f %o"},
			       {"other",""},
			       {"",""}};
gboolean disable_paranoia=FALSE;
gboolean disable_extra_paranoia=FALSE;
gboolean disable_scratch_detect=FALSE;
gboolean disable_scratch_repair=FALSE;
char mp3exename[256]="/usr/bin/bladeenc";
char mp3cmdline[256]="-%b -QUIT %f";
int selected_encoder=1;
char mp3fileformat[256]="~/mp3/%a/%d/%n.mp3";
char m3ufileformat[256]="~/mp3/%a-%d.m3u";
int ripnice=0;
int mp3nice=0;
char title_split_chars[6]="/";
gboolean rip_partial=FALSE;
gboolean delete_wavs=TRUE;
gboolean add_m3u=TRUE;
gboolean rel_m3u=TRUE;
gboolean add_to_db=FALSE;
gboolean doid3=TRUE;
gboolean no_interrupt=FALSE;
gboolean stop_first=FALSE;
gboolean play_first=TRUE;
gboolean automatic_cddb=TRUE;
gboolean automatic_reshuffle=TRUE;
gboolean keep_min_size=TRUE;
gboolean no_lower_case=FALSE;
gboolean no_underscore=FALSE;
char allow_these_chars[256]="";
int start_sector,end_sector;
int max_wavs=99;
gboolean faulty_eject=FALSE;
gboolean tray_open=FALSE;
gboolean auto_rip=FALSE;
gboolean eject_after_rip=FALSE;
gboolean beep_after_rip=FALSE;
int eject_delay=0;
int auto_eject_countdown=0;
char cddb_submit_email[256]="freedb-submit@freedb.org";

GtkWidget *cddb_genre_combo;
GList *cddb_genre_item_list=NULL;
int num_cpu=1;
int edit_num_cpu=1;
int kbits_per_sec=128;

/* Rip-related variables */

#ifndef GRIPCD
gboolean doencode=FALSE;
GtkWidget *rippage;
GtkWidget *ripprogbar;
GtkWidget *mp3progbar[MAX_NUM_CPU];
GList *encode_list=NULL;
EncodeTrack *encoded_track[MAX_NUM_CPU];
GtkWidget *partial_rip_frame;
GtkWidget *partial_rip_box;

gboolean stop_rip=FALSE;
gboolean ripping=FALSE;
gboolean ripping_a_disc=FALSE;
gboolean in_rip_thread=FALSE;
gboolean using_builtin_cdp=FALSE;
gboolean stop_thread_rip_now=FALSE;
gfloat rip_percent_done;
char ripfile[1024];
int ripsize;
int rippid;
int rip_quarter;
int rip_track;
char rip_delete_file[MAX_NUM_CPU][1024];
char mp3file[MAX_NUM_CPU][1024];
int mp3pid[MAX_NUM_CPU];
int encoding=0;
int mp3size[MAX_NUM_CPU];
int num_wavs=0;
GtkWidget *rip_prog_label;
GtkWidget *mp3_prog_label[MAX_NUM_CPU];
int rip_smile_level;
GtkWidget *rip_indicator;
GtkWidget *rip_pix[4];
GtkWidget *mp3_indicator[MAX_NUM_CPU];
GtkWidget *mp3_pix[4];
GtkWidget *smile_indicator;
GtkWidget *lcd_smile_indicator;
GtkWidget *smile_pix[8];

GtkWidget *id3_genre_combo=NULL;
GList *id3_genre_item_list=NULL;
gint id3_genre_number = 12; /* default to "Other" */

/* this array contains string representations of all known ID3 tags */
/* taken from mp3id3 in the mp3tools 0.7 package */

gchar *id3_genres[] = {
  "Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk", "Grunge",
  "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies", "Other", "Pop", "R&B",
  "Rap", "Reggae", "Rock", "Techno", "Industrial", "Alternative", "Ska",
  "Death Metal", "Pranks", "Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop",
  "Vocal", "Jazz+Funk", "Fusion", "Trance", "Classical", "Instrumental",
  "Acid", "House", "Game", "Sound Clip", "Gospel", "Noise", "AlternRock",
  "Bass", "Soul", "Punk", "Space", "Meditative", "Instrumental Pop",
  "Instrumental Rock", "Ethnic", "Gothic", "Darkwave", "Techno-Industrial",
  "Electronic", "Pop-Folk", "Eurodance", "Dream", "Southern Rock", "Comedy",
  "Cult", "Gangsta", "Top 40", "Christian Rap", "Pop/Funk", "Jungle",
  "Native American", "Cabaret", "New Wave", "Psychadelic", "Rave", "Showtunes",
  "Trailer", "Lo-Fi", "Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro",
  "Musical", "Rock & Roll", "Hard Rock", "Folk", "Folk/Rock", "National Folk",
  "Swing", "Fast Fusion", "Bebob", "Latin", "Revival", "Celtic", "Bluegrass",
  "Avantgarde",
  "Gothic Rock", "Progressive Rock", "Psychedelic Rock", "Symphonic Rock",
  "Slow Rock", "Big Band", "Chorus", "Easy Listening", "Acoustic", "Humour",
  "Speech", "Chanson", "Opera", "Chamber Music", "Sonata", "Symphony",
  "Booty Bass", "Primus", "Porn Groove", "Satire", "Slow Jam", "Club", "Tango",
  "Samba", "Folklore", "Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle",
  "Duet", "Punk Rock", "Drum Solo", "Acapella", "Euro-house", "Dance Hall",
  NULL
};

/* This array maps CDDB_ genre numbers to closest id3 genre */
int genre_cdda_2_id3[] =
{
  12,         /* CDDB_UNKNOWN   = 12 - other */
  0,          /* CDDB_BLUES     = 0 */
  32,         /* CDDB_CLASSICAL = 32 */
  2,          /* CDDB_COUNTRY   = 2 */
  12,         /* CDDB_DATA      = 12 - other */
  80,         /* CDDB_FOLK      = 80 */
  8,          /* CDDB_JAZZ      = 8 */
  12,         /* CDDB_MISC      = 12 - other */
  10,         /* CDDB_NEWAGE    = 10 */
  16,         /* CDDB_REGGAE    = 16 */
  17,         /* CDDB_ROCK      = 17 */
  24,         /* CDDB_SOUNDTRACK= 24 */
};
#endif

GtkWidget *ripexename_entry,*ripcmdline_entry,*mp3exename_entry,
  *mp3cmdline_entry,*split_chars_entry;
GtkWidget *start_sector_entry,*end_sector_entry;
GtkWidget *multi_artist_button,*use_proxy_button,*rip_partial_button;

CFGEntry cfg_entries[]={
  {"ripexename",CFG_ENTRY_STRING,256,ripexename},
  {"ripcmdline",CFG_ENTRY_STRING,256,ripcmdline},
  {"wav_filter_cmd",CFG_ENTRY_STRING,256,wav_filter_cmd},
  {"mp3exename",CFG_ENTRY_STRING,256,mp3exename},
  {"mp3cmdline",CFG_ENTRY_STRING,256,mp3cmdline},
  {"dbserver",CFG_ENTRY_STRING,256,dbserver.name},
  {"ripfileformat",CFG_ENTRY_STRING,256,ripfileformat},
  {"mp3fileformat",CFG_ENTRY_STRING,256,mp3fileformat},
  {"m3ufileformat",CFG_ENTRY_STRING,256,m3ufileformat},
  {"delete_wavs",CFG_ENTRY_BOOL,0,&delete_wavs},
  {"add_m3u",CFG_ENTRY_BOOL,0,&add_m3u},
  {"rel_m3u",CFG_ENTRY_BOOL,0,&rel_m3u},
  {"add_to_db",CFG_ENTRY_BOOL,0,&add_to_db},
  {"outputdir",CFG_ENTRY_STRING,256,outputdir},
  {"use_proxy",CFG_ENTRY_BOOL,0,&use_proxy},
  {"proxy_name",CFG_ENTRY_STRING,256,proxy_server.name},
  {"proxy_port",CFG_ENTRY_INT,0,&(proxy_server.port)},
  {"cdupdate",CFG_ENTRY_STRING,256,cdupdate},
  {"user_email",CFG_ENTRY_STRING,256,user_email},
  {"ripnice",CFG_ENTRY_INT,0,&ripnice},
  {"mp3nice",CFG_ENTRY_INT,0,&mp3nice},
  {"doid3",CFG_ENTRY_BOOL,0,&doid3},
  {"max_wavs",CFG_ENTRY_INT,0,&max_wavs},
  {"auto_rip",CFG_ENTRY_BOOL,0,&auto_rip},
  {"eject_after_rip",CFG_ENTRY_BOOL,0,&eject_after_rip},
  {"eject_delay",CFG_ENTRY_INT,0,&eject_delay},
  {"beep_after_rip",CFG_ENTRY_BOOL,0,&beep_after_rip},
  {"faulty_eject",CFG_ENTRY_BOOL,0,&faulty_eject},
  {"use_proxy_env",CFG_ENTRY_BOOL,0,&use_proxy_env},
  {"db_cgi",CFG_ENTRY_STRING,256,dbserver.cgi_prog},
  {"cddb_submit_email",CFG_ENTRY_STRING,256,cddb_submit_email},
  {"dbserver2",CFG_ENTRY_STRING,256,dbserver2.name},
  {"db2_cgi",CFG_ENTRY_STRING,256,dbserver2.cgi_prog},
  {"no_interrupt",CFG_ENTRY_BOOL,0,&no_interrupt},
  {"stop_first",CFG_ENTRY_BOOL,0,&stop_first},
  {"play_first",CFG_ENTRY_BOOL,0,&play_first},
  {"automatic_cddb",CFG_ENTRY_BOOL,0,&automatic_cddb},
  {"automatic_reshuffle",CFG_ENTRY_BOOL,0,&automatic_reshuffle},
  {"no_lower_case",CFG_ENTRY_BOOL,0,&no_lower_case},
  {"no_underscore",CFG_ENTRY_BOOL,0,&no_underscore},
  {"allow_these_chars",CFG_ENTRY_STRING,256,allow_these_chars},
  {"keep_min_size",CFG_ENTRY_BOOL,0,&keep_min_size},
  {"num_cpu",CFG_ENTRY_INT,0,&edit_num_cpu},
  {"kbits_per_sec",CFG_ENTRY_INT,0,&kbits_per_sec},
  {"selected_encoder",CFG_ENTRY_INT,0,&selected_encoder},
  {"selected_ripper",CFG_ENTRY_INT,0,&selected_ripper},
  {"disable_paranoia",CFG_ENTRY_BOOL,0,&disable_paranoia},
  {"disable_extra_paranoia",CFG_ENTRY_BOOL,0,&disable_extra_paranoia},
  {"disable_scratch_detect",CFG_ENTRY_BOOL,0,&disable_scratch_detect},
  {"disable_scratch_repair",CFG_ENTRY_BOOL,0,&disable_scratch_repair},
  {"play_mode",CFG_ENTRY_INT,0,&play_mode},
  {"playloop",CFG_ENTRY_BOOL,0,&playloop},
  {"volume",CFG_ENTRY_INT,0,&volume},
  {"",CFG_ENTRY_LAST,0,NULL}
};

#ifdef SOLARIS
void find_cdrom()
{
  if (access("/vol/dev/aliases", X_OK) == 0) {
    /* Volume manager.  Device might not be there. */
    cddevice="/vol/dev/aliases/cdrom0";
  } else if (access("/dev/rdsk/c0t6d0s2", F_OK) == 0) {
    /* Solaris 2.x w/o volume manager. */
    cddevice="/dev/rdsk/c0t6d0s2";
  } else if (access("/dev/rsr0", F_OK) == 0) {
    cddevice="/dev/rsr0";
  } else if (access("/dev/cdrom", F_OK) == 0) {
    cddevice="/dev/cdrom";
  } else {
    fprintf(stderr, "Couldn't find a CD device!\n");
    exit(1);
  }
}
#endif


void ShutDownCB(void)
{
#ifndef GRIPCD
  if(ripping|encoding)
    BoolDialog("Work is in progress.\nReally shut down?","Yes",
	       GTK_SIGNAL_FUNC(ShutDown),"No",NULL);
  else ShutDown();
#else
  ShutDown();
#endif
}

void ShutDown(void)
{
#ifndef GRIPCD
  if(ripping||encoding) KillChild();
#endif

  gtk_main_quit();
}

gboolean FileExists(char *filename)
{
  struct stat mystat;

  return (stat(filename,&mystat)>=0);
}

gboolean IsDir(char *path)
{
  struct stat mystat;

  if(stat(path,&mystat)!=0) return FALSE;

  return S_ISDIR(mystat.st_mode);
}

int BytesLeftInFS(char *path)
{
#ifdef SOLARIS
  struct statvfs stat;
#else
  struct statfs stat;
#endif
  int pos;

  if(!IsDir(path)) {
    for(pos=strlen(path);pos&&(path[pos]!='/');pos--);
    
    if(path[pos]!='/') return 0;
    
    path[pos]='\0';
#ifdef SOLARIS
    if(statvfs(path,&stat)!=0) return 0;
#else
    if(statfs(path,&stat)!=0) return 0;
#endif
    path[pos]='/';
  }
  else
#ifdef SOLARIS
    if(statvfs(path,&stat)!=0) return 0;
#else
    if(statfs(path,&stat)!=0) return 0;
#endif

  return (stat.f_bavail*stat.f_bsize);
}

void UpdateGTK(void)
{
  while(gtk_events_pending())
    gtk_main_iteration();
}

#if (GTK_MINOR_VERSION != 0)

/* See if the window is going off of the screen, and if so, move it */

void CheckWindowPosition(void)
{
  int x,y,width,height,newx,newy;
  int x_pos,y_pos;
  int scr_width,scr_height;
  gboolean doit=FALSE;

  gdk_window_get_origin(window->window,&x,&y);
  gdk_window_get_size(window->window,&width,&height);
  scr_width=gdk_screen_width();
  scr_height=gdk_screen_height();

  newx=x; newy=y;

  if((x+width)>scr_width) {
    newx=scr_width-width;
    doit=TRUE;
  }

  if((y+height)>scr_height) {
    newy=scr_height-height;
    doit=TRUE;
  }

  if(doit) {
    gdk_window_get_root_origin(window->window,&x_pos,&y_pos);

    gdk_window_move(window->window,newx-(x-x_pos),newy-(y-y_pos));
  }
}

#endif

void Debug(char *fmt,...)
{
  va_list args;

  if(do_debug) {
    va_start(args,fmt);

    vprintf(fmt,args);
  }

  va_end(args);
}

void Busy(void)
{
  gdk_window_set_cursor(window->window,wait_cursor);

  UpdateGTK();
}

void UnBusy(void)
{
  gdk_window_set_cursor(window->window,NULL);

  UpdateGTK();
}

void MinMax(void)
{
  if(minimized) {
    gtk_container_border_width(GTK_CONTAINER(winbox),3);
    gtk_widget_show(notebook);

#ifndef GRIPCD
    CopyPixmap(GTK_PIXMAP(lcd_smile_indicator),
	       GTK_PIXMAP(smile_indicator));
    CopyPixmap(GTK_PIXMAP(empty_image),
	       GTK_PIXMAP(lcd_smile_indicator));
#endif
  }
  else {
    gtk_window_set_policy(GTK_WINDOW(window),FALSE,TRUE,TRUE);
    gtk_container_border_width(GTK_CONTAINER(winbox),0);
    gtk_widget_hide(notebook);

#ifndef GRIPCD
    CopyPixmap(GTK_PIXMAP(smile_indicator),
	       GTK_PIXMAP(lcd_smile_indicator));
#endif

    UpdateGTK();
    gtk_window_set_policy(GTK_WINDOW(window),FALSE,TRUE,FALSE);
  }

  minimized=!minimized;
}

void ToggleControlButtons(void)
{
  if(control_buttons_visible) {
    gtk_window_set_policy(GTK_WINDOW(window),FALSE,TRUE,TRUE);
    gtk_widget_hide(control_button_box);

    UpdateGTK();
    gtk_window_set_policy(GTK_WINDOW(window),FALSE,TRUE,FALSE);
  }
  else {
    gtk_widget_show(control_button_box);
  }

  control_buttons_visible=!control_buttons_visible;
}

void ToggleVol(void)
{
  if(volvis) {
    gtk_window_set_policy(GTK_WINDOW(window),FALSE,TRUE,
			  minimized||keep_min_size);
    gtk_widget_hide(volume_control);
    UpdateGTK();
    gtk_window_set_policy(GTK_WINDOW(window),FALSE,TRUE,FALSE);
  }
  else {
    gtk_widget_show(volume_control);
  }

  volvis=!volvis;
}

void ToggleProg(void)
{
  if(track_prog_visible) {
    gtk_window_set_policy(GTK_WINDOW(window),FALSE,TRUE,
			  minimized||keep_min_size);
    gtk_widget_hide(playopts);
    UpdateGTK();
    gtk_window_set_policy(GTK_WINDOW(window),FALSE,TRUE,FALSE);
  }
  else {
    gtk_widget_show(playopts);
  }

  track_prog_visible=!track_prog_visible;
}

void PlaylistChanged(void)
{
  strcpy(ddata.data_playlist,gtk_entry_get_text(GTK_ENTRY(playlist_entry)));

  InitProgram(info.disc_totaltracks,play_mode);

  if(CDDBWriteDiscData(cd_desc,&ddata,NULL,TRUE)<0)
    DisplayMsg("Error saving disc data\n");
}

void UpdateMultiArtist(void)
{
  if(!ddata.data_multi_artist) {
    gtk_window_set_policy(GTK_WINDOW(window),FALSE,TRUE,TRUE);
    gtk_widget_hide(multi_artist_box);
    UpdateGTK();
    gtk_window_set_policy(GTK_WINDOW(window),FALSE,TRUE,FALSE);
  }
  else {
    gtk_widget_show(multi_artist_box);
  }
}

void ToggleTrackEdit(void)
{
  if(track_edit_visible) {
    gtk_window_set_policy(GTK_WINDOW(window),FALSE,TRUE,
			  minimized||keep_min_size);
    gtk_widget_hide(track_edit_box);
    UpdateGTK();
    gtk_window_set_policy(GTK_WINDOW(window),FALSE,TRUE,FALSE);
  }
  else {
    gtk_widget_show(track_edit_box);
  }

  track_edit_visible=!track_edit_visible;
}

void SetTitle(char *title)
{
  gtk_entry_set_text(GTK_ENTRY(title_edit_entry),title);
  gtk_entry_set_position(GTK_ENTRY(title_edit_entry),0);
}

void SetArtist(char *artist)
{
  gtk_entry_set_text(GTK_ENTRY(artist_edit_entry),artist);
  gtk_entry_set_position(GTK_ENTRY(artist_edit_entry),0);
}

void SetCDDBGenre(int genre)
{
  GtkWidget *item;

  item=GTK_WIDGET(g_list_nth(cddb_genre_item_list,genre)->data);
  gtk_list_select_child(GTK_LIST(GTK_COMBO(cddb_genre_combo)->list),
			item);  
}

void SetYear(int year)
{
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(year_spin_button),(gfloat)year);
}

void SaveDiscInfo(void)
{
  if(have_disc) {
    if(CDDBWriteDiscData(cd_desc,&ddata,NULL,TRUE)<0)
      DisplayMsg("Error saving disc data\n");
  }
  else BoolDialog("No disc present","OK",NULL,NULL,NULL);
}

void TitleEditChanged(void)
{
  strcpy(ddata.data_title,gtk_entry_get_text(GTK_ENTRY(title_edit_entry)));
  gtk_label_set(GTK_LABEL(disc_name_label),ddata.data_title);
}

void ArtistEditChanged(void)
{
  strcpy(ddata.data_artist,gtk_entry_get_text(GTK_ENTRY(artist_edit_entry)));
  gtk_label_set(GTK_LABEL(disc_artist_label),ddata.data_artist);
}

void CDDBGenreChanged(void)
{
  ddata.data_genre=
    CDDBGenreValue(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(cddb_genre_combo)->
						entry)));

#ifndef GRIPCD
  if(id3_genre_combo) UpdateID3Genre();
#endif
}

void YearEditChanged(void)
{
  ddata.data_year=
    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(year_spin_button));
}

void TrackEditChanged(void)
{
  char newname[1024];

  strcpy(ddata.data_track[CURRENT_TRACK].track_name,
	 gtk_entry_get_text(GTK_ENTRY(track_edit_entry)));

  strcpy(ddata.data_track[CURRENT_TRACK].track_artist,
	 gtk_entry_get_text(GTK_ENTRY(track_artist_edit_entry)));

  if(*ddata.data_track[CURRENT_TRACK].track_artist)
    g_snprintf(newname,1024,"%02d  %s (%s)",CURRENT_TRACK+1,
	     ddata.data_track[CURRENT_TRACK].track_name,
	     ddata.data_track[CURRENT_TRACK].track_artist);
 else
    g_snprintf(newname,1024,"%02d  %s",CURRENT_TRACK+1,
	     ddata.data_track[CURRENT_TRACK].track_name);

  gtk_clist_set_text(GTK_CLIST(trackclist),CURRENT_TRACK,0,newname);
}

void EditNextTrack(void)
{
  NextTrack();
  /*  gtk_editable_select_region(GTK_EDITABLE(track_edit_entry),0,-1);*/
  gtk_widget_grab_focus(GTK_WIDGET(track_edit_entry));
}

gint TimeOut(gpointer data)
{
  if(ffwding) FastFwd();
  if(rewinding) Rewind();

#ifdef GRIPCD
  if(!have_disc)
    CheckNewDisc();

  if(auto_eject_countdown && !(--auto_eject_countdown))
    EjectDisc();

  UpdateDisplay();
#else
  if(ripping|encoding) UpdateRipProgress();

  if(!ripping) {
    if(!have_disc)
      CheckNewDisc();
    
    UpdateDisplay();
  }
#endif

  return TRUE;
}

char *MungeString(char *str)
{
#ifndef GRIPCD
  char *src,*dst;

  for(src=dst=str;*src;src++) {
    if((*src==' ')) {
      if(no_underscore) *dst++=' ';
      else *dst++='_';
    }
    else if(!isalnum(*src)&&!strchr(allow_these_chars,*src)) continue;
    else {
      if(no_lower_case) *dst++=*src;
      else *dst++=tolower(*src);
    }
  }

  *dst='\0';
#endif
  return str;
}

void SetVolume(GtkWidget *widget,gpointer data)
{
  struct disc_volume vol;

  volume=vol.vol_front.left=vol.vol_front.right=
    vol.vol_back.left=vol.vol_back.right=GTK_ADJUSTMENT(widget)->value;

  CDSetVolume(cd_desc,&vol);
}

void NextTrack(void)
{
#ifndef GRIPCD
  if(ripping) {
    DisplayMsg("Cannot switch tracks while ripping");
    return;
  }
#endif

  CDStat(cd_desc,&info,FALSE);

  if(current_track_index<(prog_totaltracks-1)) {
    gtk_clist_select_row(GTK_CLIST(trackclist),
			 NEXT_TRACK,0);
  }
  else {
    if(playloop)
      gtk_clist_select_row(GTK_CLIST(trackclist),
			   tracks_prog[0],0);
    else stopped=TRUE;
  }
}

void PrevTrack(void)
{
#ifndef GRIPCD
  if(ripping) {
    DisplayMsg("Cannot switch tracks while ripping");
    return;
  }
#endif

  CDStat(cd_desc,&info,FALSE);

  if((info.disc_mode==CDAUDIO_PLAYING) &&
     ((info.disc_frame - info.track[info.disc_track-1].track_start) > 100))
    PlayTrack(CURRENT_TRACK);
  else {
    if(current_track_index)
      gtk_clist_select_row(GTK_CLIST(trackclist),
			   PREV_TRACK,0);
    else {
      if(playloop)
	gtk_clist_select_row(GTK_CLIST(trackclist),
			     tracks_prog[prog_totaltracks-1]
			     ,0);
    }
  }
}

void InitProgram(int num_tracks,int mode)
{
  int track;
  char *tok;

  if((mode==PM_PLAYLIST)) {
    tok=gtk_entry_get_text(GTK_ENTRY(playlist_entry));
    if(!tok||!*tok) mode=PM_NORMAL;
  }

  if(mode==PM_PLAYLIST) {
    prog_totaltracks=0;

    tok=strtok(gtk_entry_get_text(GTK_ENTRY(playlist_entry)),",");

    while(tok) {
      tracks_prog[prog_totaltracks++]=atoi(tok)-1;

      tok=strtok(NULL,",");
    }
  }
  else {
    prog_totaltracks=num_tracks;
    
    for(track=0;track<num_tracks;track++) {
      tracks_prog[track]=track;
    }
    
    if(mode==PM_SHUFFLE)
      
      /* Shuffle the tracks around a bit */

      ShuffleTracks(num_tracks);
  }
}

void ShuffleTracks(int num_tracks)
{
  int t1,t2,tmp,shuffle;

  for(shuffle=0;shuffle<(num_tracks*10);shuffle++) {
    t1=RRand(num_tracks);
    t2=RRand(num_tracks);
    
    tmp=tracks_prog[t1];
    tracks_prog[t1]=tracks_prog[t2];
    tracks_prog[t2]=tmp;
  }
}

void ToggleLoop(void)
{
  playloop=!playloop;

  if(playloop) 
    CopyPixmap(GTK_PIXMAP(loop_pic),GTK_PIXMAP(loop_indicator));
  else
    CopyPixmap(GTK_PIXMAP(noloop_pic),GTK_PIXMAP(loop_indicator));

}

void ChangePlayMode(void)
{
  play_mode=(play_mode+1)%PM_LASTMODE;
  CopyPixmap(GTK_PIXMAP(play_pix[play_mode]),GTK_PIXMAP(play_indicator));

  gtk_widget_set_sensitive(GTK_WIDGET(playlist_entry),play_mode==PM_PLAYLIST);

  InitProgram(info.disc_totaltracks,play_mode);
}

void FastFwdCB(GtkWidget *widget,gpointer data)
{
#ifndef GRIPCD
  if(ripping) {
    DisplayMsg("Cannot fast forward while ripping");
    return;
  }
#endif

  ffwding=(gboolean)data;

  if(ffwding) FastFwd();
}

void FastFwd(void)
{
  struct disc_timeval tv;

  tv.minutes=0;
  tv.seconds=5;

  if((info.disc_mode==CDAUDIO_PLAYING)||(info.disc_mode==CDAUDIO_PAUSED)) {
    CDAdvance(cd_desc,&info,&tv);
  }
}

void RewindCB(GtkWidget *widget,gpointer data)
{
#ifndef GRIPCD
  if(ripping) {
    DisplayMsg("Cannot rewind while ripping");
    return;
  }
#endif

  rewinding=(gboolean)data;

  if(rewinding) Rewind();
}

void Rewind(void)
{
  struct disc_timeval tv;

  tv.minutes=0;
  tv.seconds=-5;

  if((info.disc_mode==CDAUDIO_PLAYING)||(info.disc_mode==CDAUDIO_PAUSED)) {
    CDAdvance(cd_desc,&info,&tv);
  }
}

void NextDisc(void)
{
#ifndef GRIPCD
  if(ripping) {
    DisplayMsg("Cannot switch discs while ripping");
    return;
  }
#endif

  if(changer_slots>1) {
    current_disc=(current_disc+1)%changer_slots;
    CDChangerSelectDisc(cd_desc,current_disc);
    have_disc=FALSE;
  }
}

void StopPlay(void)
{
#ifndef GRIPCD
  if(ripping) return;
#endif

  CDStop(cd_desc);
  CDStat(cd_desc,&info,FALSE);
  stopped=TRUE;

  if(stop_first)
    gtk_clist_select_row(GTK_CLIST(trackclist),0,0);
}

void EjectDisc(void)
{
  Debug("Eject disc\n");

#ifndef GRIPCD
  if(ripping) {
    DisplayMsg("Cannot eject while ripping");
    return;
  }
#endif

  if(auto_eject_countdown) return;

  Busy();

  if(have_disc) {
    Debug("Have disc -- ejecting\n");
    CDStop(cd_desc);
    CDEject(cd_desc);
    playing=0;
    have_disc=FALSE;
    update_required=TRUE;
    current_discid=0;
    tray_open=TRUE;
  }
  else {
    if(faulty_eject) {
      if(tray_open) CDClose(cd_desc);
      else CDEject(cd_desc);
    }
    else {
      if(TrayOpen(cd_desc)!=0) CDClose(cd_desc);
      else CDEject(cd_desc);
    }

    tray_open=!tray_open;
  }

  UnBusy();
}

void PlayTrackCB(GtkWidget *widget,gpointer data)
{
  int track;

#ifndef GRIPCD
  if(ripping) {
    DisplayMsg("Cannot play while ripping");
    return;
  }
#endif

  CDStat(cd_desc,&info,FALSE);

  if(play_mode!=PM_NORMAL&&!((info.disc_mode==CDAUDIO_PLAYING)||
			     info.disc_mode==CDAUDIO_PAUSED)) {
    if(play_mode == PM_SHUFFLE && automatic_reshuffle)
      ShuffleTracks(info.disc_totaltracks);
    current_track_index=0;
    gtk_clist_select_row(GTK_CLIST(trackclist),CURRENT_TRACK,0);
  }

  track=CURRENT_TRACK;

  if(track==(info.disc_track-1)) {
    switch(info.disc_mode) {
    case CDAUDIO_PLAYING:
      CDPause(cd_desc);
      return;
      break;
    case CDAUDIO_PAUSED:
      CDResume(cd_desc);
      return;
      break;
    default:
      PlayTrack(track);
      break;
    }
  }
  else PlayTrack(track);
}

void PlayTrack(int track)
{
  Busy();
  
  if(play_mode==PM_NORMAL)
    CDPlayTrack(cd_desc,&info,track+1,info.disc_totaltracks);
  else CDPlayTrack(cd_desc,&info,track+1,track+1);

  UnBusy();

  playing=TRUE;
}

void ChangeTimeMode(void)
{
  time_display_mode=(time_display_mode+1)%4;
  UpdateDisplay();
}

void CheckNewDisc(void)
{
  int new_id;

  if(!looking_up) {
    Debug("Checking for a new disc\n");

    CDStat(cd_desc,&info,FALSE);

    if(info.disc_present) {
      CDStat(cd_desc,&info,TRUE);

      Debug("CDStat found a disc, checking tracks\n");
      
      if(CheckTracks()) {
	Debug("We have a valid disc!\n");
	
	new_id=CDDBDiscid(cd_desc);
	InitProgram(info.disc_totaltracks,play_mode);
        if(play_first)
          if(info.disc_mode == CDAUDIO_COMPLETED ||
	     info.disc_mode == CDAUDIO_NOSTATUS) {
            gtk_clist_select_row(GTK_CLIST(trackclist),0,0);
            info.disc_track = 1;
          }
	
	if(new_id) {
	  have_disc=TRUE;
	  LookupDisc(FALSE);
	}
      }
      else Debug("No non-zero length tracks\n");
    }
    else Debug("CDStat said no disc\n");
  }
}

/* Check to make sure we didn't get a false alarm from the cdrom device */

gboolean CheckTracks(void)
{
  int track;
  gboolean have_track=FALSE;

  for(track=0;track<info.disc_totaltracks;track++)
    if(info.track[track].track_length.minutes||
       info.track[track].track_length.seconds) have_track=TRUE;

  return have_track;
}

void UpdateDisplay(void)
{
  static int play_counter=0;
  static int cddb_counter=0;
  char buf[80]="";
  char icon_buf[80];
  static int frames;
  static int secs;
  static int mins;
  int totsecs;

  if(!looking_up) {
    if(cddb_counter%2)
      cddb_counter++;
  }
  else
    CopyPixmap(GTK_PIXMAP(cddb_pix[cddb_counter++%2]),
	       GTK_PIXMAP(cddb_indicator));

  if(!update_required) {
    if(have_disc) {
      CDStat(cd_desc,&info,FALSE);

      if((info.disc_mode==CDAUDIO_PLAYING)||
	 (info.disc_mode==CDAUDIO_PAUSED)) {
	if(info.disc_mode==CDAUDIO_PAUSED) {
	  if((play_counter++%2)==0) {
	    strcpy(buf,"");
	  }
	  else {
	    g_snprintf(buf,80,"%02d:%02d",mins,secs);
	  }
	}
	else {
	  if((info.disc_track-1)!=CURRENT_TRACK) {
	    gtk_clist_select_row(GTK_CLIST(trackclist),info.disc_track-1,0);
	  }

	  frames=info.disc_frame-info.track[info.disc_track-1].track_start;

	  switch(time_display_mode) {
	  case TIME_MODE_TRACK:
	    mins=info.track_time.minutes;
	    secs=info.track_time.seconds;
	    break;
	  case TIME_MODE_DISC:
	    mins=info.disc_time.minutes;
	    secs=info.disc_time.seconds;
	    break;
	  case TIME_MODE_LEFT_TRACK:
	    secs=(info.track_time.minutes*60)+info.track_time.seconds;
	    totsecs=(info.track[CURRENT_TRACK].track_length.minutes*60)+
	      info.track[CURRENT_TRACK].track_length.seconds;
	    
	    totsecs-=secs;
	    
	    mins=totsecs/60;
	    secs=totsecs%60;
	    break;
	  case TIME_MODE_LEFT_DISC:
	    secs=(info.disc_time.minutes*60)+info.disc_time.seconds;
	    totsecs=(info.disc_length.minutes*60)+info.disc_length.seconds;
	    
	    totsecs-=secs;
	    
	    mins=totsecs/60;
	    secs=totsecs%60;
	    break;
	  }
	  
#ifndef GRIPCD
	  g_snprintf(buf,80,"Current sector: %6d",frames);
	  gtk_label_set(GTK_LABEL(play_sector_label),buf);
#endif
          if (time_display_mode == TIME_MODE_LEFT_TRACK ||
              time_display_mode == TIME_MODE_LEFT_DISC)
            g_snprintf(buf,80,"-%02d:%02d",mins,secs);
          else
	    g_snprintf(buf,80,"%02d:%02d",mins,secs);
	}
      }
      else {
	if(playing&&((info.disc_mode==CDAUDIO_COMPLETED)||
	   ((info.disc_mode==CDAUDIO_NOSTATUS)&&!stopped))) {
	  NextTrack();
	  strcpy(buf,"00:00");
	  if(!stopped) PlayTrack(CURRENT_TRACK);
	}
	else if(stopped) {
	  CDStop(cd_desc);
#ifndef GRIPCD
	  frames=secs=mins=0;
	  g_snprintf(buf,80,"Current sector: %6d",frames);
	  gtk_label_set(GTK_LABEL(play_sector_label),buf);
#endif
	  
	  strcpy(buf,"00:00");
	  
	  stopped=FALSE;
	  playing=FALSE;
	}
	else return;
      }
      
      gtk_label_set(GTK_LABEL(play_time_label),buf);
      g_snprintf(icon_buf, sizeof(icon_buf), "%02d %s %s",
	       info.disc_track, buf, PROGRAM);
      gdk_window_set_icon_name(window->window, icon_buf);
    }
  }

  if(update_required) {
    UpdateTracks();

    if(have_disc) {
      g_snprintf(buf,80,"%02d:%02d",info.disc_length.minutes,
	       info.disc_length.seconds);
      g_snprintf(icon_buf, sizeof(icon_buf), "%02d %s %s",
	       info.disc_track, buf, PROGRAM);
	       
      gtk_label_set(GTK_LABEL(play_time_label),buf);
      
      if(!looking_up) {
	CopyPixmap(GTK_PIXMAP(empty_image),GTK_PIXMAP(cddb_indicator));

#ifndef GRIPCD
	if(auto_rip&&is_new_disc) {
	  ClickColumn(NULL,2,NULL);
	  DoRip(NULL,(gpointer)1);
	}

	is_new_disc=FALSE;
#endif
      }
      
      if(!no_interrupt)
        gtk_clist_select_row(GTK_CLIST(trackclist),0,0);
      else
        gtk_clist_select_row(GTK_CLIST(trackclist),info.disc_track-1,0);
    }
    else {
      gtk_label_set(GTK_LABEL(play_time_label),"--:--");
      strncpy(icon_buf, PROGRAM, sizeof(icon_buf));
      
      SetCurrentTrack(-1);
    }

    gdk_window_set_icon_name(window->window, icon_buf);
  }
}

void setCurrentTrackIndex(int track)
{
  /* Looks up the track of index track in the program */
  for(current_track_index = 0;
      (current_track_index < MAX_TRACKS)
	&& (current_track_index < prog_totaltracks)
	&& (CURRENT_TRACK != track);
      current_track_index++)
    continue;
}

void SetCurrentTrack(int track)
{
  char buf[256];
#ifndef GRIPCD
  int tracklen;
#endif

  if(track<0) {
    gtk_label_set(GTK_LABEL(current_track_label),"--");
#ifndef GRIPCD
    gtk_entry_set_text(GTK_ENTRY(start_sector_entry),"0");
    gtk_entry_set_text(GTK_ENTRY(end_sector_entry),"0");
#endif
  }
  else {
    gtk_signal_handler_block_by_func(GTK_OBJECT(track_edit_entry),
				     TrackEditChanged,NULL);
    gtk_entry_set_text(GTK_ENTRY(track_edit_entry),
		       ddata.data_track[track].track_name);

    gtk_signal_handler_unblock_by_func(GTK_OBJECT(track_edit_entry),
				       TrackEditChanged,NULL);

    gtk_signal_handler_block_by_func(GTK_OBJECT(track_artist_edit_entry),
				     TrackEditChanged,NULL);

    gtk_entry_set_text(GTK_ENTRY(track_artist_edit_entry),
		       ddata.data_track[track].track_artist);

    gtk_signal_handler_unblock_by_func(GTK_OBJECT(track_artist_edit_entry),
				       TrackEditChanged,NULL);
    g_snprintf(buf,80,"%02d",track+1);
    gtk_label_set(GTK_LABEL(current_track_label),buf);
	
#ifndef GRIPCD
    gtk_entry_set_text(GTK_ENTRY(start_sector_entry),"0");
	
    tracklen=(info.track[track+1].track_start-1)-info.track[track].track_start;
    g_snprintf(buf,80,"%d",tracklen);
    gtk_entry_set_text(GTK_ENTRY(end_sector_entry),buf);
#endif

    setCurrentTrackIndex(track);
  }
}

#ifndef GRIPCD

gboolean TrackIsChecked(int track)
{
  GdkPixmap *pm=NULL;
  GdkBitmap *bm=NULL;

  gtk_clist_get_pixmap(GTK_CLIST(trackclist),track,2,&pm,&bm);
  return((pm==GTK_PIXMAP(check_image)->pixmap));
}

void ToggleChecked(int track)
{
  SetChecked(track,!TrackIsChecked(track));
}

void SetChecked(int track,gboolean checked)
{
  if(!checked)
    gtk_clist_set_pixmap(GTK_CLIST(trackclist),track,2,
			 GTK_PIXMAP(empty_image)->pixmap,
			 GTK_PIXMAP(empty_image)->mask);
  else {
    gtk_clist_set_pixmap(GTK_CLIST(trackclist),track,2,
			 GTK_PIXMAP(check_image)->pixmap,
			 GTK_PIXMAP(check_image)->mask);
  }
}

void ClickColumn(GtkWidget *widget,gint column,gpointer data)
{
  int track;
  int numsel=0;

  for(track=0;track<info.disc_totaltracks;track++)
    if(TrackIsChecked(track)) numsel++;

  for(track=0;track<info.disc_totaltracks;track++)
    SetChecked(track,(numsel<info.disc_totaltracks/2));
}
#endif

void CListButtonPressed(GtkWidget *widget,GdkEventButton *event,gpointer data)
{
  gint row,col;

  if(event&&event->button!=1) {
    gtk_clist_get_selection_info(GTK_CLIST(trackclist),event->x,event->y,
				 &row,&col);

#ifndef GRIPCD
    ToggleChecked(row);
#endif
  }
}

void UnSelectRow(GtkWidget *widget,gint row,gint column,
		 GdkEventButton *event,gpointer data)
{
#ifndef GRIPCD
  if(TrackIsChecked(row))
    gtk_clist_set_pixmap(GTK_CLIST(trackclist),row,2,
			 GTK_PIXMAP(check_image)->pixmap,
			 GTK_PIXMAP(check_image)->mask);
#endif
}

void SelectRow(GtkWidget *widget,gint row,gint column,
	       GdkEventButton *event,gpointer data)
{
  SetCurrentTrack(row);

#ifndef GRIPCD
  if(TrackIsChecked(row))
    gtk_clist_set_pixmap(GTK_CLIST(trackclist),row,2,
			 GTK_PIXMAP(check_image)->pixmap,
			 GTK_PIXMAP(check_image)->mask);
#endif

  if(gtk_clist_row_is_visible(GTK_CLIST(trackclist),row)!=GTK_VISIBILITY_FULL)
    gtk_clist_moveto(GTK_CLIST(trackclist),row,0,0,0);

  if((info.disc_mode==CDAUDIO_PLAYING)&&(info.disc_track!=(row+1)))
    PlayTrack(row);
  else {
    if(event) {
      switch(event->type) {
      case GDK_2BUTTON_PRESS:
	PlayTrack(row);
	break;
      default:
	break;
      }
    }
  }
}

pthread_t cddb_thread;

void CDDBToggle(void)
{
#ifdef SOLARIS
  void    *status;
#endif
  if(looking_up) {
#ifdef SOLARIS
  pthread_exit(&status);
#elif defined(__FreeBSD__)
  pthread_kill(cddb_thread, 0);
#else
    pthread_kill_other_threads_np();
#endif
    Debug("Aborted\n");
    looking_up=FALSE;
    update_required=TRUE;
  }
  else {
#ifndef GRIPCD
    if(ripping) {
      DisplayMsg("Cannot do lookup while ripping");
      return;
    }
#endif
    if(have_disc) LookupDisc(TRUE);
  }
}

void LookupDisc(gboolean manual)
{
  int track;
  gboolean present;

  ddata.data_multi_artist=FALSE;
  ddata.data_year=0;

  present=CDDBStatDiscData(cd_desc);

  if(!manual&&present) {
    CDDBReadDiscData(cd_desc,&ddata);
    update_required=TRUE;
  }
  else {
    if(!manual) {
      ddata.data_id=CDDBDiscid(cd_desc);
      ddata.data_genre=7; /* "misc" */
      strcpy(ddata.data_title,"Unknown Disc");
      strcpy(ddata.data_artist,"");
      
      for(track=0;track<info.disc_totaltracks;track++) {
	sprintf(ddata.data_track[track].track_name,"Track %d",track+1);
	*(ddata.data_track[track].track_artist)='\0';
	*(ddata.data_track[track].track_extended)='\0';
	*(ddata.data_playlist)='\0';
      }

      *ddata.data_extended='\0';
      
      update_required=TRUE;
    }

    if(!local_mode && (manual?TRUE:automatic_cddb)) {
      looking_up=TRUE;
      
      pthread_create(&cddb_thread,NULL,(void *)&DoLookup,NULL);
    }
  }
}

void DoLookup(void)
{
  int cddb_found = 0;

  if(!CDDBLookupDisc(&dbserver)) {
    if(*(dbserver2.name)) {
      if(CDDBLookupDisc(&dbserver2)) {
        cddb_found = 1;
        ask_submit=TRUE;
      }
    }
  }
  else {
    cddb_found = 1;
  }

  looking_up=FALSE;
  pthread_exit(0);
}

gboolean CDDBLookupDisc(CDDBServer *server)
{
  CDDBHello hello;
  CDDBQuery query;
  CDDBEntry entry;
  gboolean success=FALSE;

  if(server->use_proxy)
    Debug("Querying %s (through %s) for disc %02x.\n",server->name,
	   server->proxy->name,
	   CDDBDiscid(cd_desc));
  else
    Debug("Querying %s for disc %02x.\n",server->name,
	   CDDBDiscid(cd_desc));

  strncpy(hello.hello_program,PROGRAM,256);
  strncpy(hello.hello_version,VERSION,256);
	
  if(!CDDBDoQuery(cd_desc,server,&hello,&query)) {
    DisplayMsg("Query failed\n");
    update_required=TRUE;
  } else {
    switch(query.query_match) {
    case MATCH_INEXACT:
    case MATCH_EXACT:
      Debug("Match for \"%s / %s\"\nDownloading data...\n",
	     query.query_list[0].list_artist,
	     query.query_list[0].list_title);
      entry.entry_genre = query.query_list[0].list_genre;
      entry.entry_id = query.query_list[0].list_id;
      CDDBRead(cd_desc,server,&hello,&entry,&ddata);
		
      Debug("Done\n");
      success=TRUE;
		
      if(CDDBWriteDiscData(cd_desc,&ddata,NULL,TRUE)<0)
	printf("Error saving disc data\n");
		
      update_required=TRUE;
      is_new_disc=TRUE;
      break;
    case MATCH_NOMATCH:
      Debug("No match\n");
      break;
    }
  }

  return success;
}

void SubmitEntryCB(void)
{
  int len;

  if(!have_disc) {
    BoolDialog("Cannot submit\nNo disc is present","OK",NULL,NULL,NULL);

    return;
  }

  if(!ddata.data_genre) {
    BoolDialog("CDDB requires a genre other than 'unknown'","OK",
	       NULL,NULL,NULL);

    return;
  }

  if(!*ddata.data_title) {
    BoolDialog("You must enter a disc title","OK",
	       NULL,NULL,NULL);

    return;
  }

  if(!*ddata.data_artist) {
    BoolDialog("You must enter a disc artist","OK",
	       NULL,NULL,NULL);

    return;
  }



  len=strlen(cddb_submit_email);

  if(!strncasecmp(cddb_submit_email+(len-9),".cddb.com",9))
    BoolDialog("You are about to submit this disc information\n"
	       "to a commercial CDDB server, which will then\n"
	       "own the data that you submit. These servers make\n"
	       "a profit out of your effort. We suggest that you\n"
	       "support free CDDB servers instead.\n\nContinue?",
	       "Yes",SubmitEntry,"No",NULL);
  else
    BoolDialog("You are about to submit this\ndisc information via email.\n\n"
	       "Continue?",
	       "Yes",SubmitEntry,"No",NULL);
}

void SubmitEntry(void)
{
  int fd;
  FILE *efp;
  char mailcmd[256];
  char filename[256];

  sprintf (filename, "/tmp/grip.XXXXXX");
  fd = mkstemp (filename);
  if (fd == -1) {
    printf("Error: Unable to create temporary file\n\n");
    return;
  }

  efp=fdopen(fd,"w");

  if(!efp) {
    close(fd);
    DisplayMsg("Error: Unable to create temporary file");
  }
  else {
    fprintf(efp , "To: %s\nFrom: %s\nSubject: cddb %s %02x\n\n", 	\
    	cddb_submit_email,user_email,CDDBGenre(ddata.data_genre), 	\
	ddata.data_id);
    if(CDDBWriteDiscData(cd_desc,&ddata,efp,FALSE)<0) {
      DisplayMsg("Error: Unable to write disc data");
      fclose(efp);
    }
    else {
      fclose(efp);
      close (fd);

      g_snprintf(mailcmd,256,"%s < %s",MAILER,filename);

      Debug("Mailing entry to %s\n",cddb_submit_email);

      system(mailcmd);

      remove(filename);
    }
  }
}

#ifndef GRIPCD

void CheckDupNames(void)
{
  int track,track2;
  int numdups[MAX_TRACKS];
  int count;
  char buf[256];

  for(track=0;track<info.disc_totaltracks;track++)
    numdups[track]=0;
  
  for(track=0;track<(info.disc_totaltracks-1);track++) {
    if(!numdups[track]) {
      count=0;

      for(track2=track+1;track2<info.disc_totaltracks;track2++) {
	if(!strcmp(ddata.data_track[track].track_name,
		   ddata.data_track[track2].track_name))
	  numdups[track2]=++count;
      }
    }
  }

  for(track=0;track<info.disc_totaltracks;track++) {
    if(numdups[track]) {
      g_snprintf(buf,260,"%s (%d)",ddata.data_track[track].track_name,
	       numdups[track]+1);

      strcpy(ddata.data_track[track].track_name,buf);
    }
  }
}

#endif /* ifndef GRIPCD */

void UpdateTracks(void)
{
  int track;
  char buf[1024];
  char *col_strings[3];

  if(have_disc) {
    auto_eject_countdown=0;  /* Reset to make sure we don't eject twice */

    current_discid=CDDBDiscid(cd_desc);

    SetTitle(ddata.data_title);
    SetArtist(ddata.data_artist);
    SetCDDBGenre(ddata.data_genre);
    SetYear(ddata.data_year);
#ifdef GTK_HAVE_FEATURES_1_1_13
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(multi_artist_button),
				 ddata.data_multi_artist);
#else
    gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(multi_artist_button),
				ddata.data_multi_artist);
#endif
    UpdateMultiArtist();

#ifndef GRIPCD
    UpdateID3Genre();
#endif

    if(*cdupdate) {
      ParseFileFmt(cdupdate,buf,1,0,0);
      Debug("CD update program is [%s]\n",buf);
      system(buf);
    }
  }
  else {
    SetTitle("No Disc");
    SetArtist("");
    SetCDDBGenre(0);
  }

  gtk_entry_set_text(GTK_ENTRY(playlist_entry),ddata.data_playlist);

  if(!first_time)
      gtk_clist_clear(GTK_CLIST(trackclist));
  else {
    setCurrentTrackIndex(info.disc_track - 1);
  }

  if(have_disc) {
#ifndef GRIPCD
    CheckDupNames();
#endif

    /* Block the select row callback so we don't mess up the current track
       while reconstructing the clist */

    gtk_signal_handler_block_by_func(GTK_OBJECT(trackclist),
				     GTK_SIGNAL_FUNC(SelectRow),NULL);

    col_strings[0]=(char *)malloc(260);
    col_strings[1]=(char *)malloc(6);
    col_strings[2]=NULL;

    for(track=0;track<info.disc_totaltracks;track++) {
      if(*ddata.data_track[track].track_artist)
	g_snprintf(col_strings[0],260,"%02d  %s (%s)",track+1,
		 ddata.data_track[track].track_name,
		 ddata.data_track[track].track_artist);
      else
	g_snprintf(col_strings[0],260,"%02d  %s",track+1,
		 ddata.data_track[track].track_name);

      g_snprintf(col_strings[1],6,"%2d:%02d",
	       info.track[track].track_length.minutes,
	       info.track[track].track_length.seconds);

      gtk_clist_append(GTK_CLIST(trackclist),col_strings);
    }

    free(col_strings[0]);
    free(col_strings[1]);

    gtk_clist_select_row(GTK_CLIST(trackclist),CURRENT_TRACK,0);

    gtk_signal_handler_unblock_by_func(GTK_OBJECT(trackclist),
				       GTK_SIGNAL_FUNC(SelectRow),NULL);

  }

  if(ask_submit) {
    BoolDialog("This disc has been found on your secondary server,\n"
	       "but not on your primary server.\n\n"
	       "Do you wish to submit this disc information?",
	       "Yes",SubmitEntry,"No",NULL);

    ask_submit=FALSE;
  }

  first_time=0;
  update_required=FALSE;
}

void UseProxyChanged(void)
{
  dbserver.use_proxy=dbserver2.use_proxy=use_proxy;
}

GtkWidget *MakeNewPage(char *name)
{
  GtkWidget *page;
  GtkWidget *label;

  page=gtk_frame_new(NULL);
  gtk_widget_show(page);

  label=gtk_label_new(name);
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),page,label);

  return page;
}

void MakeTrackPage(void)
{
  GtkWidget *vbox;
#ifdef GTK_HAVE_FEATURES_1_1_4
  GtkWidget *scroll;
#endif

  trackpage=MakeNewPage("Tracks");

  vbox=gtk_vbox_new(FALSE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  disc_name_label=gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(vbox),disc_name_label,FALSE,FALSE,0);
  gtk_widget_show(disc_name_label);

  disc_artist_label=gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(vbox),disc_artist_label,FALSE,FALSE,0);
  gtk_widget_show(disc_artist_label);

#ifndef GRIPCD
  trackclist=gtk_clist_new_with_titles(3,titles);
#else
  trackclist=gtk_clist_new_with_titles(2,titles);
#endif

  gtk_clist_set_column_justification(GTK_CLIST(trackclist),1,
				     GTK_JUSTIFY_RIGHT);

  gtk_clist_set_selection_mode(GTK_CLIST(trackclist),GTK_SELECTION_BROWSE);
  gtk_clist_column_title_passive(GTK_CLIST(trackclist),0);
  gtk_clist_column_title_passive(GTK_CLIST(trackclist),1);
#ifndef GRIPCD
  gtk_clist_column_title_active(GTK_CLIST(trackclist),2);
  gtk_clist_set_column_width(GTK_CLIST(trackclist),0,WINWIDTH-120);
  gtk_clist_set_column_width(GTK_CLIST(trackclist),1,40);
  gtk_clist_set_column_justification(GTK_CLIST(trackclist),2,
				     GTK_JUSTIFY_CENTER);
#else
  gtk_clist_set_column_width(GTK_CLIST(trackclist),0,WINWIDTH-90);
#endif

  gtk_signal_connect(GTK_OBJECT(trackclist),"select_row",
		     GTK_SIGNAL_FUNC(SelectRow),
		     NULL);
  gtk_signal_connect(GTK_OBJECT(trackclist),"unselect_row",
		     GTK_SIGNAL_FUNC(UnSelectRow),
		     (gpointer)1);
  
  gtk_signal_connect(GTK_OBJECT(trackclist),"button_press_event",
		     GTK_SIGNAL_FUNC(CListButtonPressed),NULL);

#ifndef GRIPCD
  gtk_signal_connect(GTK_OBJECT(trackclist),"click_column",
		     GTK_SIGNAL_FUNC(ClickColumn),NULL);
#endif

#ifndef GTK_HAVE_FEATURES_1_1_4
  gtk_clist_set_policy(GTK_CLIST(trackclist),GTK_POLICY_AUTOMATIC,
		       GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start(GTK_BOX(vbox),trackclist,TRUE,TRUE,0);
#else
  scroll=gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				 GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(scroll),trackclist);
  gtk_box_pack_start(GTK_BOX(vbox),scroll,TRUE,TRUE,0);
  gtk_widget_show(scroll);
#endif

  gtk_widget_show(trackclist);

  gtk_container_add(GTK_CONTAINER(trackpage),vbox);
  gtk_widget_show(vbox);
}

void ChopCR(char *str)
{
  int len;

  len=strlen(str);

  if(str[len-1]=='\n') str[len-1]='\0';
}

void DoLoadConfig(void)
{
  char filename[256];
  char *proxy_env,*tok;

  sprintf(filename,"%s/.grip",getenv("HOME"));

  if(!LoadConfig(filename,"GRIP",1,1,cfg_entries)) {
    sprintf(filename,"%s/grip.cfg",AUXDIR);

    LoadConfig(filename,"GRIP",1,1,cfg_entries);
    DoSaveConfig();
  }

#ifndef GRIPCD
  /* Phase out 'outputdir' variable */

  if(*outputdir) {
    strcpy(filename,outputdir);
    MakePath(filename);
    strcat(filename,mp3fileformat);
    strcpy(mp3fileformat,filename);

    strcpy(filename,outputdir);
    MakePath(filename);
    strcat(filename,ripfileformat);
    strcpy(ripfileformat,filename);

    *outputdir='\0';
  }
#endif

  dbserver2.use_proxy=dbserver.use_proxy=use_proxy;
  dbserver2.proxy=dbserver.proxy;

  num_cpu=edit_num_cpu;

  if(!*user_email)
#ifdef SOLARIS
    g_snprintf(user_email,256,"%s@%s",getenv("USER"),getenv("HOST"));
#else
    g_snprintf(user_email,256,"%s@%s",getenv("USER"),getenv("HOSTNAME"));
#endif

  if(use_proxy_env) {                /* Get proxy info from "http_proxy" */
    proxy_env=getenv("http_proxy");
    if(proxy_env) {
      
      /* Skip the "http://" if it's present */
      
      if(!strncasecmp(proxy_env,"http://",7)) proxy_env+=7;
      
      tok=strtok(proxy_env,":");
      if(tok) strncpy(proxy_server.name,tok,256);
      
      tok=strtok(NULL,"/");
      if(tok) proxy_server.port=atoi(tok);
      
      Debug("server is %s, port %d\n",proxy_server.name,
	    proxy_server.port);
    }
  }
}

void DoSaveConfig(void)
{
  char filename[256];

  if(edit_num_cpu>MAX_NUM_CPU) edit_num_cpu=MAX_NUM_CPU;

  g_snprintf(filename,256,"%s/.grip",getenv("HOME"));

  if(!SaveConfig(filename,"GRIP",1,cfg_entries))
    DisplayMsg("Error: Unable to save config file");
}

char *FindExe(char *exename,char **paths)
{
  char **path;
  char buf[256];

  path=paths;

  while(*path) {
    g_snprintf(buf,256,"%s/%s",*path,exename);

    if(FileExists(buf)) return *path;

    path++;
  }

  return NULL;
}

void MakeConfigPage(void)
{
  GtkWidget *vbox,*vbox2,*dbvbox;
  GtkWidget *entry;
  GtkWidget *label;
  GtkWidget *page,*page2;
  GtkWidget *check;
  GtkWidget *notebook;
#ifndef GRIPCD
  GtkWidget *hsep;
  GtkWidget *hbox;
  GtkWidget *menu,*optmenu;
  GtkWidget *item;
  MP3Encoder *enc;
  Ripper *rip;
#endif
  configpage=MakeNewPage("Config");

  vbox2=gtk_vbox_new(FALSE,2);
  gtk_container_border_width(GTK_CONTAINER(vbox2),3);

  config_notebook=gtk_notebook_new();

  page=gtk_frame_new(NULL);
  vbox=gtk_vbox_new(TRUE,0);

  check=MakeCheckButton(NULL,&no_interrupt,
			"Don't interrupt playback on exit/startup");
  gtk_box_pack_start(GTK_BOX(vbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);
 
  check=MakeCheckButton(NULL,&stop_first,"Rewind when stopped");
  gtk_box_pack_start(GTK_BOX(vbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&play_first,
			"Startup with first track if not playing");
  gtk_box_pack_start(GTK_BOX(vbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&automatic_reshuffle,
			"Reshuffle before each playback");
  gtk_box_pack_start(GTK_BOX(vbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&faulty_eject,"Work around faulty eject");
  gtk_box_pack_start(GTK_BOX(vbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  gtk_container_add(GTK_CONTAINER(page),vbox);
  gtk_widget_show(vbox);

  label=gtk_label_new("CD");
  gtk_notebook_append_page(GTK_NOTEBOOK(config_notebook),page,label);
  gtk_widget_show(page);

#ifndef GRIPCD
  page=gtk_frame_new(NULL);

  notebook=gtk_notebook_new();

  page2=gtk_frame_new(NULL);

  vbox=gtk_vbox_new(FALSE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  hbox=gtk_hbox_new(FALSE,3);

  label=gtk_label_new("Ripper:");
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
  gtk_widget_show(label);

  menu=gtk_menu_new();

  rip=ripper_defaults;

  while(*(rip->name)) {
    item=gtk_menu_item_new_with_label(rip->name);
    gtk_signal_connect(GTK_OBJECT(item),"activate",
		       GTK_SIGNAL_FUNC(RipperSelected),(gpointer)rip);
    gtk_menu_append(GTK_MENU(menu),item);
    gtk_widget_show(item);

    rip++;
  }

  gtk_menu_set_active(GTK_MENU(menu),selected_ripper);

  optmenu=gtk_option_menu_new();
  gtk_option_menu_set_menu(GTK_OPTION_MENU(optmenu),menu);
  gtk_widget_show(menu);
  gtk_box_pack_start(GTK_BOX(hbox),optmenu,TRUE,TRUE,0);
  gtk_widget_show(optmenu);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  hsep=gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(vbox),hsep,TRUE,TRUE,0);
  gtk_widget_show(hsep);

  rip_exe_box=gtk_vbox_new(TRUE,0);

  entry=MakeStrEntry(&ripexename_entry,ripexename,
		     "Ripping executable",255,TRUE);
  gtk_box_pack_start(GTK_BOX(rip_exe_box),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  entry=MakeStrEntry(&ripcmdline_entry,ripcmdline,"Rip command-line",255,TRUE);
  gtk_box_pack_start(GTK_BOX(rip_exe_box),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  gtk_box_pack_start(GTK_BOX(vbox),rip_exe_box,TRUE,TRUE,0);
  if(!using_builtin_cdp) gtk_widget_show(rip_exe_box);

#ifdef CDPAR
  rip_builtin_box=gtk_vbox_new(TRUE,0);

  check=MakeCheckButton(NULL,&disable_paranoia,"Disable paranoia");
  gtk_box_pack_start(GTK_BOX(rip_builtin_box),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&disable_extra_paranoia,
			"Disable extra paranoia");
  gtk_box_pack_start(GTK_BOX(rip_builtin_box),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  hbox=gtk_hbox_new(FALSE,3);

  label=gtk_label_new("Disable scratch");
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
  gtk_widget_show(label);
  
  check=MakeCheckButton(NULL,&disable_scratch_detect,"detection");
  gtk_box_pack_start(GTK_BOX(hbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&disable_scratch_repair,"repair");
  gtk_box_pack_start(GTK_BOX(hbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  gtk_box_pack_start(GTK_BOX(rip_builtin_box),hbox,TRUE,TRUE,0);
  gtk_widget_show(hbox);


  gtk_box_pack_start(GTK_BOX(vbox),rip_builtin_box,TRUE,TRUE,0);
  if(using_builtin_cdp) gtk_widget_show(rip_builtin_box);
#endif

  entry=MakeStrEntry(NULL,ripfileformat,"Rip file format",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  gtk_container_add(GTK_CONTAINER(page2),vbox);
  gtk_widget_show(vbox);

  label=gtk_label_new("Ripper");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),page2,label);
  gtk_widget_show(page2);

  page2=gtk_frame_new(NULL);

  vbox=gtk_vbox_new(FALSE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  entry=MakeNumEntry(NULL,&ripnice,"Rip 'nice' value",3);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);
  
  entry=MakeNumEntry(NULL,&max_wavs,"Max non-encoded .wav's",3);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  check=MakeCheckButton(NULL,&auto_rip,"Auto-rip on insert");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&beep_after_rip,"Beep after rip");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&eject_after_rip,"Auto-eject after rip");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  entry=MakeNumEntry(NULL,&eject_delay,"Auto-eject delay",3);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  entry=MakeStrEntry(NULL,wav_filter_cmd,"Wav filter command",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);
  
  gtk_container_add(GTK_CONTAINER(page2),vbox);
  gtk_widget_show(vbox);

  label=gtk_label_new("Options");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),page2,label);
  gtk_widget_show(page2);

  gtk_container_add(GTK_CONTAINER(page),notebook);
  gtk_widget_show(notebook);

  label=gtk_label_new("Rip");
  gtk_notebook_append_page(GTK_NOTEBOOK(config_notebook),page,label);
  gtk_widget_show(page);

  page=gtk_frame_new(NULL);

  notebook=gtk_notebook_new();

  page2=gtk_frame_new(NULL);

  vbox=gtk_vbox_new(FALSE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  hbox=gtk_hbox_new(FALSE,3);

  label=gtk_label_new("Encoder:");
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
  gtk_widget_show(label);

  menu=gtk_menu_new();

  enc=encoder_defaults;

  while(*(enc->name)) {
    item=gtk_menu_item_new_with_label(enc->name);
    gtk_signal_connect(GTK_OBJECT(item),"activate",
		       GTK_SIGNAL_FUNC(EncoderSelected),(gpointer)enc);
    gtk_menu_append(GTK_MENU(menu),item);
    gtk_widget_show(item);

    enc++;
  }

  gtk_menu_set_active(GTK_MENU(menu),selected_encoder);

  optmenu=gtk_option_menu_new();
  gtk_option_menu_set_menu(GTK_OPTION_MENU(optmenu),menu);
  gtk_widget_show(menu);
  gtk_box_pack_start(GTK_BOX(hbox),optmenu,TRUE,TRUE,0);
  gtk_widget_show(optmenu);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  hsep=gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(vbox),hsep,TRUE,TRUE,0);
  gtk_widget_show(hsep);

  entry=MakeStrEntry(&mp3exename_entry,mp3exename,"MP3 executable",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  entry=MakeStrEntry(&mp3cmdline_entry,mp3cmdline,"MP3 command-line",
		     255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  entry=MakeStrEntry(NULL,mp3fileformat,"MP3 file format",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  gtk_container_add(GTK_CONTAINER(page2),vbox);
  gtk_widget_show(vbox);

  label=gtk_label_new("Encoder");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),page2,label);
  gtk_widget_show(page2);

  page2=gtk_frame_new(NULL);

  vbox=gtk_vbox_new(FALSE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  check=MakeCheckButton(NULL,&delete_wavs,"Delete .wav after encoding");
  gtk_box_pack_start(GTK_BOX(vbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

#ifdef MP3DB
  check=MakeCheckButton(NULL,&add_to_db,"Insert info into SQL database");
  gtk_box_pack_start(GTK_BOX(vbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);
#endif

  check=MakeCheckButton(NULL,&add_m3u,"Create .m3u files");
  gtk_box_pack_start(GTK_BOX(vbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&rel_m3u,"Use relative paths in .m3u files");
  gtk_box_pack_start(GTK_BOX(vbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  entry=MakeStrEntry(NULL,m3ufileformat,"M3U file format",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  entry=MakeNumEntry(NULL,&kbits_per_sec,"Encoding bitrate (kbits/sec)",3);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);
  
  entry=MakeNumEntry(NULL,&edit_num_cpu,"Number of CPUs to use",3);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);
  
  entry=MakeNumEntry(NULL,&mp3nice,"Mp3 'nice' value",3);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);
  
  gtk_container_add(GTK_CONTAINER(page2),vbox);
  gtk_widget_show(vbox);

  label=gtk_label_new("Options");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),page2,label);
  gtk_widget_show(page2);

  gtk_container_add(GTK_CONTAINER(page),notebook);
  gtk_widget_show(notebook);

  label=gtk_label_new("MP3");
  gtk_notebook_append_page(GTK_NOTEBOOK(config_notebook),page,label);
  gtk_widget_show(page);

  page=gtk_frame_new(NULL);

  vbox=gtk_vbox_new(TRUE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  check=MakeCheckButton(NULL,&doid3,"Add ID3 tags to MP3 files");
  gtk_box_pack_start(GTK_BOX(vbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  gtk_container_add(GTK_CONTAINER(page),vbox);
  gtk_widget_show(vbox);

  label = gtk_label_new("ID3");
  gtk_notebook_append_page(GTK_NOTEBOOK(config_notebook),page,label);
  gtk_widget_show(page);
#endif

  page=gtk_frame_new(NULL);

  dbvbox=gtk_vbox_new(FALSE,2);
  gtk_container_border_width(GTK_CONTAINER(dbvbox),3);

  notebook=gtk_notebook_new();

  page2=gtk_frame_new(NULL);

  vbox=gtk_vbox_new(TRUE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  entry=MakeStrEntry(NULL,dbserver.name,"DB server",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  entry=MakeStrEntry(NULL,dbserver.cgi_prog,"CGI path",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  gtk_container_add(GTK_CONTAINER(page2),vbox);
  gtk_widget_show(vbox);

  label=gtk_label_new("Primary Server");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),page2,label);
  gtk_widget_show(page2);


  page2=gtk_frame_new(NULL);

  vbox=gtk_vbox_new(TRUE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  entry=MakeStrEntry(NULL,dbserver2.name,"DB server",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  entry=MakeStrEntry(NULL,dbserver2.cgi_prog,"CGI path",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  gtk_container_add(GTK_CONTAINER(page2),vbox);
  gtk_widget_show(vbox);

  label=gtk_label_new("Secondary Server");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),page2,label);
  gtk_widget_show(page2);


  gtk_box_pack_start(GTK_BOX(dbvbox),notebook,TRUE,TRUE,0);
  gtk_widget_show(notebook);


  entry=MakeStrEntry(NULL,cddb_submit_email,"DB Submit email",255,TRUE);
  gtk_box_pack_start(GTK_BOX(dbvbox),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  check=MakeCheckButton(NULL,&automatic_cddb,
			"Perform CDDB lookup automatically");
  gtk_box_pack_start(GTK_BOX(dbvbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  gtk_container_add(GTK_CONTAINER(page),dbvbox);
  gtk_widget_show(dbvbox);


  label=gtk_label_new("CDDB");
  gtk_notebook_append_page(GTK_NOTEBOOK(config_notebook),page,label);
  gtk_widget_show(page);


  page=gtk_frame_new(NULL);

  vbox=gtk_vbox_new(TRUE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  check=MakeCheckButton(&use_proxy_button,&use_proxy,"Use proxy server");
  gtk_signal_connect(GTK_OBJECT(use_proxy_button),"clicked",
		     GTK_SIGNAL_FUNC(UseProxyChanged),NULL);
  gtk_box_pack_start(GTK_BOX(vbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&use_proxy_env,
			"Get server from 'http_proxy' env. var");
  gtk_box_pack_start(GTK_BOX(vbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  entry=MakeStrEntry(NULL,proxy_server.name,"Proxy server",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  entry=MakeNumEntry(NULL,&(proxy_server.port),"Proxy port",5);
  gtk_box_pack_start(GTK_BOX(vbox),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  gtk_container_add(GTK_CONTAINER(page),vbox);
  gtk_widget_show(vbox);

  label=gtk_label_new("Proxy");
  gtk_notebook_append_page(GTK_NOTEBOOK(config_notebook),page,label);
  gtk_widget_show(page);

  page=gtk_frame_new(NULL);

  vbox=gtk_vbox_new(TRUE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  entry=MakeStrEntry(NULL,user_email,"Email address",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  entry=MakeStrEntry(NULL,cdupdate,"CD update program",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  check=MakeCheckButton(NULL,&no_lower_case,
			"Do not lowercase filenames");
  gtk_box_pack_start(GTK_BOX(vbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&no_underscore,
			"Do not change spaces to underscores");
  gtk_box_pack_start(GTK_BOX(vbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  entry=MakeStrEntry(NULL,allow_these_chars,
		     "Characters to not strip in filenames",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,TRUE,TRUE,0);
  gtk_widget_show(entry);

  check=MakeCheckButton(NULL,&keep_min_size,
			"Keep application minimum size");
  gtk_box_pack_start(GTK_BOX(vbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  gtk_container_add(GTK_CONTAINER(page),vbox);
  gtk_widget_show(vbox);

  label=gtk_label_new("Misc");
  gtk_notebook_append_page(GTK_NOTEBOOK(config_notebook),page,label);
  gtk_widget_show(page);

  gtk_box_pack_start(GTK_BOX(vbox2),config_notebook,TRUE,TRUE,0);
  gtk_widget_show(config_notebook);

  gtk_container_add(GTK_CONTAINER(configpage),vbox2);
  gtk_widget_show(vbox2);
}

void Homepage(void)
{
  system("netscape -remote openURL\\(http://www.nostatic.org/grip\\)");
}

void MakeHelpPage(void)
{
  GtkWidget *vbox;
  GtkWidget *button;
  GtkWidget *label;

  helppage=MakeNewPage("Help");

  vbox=gtk_vbox_new(TRUE,2);

  label=gtk_label_new("Helpful information can be found in the\n"
		      "README file that came with the distribution.");
  gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  label=gtk_label_new("If you would like to submit a bug report,\n"
		      "please ensure that you have the latest version\n"
		      "of Grip (see the \"About\" page for a link) and\n"
		      "have carefully read both the README and\n"
		      "TODO files.");
  gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  button=gtk_button_new_with_label("Submit Bug Report");
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
      GTK_SIGNAL_FUNC(BugReport),NULL);
  gtk_box_pack_start(GTK_BOX(vbox),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  gtk_container_add(GTK_CONTAINER(helppage),vbox);
  gtk_widget_show(vbox);
}

void MakeAboutPage(void)
{
  GtkWidget *vbox,*vbox2,*hbox;
  GtkWidget *label;
  GtkWidget *logo;
  GtkWidget *ebox;
  GtkWidget *button;
  char versionbuf[20];

  aboutpage=MakeNewPage("About");

  ebox=gtk_event_box_new();
  gtk_widget_set_style(ebox,style_wb);

  vbox=gtk_vbox_new(TRUE,5);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

#ifndef GRIPCD
  logo=Loadxpm(grip_xpm);
#else
  logo=Loadxpm(gcd_xpm);
#endif

  gtk_box_pack_start(GTK_BOX(vbox),logo,FALSE,FALSE,0);
  gtk_widget_show(logo);

  vbox2=gtk_vbox_new(TRUE,0);

  sprintf(versionbuf,"Version %s",VERSION);
  label=gtk_label_new(versionbuf);
  gtk_widget_set_style(label,style_wb);
  gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  label=gtk_label_new("Copyright (c) 1998-1999, Mike Oliphant");
  gtk_widget_set_style(label,style_wb);
  gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,FALSE,0);
  gtk_widget_show(label);

#ifdef SOLARIS
  label=gtk_label_new("Solaris Port, David Meleedy");
  gtk_widget_set_style(label,style_wb);
  gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,FALSE,0);
  gtk_widget_show(label);
#endif


  hbox=gtk_hbox_new(TRUE,0);

  button=gtk_button_new_with_label("http://www.nostatic.org/grip");
  gtk_widget_set_style(button,style_dark_grey);
  gtk_widget_set_style(GTK_BUTTON(button)->child,style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(Homepage),NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);
  

  gtk_container_add(GTK_CONTAINER(vbox),vbox2);
  gtk_widget_show(vbox2);

  gtk_container_add(GTK_CONTAINER(ebox),vbox);
  gtk_widget_show(vbox);

  gtk_container_add(GTK_CONTAINER(aboutpage),ebox);
  gtk_widget_show(ebox);
}

GtkTooltips *MakeToolTip(void)
{
  GtkTooltips *tip;

  tip=gtk_tooltips_new();

  gtk_tooltips_set_delay(tip,1250);

  return tip;
}

GtkWidget *MakeEditBox(void)
{
  GtkWidget *vbox,*hbox;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *frame;
  GtkWidget *item;
  GtkWidget *check;
  GtkWidget *entry;
  GtkObject *adj;
#ifndef GRIPCD
  gchar **id3_genre;
  gint id3_genre_count;
#endif
  int genre;
  int len;

  frame=gtk_frame_new(NULL);

  vbox=gtk_vbox_new(FALSE,0);

  hbox=gtk_hbox_new(FALSE,3);

  label=gtk_label_new("Disc title");
  len=gdk_string_width(label->style->font,"CDDB genre")+5;
  gtk_widget_set_usize(label,len,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  title_edit_entry=gtk_entry_new_with_max_length(256);
  gtk_signal_connect(GTK_OBJECT(title_edit_entry),"changed",
      GTK_SIGNAL_FUNC(TitleEditChanged),NULL);
  gtk_entry_set_position(GTK_ENTRY(title_edit_entry),0);
  gtk_box_pack_start(GTK_BOX(hbox),title_edit_entry,TRUE,TRUE,0);
  gtk_widget_show(title_edit_entry);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  hbox=gtk_hbox_new(FALSE,3);

  label=gtk_label_new("Disc artist");
  gtk_widget_set_usize(label,len,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  artist_edit_entry=gtk_entry_new_with_max_length(256);
  gtk_signal_connect(GTK_OBJECT(artist_edit_entry),"changed",
      GTK_SIGNAL_FUNC(ArtistEditChanged),NULL);
  gtk_entry_set_position(GTK_ENTRY(artist_edit_entry),0);
  gtk_box_pack_start(GTK_BOX(hbox),artist_edit_entry,TRUE,TRUE,0);
  gtk_widget_show(artist_edit_entry);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  cddb_genre_combo=gtk_combo_new();
  gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(cddb_genre_combo)->entry),FALSE);

  hbox=gtk_hbox_new(FALSE,3);

  label=gtk_label_new("CDDB genre");
  gtk_widget_set_usize(label,len,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  for(genre=0;genre<12;genre++) {
    item=gtk_list_item_new_with_label(CDDBGenre(genre));
    cddb_genre_item_list=g_list_append(cddb_genre_item_list,item);
    gtk_signal_connect(GTK_OBJECT(item), "select",
		       GTK_SIGNAL_FUNC(CDDBGenreChanged),NULL);
    gtk_container_add(GTK_CONTAINER(GTK_COMBO(cddb_genre_combo)->list),item);
    gtk_widget_show(item);
  }

  gtk_box_pack_start(GTK_BOX(hbox),cddb_genre_combo,TRUE,TRUE,0);
  gtk_widget_show(cddb_genre_combo);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

#ifndef GRIPCD
  hbox=gtk_hbox_new(FALSE,3);

  label=gtk_label_new("ID3 genre:");
  gtk_widget_set_usize(label,len,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  id3_genre_combo=gtk_combo_new();

  for(id3_genre=id3_genres,id3_genre_count=0;*id3_genre;id3_genre++,
	id3_genre_count++) {
    item = gtk_list_item_new_with_label(*id3_genre);
    id3_genre_item_list = g_list_append(id3_genre_item_list, item);
    gtk_signal_connect(GTK_OBJECT(item), "select",
		       GTK_SIGNAL_FUNC(ID3GenreChanged),
		       (gpointer)id3_genre_count);
    gtk_container_add(GTK_CONTAINER(GTK_COMBO(id3_genre_combo)->list), item);
    gtk_widget_show(item);
  }

  gtk_box_pack_start(GTK_BOX(hbox),id3_genre_combo,TRUE,TRUE,0);
  gtk_widget_show(id3_genre_combo);

  item = GTK_WIDGET(g_list_nth(id3_genre_item_list,id3_genre_number)->data);
  gtk_list_select_child (GTK_LIST (GTK_COMBO(id3_genre_combo)->list), item);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);
#endif

  hbox=gtk_hbox_new(FALSE,3);

  label=gtk_label_new("Disc year");
  gtk_widget_set_usize(label,len,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  adj=gtk_adjustment_new(0,0,9999,1.0,5.0,0);

  year_spin_button=gtk_spin_button_new(GTK_ADJUSTMENT(adj),0.5,0);
  gtk_signal_connect(GTK_OBJECT(year_spin_button),"changed",
		     GTK_SIGNAL_FUNC(YearEditChanged),NULL);
  gtk_box_pack_start(GTK_BOX(hbox),year_spin_button,TRUE,TRUE,0);
  gtk_widget_show(year_spin_button);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  hbox=gtk_hbox_new(FALSE,3);

  label=gtk_label_new("Track name");
  gtk_widget_set_usize(label,len,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  track_edit_entry=gtk_entry_new_with_max_length(256);
  gtk_signal_connect(GTK_OBJECT(track_edit_entry),"changed",
		     GTK_SIGNAL_FUNC(TrackEditChanged),NULL);
#ifndef GRIPCD
  gtk_signal_connect(GTK_OBJECT(track_edit_entry),"activate",
		     GTK_SIGNAL_FUNC(CheckDupNames),NULL);
  gtk_signal_connect(GTK_OBJECT(track_edit_entry),"focus_out_event",
		     GTK_SIGNAL_FUNC(CheckDupNames),NULL);
#endif
  gtk_signal_connect(GTK_OBJECT(track_edit_entry),"activate",
		     GTK_SIGNAL_FUNC(EditNextTrack),NULL);
  gtk_box_pack_start(GTK_BOX(hbox),track_edit_entry,TRUE,TRUE,0);
  gtk_widget_show(track_edit_entry);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  multi_artist_box=gtk_vbox_new(FALSE,0);

  hbox=gtk_hbox_new(FALSE,3);

  label=gtk_label_new("Track artist");
  gtk_widget_set_usize(label,len,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  track_artist_edit_entry=gtk_entry_new_with_max_length(256);
  gtk_signal_connect(GTK_OBJECT(track_artist_edit_entry),"changed",
		     GTK_SIGNAL_FUNC(TrackEditChanged),NULL);
  gtk_box_pack_start(GTK_BOX(hbox),track_artist_edit_entry,
		     TRUE,TRUE,0);
  gtk_widget_show(track_artist_edit_entry);

  gtk_box_pack_start(GTK_BOX(multi_artist_box),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  hbox=gtk_hbox_new(FALSE,3);

  label=gtk_label_new("Split:");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  button=gtk_button_new_with_label("Title/Artist");
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(SplitTitleArtist),(gpointer)0);
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  button=gtk_button_new_with_label("Artist/Title");
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(SplitTitleArtist),(gpointer)1);
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  entry=MakeStrEntry(&split_chars_entry,title_split_chars,
		     "Split chars",5,TRUE);
  gtk_widget_set_usize(split_chars_entry,
		       5*gdk_string_width(split_chars_entry->style->font,
					  "W"),0);
  gtk_box_pack_end(GTK_BOX(hbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  gtk_box_pack_start(GTK_BOX(multi_artist_box),hbox,FALSE,FALSE,2);
  gtk_widget_show(hbox);

  gtk_box_pack_start(GTK_BOX(vbox),multi_artist_box,FALSE,FALSE,0);
  gtk_widget_show(multi_artist_box);

  hbox=gtk_hbox_new(FALSE,0);

  check=MakeCheckButton(&multi_artist_button,&(ddata.data_multi_artist),
			"Multi-artist");
  gtk_signal_connect(GTK_OBJECT(multi_artist_button),"clicked",
		     GTK_SIGNAL_FUNC(UpdateMultiArtist),NULL);
  gtk_box_pack_start(GTK_BOX(hbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  button=ImageButton(save_xpm);
  gtk_widget_set_style(button,style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
      GTK_SIGNAL_FUNC(SaveDiscInfo),NULL);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "Save disc info",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  button=ImageButton(mail_xpm);
  gtk_widget_set_style(button,style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
      GTK_SIGNAL_FUNC(SubmitEntryCB),NULL);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "Submit disc info",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);


  gtk_container_add(GTK_CONTAINER(frame),vbox);
  gtk_widget_show(vbox);

  return frame;
}

GtkWidget *MakePlayOpts(void)
{
  GtkWidget *ebox;
  GtkWidget *hbox;
  GtkWidget *button;

  play_pix[0]=Loadxpm(playnorm_xpm);
  play_pix[1]=Loadxpm(random_xpm);
  play_pix[2]=Loadxpm(playlist_xpm);

  ebox=gtk_event_box_new();
  gtk_widget_set_style(ebox,style_wb);

  hbox=gtk_hbox_new(FALSE,2);

  playlist_entry=gtk_entry_new_with_max_length(256);
  gtk_signal_connect(GTK_OBJECT(playlist_entry),"activate",
		     GTK_SIGNAL_FUNC(PlaylistChanged),NULL);
  gtk_signal_connect(GTK_OBJECT(playlist_entry),"focus_out_event",
		     GTK_SIGNAL_FUNC(PlaylistChanged),NULL);
  gtk_widget_set_sensitive(GTK_WIDGET(playlist_entry),play_mode==PM_PLAYLIST);
  gtk_box_pack_start(GTK_BOX(hbox),playlist_entry,TRUE,TRUE,0);
  gtk_widget_show(playlist_entry);

  play_indicator=NewBlankPixmap();
  CopyPixmap(GTK_PIXMAP(play_pix[play_mode]),GTK_PIXMAP(play_indicator));

  button=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(button),play_indicator);
  gtk_widget_show(play_indicator);
  gtk_widget_set_style(button,style_dark_grey);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(ChangePlayMode),0);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "Rotate play mode",NULL);
  gtk_widget_show(button);

  loop_pic=Loadxpm(loop_xpm);
  noloop_pic=Loadxpm(noloop_xpm);

  loop_indicator=NewBlankPixmap();

  if(playloop)
    CopyPixmap(GTK_PIXMAP(loop_pic),GTK_PIXMAP(loop_indicator));
  else
    CopyPixmap(GTK_PIXMAP(noloop_pic),GTK_PIXMAP(loop_indicator));

  button=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(button),loop_indicator);
  gtk_widget_show(loop_indicator);
  gtk_widget_set_style(button,style_dark_grey);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(ToggleLoop),0);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "Toggle loop play",NULL);
  gtk_widget_show(button);

  gtk_container_add(GTK_CONTAINER(ebox),hbox);
  gtk_widget_show(hbox);

  return ebox;
}

GtkWidget *MakeControls(void)
{
  GtkWidget *vbox,*vbox3,*hbox,*imagebox,*hbox2;
  GtkWidget *indicator_box;
  GtkWidget *button;
  GtkWidget *ebox,*lcdbox;
  GtkWidget *image;
  GtkObject *adj;
  /*  struct disc_volume vol;*/
#ifndef GRIPCD
  int mycpu;
#endif

  ebox=gtk_event_box_new();
  gtk_widget_set_style(ebox,style_wb);

  vbox=gtk_vbox_new(FALSE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox),0);

  vbox3=gtk_vbox_new(FALSE,2);
  gtk_container_border_width(GTK_CONTAINER(vbox3),2);

  lcdbox=gtk_event_box_new();
  gtk_signal_connect(GTK_OBJECT(lcdbox),"button_press_event",
		     GTK_SIGNAL_FUNC(ToggleControlButtons),NULL);
  gtk_widget_set_style(lcdbox,style_LCD);

  hbox2=gtk_hbox_new(FALSE,0);

  imagebox=gtk_vbox_new(FALSE,0);

  image=Loadxpm(upleft_xpm);
  gtk_box_pack_start(GTK_BOX(imagebox),image,FALSE,FALSE,0);
  gtk_widget_show(image);

  image=Loadxpm(lowleft_xpm);
  gtk_box_pack_end(GTK_BOX(imagebox),image,FALSE,FALSE,0);
  gtk_widget_show(image);

  gtk_box_pack_start(GTK_BOX(hbox2),imagebox,FALSE,FALSE,0);
  gtk_widget_show(imagebox);
  
  hbox=gtk_hbox_new(TRUE,0);
  gtk_container_border_width(GTK_CONTAINER(hbox),0);

  current_track_label=gtk_label_new("--");
  gtk_box_pack_start(GTK_BOX(hbox),current_track_label,FALSE,FALSE,0);
  gtk_widget_show(current_track_label);

  button=gtk_button_new();
  gtk_widget_set_style(button,style_LCD);
#if (GTK_MINOR_VERSION != 0)
  gtk_button_set_relief(GTK_BUTTON(button),GTK_RELIEF_NONE);
#endif
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(ChangeTimeMode),NULL);

  play_time_label=gtk_label_new("--:--");
  gtk_container_add(GTK_CONTAINER(button),play_time_label);
  gtk_widget_show(play_time_label);

  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_widget_show(button);

#ifndef GRIPCD
  rip_pix[0]=Loadxpm(rip0_xpm);
  rip_pix[1]=Loadxpm(rip1_xpm);
  rip_pix[2]=Loadxpm(rip2_xpm);
  rip_pix[3]=Loadxpm(rip3_xpm);

  mp3_pix[0]=Loadxpm(enc0_xpm);
  mp3_pix[1]=Loadxpm(enc1_xpm);
  mp3_pix[2]=Loadxpm(enc2_xpm);
  mp3_pix[3]=Loadxpm(enc3_xpm);
#endif

  cddb_pix[0]=Loadxpm(cddb0_xpm);
  cddb_pix[1]=Loadxpm(cddb1_xpm);

  indicator_box=gtk_hbox_new(TRUE,0);

#ifndef GRIPCD
  rip_indicator=NewBlankPixmap();
#if (GTK_MINOR_VERSION != 0)
  gtk_box_pack_start(GTK_BOX(indicator_box),rip_indicator,TRUE,TRUE,0);
  gtk_widget_show(rip_indicator);
#endif

#ifndef GRIPCD
  lcd_smile_indicator=NewBlankPixmap();
  gtk_tooltips_set_tip(MakeToolTip(),lcd_smile_indicator,
		       "Rip status",NULL);
  gtk_box_pack_start(GTK_BOX(indicator_box),lcd_smile_indicator,TRUE,TRUE,0);
  gtk_widget_show(lcd_smile_indicator);
#endif

  for(mycpu=0;mycpu<num_cpu;mycpu++){
    mp3_indicator[mycpu]=NewBlankPixmap();
#if (GTK_MINOR_VERSION != 0)
    gtk_box_pack_start(GTK_BOX(indicator_box),mp3_indicator[mycpu],TRUE,TRUE,0);
#endif
    gtk_widget_show(mp3_indicator[mycpu]);
  }
#endif
  
  cddb_indicator=NewBlankPixmap();
#if (GTK_MINOR_VERSION != 0)
  gtk_box_pack_start(GTK_BOX(indicator_box),cddb_indicator,TRUE,TRUE,0);
  gtk_widget_show(cddb_indicator);
#endif  
  gtk_box_pack_start(GTK_BOX(hbox),indicator_box,TRUE,TRUE,0);
  gtk_widget_show(indicator_box);

  gtk_container_add(GTK_CONTAINER(hbox2),hbox);
  gtk_widget_show(hbox);

  imagebox=gtk_vbox_new(FALSE,0);

  image=Loadxpm(upright_xpm);
  gtk_box_pack_start(GTK_BOX(imagebox),image,FALSE,FALSE,0);
  gtk_widget_show(image);

  image=Loadxpm(lowright_xpm);
  gtk_box_pack_end(GTK_BOX(imagebox),image,FALSE,FALSE,0);
  gtk_widget_show(image);

  gtk_box_pack_start(GTK_BOX(hbox2),imagebox,FALSE,FALSE,0);
  gtk_widget_show(imagebox);
  
  gtk_container_add(GTK_CONTAINER(lcdbox),hbox2);
  gtk_widget_show(hbox2);

  gtk_box_pack_start(GTK_BOX(vbox3),lcdbox,FALSE,FALSE,0);
  gtk_widget_show(lcdbox);

  gtk_box_pack_start(GTK_BOX(vbox),vbox3,FALSE,FALSE,0);
  gtk_widget_show(vbox3);

  adj=gtk_adjustment_new((gfloat)volume,0.0,255.0,1.0,1.0,0.0);
  gtk_signal_connect(adj,"value_changed",
		     GTK_SIGNAL_FUNC(SetVolume),NULL);
  volume_control=gtk_hscale_new(GTK_ADJUSTMENT(adj));

  gtk_scale_set_draw_value(GTK_SCALE(volume_control),FALSE);
  gtk_widget_set_name(volume_control,"darkgrey");
  gtk_box_pack_start(GTK_BOX(vbox),volume_control,FALSE,FALSE,0);

  /*  CDGetVolume(cd_desc,&vol);
  gtk_adjustment_set_value(GTK_ADJUSTMENT(adj),(vol.vol_front.left+
  vol.vol_front.right)/2);*/

  if(volvis) gtk_widget_show(volume_control);

  control_button_box=gtk_vbox_new(TRUE,0);

  hbox=gtk_hbox_new(TRUE,0);

  button=ImageButton(playpaus_xpm);
  gtk_widget_set_style(button,style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(PlayTrackCB),NULL);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "Play track / Pause play",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  button=ImageButton(rew_xpm);
  gtk_widget_set_style(button,style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"pressed",
		     GTK_SIGNAL_FUNC(RewindCB),(gpointer)1);
  gtk_signal_connect(GTK_OBJECT(button),"released",
		     GTK_SIGNAL_FUNC(RewindCB),(gpointer)0);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "Rewind",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  button=ImageButton(ff_xpm);
  gtk_widget_set_style(button,style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"pressed",
		     GTK_SIGNAL_FUNC(FastFwdCB),(gpointer)1);
  gtk_signal_connect(GTK_OBJECT(button),"released",
		     GTK_SIGNAL_FUNC(FastFwdCB),(gpointer)0);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "FastForward",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  button=ImageButton(prevtrk_xpm);
  gtk_widget_set_style(button,style_dark_grey);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(PrevTrack),0);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "Go to previous track",NULL);
  gtk_widget_show(button);

  button=ImageButton(nexttrk_xpm);
  gtk_widget_set_style(button,style_dark_grey);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(NextTrack),0);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "Go to next track",NULL);
  gtk_widget_show(button);

  button=ImageButton(progtrack_xpm);
  gtk_widget_set_style(button,style_dark_grey);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(ToggleProg),0);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "Toggle play mode options",NULL);
  gtk_widget_show(button);

  if(changer_slots>1) {
    button=ImageButton(rotate_xpm);
    gtk_widget_set_style(button,style_dark_grey);
    gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
    gtk_signal_connect(GTK_OBJECT(button),"clicked",
		       GTK_SIGNAL_FUNC(NextDisc),0);
    gtk_tooltips_set_tip(MakeToolTip(),button,
			 "Next Disc",NULL);
    gtk_widget_show(button);
  }

  gtk_box_pack_start(GTK_BOX(control_button_box),hbox,TRUE,TRUE,0);
  gtk_widget_show(hbox);

  hbox=gtk_hbox_new(TRUE,0);

  button=ImageButton(stop_xpm);
  gtk_widget_set_style(button,style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(StopPlay),0);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "Stop play",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  button=ImageButton(eject_xpm);
  gtk_widget_set_style(button,style_dark_grey);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(EjectDisc),0);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "Eject disc",NULL);
  gtk_widget_show(button);

  button=ImageButton(vol_xpm);
  gtk_widget_set_style(button,style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(ToggleVol),0);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "Toggle Volume Control",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  button=ImageButton(edit_xpm);
  gtk_widget_set_style(button,style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(ToggleTrackEdit),0);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "Toggle disc editor",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  if(!local_mode) {
    button=ImageButton(cddbwht_xpm);
    gtk_widget_set_style(button,style_dark_grey);
    gtk_signal_connect(GTK_OBJECT(button),"clicked",
		       GTK_SIGNAL_FUNC(CDDBToggle),0);
    gtk_tooltips_set_tip(MakeToolTip(),button,
			 "Initiate/abort CDDB lookup",NULL);
    gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
    gtk_widget_show(button);
  }

  button=ImageButton(minmax_xpm);
  gtk_widget_set_style(button,style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(MinMax),0);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "Toggle track display",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  button=ImageButton(quit_xpm);
  gtk_widget_set_style(button,style_dark_grey);
#ifndef GRIPCD
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "Exit Grip",NULL);
#else
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       "Exit GCD",NULL);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(ShutDownCB),0);
  gtk_widget_show(button);
  
  gtk_box_pack_start(GTK_BOX(control_button_box),hbox,TRUE,TRUE,0);
  gtk_widget_show(hbox);

  gtk_box_pack_start(GTK_BOX(vbox),control_button_box,TRUE,TRUE,0);
  gtk_widget_show(control_button_box);


  gtk_container_add(GTK_CONTAINER(ebox),vbox);
  gtk_widget_show(vbox);

  return ebox;
}


GtkWidget *ImageButton(char **xpm)
{
  GtkWidget *button;
  GtkWidget *image;

  button=gtk_button_new();

  image=Loadxpm(xpm);
  gtk_container_add(GTK_CONTAINER(button),image);
  gtk_widget_show(image);

  return button;
}

GtkWidget *ImageToggleButton(char **xpm, gint state)
{
  GtkWidget *button;
  GtkWidget *image;

  button=gtk_toggle_button_new();

#if GTK_HAVE_FEATURES_1_1_13
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),state);
#else
  gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button),state);
#endif

  image=Loadxpm(xpm);
  gtk_container_add(GTK_CONTAINER(button),image);
  gtk_widget_show(image);

  return button;
}

GtkWidget *Loadxpm(char **xpm)
{
  GdkBitmap *mask;
  GtkStyle *style;
  GtkWidget *pixmapwid;
  GdkPixmap *pixmap;

  style=gtk_widget_get_style(window);

  pixmap=gdk_pixmap_create_from_xpm_d(window->window,&mask,
				      &style->bg[GTK_STATE_NORMAL],
				      (gchar **)xpm);

  pixmapwid=gtk_pixmap_new(pixmap,mask);

  return pixmapwid;
}

GtkWidget *NewBlankPixmap(void)
{
  GdkPixmap *gdkpix;
  GdkBitmap *mask;

  gtk_pixmap_get(GTK_PIXMAP(empty_image),&gdkpix,&mask);

  return gtk_pixmap_new(gdkpix,mask);
}

void CopyPixmap(GtkPixmap *src,GtkPixmap *dest)
{
  GdkPixmap *gdkpix;
  GdkBitmap *mask;

  gtk_pixmap_get(src,&gdkpix,&mask);
  gtk_pixmap_set(dest,gdkpix,mask);
}

GdkColor *MakeColor(int red,int green,int blue)
{
  GdkColor *c;

  c=(GdkColor *)g_malloc(sizeof(GdkColor));
  c->red=red;
  c->green=green;
  c->blue=blue;

  gdk_color_alloc(gdk_colormap_get_system(),c);

  return c;
}

static gfloat style_color_mods[5]={0.0,-0.1,0.2,-0.2};

GtkStyle *MakeStyle(GdkColor *fg,GdkColor *bg,gboolean do_grade)
{
  GtkStyle *def;
  GtkStyle *sty;
  int state;

  def=gtk_widget_get_default_style();
  sty=gtk_style_copy(def);

  for(state=0;state<5;state++) {
    sty->fg[state]=*fg;

    sty->bg[state]=*bg;

    if(do_grade) {
      sty->bg[state].red+=sty->bg[state].red*style_color_mods[state];
      sty->bg[state].green+=sty->bg[state].green*style_color_mods[state];
      sty->bg[state].blue+=sty->bg[state].blue*style_color_mods[state];
    }
  }

  return sty;
}

void Usage(void)
{
#ifndef GRIPCD
  printf("\nUsage: grip [-d <device>] [-s] [-l]\n\n");
#else
  printf("\nUsage: gcd [-d <device>] [-f] [-l]\n\n");
#endif
  printf("  -d <device>        Use <device> as cd-rom device\n");
#ifndef GRIPCD
  printf("  -s                 Launch in \"small\" (cd-only) mode\n");
#else
  printf("  -f                 Launch in \"full\" (track-display) mode\n");
#endif
  printf("  -l                 'local' mode -- don't try to use CDDB\n");
  printf("  -v                 Verbose (debug) mode\n");
  printf("\n");
  printf("See the README file for more information\n\n");

  exit(-1);
}

void ParseArgs(int numargs,char *args[])
{
  int num=1;
  int pos;
  gboolean skipout;

  while(num<numargs) {
    if(*args[num]!='-') Usage();

    for(pos=1,skipout=FALSE;args[num][pos]&&!skipout;pos++) {
      switch(args[num][pos]) {
      case 'h':
      case 'H':
      case '?':
	Usage();
	break;
      case 'd':
	num++;
	if(num==numargs) Usage();
	else {
	  strncpy(cddevice,args[num],80);
	  skipout=TRUE;
	}
	break;
      case 's':
	minimized=TRUE;
	break;
      case 'f':
	minimized=FALSE;
	break;
      case 'l':
	local_mode=TRUE;
	break;
      case 'v':
	do_debug=TRUE;
	break;
      default:
	printf("Unrecognized argument [-%c]\n",args[num][1]);
	Usage();
	break;
      }
    }
    
    num++;
  }
}

void SeparateFields(char *buf,char *field1,char *field2,char *sep)
{
  char *tmp;
  char spare[80];

  tmp=strtok(buf,sep);

  if(!tmp) return;

  strncpy(spare,ChopWhite(tmp),80);

  tmp=strtok(NULL,"");

  if(tmp) {
    strncpy(field2,ChopWhite(tmp),80);
  }
  else *field2='\0';

  strcpy(field1,spare);
}

void SplitTitleArtist(GtkWidget *widget,gpointer data)
{
  int track;

  for(track=0;track<info.disc_totaltracks;track++) {
    if((int)data==0)
      SeparateFields(ddata.data_track[track].track_name,
		     ddata.data_track[track].track_name,
		     ddata.data_track[track].track_artist,title_split_chars);
    else 
      SeparateFields(ddata.data_track[track].track_name,
		     ddata.data_track[track].track_artist,
		     ddata.data_track[track].track_name,title_split_chars);
  }

  UpdateTracks();
}

void ParseFileFmt(char *instr,char *outstr,int track,int startsec,int endsec)
{
  int left=1024;
  char *tok;
  struct passwd *pwd;
  gboolean do_munge;

  *outstr='\0';

  /* Expand ~ in dir -- modeled loosely after code from gtkfind by
                        Matthew Grossman */

  if(*instr=='~') {
    instr++;

    if((*instr=='\0') || (*instr=='/')) {   /* This user's dir */
      left-=g_snprintf(outstr,left,"%s",getenv("HOME"));
    }
    else {  /* Another user's dir */
      tok=strchr(instr,'/');

      if(tok) {   /* Ugly, but it works */
	*tok='\0';
	pwd=getpwnam(instr);
	instr+=strlen(instr);
	*tok='/';
      }
      else {
	pwd=getpwnam(instr);
	instr+=strlen(instr);
      }

      if(!pwd)
	printf("Error: unable to translate filename. No such user as %s\n",
	       tok);
      else {
	left-=g_snprintf(outstr,left,"%s",pwd->pw_dir);
      }
    }
    
    outstr+=strlen(outstr);
  }

  for(;*instr&&(left>0);instr++) {
    if(*instr=='%') {
      do_munge=TRUE;

      if(*(instr+1)=='*') {
	  do_munge=FALSE;
	  instr++;
      }

      switch(*(++instr)) {
      case '%':
	g_snprintf(outstr,left,"%%");
	break;
      case 't':
	g_snprintf(outstr,left,"%02d",track);
	break;
      case 'b':
	g_snprintf(outstr,left,"%d",startsec);
	break;
      case 'e':
	g_snprintf(outstr,left,"%d",endsec);
	break;
      case 'n':
	if(*ddata.data_track[track-1].track_name)
	  g_snprintf(outstr,left,"%s",ddata.data_track[track-1].track_name);
	else g_snprintf(outstr,left,"Track%d",track);
	break;
      case 'a':
	if(*ddata.data_track[track-1].track_artist)
	  g_snprintf(outstr,left,"%s",ddata.data_track[track-1].track_artist);
	else {
	  if(*ddata.data_artist)
	    g_snprintf(outstr,left,"%s",ddata.data_artist);
	  else strncpy(outstr,"NoArtist",left);
	}
	break;
      case 'A':
	if(*ddata.data_artist)
	  g_snprintf(outstr,left,"%s",ddata.data_artist);
	else strncpy(outstr,"NoArtist",left);	
	break;
      case 'd':
	if(*ddata.data_title)
	  g_snprintf(outstr,left,"%s",ddata.data_title);
	else strncpy(outstr,"NoTitle",left);
	break;
      case 'i':
	g_snprintf(outstr,1024,"%02x",CDDBDiscid(cd_desc));
	break;
#ifndef GRIPCD
      case 'g':
	g_snprintf(outstr,left,"%d",id3_genre_number);
	break;
      case 'G':
	g_snprintf(outstr,left,"%s",id3_genres[id3_genre_number]);
	break;
#endif
      }

      if(do_munge) MungeString(outstr);

      left-=strlen(outstr);
      outstr+=strlen(outstr);
    }
    else {
      if(left>0) {
	*outstr++=*instr;
	left--;
      }
    }
  }

  *outstr='\0';
}

#ifndef GRIPCD

/* Rip-related functions */

void RipPartialChanged(void)
{
  gtk_widget_set_sensitive(partial_rip_box,rip_partial);
}

void ParseEncFileFmt(char *instr,char *outstr,EncodeTrack *enc_track)
{
  int left=1024;
  char *tok;
  struct passwd *pwd;
  gboolean do_munge;

  *outstr='\0';

  /* Expand ~ in dir -- modeled loosely after code from gtkfind by
                        Matthew Grossman */

  if(*instr=='~') {
    instr++;

    if((*instr=='\0') || (*instr=='/')) {   /* This user's dir */
      left-=g_snprintf(outstr,left,"%s",getenv("HOME"));
    }
    else {  /* Another user's dir */
      tok=strchr(instr,'/');

      if(tok) {   /* Ugly, but it works */
	*tok='\0';
	pwd=getpwnam(instr);
	instr+=strlen(instr);
	*tok='/';
      }
      else {
	pwd=getpwnam(instr);
	instr+=strlen(instr);
      }

      if(!pwd)
	printf("Error: unable to translate filename. No such user as %s\n",
	       tok);
      else {
	left-=g_snprintf(outstr,left,"%s",pwd->pw_dir);
      }
    }
    
    outstr+=strlen(outstr);
  }

  for(;*instr&&(left>0);instr++) {
    if(*instr=='%') {
      do_munge=TRUE;

      if(*(instr+1)=='*') {
	  do_munge=FALSE;
	  instr++;
      }

      switch(*(++instr)) {
      case '%':
	g_snprintf(outstr,left,"%%");
	break;
      case 't':
	g_snprintf(outstr,left,"%02d",enc_track->track_num+1);
	break;
      case 'b':
	g_snprintf(outstr,left,"%d",enc_track->start_frame);
	break;
      case 'e':
	g_snprintf(outstr,left,"%d",enc_track->end_frame);
	break;
      case 'n':
	if(*(enc_track->song_name))
	  g_snprintf(outstr,left,"%s",enc_track->song_name);
	else g_snprintf(outstr,left,"Track%d",enc_track->track_num);
	break;
      case 'a':
	if(*(enc_track->song_artist))
	  g_snprintf(outstr,left,"%s",enc_track->song_artist);
	else {
	  if(*(enc_track->disc_artist))
	    g_snprintf(outstr,left,"%s",enc_track->disc_artist);
	  else strncpy(outstr,"NoArtist",left);
	}
	break;
      case 'A':
	if(*(enc_track->disc_artist))
	  g_snprintf(outstr,left,"%s",enc_track->disc_artist);
	else strncpy(outstr,"NoArtist",left);	
	break;
      case 'd':
	if(*(enc_track->disc_name))
	  g_snprintf(outstr,left,"%s",enc_track->disc_name);
	else strncpy(outstr,"NoTitle",left);
	break;
      case 'i':
	g_snprintf(outstr,1024,"%02x",CDDBDiscid(enc_track->discid));
	break;
      case 'y':
	g_snprintf(outstr,left,"%d",enc_track->song_year);
	break;
      case 'g':
	g_snprintf(outstr,left,"%d",enc_track->id3_genre);
	break;
      case 'G':
	g_snprintf(outstr,left,"%s",id3_genres[enc_track->id3_genre]);
	break;
      }

      if(do_munge) MungeString(outstr);

      left-=strlen(outstr);
      outstr+=strlen(outstr);
    }
    else {
      if(left>0) {
	*outstr++=*instr;
	left--;
      }
    }
  }

  *outstr='\0';
}

/* Make file1 relative to file2 */
char *MakeRelative(char *file1,char *file2)
{
  int pos;
  char *rel=file1;

  for(pos=0;file2[pos];pos++) {
    if(pos&&(file2[pos]=='/')) {
      if(!strncmp(file1,file2,pos)) {
	rel=file1+pos+1;
      }
    }
  }

  return rel;
}

void AddM3U(void)
{
  int i;
  EncodeTrack et;
  FILE *fp;
  char m3unam[1024];
  char mp3nam[1024];
  char *relnam;
  
  if(!have_disc) return;
  
  ParseFileFmt(m3ufileformat,m3unam,1,0,0);
  MakeDirs(m3unam);

  fp=fopen(m3unam, "w");
  if(fp==NULL) {
    perror("can't open m3u file");
    return;
  }
  
  for(i=0;i<info.disc_totaltracks;i++) {
    /* Only add to the m3u if the track is selected for ripping */
    if(TrackIsChecked(i)) {
      FillInEncode(i,&et);
      ParseEncFileFmt(mp3fileformat,mp3nam,&et);

      if(rel_m3u) {
	relnam=MakeRelative(mp3nam,m3unam);
	fprintf(fp,"%s\n",relnam);
      }
      else 
	fprintf(fp,"%s\n",mp3nam);
    }
  }

  fclose(fp);
}


#ifdef MP3DB
void DBScan(void)
{
  int track;
  EncodeTrack enc_track;

  if(!have_disc) return;

  for(track=0;track<info.disc_totaltracks;track++) {
    FillInEncode(track,&enc_track);
    AddSQLEntry(&enc_track);
  }
}

void AddSQLEntry(EncodeTrack *enc_track)
{
  int sqlpid;
  char track_str[4];
  char frame_str[11];
  char length_str[11];
  char playtime_str[6];
  char year_str[5];
  char filename[1024];

  ParseEncFileFmt(mp3fileformat,filename,enc_track);

  g_snprintf(track_str,4,"%d",enc_track->track_num);
  g_snprintf(frame_str,11,"%d",enc_track->start_frame);
  g_snprintf(length_str,11,"%d",enc_track->end_frame-enc_track->start_frame);
  g_snprintf(playtime_str,6,"%d:%d",enc_track->mins,enc_track->secs);
  g_snprintf(year_str,5,"%d",enc_track->song_year);

  sqlpid=fork();

  if(sqlpid==0) {
    close(ConnectionNumber(GDK_DISPLAY()));
    RedirectIO(!do_debug);

    if(*enc_track->song_artist)
      execl(INSTALLDIR "/mp3insert","mp3insert",
	    "-p",filename,
	    "-a",enc_track->disc_artist,
	    "-i",enc_track->song_artist,
	    "-t",enc_track->song_name,"-d",enc_track->disc_name,
	    "-g",id3_genres[enc_track->id3_genre],"-y",year_str,
	    "-n",track_str,
	    "-f",frame_str,"-l",length_str,"-m",playtime_str,NULL);
    else
      execl(INSTALLDIR "/mp3insert","mp3insert",
	    "-p",filename,
	    "-a",enc_track->disc_artist,
	    "-t",enc_track->song_name,"-d",enc_track->disc_name,
	    "-g",id3_genres[enc_track->id3_genre],"-y",year_str,
	    "-n",track_str,
	    "-f",frame_str,"-l",length_str,"-m",playtime_str,NULL);
    
    _exit(0);
  }

  waitpid(sqlpid,NULL,0);

}
#endif /* #ifdef MP3DB */

void UpdateRipProgress(void)
{
  struct stat mystat;
  int quarter;
  gfloat percent;
  int mycpu; 
  char buf[1024];

  if(ripping) {
    if(stat(ripfile,&mystat)>=0) {
      percent=(gfloat)mystat.st_size/(gfloat)ripsize;
      if(percent>1.0) percent=1.0;
    }
    else {
      percent=0;
    }
    
    gtk_progress_bar_update(GTK_PROGRESS_BAR(ripprogbar),percent);
	
    quarter=(int)(percent*4.0);
	
    if(quarter<4)
      CopyPixmap(GTK_PIXMAP(rip_pix[quarter]),GTK_PIXMAP(rip_indicator));

    if(using_builtin_cdp) {
      if(minimized)
	CopyPixmap(GTK_PIXMAP(smile_pix[rip_smile_level]),
		   GTK_PIXMAP(lcd_smile_indicator));
      else
	CopyPixmap(GTK_PIXMAP(smile_pix[rip_smile_level]),
		   GTK_PIXMAP(smile_indicator));
    }

    if((using_builtin_cdp&&!in_rip_thread) ||
       (!using_builtin_cdp&&waitpid(rippid,NULL,WNOHANG))) {
      if(!using_builtin_cdp) waitpid(rippid,NULL,0);
      else {
	CopyPixmap(GTK_PIXMAP(empty_image),
		   GTK_PIXMAP(lcd_smile_indicator));
	CopyPixmap(GTK_PIXMAP(empty_image),
		   GTK_PIXMAP(smile_indicator));
      }

      Debug("Rip finished\n");

      ripping=FALSE;
      SetChecked(rip_track,FALSE);

      /* Do filtering of .wav file */

      if(*wav_filter_cmd) {
	ParseWavCmd(wav_filter_cmd,buf,ripfile);
	system(buf);
      }

      gtk_progress_bar_update(GTK_PROGRESS_BAR(ripprogbar),0.0);
      CopyPixmap(GTK_PIXMAP(empty_image),GTK_PIXMAP(rip_indicator));

      if(!stop_rip) {
	if(doencode) {
	  AddToEncode(rip_track);
	  MP3Encode();
	}

	Debug("Rip partial %d  num wavs %d\n",rip_partial,num_wavs);

	Debug("Next track is %d, total is %d\n",
	      NextTrackToRip(),info.disc_totaltracks);

	if(!rip_partial&&(num_wavs<max_wavs||
			  NextTrackToRip()==info.disc_totaltracks)) {
	  Debug("Check if we need to rip another track\n");
	  if(!RipNextTrack()) RipIsFinished();
	}
      }

      if(!ripping) {
	if(!encoding) stop_rip=FALSE;
	gtk_label_set(GTK_LABEL(rip_prog_label),"Rip: Idle");
      }
    }
  }
  
  for(mycpu=0;mycpu<num_cpu;mycpu++){
    if(encoding&(1<<mycpu)) {
      if(stat(mp3file[mycpu],&mystat)>=0) {
        percent=(gfloat)mystat.st_size/(gfloat)mp3size[mycpu];
        if(percent>1.0) percent=1.0;
      }
      else {
        percent=0;
      } 

      gtk_progress_bar_update(GTK_PROGRESS_BAR(mp3progbar[mycpu]),percent);
        
      quarter=(int)(percent*4.0);
 
      if(quarter<4)
        CopyPixmap(GTK_PIXMAP(mp3_pix[quarter]),
		   GTK_PIXMAP(mp3_indicator[mycpu]));
   
      if(waitpid(mp3pid[mycpu],NULL,WNOHANG)) {
        waitpid(mp3pid[mycpu],NULL,0);
        encoding&=~(1<<mycpu);

	Debug("Finished encoding on cpu %d\n",mycpu);

        gtk_progress_bar_update(GTK_PROGRESS_BAR(mp3progbar[mycpu]),0.0);
        CopyPixmap(GTK_PIXMAP(empty_image),GTK_PIXMAP(mp3_indicator[mycpu]));

        if(delete_wavs) {
	  Debug("Deleting [%s]\n",rip_delete_file[mycpu]);
	  unlink(rip_delete_file[mycpu]);
	}

        num_wavs--;

        if(!stop_rip) {
          if(doid3) ID3Add(mp3file[mycpu],encoded_track[mycpu]);

#ifdef MP3DB
	  if(add_to_db) AddSQLEntry(encoded_track[mycpu]);
#endif

          if(ripping_a_disc&&!rip_partial&&!ripping&&num_wavs<max_wavs) {
	    if(RipNextTrack()) doencode=TRUE;
	    else RipIsFinished();
          }

	  g_free(encoded_track[mycpu]);
         
          if(!rip_partial&&encode_list) {
	    MP3Encode();
          }
        }
	else stop_rip=FALSE;

        if(!(encoding&(1<<mycpu)))
	  gtk_label_set(GTK_LABEL(mp3_prog_label[mycpu]),"MP3: Idle");
      }
    }  
  }
}

char *FindRoot(char *str)
{
  char *c;

  for(c=str+strlen(str);c>str;c--) {
    if(*c=='/') return c+1;
  }

  return c;
}

void MakeArgs(char *str,char **args,int maxargs)
{
  int arg;
  gboolean inquotes=FALSE;

  for(arg=0,args[0]=str;*str&&arg<(maxargs-1);str++) {
    if(*str=='"') {
      if(inquotes) *str='\0';
      else args[arg]=str+1;

      inquotes=!inquotes;
    }
    else if(*str==' '&&!inquotes) {
      *str='\0';
      args[++arg]=str+1;
    }
  }

  args[++arg]=NULL;
}

void ParseRipCmd(char *instr,char *outstr,int track,int startsec,int endsec,
		 char *filename)
{
  int left=1024;

  for(;*instr&&(left>0);instr++) {
    if(*instr=='%') {
      switch(*(++instr)) {
      case '%':
	g_snprintf(outstr,left,"%%");
	break;
      case 't':
	g_snprintf(outstr,left,"%d",track);
	break;
      case 'b':
	g_snprintf(outstr,left,"%d",startsec);
	break;
      case 'e':
	g_snprintf(outstr,left,"%d",endsec);
	break;
      case 'f':
	g_snprintf(outstr,left,"\"%s\"",filename);
	break;
      case 'c':
	strncpy(outstr,cddevice,left);
	break;
      }

      left-=strlen(outstr);
      outstr+=strlen(outstr);
    }
    else {
      if(left>0) {
	*outstr++=*instr;
	left--;
      }
    }
  }

  *outstr='\0';
}

void ParseMP3Cmd(char *instr,char *outstr,char *ripfile,char *mp3,int cpu)
{
  int left=1024;

  for(;*instr&&(left>0);instr++) {
    if(*instr=='%') {
      switch(*(++instr)) {
      case '%':
	g_snprintf(outstr,left,"%%");
	break;
      case 'f':
	g_snprintf(outstr,left,"\"%s\"",ripfile);
	break;
      case 'o':
	g_snprintf(outstr,left,"\"%s\"",mp3);
	break;
      case 'b':
	g_snprintf(outstr,left,"%d",kbits_per_sec);
	break;
      case 'c':
	g_snprintf(outstr,left,"%d",cpu);
	break;
      }

      left-=strlen(outstr);
      outstr+=strlen(outstr);
    }
    else {
      if(left>0) {
	*outstr++=*instr;
	left--;
      }
    }
  }

  *outstr='\0';
}

void ParseWavCmd(char *instr,char *outstr,char *ripfile)
{
  int left=1024;

  for(;*instr&&(left>0);instr++) {
    if(*instr=='%') {
      switch(*(++instr)) {
      case '%':
	g_snprintf(outstr,left,"%%");
	break;
      case 'f':
	g_snprintf(outstr,left,"\"%s\"",ripfile);
	break;
      }

      left-=strlen(outstr);
      outstr+=strlen(outstr);
    }
    else {
      if(left>0) {
	*outstr++=*instr;
	left--;
      }
    }
  }

  *outstr='\0';
}

void KillChild(void)
{
  int mycpu;
  GList *elist;
  EncodeTrack *enc_track;
  
  if(!(ripping||encoding)) return;

  stop_rip=TRUE;

  if(ripping) {
    if(using_builtin_cdp) {
      stop_thread_rip_now=TRUE;
    }
    else kill(rippid,SIGKILL);
    doencode=FALSE;
  }
  
  for(mycpu=0;mycpu<num_cpu;mycpu++){
    if(encoding&(1<<mycpu)) kill(mp3pid[mycpu],SIGKILL);
  }

  elist=encode_list;

  while(elist) {
    enc_track=(EncodeTrack *)elist->data;
    elist=elist->next;

    encode_list=g_list_remove(elist,enc_track);
    g_free(enc_track);
  }
}

void RedirectIO(gboolean redirect_output)
{
  int fd;

  fd=open("/dev/null",O_RDWR);
  dup2(fd,0);

  if(redirect_output) {
    dup2(fd,1);
    dup2(fd,2);
  }

  close(fd); 
  close(cd_desc);
}

void ID3Add(char *file,EncodeTrack *enc_track)
{
  char year[5];

  g_snprintf(year,5,"%d",enc_track->song_year);

  ID3TagFile(file,(*(enc_track->song_name))?enc_track->song_name:"Unknown",
	     (*(enc_track->song_artist))?enc_track->song_artist:
	     (*(enc_track->disc_artist))?enc_track->disc_artist:"Unknown",
	     (*(enc_track->disc_name))?enc_track->disc_name:"Unknown",
	     year,"Created by Grip",enc_track->id3_genre,
	     enc_track->track_num+1);
}

void FillInEncode(int track,EncodeTrack *new_track)
{
  new_track->track_num=track;
  new_track->start_frame=info.track[track].track_start;
  new_track->end_frame=info.track[track+1].track_start-1;
  new_track->mins=info.track[track].track_length.minutes;
  new_track->secs=info.track[track].track_length.seconds;
  new_track->song_year=ddata.data_year;
  strncpy(new_track->song_name,ddata.data_track[track].track_name,80);
  strncpy(new_track->song_artist,ddata.data_track[track].track_artist,80);
  strncpy(new_track->disc_name,ddata.data_title,80);
  strncpy(new_track->disc_artist,ddata.data_artist,80);
  new_track->id3_genre=id3_genre_number;
  new_track->discid=CDDBDiscid(cd_desc);
}

void AddToEncode(int track)
{
  EncodeTrack *new_track;

  new_track=(EncodeTrack *)g_new(EncodeTrack,1);

  FillInEncode(track,new_track);

  encode_list=g_list_append(encode_list,new_track);

  Debug("Added track %d to encode list\n",track+1);
}

gboolean MP3Encode(void)
{
  char parsedstr[1024];
  char tmp[1024];
  char *args[20];
  int startsec,endsec;
  int bytesleft;
  EncodeTrack *enc_track;
  int encode_track;
  int cpu;

  if(!encode_list) return FALSE;

  for(cpu=0;(cpu<num_cpu)&&(encoding&(1<<cpu));cpu++);

  if(cpu==num_cpu) {
    Debug("No free cpus\n");
    return FALSE;
  }

  enc_track=(EncodeTrack *)(g_list_first(encode_list)->data);
  encode_track=enc_track->track_num;

  encode_list=g_list_remove(encode_list,enc_track);
  encoded_track[cpu]=enc_track;

  CopyPixmap(GTK_PIXMAP(mp3_pix[0]),GTK_PIXMAP(mp3_indicator[cpu]));
  
  sprintf(tmp,"MP3: Trk %d",encode_track+1);
  gtk_label_set(GTK_LABEL(mp3_prog_label[cpu]),tmp);
  
  Debug("MP3 track %d\n",encode_track+1);
  
  if(rip_partial) {
    startsec=start_sector;
    endsec=end_sector;
  }
  else {
    startsec=0;
    endsec=enc_track->end_frame-enc_track->start_frame;
  }
  
  ParseEncFileFmt(mp3fileformat,mp3file[cpu],enc_track);
  MakeDirs(mp3file[cpu]);
  
  bytesleft=BytesLeftInFS(ripfile);
  
  /* DO CHECK HERE!!! */
  
  Debug("%i: Encoding to %s\n",cpu+1,mp3file[cpu]);
  
  unlink(mp3file[cpu]);
  
  mp3size[cpu]=(int)((gfloat)((endsec-startsec)+1)*
		     (gfloat)(kbits_per_sec*1024)/600.0);
  
  /* Construct the .wav filename to encode */
  
  ParseEncFileFmt(ripfileformat,rip_delete_file[cpu],enc_track);
  ParseMP3Cmd(mp3cmdline,parsedstr,rip_delete_file[cpu],mp3file[cpu],cpu);
  
  Debug("MP3 commandline is [%s]\n",parsedstr);
  
  args[0]=FindRoot(mp3exename);
  
  MakeArgs(parsedstr,args+1,19);
  
  mp3pid[cpu]=fork();
  
  if(mp3pid[cpu]==0) {
    close(ConnectionNumber(GDK_DISPLAY()));
    RedirectIO(!do_debug);
    setsid();
    nice(mp3nice);
    execv(mp3exename,args);
    _exit(0);
  }
  
  encoding|=(1<<cpu);

  return TRUE;
}

void RipIsFinished(void)
{
  Debug("Ripping is finished\n");

  ripping_a_disc=FALSE;

  if(beep_after_rip) printf("%c%c",7,7);
  
  if(eject_after_rip) {
    EjectDisc();

    if(eject_delay) auto_eject_countdown=eject_delay;
  }
}

void DoRip(GtkWidget *widget,gpointer data)
{
  gboolean result;

  if(!using_builtin_cdp&&!FileExists(ripexename)) {
    BoolDialog("Invalid rip executable\nCheck your rip config","OK",
	       NULL,NULL,NULL);

    return;
  }

  if(data&&!FileExists(mp3exename)) {
    BoolDialog("Invalid MP3 executable\nCheck your MP3 config","OK",
	       NULL,NULL,NULL);

    return;
  }

  CDStop(cd_desc);
  stopped=TRUE;
    
  if(ripping) return;

  num_wavs=0;

  if(rip_partial)
    rip_track=CURRENT_TRACK;
  else {
    if(add_m3u) AddM3U();
    setCurrentTrackIndex(0);
    rip_track=0;
  }

  if(data) doencode=TRUE;
  else doencode=FALSE;

  result=RipNextTrack();
  if(!result) {
    doencode=FALSE;
    BoolDialog("No tracks selected","OK",NULL,NULL,NULL);
  }
}

void MakeDirs(char *path)
{
  char dir[256];
  char *s;
  int len;

  for(len=0,s=path;*s;s++,len++) {
    if(*s=='/') {
      strncpy(dir,path,len);
      dir[len]='\0';

      if(!FileExists(dir))
	mkdir(dir,0755);
    }
  }
}

char *MakePath(char *str)
{
  int len;

  len=strlen(str)-1;

  if(str[len]!='/') {
    str[len+1]='/';
    str[len+2]='\0';
  }

  return str;
}

#ifdef CDPAR
pthread_t cdp_thread;
CDPArgs cdp;

void ThreadRip(void *arg)
{
  CDPArgs *cdp;

  cdp=(CDPArgs *)arg;

  Debug("Calling CDPRip\n");

  CDPRip(cdp->device,cdp->track,cdp->first_sector,cdp->last_sector,
	 cdp->outfile,cdp->paranoia_mode);

  in_rip_thread=FALSE;

  pthread_exit(0);
}

#endif

int NextTrackToRip(void)
{
  int track;

  for(track=0;(track<info.disc_totaltracks)&&
	  !TrackIsChecked(track);track++);

  return track;
}

gboolean RipNextTrack(void)
{
  char parsedstr[1024];
  char tmp[1024];
  int startsec,endsec;
  char *args[20];
  int bytesleft;
  struct stat mystat;

  Debug("In RipNextTrack\n");

  if(ripping) return FALSE;

  if(!rip_partial)
    rip_track=NextTrackToRip();

  Debug("First checked track is %d\n",rip_track+1);

  if(rip_track==info.disc_totaltracks)  /* Finished ripping */
    return FALSE;

  /* We have a track to rip */

  if(have_disc&&rip_track>=0) {
    Debug("Ripping away!\n");

    if(!rip_partial){
      gtk_clist_select_row(GTK_CLIST(trackclist),rip_track,0);		
    }

    CopyPixmap(GTK_PIXMAP(rip_pix[0]),GTK_PIXMAP(rip_indicator));

    sprintf(tmp,"Rip: Trk %d",rip_track+1);
    gtk_label_set(GTK_LABEL(rip_prog_label),tmp);

    CDStop(cd_desc);

    if(rip_partial) {
      startsec=start_sector;
      endsec=end_sector;
    }
    else {
      startsec=0;
      endsec=(info.track[rip_track+1].track_start-1)-
	info.track[rip_track].track_start;
    }

    ripsize=44+((endsec-startsec)+1)*2352;
    ParseFileFmt(ripfileformat,ripfile,rip_track+1,startsec,endsec);

    MakeDirs(ripfile);

    bytesleft=BytesLeftInFS(ripfile);

    /* DO CHECK HERE!!! */

    Debug("Ripping track %d to %s\n",rip_track+1,ripfile);

    if(stat(ripfile,&mystat)>=0) {
      if(mystat.st_size == ripsize) { 
	Debug("File %s has already been ripped. Skipping...\n",ripfile);
	num_wavs++;
	ripping=TRUE;
	ripping_a_disc=TRUE;

	return TRUE;
      }
      else unlink(ripfile);
    }

#ifdef CDPAR
    if(selected_ripper==0) {
      cdp.device=cddevice;
      cdp.track=rip_track+1;
      cdp.first_sector=startsec;
      cdp.last_sector=endsec;
      cdp.outfile=ripfile;
      cdp.paranoia_mode=PARANOIA_MODE_FULL^PARANOIA_MODE_NEVERSKIP; 
      if(disable_paranoia) cdp.paranoia_mode=PARANOIA_MODE_DISABLE;
      if(disable_extra_paranoia) {
	cdp.paranoia_mode|=PARANOIA_MODE_OVERLAP;
	cdp.paranoia_mode&=~PARANOIA_MODE_VERIFY;
      }
      if(disable_scratch_detect)
	cdp.paranoia_mode&=~(PARANOIA_MODE_SCRATCH|PARANOIA_MODE_REPAIR);
      if(disable_scratch_repair) 
	cdp.paranoia_mode&=~PARANOIA_MODE_REPAIR;

      in_rip_thread=TRUE;
      rip_smile_level=0;
      pthread_create(&cdp_thread,NULL,(void *)ThreadRip,(void *)&cdp);
    }
    else {
#endif
      ParseRipCmd(ripcmdline,parsedstr,rip_track+1,startsec,endsec,
		  ripfile);
      
      Debug("Rip executable is [%s]\n",ripexename);
      Debug("cmdline is [%s]\n",parsedstr);
      
      args[0]=FindRoot(ripexename);
      
      MakeArgs(parsedstr,args+1,19);
      
      rippid=fork();
      
      if(rippid==0) {
	close(ConnectionNumber(GDK_DISPLAY()));
	RedirectIO(!do_debug);
	nice(ripnice);
	execv(ripexename,args);
	
	Debug("Exec failed\n");
	_exit(0);
      }
#ifdef CDPAR
    }
#endif

    ripping=TRUE;
    ripping_a_disc=TRUE;

    num_wavs++;

    return TRUE;
  }
  else return FALSE;
}

void PlaySegmentCB(GtkWidget *widget,gpointer data)
{
  PlaySegment(CURRENT_TRACK);
}

void PlaySegment(int track)
{
  CDPlayFrames(cd_desc,info.track[track].track_start+start_sector,
	       info.track[track].track_start+end_sector);
}

void UpdateID3Genre(void)
{
  int id3_number;
  GtkWidget *id3_genre;

  id3_number=genre_cdda_2_id3[ddata.data_genre];
  id3_genre=GTK_WIDGET(g_list_nth(id3_genre_item_list,id3_number)->data);
  gtk_list_select_child(GTK_LIST(GTK_COMBO(id3_genre_combo)->list),
			id3_genre);  
}

void ID3GenreChanged(GtkWidget *widget,gint number)
{
  id3_genre_number = number;
}

GtkWidget *MakeRangeSelects(void)
{
  GtkWidget *vbox;
  GtkWidget *numentry;

  vbox=gtk_vbox_new(FALSE,0);

  numentry=MakeNumEntry(&start_sector_entry,&start_sector,"Start sector",10);
  gtk_box_pack_start(GTK_BOX(vbox),numentry,FALSE,FALSE,0);
  gtk_widget_show(numentry);
  
  numentry=MakeNumEntry(&end_sector_entry,&end_sector,"End sector",10);
  gtk_box_pack_start(GTK_BOX(vbox),numentry,FALSE,FALSE,0);
  gtk_widget_show(numentry);
  
  return vbox;
}

void MakeRipPage(void)
{
  GtkWidget *rangesel;
  GtkWidget *vbox,*vbox2,*hbox,*hbox2;
  GtkWidget *button;
  GtkWidget *hsep;
  GtkWidget *check;
  int mycpu;

  rippage=MakeNewPage("Rip");

  vbox=gtk_vbox_new(FALSE,2);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  hbox=gtk_hbox_new(FALSE,5);

  vbox2=gtk_vbox_new(FALSE,0);

  button=gtk_button_new_with_label("Rip+Encode");
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(DoRip),(gpointer)1);
  gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  button=gtk_button_new_with_label("Rip only");
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(DoRip),(gpointer)0);
  gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  button=gtk_button_new_with_label("Abort execution");
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(KillChild),NULL);
  gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);
  gtk_widget_show(button);

#ifdef MP3DB
  button=gtk_button_new_with_label("MP3 DB Scan");
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(DBScan),NULL);
  gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);
  gtk_widget_show(button);
#endif

  gtk_box_pack_start(GTK_BOX(hbox),vbox2,TRUE,TRUE,0);
  gtk_widget_show(vbox2);

  partial_rip_frame=gtk_frame_new(NULL);

  vbox2=gtk_vbox_new(FALSE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox2),3);

  check=MakeCheckButton(&rip_partial_button,&rip_partial,"Rip partial track");
  gtk_signal_connect(GTK_OBJECT(rip_partial_button),"clicked",
		     GTK_SIGNAL_FUNC(RipPartialChanged),NULL);
  gtk_box_pack_start(GTK_BOX(vbox2),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  partial_rip_box=gtk_vbox_new(FALSE,0);
  gtk_widget_set_sensitive(partial_rip_box,rip_partial);

  hbox2=gtk_hbox_new(FALSE,5);

  button=gtk_button_new_with_label("Play");
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(PlaySegmentCB),
		     (gpointer)NULL);
  gtk_box_pack_start(GTK_BOX(hbox2),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  play_sector_label=gtk_label_new("Current sector:      0");
  gtk_box_pack_start(GTK_BOX(hbox2),play_sector_label,FALSE,FALSE,0);
  gtk_widget_show(play_sector_label);

  gtk_box_pack_start(GTK_BOX(partial_rip_box),hbox2,FALSE,FALSE,0);
  gtk_widget_show(hbox2);

  rangesel=MakeRangeSelects();
  gtk_box_pack_start(GTK_BOX(partial_rip_box),rangesel,FALSE,FALSE,0);
  gtk_widget_show(rangesel);

  gtk_box_pack_start(GTK_BOX(vbox2),partial_rip_box,TRUE,TRUE,0);
  gtk_widget_show(partial_rip_box);

  gtk_container_add(GTK_CONTAINER(partial_rip_frame),vbox2);
  gtk_widget_show(vbox2);

  gtk_box_pack_start(GTK_BOX(hbox),partial_rip_frame,TRUE,TRUE,0);
  gtk_widget_show(partial_rip_frame);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);
  gtk_widget_show(hbox);

  hsep=gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(vbox),hsep,TRUE,TRUE,0);
  gtk_widget_show(hsep);

  vbox2=gtk_vbox_new(FALSE,0);

  hbox=gtk_hbox_new(FALSE,3);

  rip_prog_label=gtk_label_new("Rip: Idle");
  gtk_widget_set_usize(rip_prog_label,WINWIDTH-215,0);
  gtk_box_pack_start(GTK_BOX(hbox),rip_prog_label,FALSE,FALSE,0);
  gtk_widget_show(rip_prog_label);

  ripprogbar=gtk_progress_bar_new();
  if(using_builtin_cdp) gtk_widget_set_usize(ripprogbar,WINWIDTH-110,8);
  else gtk_widget_set_usize(ripprogbar,WINWIDTH-95,8);
  gtk_box_pack_start(GTK_BOX(hbox),ripprogbar,FALSE,FALSE,0);
  gtk_widget_show(ripprogbar);

  smile_pix[0]=Loadxpm(smile1_xpm);
  smile_pix[1]=Loadxpm(smile2_xpm);
  smile_pix[2]=Loadxpm(smile3_xpm);
  smile_pix[3]=Loadxpm(smile4_xpm);
  smile_pix[4]=Loadxpm(smile5_xpm);
  smile_pix[5]=Loadxpm(smile6_xpm);
  smile_pix[6]=Loadxpm(smile7_xpm);
  smile_pix[7]=Loadxpm(smile8_xpm);

  smile_indicator=NewBlankPixmap();
  gtk_tooltips_set_tip(MakeToolTip(),smile_indicator,
		       "Rip status",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),smile_indicator,FALSE,FALSE,0);
  gtk_widget_show(smile_indicator);

  gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  for(mycpu=0;mycpu<num_cpu;mycpu++){
    hbox=gtk_hbox_new(FALSE,3);

    mp3_prog_label[mycpu]=gtk_label_new("MP3: Idle");
    gtk_widget_set_usize(mp3_prog_label[mycpu],WINWIDTH-215,0);
    gtk_box_pack_start(GTK_BOX(hbox),mp3_prog_label[mycpu],FALSE,FALSE,0);
    gtk_widget_show(mp3_prog_label[mycpu]);

    mp3progbar[mycpu]=gtk_progress_bar_new();
    gtk_widget_set_usize(mp3progbar[mycpu],WINWIDTH-95,8);
    gtk_box_pack_start(GTK_BOX(hbox),mp3progbar[mycpu],FALSE,FALSE,0);
    gtk_widget_show(mp3progbar[mycpu]);

    gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,0);
    gtk_widget_show(hbox);
  }

  gtk_box_pack_start(GTK_BOX(vbox),vbox2,TRUE,TRUE,0);
  gtk_widget_show(vbox2);

  gtk_container_add(GTK_CONTAINER(rippage),vbox);
  gtk_widget_show(vbox);
}

void RipperSelected(GtkWidget *widget,Ripper *rip)
{
  char buf[256];
  char *path=NULL;

  selected_ripper=rip-ripper_defaults;

#ifdef CDPAR
  if(selected_ripper==0) {
    using_builtin_cdp=TRUE;
    gtk_widget_set_usize(ripprogbar,WINWIDTH-110,8);

    gtk_widget_hide(rip_exe_box);
    gtk_widget_show(rip_builtin_box);
  }
  else {
    using_builtin_cdp=FALSE;
    gtk_widget_set_usize(ripprogbar,WINWIDTH-95,8);

    gtk_widget_show(rip_exe_box);
    gtk_widget_hide(rip_builtin_box);
  }
#endif

  if(!using_builtin_cdp) {
    if(strcmp(rip->name,"other")) {
      path=FindExe(rip->name,bin_search_paths);
      
      if(!path) path=bin_search_paths[0];
      
      g_snprintf(buf,256,"%s/%s",path,rip->name);
      
      gtk_entry_set_text(GTK_ENTRY(ripexename_entry),buf);
    }
    else gtk_entry_set_text(GTK_ENTRY(ripexename_entry),"");
    
    
    gtk_entry_set_text(GTK_ENTRY(ripcmdline_entry),rip->cmdline);
  }
}

void EncoderSelected(GtkWidget *widget,MP3Encoder *enc)
{
  char buf[256];
  char *path=NULL;

  if(strcmp(enc->name,"other")) {
    path=FindExe(enc->name,bin_search_paths);

    if(!path) path=bin_search_paths[0];
    
    g_snprintf(buf,256,"%s/%s",path,enc->name);

    gtk_entry_set_text(GTK_ENTRY(mp3exename_entry),buf);
  }
  else gtk_entry_set_text(GTK_ENTRY(mp3exename_entry),"");

  gtk_entry_set_text(GTK_ENTRY(mp3cmdline_entry),enc->cmdline);

  selected_encoder=enc-encoder_defaults;
}

#endif /* ifndef GRIPCD */

void MakeStyles(void)
{
  gdk_color_white(gdk_window_get_colormap(window->window),&gdkwhite);
  gdk_color_black(gdk_window_get_colormap(window->window),&gdkblack);

  color_LCD=MakeColor(33686,38273,29557);
  color_dark_grey=MakeColor(0x4444,0x4444,0x4444);
  
  style_wb=MakeStyle(&gdkwhite,&gdkblack,FALSE);
  style_LCD=MakeStyle(color_LCD,color_LCD,FALSE);
  style_dark_grey=MakeStyle(&gdkwhite,color_dark_grey,TRUE);
}

int main(int argc,char *argv[])
{
  char buf[256];

  srand(time(NULL));

  ParseArgs(argc,argv);

  DoLoadConfig();

  gtk_set_locale();
  gtk_init(&argc,&argv);
  
  if((GTK_MINOR_VERSION!=gtk_minor_version)) {
    printf("Warning: This program was compiled "
	   "using gtk+ version %d.%d.%d, and you are running gtk+ "
	   "version %d.%d.%d...\n",
	   GTK_MAJOR_VERSION,GTK_MINOR_VERSION,GTK_MICRO_VERSION,
	   gtk_major_version,gtk_minor_version,gtk_micro_version);
  }

  gtk_rc_parse("~/.gtkrc");
  
  window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_realize(window);
  
  gtk_window_set_policy(GTK_WINDOW(window),FALSE,TRUE,FALSE);
  
  sprintf(buf,"%s %s",PROGRAM,VERSION);
  gtk_window_set_title(GTK_WINDOW(window),buf);
  
  gtk_signal_connect(GTK_OBJECT(window),"delete_event",
		     GTK_SIGNAL_FUNC(ShutDownCB),NULL);
  
#if (GTK_MINOR_VERSION != 0)
  gtk_signal_connect_after(GTK_OBJECT(window),"size_allocate",
			   GTK_SIGNAL_FUNC(CheckWindowPosition),NULL);
#endif
  
  MakeStyles();
  
  notebook=gtk_notebook_new();
  gtk_widget_set_usize(notebook,WINWIDTH,0);
  
  winbox=gtk_vbox_new(FALSE,3);
  if(!minimized) gtk_container_border_width(GTK_CONTAINER(winbox),3);
  
  MakeTrackPage();
  
  check_image=Loadxpm(check_xpm);
  empty_image=Loadxpm(empty_xpm);
  
#ifdef CDPAR
  if(selected_ripper==0) using_builtin_cdp=TRUE;
#endif
  
#ifndef GRIPCD
  MakeRipPage();
#endif
  
  MakeConfigPage();
  
  gtk_box_pack_start(GTK_BOX(winbox),notebook,TRUE,TRUE,0);
  if(!minimized) gtk_widget_show(notebook);
  
  gtk_container_add(GTK_CONTAINER(window),winbox);
  gtk_widget_show(winbox);
    
  MakeHelpPage();
  MakeAboutPage();
  
  track_edit_box=MakeEditBox();
  
  gtk_box_pack_start(GTK_BOX(winbox),track_edit_box,FALSE,FALSE,0);
  
  playopts=MakePlayOpts();
  gtk_box_pack_start(GTK_BOX(winbox),playopts,FALSE,FALSE,0);
  
  controls=MakeControls();
  gtk_box_pack_start(GTK_BOX(winbox),controls,FALSE,FALSE,0);
  gtk_widget_show(controls);
  
  gtk_widget_show(window);
  
  wait_cursor=gdk_cursor_new(GDK_WATCH);
  
  gtk_timeout_add(1000,TimeOut,0);

  UpdateGTK(); /* Display the window while we start up the cdrom */

  /* Try to open the cd device */

  Busy();
#ifdef SOLARIS
  find_cdrom();
#endif

  cd_desc=CDInitDevice(cddevice);
  if(cd_desc==-1) {
    BoolDialog("Unable to open the cd device. Please make sure that you are\n"
	       "using the correct device (passed to the program with \"-d\")\n"
	       "and that you have permission to access the device. See the\n"
	       "README file for more information.","OK",NULL,NULL,NULL);
    /*	       GTK_SIGNAL_FUNC(ShutDown),NULL,NULL);*/
  }
  else {
    have_working_device=TRUE;

    if(!no_interrupt)
      CDStop(cd_desc);
  
    changer_slots=CDChangerSlots(cd_desc);

    CheckNewDisc();
    UpdateDisplay();
  }

  UnBusy();
  
  gtk_main();

  if(!no_interrupt)
    CDStop(cd_desc);

  DoSaveConfig();

  return 0;
}
