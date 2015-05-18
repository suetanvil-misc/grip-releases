/* rip.c
 *
 * Copyright (c) 1998-2002  Mike Oliphant <oliphant@gtk.org>
 *
 *   http://www.nostatic.org/grip
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <sys/stat.h>
#ifdef SOLARIS
#include <sys/statvfs.h>
#endif
#if defined(__linux__)
#include <sys/vfs.h>
#endif
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include "grip.h"
#include "rip.h"
#include "dialog.h"
#include "cdplay.h"
#include "cddev.h"
#include "gripcfg.h"
#include "launch.h"
#include "grip_id3.h"
#include "config.h"
#include "common.h"
#include "gain_analysis.h"

#ifdef CDPAR
#define size32 gint32
#define size16 gint16
#include <cdda_interface.h>
#include <cdda_paranoia.h>
extern int rip_smile_level;
#endif

static void RipPartialChanged(GtkWidget *widget,gpointer data);
static void PlaySegmentCB(GtkWidget *widget,gpointer data);
static GtkWidget *MakeRangeSelects(GripInfo *ginfo);
static void AddSQLEntry(GripInfo *ginfo,EncodeTrack *enc_track);
static void DBScan(GtkWidget *widget,gpointer data);
static char *MakeRelative(char *file1,char *file2);
static void AddM3U(GripInfo *ginfo);
static void ID3Add(GripInfo *ginfo,char *file,EncodeTrack *enc_track);
static void DoWavFilter(GripInfo *ginfo);
static void DoDiscFilter(GripInfo *ginfo);
static void RipIsFinished(GripInfo *ginfo);
static void CheckDupNames(GripInfo *ginfo);
static int NextTrackToRip(GripInfo *ginfo);
static gboolean RipNextTrack(GripInfo *ginfo);
static void ThreadRip(void *arg);
static void AddToEncode(GripInfo *ginfo,int track);
static gboolean MP3Encode(GripInfo *ginfo);


void MakeRipPage(GripInfo *ginfo)
{
  GripGUI *uinfo;
  GtkWidget *rippage;
  GtkWidget *rangesel;
  GtkWidget *vbox,*vbox2,*hbox,*hbox2;
  GtkWidget *button;
  GtkWidget *hsep;
  GtkWidget *check;
  GtkWidget *partial_rip_frame;
  int mycpu;
  int label_width;

  uinfo=&(ginfo->gui_info);

  rippage=MakeNewPage(uinfo->notebook,_("Rip"));

  vbox=gtk_vbox_new(FALSE,2);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  hbox=gtk_hbox_new(FALSE,5);

  vbox2=gtk_vbox_new(FALSE,0);

  button=gtk_button_new_with_label(_("Rip+Encode"));
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Rip and encode selected tracks"),NULL);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(DoRipEncode),(gpointer)ginfo);
  gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  button=gtk_button_new_with_label(_("Rip Only"));
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Rip but do not encode selected tracks"),NULL);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(DoRip),(gpointer)ginfo);
  gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  button=gtk_button_new_with_label(_("Abort Rip and Encode"));
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Kill all active rip and encode processes"),NULL);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(KillRip),(gpointer)ginfo);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(KillEncode),(gpointer)ginfo);
  gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  button=gtk_button_new_with_label(_("Abort Ripping Only"));
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Kill rip process"),NULL);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(KillRip),(gpointer)ginfo);
  gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  button=gtk_button_new_with_label(_("DDJ Scan"));
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Insert disc information into the DigitalDJ database"),
		       NULL);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(DBScan),(gpointer)ginfo);
  gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  gtk_box_pack_start(GTK_BOX(hbox),vbox2,TRUE,TRUE,0);
  gtk_widget_show(vbox2);

  partial_rip_frame=gtk_frame_new(NULL);

  vbox2=gtk_vbox_new(FALSE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox2),3);

  check=MakeCheckButton(NULL,&ginfo->rip_partial,_("Rip partial track"));
  gtk_signal_connect(GTK_OBJECT(check),"clicked",
  		     GTK_SIGNAL_FUNC(RipPartialChanged),(gpointer)ginfo);
  gtk_box_pack_start(GTK_BOX(vbox2),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  uinfo->partial_rip_box=gtk_vbox_new(FALSE,0);
  gtk_widget_set_sensitive(uinfo->partial_rip_box,ginfo->rip_partial);

  hbox2=gtk_hbox_new(FALSE,5);

  button=gtk_button_new_with_label(_("Play"));
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
  		     GTK_SIGNAL_FUNC(PlaySegmentCB),
  		     (gpointer)ginfo);
  gtk_box_pack_start(GTK_BOX(hbox2),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  uinfo->play_sector_label=gtk_label_new(_("Current sector:      0"));
  gtk_box_pack_start(GTK_BOX(hbox2),uinfo->play_sector_label,FALSE,FALSE,0);
  gtk_widget_show(uinfo->play_sector_label);

  gtk_box_pack_start(GTK_BOX(uinfo->partial_rip_box),hbox2,FALSE,FALSE,0);
  gtk_widget_show(hbox2);

  rangesel=MakeRangeSelects(ginfo);
  gtk_box_pack_start(GTK_BOX(uinfo->partial_rip_box),rangesel,FALSE,FALSE,0);
  gtk_widget_show(rangesel);

  gtk_box_pack_start(GTK_BOX(vbox2),uinfo->partial_rip_box,TRUE,TRUE,0);
  gtk_widget_show(uinfo->partial_rip_box);

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

  uinfo->rip_prog_label=gtk_label_new(_("Rip: Idle"));

  /* This should be the largest this string can get */
  label_width=gdk_string_width(uinfo->app->style->font,
			       _("MP3: Trk 99 (99.9x)"))+20;

  gtk_widget_set_usize(uinfo->rip_prog_label,label_width,0);
  gtk_box_pack_start(GTK_BOX(hbox),uinfo->rip_prog_label,FALSE,FALSE,0);
  gtk_label_set(GTK_LABEL(uinfo->rip_prog_label),_("Rip: Idle"));
  gtk_widget_show(uinfo->rip_prog_label);

  uinfo->ripprogbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(hbox),uinfo->ripprogbar,FALSE,FALSE,0);
  gtk_widget_show(uinfo->ripprogbar);

  uinfo->smile_indicator=NewBlankPixmap(uinfo->app);
  gtk_tooltips_set_tip(MakeToolTip(),uinfo->smile_indicator,
		       _("Rip status"),NULL);
  gtk_box_pack_start(GTK_BOX(hbox),uinfo->smile_indicator,FALSE,FALSE,0);
  gtk_widget_show(uinfo->smile_indicator);

  gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  for(mycpu=0;mycpu<ginfo->num_cpu;mycpu++){
    hbox=gtk_hbox_new(FALSE,3);

    uinfo->mp3_prog_label[mycpu]=gtk_label_new(_("MP3: Idle"));
    gtk_widget_set_usize(uinfo->mp3_prog_label[mycpu],label_width,0);

    gtk_box_pack_start(GTK_BOX(hbox),uinfo->mp3_prog_label[mycpu],
		       FALSE,FALSE,0);
    gtk_widget_show(uinfo->mp3_prog_label[mycpu]);

    uinfo->mp3progbar[mycpu]=gtk_progress_bar_new();

    gtk_box_pack_start(GTK_BOX(hbox),uinfo->mp3progbar[mycpu],FALSE,FALSE,0);
    gtk_widget_show(uinfo->mp3progbar[mycpu]);

    gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,0);
    gtk_widget_show(hbox);
  }

  gtk_box_pack_start(GTK_BOX(vbox),vbox2,TRUE,TRUE,0);
  gtk_widget_show(vbox2);

  gtk_container_add(GTK_CONTAINER(rippage),vbox);
  gtk_widget_show(vbox);
}

static void RipPartialChanged(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  gtk_widget_set_sensitive(ginfo->gui_info.partial_rip_box,ginfo->rip_partial);
}

static void PlaySegmentCB(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  PlaySegment(ginfo,CURRENT_TRACK);
}

static GtkWidget *MakeRangeSelects(GripInfo *ginfo)
{
  GtkWidget *vbox;
  GtkWidget *numentry;

  vbox=gtk_vbox_new(FALSE,0);

  numentry=MakeNumEntry(&(ginfo->gui_info.start_sector_entry),
			&ginfo->start_sector,_("Start sector"),10);
  gtk_box_pack_start(GTK_BOX(vbox),numentry,FALSE,FALSE,0);
  gtk_widget_show(numentry);
  
  numentry=MakeNumEntry(&(ginfo->gui_info.end_sector_entry),
			&ginfo->end_sector,_("End sector"),10);
  gtk_box_pack_start(GTK_BOX(vbox),numentry,FALSE,FALSE,0);
  gtk_widget_show(numentry);
  
  return vbox;
}

static void DBScan(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;
  int track;
  EncodeTrack enc_track;
  GString *str;

  ginfo=(GripInfo *)data;

  if(!ginfo->have_disc) return;

  for(track=0;track<ginfo->disc.num_tracks;track++) {
    FillInTrackInfo(ginfo,track,&enc_track);
    
    str=g_string_new(NULL);
    
    TranslateString(ginfo->mp3fileformat,str,TranslateSwitch,
		    &enc_track,&(ginfo->sprefs));
    
    g_snprintf(enc_track.mp3_filename,256,"%s",str->str);
    g_string_free(str,TRUE);
    
    AddSQLEntry(ginfo,&enc_track);
  }
}

static void AddSQLEntry(GripInfo *ginfo,EncodeTrack *enc_track)
{
  int sqlpid;
  char track_str[4];
  char frame_str[11];
  char length_str[11];
  char playtime_str[6];
  char year_str[5];

  g_snprintf(track_str,4,"%d",enc_track->track_num);
  g_snprintf(frame_str,11,"%d",enc_track->start_frame);
  g_snprintf(length_str,11,"%d",enc_track->end_frame-enc_track->start_frame);
  g_snprintf(playtime_str,6,"%d:%d",enc_track->mins,enc_track->secs);
  g_snprintf(year_str,5,"%d",enc_track->song_year);

  sqlpid=fork();

  if(sqlpid==0) {
    CloseStuff(ginfo);

    if(*enc_track->song_artist)
      execlp("mp3insert","mp3insert",
	    "-p",enc_track->mp3_filename,
	    "-a",enc_track->disc_artist,
	    "-i",enc_track->song_artist,
	    "-t",enc_track->song_name,"-d",enc_track->disc_name,
	    "-g",ID3GenreString(enc_track->id3_genre),"-y",year_str,
	    "-n",track_str,
	    "-f",frame_str,"-l",length_str,"-m",playtime_str,NULL);
    else
      execlp("mp3insert","mp3insert",
	    "-p",enc_track->mp3_filename,
	    "-a",enc_track->disc_artist,
	    "-t",enc_track->song_name,"-d",enc_track->disc_name,
	    "-g",ID3GenreString(enc_track->id3_genre),"-y",year_str,
	    "-n",track_str,
	    "-f",frame_str,"-l",length_str,"-m",playtime_str,NULL);
    
    _exit(0);
  }

  waitpid(sqlpid,NULL,0);
}

gboolean IsDir(char *path)
{
  struct stat mystat;

  if(stat(path,&mystat)!=0) return FALSE;

  return S_ISDIR(mystat.st_mode);
}

unsigned long long BytesLeftInFS(char *path)
{
  unsigned long long bytesleft;

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

  bytesleft=stat.f_bavail;
  bytesleft*=stat.f_bsize;

  return bytesleft;
}

char *FindRoot(char *str)
{
  char *c;

  for(c=str+strlen(str);c>str;c--) {
    if(*c=='/') return c+1;
  }

  return c;
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
	mkdir(dir,0777);
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

/* Make file1 relative to file2 */
static char *MakeRelative(char *file1,char *file2)
{
  int pos, pos2=0, slashcnt, i;
  char *rel=file1;
  char tem[PATH_MAX]="";

  slashcnt=0;

  /* This part finds relative names assuming m3u is not branched in a
     different directory from mp3 */
  for(pos=0;file2[pos];pos++) {
    if(pos&&(file2[pos]=='/')) {
      if(!strncmp(file1,file2,pos)) {
	rel=file1+pos+1;
	pos2=pos;
      }
    }
  }

  /* Now check to see if the m3u file branches to a different directory. */
  for(pos2=pos2+1;file2[pos2];pos2++) {
    if(file2[pos2]=='/'){             
      slashcnt++;
    }
  } 

  /* Now add correct number of "../"s to make the path relative */
  for(i=0;i<slashcnt;i++) {
    strcpy(tem,"../");
    strncat(tem,rel,strlen(rel));
    strcpy(rel,tem);
  }

  return rel; 
}

static void AddM3U(GripInfo *ginfo)
{
  int i;
  EncodeTrack enc_track;
  FILE *fp;
  char tmp[PATH_MAX];
  char m3unam[PATH_MAX];
  char *relnam;
  GString *str;

  if(!ginfo->have_disc) return;
  
  str=g_string_new(NULL);

  /* Use track 0 to fill in M3u switches */
  FillInTrackInfo(ginfo,0,&enc_track);

  TranslateString(ginfo->m3ufileformat,str,TranslateSwitch,
		    &enc_track,&(ginfo->sprefs));

  g_snprintf(m3unam,PATH_MAX,"%s",str->str);

  MakeDirs(m3unam);

  fp=fopen(str->str, "w");
  if(fp==NULL) {
    printf(_("Error: can't open m3u file\n"));
    return;
  }
  
  for(i=0;i<ginfo->disc.num_tracks;i++) {
    /* Only add to the m3u if the track is selected for ripping */
    if(TrackIsChecked(&(ginfo->gui_info),i)) {
      g_string_truncate(str,0);

      FillInTrackInfo(ginfo,i,&enc_track);
      TranslateString(ginfo->mp3fileformat,str,TranslateSwitch,
		      &enc_track,&(ginfo->sprefs));

      if(ginfo->rel_m3u) {
	g_snprintf(tmp,PATH_MAX,"%s",str->str);
	relnam=MakeRelative(tmp,m3unam);
	fprintf(fp,"%s\n",relnam);
      }
      else 
	fprintf(fp,"%s\n",str->str);
    }
  }

  g_string_free(str,TRUE);
  
  fclose(fp);
}

void KillRip(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;
  
  if(!ginfo->ripping) return;

  /* Need to decrement num_wavs since we didn't finish ripping
     the current track */
  ginfo->num_wavs--;

  ginfo->stop_rip=TRUE;
  ginfo->ripping_a_disc=FALSE;

  if(ginfo->using_builtin_cdp) {
#ifdef CDPAR
    ginfo->stop_thread_rip_now=TRUE;
#endif
  }
  else kill(ginfo->rippid,SIGKILL);
  /*    ginfo->doencode=FALSE;*/
}

void KillEncode(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;
  int mycpu;
  GList *elist;
  EncodeTrack *enc_track;

  ginfo=(GripInfo *)data;
  
  if(!ginfo->encoding) return;

  ginfo->stop_encode=TRUE;
  ginfo->num_wavs=0;

  for(mycpu=0;mycpu<ginfo->num_cpu;mycpu++){
    if(ginfo->encoding&(1<<mycpu)) kill(ginfo->mp3pid[mycpu],SIGKILL);
  }
  
  elist=ginfo->encode_list;
  
  /* Remove all entries in the encode list */
  while(elist) {
    enc_track=(EncodeTrack *)elist->data;
    elist=elist->next;
    
    ginfo->encode_list=g_list_remove(elist,enc_track);
    g_free(enc_track);
  }
}

static void ID3Add(GripInfo *ginfo,char *file,EncodeTrack *enc_track)
{
  char year[5];
  GString *comment;

  comment=g_string_new(NULL);
  TranslateString(ginfo->id3_comment,comment,TranslateSwitch,enc_track,
		  &(ginfo->sprefs));

  g_snprintf(year,5,"%d",enc_track->song_year);

  /* If we've got id3lib, we have the option of doing v2 tags */
#ifdef HAVE_ID3LIB
  if(ginfo->doid3v2) {
    ID3v2TagFile(file,(*(enc_track->song_name))?enc_track->song_name:"Unknown",
		 (*(enc_track->song_artist))?enc_track->song_artist:
		 (*(enc_track->disc_artist))?enc_track->disc_artist:"Unknown",
		 (*(enc_track->disc_name))?enc_track->disc_name:"Unknown",
		 year,comment->str,enc_track->id3_genre,
		 enc_track->track_num+1);
  }
  else {
#endif
  ID3v1TagFile(file,(*(enc_track->song_name))?enc_track->song_name:"Unknown",
	       (*(enc_track->song_artist))?enc_track->song_artist:
	       (*(enc_track->disc_artist))?enc_track->disc_artist:"Unknown",
	       (*(enc_track->disc_name))?enc_track->disc_name:"Unknown",
	       year,comment->str,enc_track->id3_genre,
	       enc_track->track_num+1);
#ifdef HAVE_ID3LIB
  }
#endif

  g_string_free(comment,TRUE);
}

static void DoWavFilter(GripInfo *ginfo)
{
  EncodeTrack enc_track;

  FillInTrackInfo(ginfo,ginfo->rip_track,&enc_track);
  strcpy(enc_track.wav_filename,ginfo->ripfile);

  TranslateAndLaunch(ginfo->wav_filter_cmd,TranslateSwitch,&enc_track,
		     &(ginfo->sprefs),CloseStuff,(void *)ginfo);
}

static void DoDiscFilter(GripInfo *ginfo)
{
  EncodeTrack enc_track;

  FillInTrackInfo(ginfo,ginfo->rip_track,&enc_track);
  strcpy(enc_track.wav_filename,ginfo->ripfile);

  TranslateAndLaunch(ginfo->disc_filter_cmd,TranslateSwitch,&enc_track,
		     &(ginfo->sprefs),CloseStuff,(void *)ginfo);
}

void UpdateRipProgress(GripInfo *ginfo)
{
  GripGUI *uinfo;
  struct stat mystat;
  int quarter;
  gfloat percent=0;
  int mycpu;
  char buf[PATH_MAX];
  time_t now;
  gfloat elapsed=0;
  gfloat speed;

  uinfo=&(ginfo->gui_info);

  if(ginfo->ripping) {
    if(stat(ginfo->ripfile,&mystat)>=0) {
      percent=(gfloat)mystat.st_size/(gfloat)ginfo->ripsize;
      if(percent>1.0) percent=1.0;
    }
    else {
      percent=0;
    }
   
    gtk_progress_bar_update(GTK_PROGRESS_BAR(uinfo->ripprogbar),percent);

    now = time(NULL);
    elapsed = (gfloat)now - (gfloat)ginfo->rip_started;

    /* 1x is 44100*2*2 = 176400 bytes/sec */
    speed = (gfloat)(mystat.st_size)/(176400.0f*elapsed);
    
    /* startup */
    if(speed >= 50.0f) {
      speed = 0.0f;
      ginfo->rip_started = now;
    }
    
    sprintf(buf,_("Rip: Trk %d (%3.1fx)"),ginfo->rip_track+1,speed);
    gtk_label_set(GTK_LABEL(uinfo->rip_prog_label),buf);

    quarter=(int)(percent*4.0);
	
    if(quarter<4)
      CopyPixmap(GTK_PIXMAP(uinfo->rip_pix[quarter]),
		 GTK_PIXMAP(uinfo->rip_indicator));

#ifdef CDPAR
    if(ginfo->using_builtin_cdp) {
      if(uinfo->minimized)
	CopyPixmap(GTK_PIXMAP(uinfo->smile_pix[ginfo->rip_smile_level]),
		   GTK_PIXMAP(uinfo->lcd_smile_indicator));
      else
	CopyPixmap(GTK_PIXMAP(uinfo->smile_pix[ginfo->rip_smile_level]),
		   GTK_PIXMAP(uinfo->smile_indicator));
    }
#endif

    /* Check if a rip finished */
    if((ginfo->using_builtin_cdp&&!ginfo->in_rip_thread) ||
       (!ginfo->using_builtin_cdp&&waitpid(ginfo->rippid,NULL,WNOHANG))) {
      if(!ginfo->using_builtin_cdp) waitpid(ginfo->rippid,NULL,0);
      else {
	CopyPixmap(GTK_PIXMAP(uinfo->empty_image),
		   GTK_PIXMAP(uinfo->lcd_smile_indicator));
	CopyPixmap(GTK_PIXMAP(uinfo->empty_image),
		   GTK_PIXMAP(uinfo->smile_indicator));
      }

      Debug(_("Rip finished\n"));

      ginfo->ripping=FALSE;
      SetChecked(uinfo,ginfo->rip_track,FALSE);

      /* Get the title gain */
      if(ginfo->calc_gain) {
	ginfo->track_gain_adjustment=GetTitleGain();
      }

      /* Do filtering of .wav file */

      if(*ginfo->wav_filter_cmd) DoWavFilter(ginfo);

      gtk_progress_bar_update(GTK_PROGRESS_BAR(uinfo->ripprogbar),0.0);
      CopyPixmap(GTK_PIXMAP(uinfo->empty_image),
		 GTK_PIXMAP(uinfo->rip_indicator));

      if(!ginfo->stop_rip) {
	if(ginfo->doencode) {
	  AddToEncode(ginfo,ginfo->rip_track);
	  MP3Encode(ginfo);
	}

	Debug(_("Rip partial %d  num wavs %d\n"),ginfo->rip_partial,
	      ginfo->num_wavs);

	Debug(_("Next track is %d, total is %d\n"),
	      NextTrackToRip(ginfo),ginfo->disc.num_tracks);

	if(!ginfo->rip_partial&&
	   (ginfo->num_wavs<ginfo->max_wavs||
	    NextTrackToRip(ginfo)==ginfo->disc.num_tracks)) {
	  Debug(_("Check if we need to rip another track\n"));
	  if(!RipNextTrack(ginfo)) RipIsFinished(ginfo);
	}
      }

      if(!ginfo->ripping) {
	ginfo->stop_rip=FALSE;
	gtk_label_set(GTK_LABEL(uinfo->rip_prog_label),_("Rip: Idle"));
      }
    }
  }
  
  /* Check if an encode finished */
  for(mycpu=0;mycpu<ginfo->num_cpu;mycpu++){
    if(ginfo->encoding&(1<<mycpu)) {
      if(stat(ginfo->mp3file[mycpu],&mystat)>=0) {
        percent=(gfloat)mystat.st_size/(gfloat)ginfo->mp3size[mycpu];
        if(percent>1.0) percent=1.0;
      }
      else {
        percent=0;
      } 

      gtk_progress_bar_update(GTK_PROGRESS_BAR(uinfo->mp3progbar[mycpu]),
			      percent);
       
      now = time(NULL);
      elapsed = (gfloat)now - (gfloat)ginfo->mp3_started[mycpu];

      speed = (gfloat)mystat.st_size/
	((gfloat)ginfo->kbits_per_sec * 128.0f * elapsed);
      
      sprintf(buf,_("MP3: Trk %d (%3.1fx)"),
	      ginfo->mp3_enc_track[mycpu]+1,speed);
      gtk_label_set(GTK_LABEL(uinfo->mp3_prog_label[mycpu]),buf);
 
      quarter=(int)(percent*4.0);
 
      if(quarter<4)
        CopyPixmap(GTK_PIXMAP(uinfo->mp3_pix[quarter]),
		   GTK_PIXMAP(uinfo->mp3_indicator[mycpu]));
   
      if(waitpid(ginfo->mp3pid[mycpu],NULL,WNOHANG)) {
        waitpid(ginfo->mp3pid[mycpu],NULL,0);
        ginfo->encoding&=~(1<<mycpu);

	Debug(_("Finished encoding on cpu %d\n"),mycpu);

        gtk_progress_bar_update(GTK_PROGRESS_BAR(uinfo->mp3progbar[mycpu]),
				0.0);
        CopyPixmap(GTK_PIXMAP(uinfo->empty_image),
		   GTK_PIXMAP(uinfo->mp3_indicator[mycpu]));

        if(ginfo->delete_wavs) {
	  Debug(_("Deleting [%s]\n"),ginfo->rip_delete_file[mycpu]);
	  unlink(ginfo->rip_delete_file[mycpu]);
	}

        ginfo->num_wavs--;

        if(!ginfo->stop_encode) {
          if(ginfo->doid3) ID3Add(ginfo,ginfo->mp3file[mycpu],
				  ginfo->encoded_track[mycpu]);

	  if(ginfo->add_to_db) AddSQLEntry(ginfo,ginfo->encoded_track[mycpu]);

	  if(*ginfo->mp3_filter_cmd)
	    TranslateAndLaunch(ginfo->mp3_filter_cmd,TranslateSwitch,
			       ginfo->encoded_track[mycpu],
			       &(ginfo->sprefs),CloseStuff,(void *)ginfo);


          if(ginfo->ripping_a_disc&&!ginfo->rip_partial&&
	     !ginfo->ripping&&ginfo->num_wavs<ginfo->max_wavs) {
	    if(RipNextTrack(ginfo)) ginfo->doencode=TRUE;
	    else RipIsFinished(ginfo);
          }

	  g_free(ginfo->encoded_track[mycpu]);
         
	  if(!ginfo->rip_partial&&ginfo->encode_list) {
	    MP3Encode(ginfo);
	  }
        }
	else ginfo->stop_encode=FALSE;

        if(!(ginfo->encoding&(1<<mycpu)))
	  gtk_label_set(GTK_LABEL(uinfo->mp3_prog_label[mycpu]),
			_("MP3: Idle"));
      }
    }  
  }
}

static void RipIsFinished(GripInfo *ginfo)
{
  Debug(_("Ripping is finished\n"));

  ginfo->ripping_a_disc=FALSE;

  if(ginfo->beep_after_rip) printf("%c%c",7,7);
  
  if(ginfo->eject_after_rip) {
    EjectDisc(NULL,ginfo);

    if(ginfo->eject_delay) ginfo->auto_eject_countdown=ginfo->eject_delay;
  }
}

char *TranslateSwitch(char switch_char,void *data,gboolean *munge)
{
  static char res[PATH_MAX];
  EncodeTrack *enc_track;

  enc_track=(EncodeTrack *)data;

  switch(switch_char) {
  case 'b':
    g_snprintf(res,PATH_MAX,"%d",enc_track->ginfo->kbits_per_sec);
    *munge=FALSE;
    break;
  case 'c':
    g_snprintf(res,PATH_MAX,"%s",enc_track->ginfo->cd_device);
    *munge=FALSE;
    break;
  case 'w':
    g_snprintf(res,PATH_MAX,"%s",enc_track->wav_filename);
    *munge=FALSE;
    break;
  case 'm':
    g_snprintf(res,PATH_MAX,"%s",enc_track->mp3_filename);
    *munge=FALSE;
    break;
  case 't':
    g_snprintf(res,PATH_MAX,"%02d",enc_track->track_num+1);
    *munge=FALSE;
    break;
  case 's':
    g_snprintf(res,PATH_MAX,"%d",enc_track->ginfo->start_sector);
    *munge=FALSE;
    break;
  case 'e':
    g_snprintf(res,PATH_MAX,"%d",enc_track->ginfo->end_sector);
    *munge=FALSE;
    break;
  case 'n':
    if(*(enc_track->song_name))
      g_snprintf(res,PATH_MAX,"%s",enc_track->song_name);
    else g_snprintf(res,PATH_MAX,"Track%02d",enc_track->track_num);
    break;
  case 'a':
    if(*(enc_track->song_artist))
      g_snprintf(res,PATH_MAX,"%s",enc_track->song_artist);
    else {
      if(*(enc_track->disc_artist))
	g_snprintf(res,PATH_MAX,"%s",enc_track->disc_artist);
      else strncpy(res,_("NoArtist"),PATH_MAX);
    }
    break;
  case 'A':
    if(*(enc_track->disc_artist))
      g_snprintf(res,PATH_MAX,"%s",enc_track->disc_artist);
    else strncpy(res,_("NoArtist"),PATH_MAX);	
    break;
  case 'd':
    if(*(enc_track->disc_name))
      g_snprintf(res,PATH_MAX,"%s",enc_track->disc_name);
    else strncpy(res,_("NoTitle"),PATH_MAX);
    break;
  case 'i':
    g_snprintf(res,PATH_MAX,"%02x",enc_track->discid);
    *munge=FALSE;
    break;
  case 'y':
    g_snprintf(res,PATH_MAX,"%d",enc_track->song_year);
    *munge=FALSE;
    break;
  case 'g':
    g_snprintf(res,PATH_MAX,"%d",enc_track->id3_genre);
    *munge=FALSE;
    break;
  case 'G':
    g_snprintf(res,PATH_MAX,"%s",ID3GenreString(enc_track->id3_genre));
    break;
  case 'r':
    g_snprintf(res,PATH_MAX,"%+6.2f",enc_track->track_gain_adjustment);
    *munge=FALSE;
    break;
  case 'R':
    g_snprintf(res,PATH_MAX,"%+6.2f",enc_track->disc_gain_adjustment);
    *munge=FALSE;
    break;
  default:
    *res='\0';
    break;
  }

  return res;
}


static void CheckDupNames(GripInfo *ginfo)
{
  int track,track2;
  int numdups[MAX_TRACKS];
  int count;
  char buf[256];

  for(track=0;track<ginfo->disc.num_tracks;track++)
    numdups[track]=0;

  for(track=0;track<(ginfo->disc.num_tracks-1);track++) {
    if(!numdups[track]) {
      count=0;

      for(track2=track+1;track2<ginfo->disc.num_tracks;track2++) {
	if(!strcmp(ginfo->ddata.data_track[track].track_name,
		   ginfo->ddata.data_track[track2].track_name))
	  numdups[track2]=++count;
      }
    }
  }

  for(track=0;track<ginfo->disc.num_tracks;track++) {
    if(numdups[track]) {
      g_snprintf(buf,260,"%s (%d)",ginfo->ddata.data_track[track].track_name,
		 numdups[track]+1);

      strcpy(ginfo->ddata.data_track[track].track_name,buf);
    }
  }
}

void DoRipEncode(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  DoRip(NULL,(gpointer)ginfo);
}

void DoRip(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;
  gboolean result;

  ginfo=(GripInfo *)data;

  if(widget) ginfo->doencode=FALSE;
  else ginfo->doencode=TRUE;

  if(!ginfo->using_builtin_cdp&&!FileExists(ginfo->ripexename)) {
    DisplayMsg(_("Invalid rip executable\nCheck your rip config"));

    ginfo->doencode=FALSE;
    return;
  }

  if(ginfo->doencode&&!FileExists(ginfo->mp3exename)) {
    DisplayMsg(_("Invalid MP3 executable\nCheck your MP3 config"));

    ginfo->doencode=FALSE;
    return;
  }

  CDStop(&(ginfo->disc));
  ginfo->stopped=TRUE;
    
  if(ginfo->ripping) {
    ginfo->doencode=FALSE;
    return;
  }

  /* Initialize gain calculation */
  if(ginfo->calc_gain) 
    InitGainAnalysis(44100);

  CheckDupNames(ginfo);

  if(ginfo->rip_partial)
    ginfo->rip_track=CURRENT_TRACK;
  else {
    if(ginfo->add_m3u) AddM3U(ginfo);
    SetCurrentTrackIndex(ginfo,0);
    ginfo->rip_track=0;
  }

  result=RipNextTrack(ginfo);
  if(!result) {
    ginfo->doencode=FALSE;
    DisplayMsg(_("No tracks selected"));
  }
}

static int NextTrackToRip(GripInfo *ginfo)
{
  int track;

  for(track=0;(track<ginfo->disc.num_tracks)&&
	(!TrackIsChecked(&(ginfo->gui_info),track)||
	IsDataTrack(&(ginfo->disc),track));track++);

  return track;
}

static gboolean RipNextTrack(GripInfo *ginfo)
{
  GripGUI *uinfo;
  char tmp[PATH_MAX];
  int arg;
  GString *args[100];
  char *char_args[101];
  unsigned long long bytesleft;
  struct stat mystat;
  GString *str;
  EncodeTrack enc_track;

  uinfo=&(ginfo->gui_info);

  Debug(_("In RipNextTrack\n"));

  if(ginfo->ripping) return FALSE;

  if(!ginfo->rip_partial)
    ginfo->rip_track=NextTrackToRip(ginfo);

  Debug(_("First checked track is %d\n"),ginfo->rip_track+1);

  /* See if we are finished ripping */
  if(ginfo->rip_track==ginfo->disc.num_tracks) {
    if(ginfo->calc_gain) {
      ginfo->disc_gain_adjustment=GetAlbumGain();
    }

    if(ginfo->disc_filter_cmd)
      DoDiscFilter(ginfo);

    return FALSE;
  }

  /* We have a track to rip */

  if(ginfo->have_disc&&ginfo->rip_track>=0) {
    Debug(_("Ripping away!\n"));

    if(!ginfo->rip_partial){
      gtk_clist_select_row(GTK_CLIST(uinfo->trackclist),ginfo->rip_track,0);
    }

    CopyPixmap(GTK_PIXMAP(uinfo->rip_pix[0]),GTK_PIXMAP(uinfo->rip_indicator));

    ginfo->rip_started = time(NULL);
    sprintf(tmp,_("Rip: Trk %d (0.0x)"),ginfo->rip_track+1);
    gtk_label_set(GTK_LABEL(uinfo->rip_prog_label),tmp);

    CDStop(&(ginfo->disc));

    if(!ginfo->rip_partial) {
      ginfo->start_sector=0;
      ginfo->end_sector=(ginfo->disc.track[ginfo->rip_track+1].start_frame-1)-
	ginfo->disc.track[ginfo->rip_track].start_frame;

      /* Compensate for the gap before a data track */
      if((ginfo->rip_track<(ginfo->disc.num_tracks-1)&&
	  IsDataTrack(&(ginfo->disc),ginfo->rip_track+1)&&
	  (ginfo->end_sector-ginfo->start_sector)>11399))
	ginfo->end_sector-=11400;
    }

    ginfo->ripsize=44+((ginfo->end_sector-ginfo->start_sector)+1)*2352;

    str=g_string_new(NULL);
    FillInTrackInfo(ginfo,ginfo->rip_track,&enc_track);

    TranslateString(ginfo->ripfileformat,str,TranslateSwitch,
		    &enc_track,&(ginfo->sprefs));

    g_snprintf(ginfo->ripfile,256,"%s",str->str);
    g_string_free(str,TRUE);

    MakeDirs(ginfo->ripfile);

    Debug(_("Ripping track %d to %s\n"),ginfo->rip_track+1,ginfo->ripfile);

    if(stat(ginfo->ripfile,&mystat)>=0) {
      if(mystat.st_size == ginfo->ripsize) { 
	Debug(_("File %s has already been ripped. Skipping...\n"),\
	      ginfo->ripfile);
	ginfo->num_wavs++;
	ginfo->ripping=TRUE;
	ginfo->ripping_a_disc=TRUE;

	return TRUE;
      }
      else unlink(ginfo->ripfile);
    }

    bytesleft=BytesLeftInFS(ginfo->ripfile);

    if(bytesleft<(ginfo->ripsize*1.5)) {
      DisplayMsg(_("Out of space in output directory"));

      return FALSE;
    }

#ifdef CDPAR
    if(ginfo->selected_ripper==0) {
      ginfo->in_rip_thread=TRUE;

      pthread_create(&ginfo->cdp_thread,NULL,(void *)ThreadRip,
		     (void *)ginfo);
      pthread_detach(ginfo->cdp_thread);
    }
    else {
#endif
      strcpy(enc_track.wav_filename,ginfo->ripfile);

      MakeTranslatedArgs(ginfo->ripcmdline,args,100,TranslateSwitch,
			 &enc_track,&(ginfo->sprefs));

      for(arg=0;args[arg];arg++) {
	char_args[arg+1]=args[arg]->str;
      }

      char_args[arg+1]=NULL;

      char_args[0]=FindRoot(ginfo->ripexename);

      ginfo->rippid=fork();
      
      if(ginfo->rippid==0) {
	CloseStuff(ginfo);
	nice(ginfo->ripnice);
	execv(ginfo->ripexename,char_args);
	
	Debug(_("Exec failed\n"));
	_exit(0);
      }

      for(arg=0;args[arg];arg++) {
	g_string_free(args[arg],TRUE);
      }
#ifdef CDPAR
    }
#endif

    ginfo->ripping=TRUE;
    ginfo->ripping_a_disc=TRUE;

    ginfo->num_wavs++;

    return TRUE;
  }
  else return FALSE;
}

#ifdef CDPAR
static void ThreadRip(void *arg)
{
  GripInfo *ginfo;
  int paranoia_mode;

  ginfo=(GripInfo *)arg;

  Debug(_("Calling CDPRip\n"));

  paranoia_mode=PARANOIA_MODE_FULL^PARANOIA_MODE_NEVERSKIP; 
  if(ginfo->disable_paranoia)
    paranoia_mode=PARANOIA_MODE_DISABLE;
  else if(ginfo->disable_extra_paranoia) {
    paranoia_mode|=PARANOIA_MODE_OVERLAP;
    paranoia_mode&=~PARANOIA_MODE_VERIFY;
  }
  if(ginfo->disable_scratch_detect)
    paranoia_mode&=
      ~(PARANOIA_MODE_SCRATCH|PARANOIA_MODE_REPAIR);
  if(ginfo->disable_scratch_repair) 
    paranoia_mode&=~PARANOIA_MODE_REPAIR;
  
  ginfo->rip_smile_level=0;
  
  CDPRip(ginfo->cd_device,ginfo->force_scsi,ginfo->rip_track+1,
	 ginfo->start_sector,
	 ginfo->end_sector,ginfo->ripfile,paranoia_mode,
	 &(ginfo->rip_smile_level),&(ginfo->rip_percent_done),
	 &(ginfo->stop_thread_rip_now),ginfo->calc_gain);
  
  ginfo->in_rip_thread=FALSE;

  pthread_exit(0);
}
#endif /* ifdef CDPAR */

void FillInTrackInfo(GripInfo *ginfo,int track,EncodeTrack *new_track)
{
  new_track->ginfo=ginfo;

  new_track->wav_filename[0]='\0';
  new_track->mp3_filename[0]='\0';

  new_track->track_num=track;
  new_track->start_frame=ginfo->disc.track[track].start_frame;
  new_track->end_frame=ginfo->disc.track[track+1].start_frame-1;
  new_track->track_gain_adjustment=ginfo->track_gain_adjustment;
  new_track->disc_gain_adjustment=ginfo->disc_gain_adjustment;

  /* Compensate for the gap before a data track */
  if((track<(ginfo->disc.num_tracks-1)&&
      IsDataTrack(&(ginfo->disc),track+1)&&
      (new_track->end_frame-new_track->start_frame)>11399))
    new_track->end_frame-=11400;

  new_track->mins=ginfo->disc.track[track].length.mins;
  new_track->secs=ginfo->disc.track[track].length.secs;
  new_track->song_year=ginfo->ddata.data_year;
  g_snprintf(new_track->song_name,256,"%s",
	     ginfo->ddata.data_track[track].track_name);
  g_snprintf(new_track->song_artist,256,"%s",
	     ginfo->ddata.data_track[track].track_artist);
  g_snprintf(new_track->disc_name,256,"%s",ginfo->ddata.data_title);
  g_snprintf(new_track->disc_artist,256,"%s",ginfo->ddata.data_artist);
  new_track->id3_genre=ginfo->ddata.data_id3genre;
  new_track->discid=ginfo->ddata.data_id;
}

static void AddToEncode(GripInfo *ginfo,int track)
{
  EncodeTrack *new_track;

  new_track=(EncodeTrack *)g_new(EncodeTrack,1);

  FillInTrackInfo(ginfo,track,new_track);
  strcpy(new_track->wav_filename,ginfo->ripfile);

  ginfo->encode_list=g_list_append(ginfo->encode_list,new_track);

  Debug(_("Added track %d to encode list\n"),track+1);
}

static gboolean MP3Encode(GripInfo *ginfo)
{
  GripGUI *uinfo;
  char tmp[PATH_MAX];
  int arg;
  GString *args[100];
  char *char_args[101];
  unsigned long long bytesleft;
  EncodeTrack *enc_track;
  GString *str;
  int encode_track;
  int cpu;

  uinfo=&(ginfo->gui_info);

  if(!ginfo->encode_list) return FALSE;

  for(cpu=0;(cpu<ginfo->num_cpu)&&(ginfo->encoding&(1<<cpu));cpu++);

  if(cpu==ginfo->num_cpu) {
    Debug(_("No free cpus\n"));
    return FALSE;
  }

  enc_track=(EncodeTrack *)(g_list_first(ginfo->encode_list)->data);
  encode_track=enc_track->track_num;

  ginfo->encode_list=g_list_remove(ginfo->encode_list,enc_track);
  ginfo->encoded_track[cpu]=enc_track;

  CopyPixmap(GTK_PIXMAP(uinfo->mp3_pix[0]),
	     GTK_PIXMAP(uinfo->mp3_indicator[cpu]));
  
  ginfo->mp3_started[cpu] = time(NULL);
  ginfo->mp3_enc_track[cpu] = encode_track;

  sprintf(tmp,_("MP3: Trk %d (0.0x)"),encode_track+1);
  gtk_label_set(GTK_LABEL(uinfo->mp3_prog_label[cpu]),tmp);
  
  Debug(_("MP3 track %d\n"),encode_track+1);

  strcpy(ginfo->rip_delete_file[cpu],enc_track->wav_filename);

  str=g_string_new(NULL);

  TranslateString(ginfo->mp3fileformat,str,TranslateSwitch,
		  enc_track,&(ginfo->sprefs));

  g_snprintf(ginfo->mp3file[cpu],256,"%s",str->str);
  g_string_free(str,TRUE);

  MakeDirs(ginfo->mp3file[cpu]);
  
  bytesleft=BytesLeftInFS(ginfo->mp3file[cpu]);
  
  Debug(_("%i: Encoding to %s\n"),cpu+1,ginfo->mp3file[cpu]);
  
  unlink(ginfo->mp3file[cpu]);
  
  ginfo->mp3size[cpu]=
    (int)((gfloat)((enc_track->end_frame-enc_track->start_frame)+1)*
	  (gfloat)(ginfo->kbits_per_sec*1024)/600.0);
  
  if(bytesleft<(ginfo->mp3size[cpu]*1.5)) {
    DisplayMsg(_("Out of space in output directory"));
    
    return FALSE;
  }
  
  strcpy(enc_track->mp3_filename,ginfo->mp3file[cpu]);

  MakeTranslatedArgs(ginfo->mp3cmdline,args,100,TranslateSwitch,
		     enc_track,&(ginfo->sprefs));
  
  for(arg=0;args[arg];arg++) {
    char_args[arg+1]=args[arg]->str;
  }
  
  char_args[arg+1]=NULL;
  
  char_args[0]=FindRoot(ginfo->mp3exename);

  ginfo->mp3pid[cpu]=fork();
  
  if(ginfo->mp3pid[cpu]==0) {
    CloseStuff(ginfo);
    setsid();
    nice(ginfo->mp3nice);
    execv(ginfo->mp3exename,char_args);
    _exit(0);
  }

  for(arg=0;args[arg];arg++) {
    g_string_free(args[arg],TRUE);
  }

  ginfo->encoding|=(1<<cpu);

  return TRUE;
}
