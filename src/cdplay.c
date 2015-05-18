/* cdplay.c
 *
 * Copyright (c) 1998-2001  Mike Oliphant <oliphant@gtk.org>
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

#include "cdplay.h"
#include "grip.h"
#include "config.h"
#include "common.h"
#include "discdb.h"
#include "cddev.h"
#include "discedit.h"
#include "dialog.h"
#include "rip.h"
#include "grip_id3.h"

static void ShutDownCB(GtkWidget *widget,gpointer data);
static void DiscDBToggle(GtkWidget *widget,gpointer data);
static void DoLookup(void *data);
static void SetCurrentTrack(GripInfo *ginfo,int track);
static void ToggleChecked(GripGUI *uinfo,int track);
static void ClickColumn(GtkWidget *widget,gint column,gpointer data);
static void CListButtonPressed(GtkWidget *widget,GdkEventButton *event,
			       gpointer data);
static void UnSelectRow(GtkWidget *widget,gint row,gint column,
			GdkEventButton *event,gpointer data);
static void SelectRow(GtkWidget *widget,gint row,gint column,
		      GdkEventButton *event,gpointer data);
static void PlaylistChanged(GtkWindow *window,GtkWidget *widget,gpointer data);
static void ToggleLoop(GtkWidget *widget,gpointer data);
static void ChangePlayMode(GtkWidget *widget,gpointer data);
static void ChangeTimeMode(GtkWidget *widget,gpointer data);
static void MinMax(GtkWidget *widget,gpointer data);
static void ToggleProg(GtkWidget *widget,gpointer data);
static void ToggleControlButtons(GtkWidget *widget,GdkEventButton *event,
				 gpointer data);
static void ToggleVol(GtkWidget *widget,gpointer data);
static void SetVolume(GtkWidget *widget,gpointer data);
static void FastFwdCB(GtkWidget *widget,gpointer data);
static void RewindCB(GtkWidget *widget,gpointer data);
static void NextDisc(GtkWidget *widget,gpointer data);
static void StopPlayCB(GtkWidget *widget,gpointer data);
static void PlayTrackCB(GtkWidget *widget,gpointer data);
static void PlayTrack(GripInfo *ginfo,int track);
static void NextTrackCB(GtkWidget *widget,gpointer data);
static void PrevTrackCB(GtkWidget *widget,gpointer data);
static void PrevTrack(GripInfo *ginfo);
static void InitProgram(GripInfo *ginfo);
static void ShuffleTracks(GripInfo *ginfo);
static gboolean CheckTracks(DiscInfo *disc);

static void ShutDownCB(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  GripDie(ginfo->gui_info.app,NULL);
}

static void DiscDBToggle(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;
#if defined(__sun__)
  void    *status;
#endif

  ginfo=(GripInfo *)data;
  
  if(ginfo->looking_up) {
#if defined(__sun__)
    pthread_exit(&status);
#elif defined(__FreeBSD__)
    pthread_kill(ginfo->discdb_thread,0);
#else
    pthread_kill_other_threads_np();
#endif
    Debug(_("Aborted\n"));

    ginfo->looking_up=FALSE;
    ginfo->update_required=TRUE;
  }
  else {
#ifndef GRIPCD
    if(ginfo->ripping_a_disc) {
      DisplayMsg(_("Cannot do lookup while ripping"));

      return;
    }
#endif
 
    if(ginfo->have_disc)
      LookupDisc(ginfo,TRUE);
  }
}

void LookupDisc(GripInfo *ginfo,gboolean manual)
{
  int track;
  gboolean present;
  DiscInfo *disc;
  DiscData *ddata;

  disc=&(ginfo->disc);
  ddata=&(ginfo->ddata);

  ddata->data_multi_artist=FALSE;
  ddata->data_year=0;

  present=DiscDBStatDiscData(disc);

  if(!manual&&present) {
    DiscDBReadDiscData(disc,ddata);
    if(ginfo->ddata.data_id3genre==-1)
      ginfo->ddata.data_id3genre=DiscDB2ID3(ginfo->ddata.data_genre);

    ginfo->update_required=TRUE;
    ginfo->is_new_disc=TRUE;
  }
  else {
    if(!manual) {
      ddata->data_id=DiscDBDiscid(disc);
      ddata->data_genre=7; /* "misc" */
      strcpy(ddata->data_title,_("Unknown Disc"));
      strcpy(ddata->data_artist,"");
      
      for(track=0;track<disc->num_tracks;track++) {
	sprintf(ddata->data_track[track].track_name,_("Track %02d"),track+1);
	*(ddata->data_track[track].track_artist)='\0';
	*(ddata->data_track[track].track_extended)='\0';
	*(ddata->data_playlist)='\0';
      }

      *ddata->data_extended='\0';
      
      ginfo->update_required=TRUE;
    }

    if(!ginfo->local_mode && (manual?TRUE:ginfo->automatic_discdb)) {
      ginfo->looking_up=TRUE;
      
      pthread_create(&(ginfo->discdb_thread),NULL,(void *)&DoLookup,
		     (void *)ginfo);
      pthread_detach(ginfo->discdb_thread);
    }
  }
}

static void DoLookup(void *data)
{
  GripInfo *ginfo;
  gboolean discdb_found=FALSE;

  ginfo=(GripInfo *)data;

  if(!DiscDBLookupDisc(ginfo,&(ginfo->dbserver))) {
    if(*(ginfo->dbserver2.name)) {
      if(DiscDBLookupDisc(ginfo,&(ginfo->dbserver2))) {
        discdb_found=TRUE;
        ginfo->ask_submit=TRUE;
      }
    }
  }
  else {
    discdb_found=TRUE;
  }

  if(ginfo->ddata.data_id3genre==-1)
    ginfo->ddata.data_id3genre=DiscDB2ID3(ginfo->ddata.data_genre);

  /* Deleted this since we aren't in the main thread */
  /*  if(!discdb_found)
      DisplayMsg(_("Disc database query failed\n"));*/

  ginfo->looking_up=FALSE;
  pthread_exit(0);
}

gboolean DiscDBLookupDisc(GripInfo *ginfo,DiscDBServer *server)
{
  DiscDBHello hello;
  DiscDBQuery query;
  DiscDBEntry entry;
  gboolean success=FALSE;
  DiscInfo *disc;
  DiscData *ddata;

  disc=&(ginfo->disc);
  ddata=&(ginfo->ddata);

  if(server->use_proxy)
    Debug(_("Querying %s (through %s) for disc %02x.\n"),server->name,
	   server->proxy->name,
	   DiscDBDiscid(disc));
  else
    Debug(_("Querying %s for disc %02x.\n"),server->name,
	   DiscDBDiscid(disc));

  strncpy(hello.hello_program,"Grip",256);
  strncpy(hello.hello_version,VERSION,256);
	
  if(!DiscDBDoQuery(disc,server,&hello,&query)) {
    ginfo->update_required=TRUE;
  } else {
    switch(query.query_match) {
    case MATCH_INEXACT:
    case MATCH_EXACT:
      Debug(_("Match for \"%s / %s\"\nDownloading data...\n"),
	     query.query_list[0].list_artist,
	     query.query_list[0].list_title);
      entry.entry_genre = query.query_list[0].list_genre;
      entry.entry_id = query.query_list[0].list_id;
      DiscDBRead(disc,server,&hello,&entry,ddata);

      Debug(_("Done\n"));
      success=TRUE;
		
      if(DiscDBWriteDiscData(disc,ddata,NULL,TRUE,FALSE)<0)
	printf(_("Error saving disc data\n"));

      ginfo->update_required=TRUE;
      ginfo->is_new_disc=TRUE;
      break;
    case MATCH_NOMATCH:
      Debug(_("No match\n"));
      break;
    }
  }

  return success;
}

void MakeTrackPage(GripInfo *ginfo)
{
  GtkWidget *trackpage;
  GtkWidget *vbox;
  GripGUI *uinfo;
  GtkRequisition sizereq;
#ifdef GTK_HAVE_FEATURES_1_1_4
  GtkWidget *scroll;
#endif
#ifndef GRIPCD
  gchar *titles[3]={_("Track"),_("Length "),_("Rip")};
#else
  gchar *titles[3]={_("Track"),_("Length "),_("PL")};
#endif
  
  uinfo=&(ginfo->gui_info);

  trackpage=MakeNewPage(uinfo->notebook,_("Tracks"));

  vbox=gtk_vbox_new(FALSE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  uinfo->disc_name_label=gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(vbox),uinfo->disc_name_label,FALSE,FALSE,0);
  gtk_widget_show(uinfo->disc_name_label);

  uinfo->disc_artist_label=gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(vbox),uinfo->disc_artist_label,FALSE,FALSE,0);
  gtk_widget_show(uinfo->disc_artist_label);

#ifndef GRIPCD
  uinfo->trackclist=gtk_clist_new_with_titles(3,titles);
#else
  uinfo->trackclist=gtk_clist_new_with_titles(2,titles);
#endif

  gtk_clist_set_column_justification(GTK_CLIST(uinfo->trackclist),1,
				     GTK_JUSTIFY_RIGHT);

  gtk_clist_set_selection_mode(GTK_CLIST(uinfo->trackclist),
			       GTK_SELECTION_BROWSE);
  gtk_clist_column_title_passive(GTK_CLIST(uinfo->trackclist),0);
  gtk_clist_column_title_passive(GTK_CLIST(uinfo->trackclist),1);
  gtk_clist_set_column_width(GTK_CLIST(uinfo->trackclist),0,
			     SizeInDubs(uinfo->trackclist->style->font,18));
#ifndef GRIPCD
  gtk_clist_column_title_active(GTK_CLIST(uinfo->trackclist),2);

  gtk_clist_set_column_auto_resize(GTK_CLIST(uinfo->trackclist),1,TRUE);
  gtk_clist_set_column_justification(GTK_CLIST(uinfo->trackclist),2,
				     GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_resizeable(GTK_CLIST(uinfo->trackclist),2,FALSE);
#endif

  gtk_signal_connect(GTK_OBJECT(uinfo->trackclist),"select_row",
		     GTK_SIGNAL_FUNC(SelectRow),
		     (gpointer)ginfo);
  gtk_signal_connect(GTK_OBJECT(uinfo->trackclist),"unselect_row",
		     GTK_SIGNAL_FUNC(UnSelectRow),
		     (gpointer)uinfo);
  
  gtk_signal_connect(GTK_OBJECT(uinfo->trackclist),"button_press_event",
		     GTK_SIGNAL_FUNC(CListButtonPressed),(gpointer)uinfo);

#ifndef GRIPCD
  gtk_signal_connect(GTK_OBJECT(uinfo->trackclist),"click_column",
		     GTK_SIGNAL_FUNC(ClickColumn),(gpointer)ginfo);
#endif

  gtk_widget_size_request(uinfo->trackclist,&sizereq);

#ifndef GTK_HAVE_FEATURES_1_1_4
  gtk_clist_set_policy(GTK_CLIST(uinfo->trackclist),GTK_POLICY_AUTOMATIC,
		       GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start(GTK_BOX(vbox),uinfo->trackclist,TRUE,TRUE,0);
#else
  scroll=gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				 GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(scroll),uinfo->trackclist);
  gtk_box_pack_start(GTK_BOX(vbox),scroll,TRUE,TRUE,0);

  gtk_widget_show(scroll);
#endif

  gtk_widget_show(uinfo->trackclist);

  gtk_widget_set_usize(trackpage,sizereq.width+30,-1);
  gtk_container_add(GTK_CONTAINER(trackpage),vbox);
  gtk_widget_show(vbox);
}

void SetCurrentTrackIndex(GripInfo *ginfo,int track)
{
  /* Looks up the track of index track in the program */
  for(ginfo->current_track_index = 0;
      (ginfo->current_track_index < MAX_TRACKS)
	&& (ginfo->current_track_index < ginfo->prog_totaltracks)
	&& (CURRENT_TRACK != track);
      ginfo->current_track_index++)
    continue;
}

static void SetCurrentTrack(GripInfo *ginfo,int track)
{
  char buf[256];
#ifndef GRIPCD
  int tracklen;
#endif
  GripGUI *uinfo;

  uinfo=&(ginfo->gui_info);

  if(track<0) {
    gtk_label_set(GTK_LABEL(uinfo->current_track_label),"--");
#ifndef GRIPCD
    gtk_entry_set_text(GTK_ENTRY(uinfo->start_sector_entry),"0");
    gtk_entry_set_text(GTK_ENTRY(uinfo->end_sector_entry),"0");
#endif

    gtk_signal_handler_block_by_func(GTK_OBJECT(uinfo->track_edit_entry),
				     TrackEditChanged,(gpointer)ginfo);
    gtk_entry_set_text(GTK_ENTRY(uinfo->track_edit_entry),"");
    
    gtk_signal_handler_unblock_by_func(GTK_OBJECT(uinfo->track_edit_entry),
	 			       TrackEditChanged,(gpointer)ginfo);
    
    gtk_signal_handler_block_by_func(GTK_OBJECT(uinfo->
						track_artist_edit_entry),
	 			     TrackEditChanged,(gpointer)ginfo);
    
    gtk_entry_set_text(GTK_ENTRY(uinfo->track_artist_edit_entry),"");
    
    gtk_signal_handler_unblock_by_func(GTK_OBJECT(uinfo->
						  track_artist_edit_entry),
	 			       TrackEditChanged,(gpointer)ginfo);
  }
  else {
    gtk_signal_handler_block_by_func(GTK_OBJECT(uinfo->track_edit_entry),
				     TrackEditChanged,(gpointer)ginfo);
    gtk_entry_set_text(GTK_ENTRY(uinfo->track_edit_entry),
 		       ginfo->ddata.data_track[track].track_name);

    gtk_signal_handler_unblock_by_func(GTK_OBJECT(uinfo->track_edit_entry),
	 			       TrackEditChanged,(gpointer)ginfo);

    gtk_signal_handler_block_by_func(GTK_OBJECT(uinfo->
						track_artist_edit_entry),
	 			     TrackEditChanged,(gpointer)ginfo);

    gtk_entry_set_text(GTK_ENTRY(uinfo->track_artist_edit_entry),
	 	       ginfo->ddata.data_track[track].track_artist);

    gtk_signal_handler_unblock_by_func(GTK_OBJECT(uinfo->
						  track_artist_edit_entry),
	 			       TrackEditChanged,(gpointer)ginfo);
    g_snprintf(buf,80,"%02d",track+1);
    gtk_label_set(GTK_LABEL(uinfo->current_track_label),buf);
	
#ifndef GRIPCD
    gtk_entry_set_text(GTK_ENTRY(uinfo->start_sector_entry),"0");
	
    tracklen=(ginfo->disc.track[track+1].start_frame-1)-
      ginfo->disc.track[track].start_frame;
    g_snprintf(buf,80,"%d",tracklen);
    gtk_entry_set_text(GTK_ENTRY(uinfo->end_sector_entry),buf);
#endif

    SetCurrentTrackIndex(ginfo,track);
  }
}

#ifndef GRIPCD

gboolean TrackIsChecked(GripGUI *uinfo,int track)
{
  GdkPixmap *pm=NULL;
  GdkBitmap *bm=NULL;

  gtk_clist_get_pixmap(GTK_CLIST(uinfo->trackclist),track,2,&pm,&bm);
  return((pm==GTK_PIXMAP(uinfo->check_image)->pixmap));
}

static void ToggleChecked(GripGUI *uinfo,int track)
{
  SetChecked(uinfo,track,!TrackIsChecked(uinfo,track));
}

void SetChecked(GripGUI *uinfo,int track,gboolean checked)
{
  if(!checked)
    gtk_clist_set_pixmap(GTK_CLIST(uinfo->trackclist),track,2,
			 GTK_PIXMAP(uinfo->empty_image)->pixmap,
			 GTK_PIXMAP(uinfo->empty_image)->mask);
  else {
    gtk_clist_set_pixmap(GTK_CLIST(uinfo->trackclist),track,2,
			 GTK_PIXMAP(uinfo->check_image)->pixmap,
			 GTK_PIXMAP(uinfo->check_image)->mask);
  }
}

static void ClickColumn(GtkWidget *widget,gint column,gpointer data)
{
  int track;
  int numsel=0;
  gboolean check;
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  for(track=0;track<ginfo->disc.num_tracks;track++)
    if(TrackIsChecked(&(ginfo->gui_info),track)) numsel++;

  if(ginfo->disc.num_tracks>1) {
    check=(numsel<ginfo->disc.num_tracks/2);
  }
  else {
    check=(numsel==0);
  }

  for(track=0;track<ginfo->disc.num_tracks;track++)
    SetChecked(&(ginfo->gui_info),track,check);
}
#endif

static void CListButtonPressed(GtkWidget *widget,GdkEventButton *event,
			       gpointer data)
{
  gint row,col;
  GripGUI *uinfo;

  uinfo=(GripGUI *)data;
  
  if(event) {
    gtk_clist_get_selection_info(GTK_CLIST(uinfo->trackclist),
				 event->x,event->y,
				 &row,&col);
    Debug(_("Column/Button: %d/%d\n"),col,event->button);


    if((col==2&&event->button<4) || event->button==3) {
      
#ifndef GRIPCD
      ToggleChecked(uinfo,row);
#endif
    }
  }
}

static void UnSelectRow(GtkWidget *widget,gint row,gint column,
			GdkEventButton *event,gpointer data)
{
  GripGUI *uinfo;

  uinfo=(GripGUI *)data;

#ifndef GRIPCD
  if(TrackIsChecked(uinfo,row))
    gtk_clist_set_pixmap(GTK_CLIST(uinfo->trackclist),row,2,
			 GTK_PIXMAP(uinfo->check_image)->pixmap,
			 GTK_PIXMAP(uinfo->check_image)->mask);
#endif
}

static void SelectRow(GtkWidget *widget,gint row,gint column,
		      GdkEventButton *event,gpointer data)
{
  GripInfo *ginfo;
  GripGUI *uinfo;

  ginfo=(GripInfo *)data;
  uinfo=&(ginfo->gui_info);

  SetCurrentTrack(ginfo,row);

#ifndef GRIPCD
  if(TrackIsChecked(uinfo,row))
    gtk_clist_set_pixmap(GTK_CLIST(uinfo->trackclist),row,2,
			 GTK_PIXMAP(uinfo->check_image)->pixmap,
			 GTK_PIXMAP(uinfo->check_image)->mask);
#endif

  if(gtk_clist_row_is_visible(GTK_CLIST(uinfo->trackclist),row)
     !=GTK_VISIBILITY_FULL)
    gtk_clist_moveto(GTK_CLIST(uinfo->trackclist),row,0,0,0);

  if((ginfo->disc.disc_mode==CDAUDIO_PLAYING)&&
     (ginfo->disc.curr_track!=(row+1)))
    PlayTrack(ginfo,row);
  else {
    if(event) {
      switch(event->type) {
      case GDK_2BUTTON_PRESS:
	PlayTrack(ginfo,row);
	break;
      default:
	break;
      }
    }
  }
}

static void PlaylistChanged(GtkWindow *window,GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  strcpy(ginfo->ddata.data_playlist,
	 gtk_entry_get_text(GTK_ENTRY(ginfo->gui_info.playlist_entry)));

  InitProgram(ginfo);

  if(DiscDBWriteDiscData(&(ginfo->disc),&(ginfo->ddata),NULL,TRUE,FALSE)<0)
    DisplayMsg(_("Error saving disc data\n"));
}

static void ToggleLoop(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  ginfo->playloop=!ginfo->playloop;

  if(ginfo->playloop) 
    CopyPixmap(GTK_PIXMAP(ginfo->gui_info.loop_image),\
	       GTK_PIXMAP(ginfo->gui_info.loop_indicator));
  else
    CopyPixmap(GTK_PIXMAP(ginfo->gui_info.noloop_image),
	       GTK_PIXMAP(ginfo->gui_info.loop_indicator));

}

static void ChangePlayMode(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  ginfo->play_mode=(ginfo->play_mode+1)%PM_LASTMODE;

  CopyPixmap(GTK_PIXMAP(ginfo->gui_info.play_pix[ginfo->play_mode]),
	     GTK_PIXMAP(ginfo->gui_info.play_indicator));

  gtk_widget_set_sensitive(GTK_WIDGET(ginfo->gui_info.playlist_entry),
			   ginfo->play_mode==PM_PLAYLIST);

  InitProgram(ginfo);
}

GtkWidget *MakePlayOpts(GripInfo *ginfo)
{
  GripGUI *uinfo;
  GtkWidget *ebox;
  GtkWidget *hbox;
  GtkWidget *button;

  uinfo=&(ginfo->gui_info);

  ebox=gtk_event_box_new();
  gtk_widget_set_style(ebox,uinfo->style_wb);

  hbox=gtk_hbox_new(FALSE,2);

  uinfo->playlist_entry=gtk_entry_new_with_max_length(256);
  gtk_signal_connect(GTK_OBJECT(uinfo->playlist_entry),"focus_out_event",
  		     GTK_SIGNAL_FUNC(PlaylistChanged),(gpointer)ginfo);
  gtk_widget_set_sensitive(GTK_WIDGET(uinfo->playlist_entry),
			   ginfo->play_mode==PM_PLAYLIST);
  gtk_box_pack_start(GTK_BOX(hbox),uinfo->playlist_entry,TRUE,TRUE,0);
  gtk_widget_show(uinfo->playlist_entry);

  uinfo->play_indicator=NewBlankPixmap(uinfo->app);
  CopyPixmap(GTK_PIXMAP(uinfo->play_pix[ginfo->play_mode]),
	     GTK_PIXMAP(uinfo->play_indicator));

  button=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(button),uinfo->play_indicator);
  gtk_widget_show(uinfo->play_indicator);
  gtk_widget_set_style(button,uinfo->style_dark_grey);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
  		     GTK_SIGNAL_FUNC(ChangePlayMode),(gpointer)ginfo);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Rotate play mode"),NULL);
  gtk_widget_show(button);

  uinfo->loop_indicator=NewBlankPixmap(uinfo->app);

  if(ginfo->playloop)
    CopyPixmap(GTK_PIXMAP(uinfo->loop_image),
	       GTK_PIXMAP(uinfo->loop_indicator));
  else
    CopyPixmap(GTK_PIXMAP(uinfo->noloop_image),
	       GTK_PIXMAP(uinfo->loop_indicator));

  button=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(button),uinfo->loop_indicator);
  gtk_widget_show(uinfo->loop_indicator);
  gtk_widget_set_style(button,uinfo->style_dark_grey);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
  		     GTK_SIGNAL_FUNC(ToggleLoop),(gpointer)ginfo);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Toggle loop play"),NULL);
  gtk_widget_show(button);

  gtk_container_add(GTK_CONTAINER(ebox),hbox);
  gtk_widget_show(hbox);

  return ebox;
}

GtkWidget *MakeControls(GripInfo *ginfo)
{
  GripGUI *uinfo;
  GtkWidget *vbox,*vbox3,*hbox,*imagebox,*hbox2;
  GtkWidget *indicator_box;
  GtkWidget *button;
  GtkWidget *ebox,*lcdbox;
  GtkObject *adj;
#ifndef GRIPCD
  int mycpu;
#endif

  uinfo=&(ginfo->gui_info);

  ebox=gtk_event_box_new();
  gtk_widget_set_style(ebox,uinfo->style_wb);

  vbox=gtk_vbox_new(FALSE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox),0);

  vbox3=gtk_vbox_new(FALSE,2);
  gtk_container_border_width(GTK_CONTAINER(vbox3),2);

  lcdbox=gtk_event_box_new();
  gtk_signal_connect(GTK_OBJECT(lcdbox),"button_press_event",
		     GTK_SIGNAL_FUNC(ToggleControlButtons),(gpointer)ginfo);
  gtk_widget_set_style(lcdbox,uinfo->style_LCD);

  hbox2=gtk_hbox_new(FALSE,0);

  imagebox=gtk_vbox_new(FALSE,0);

  gtk_box_pack_start(GTK_BOX(imagebox),uinfo->upleft_image,FALSE,FALSE,0);
  gtk_widget_show(uinfo->upleft_image);

  gtk_box_pack_end(GTK_BOX(imagebox),uinfo->lowleft_image,FALSE,FALSE,0);
  gtk_widget_show(uinfo->lowleft_image);

  gtk_box_pack_start(GTK_BOX(hbox2),imagebox,FALSE,FALSE,0);
  gtk_widget_show(imagebox);
  
  hbox=gtk_hbox_new(TRUE,0);
  gtk_container_border_width(GTK_CONTAINER(hbox),0);

  uinfo->current_track_label=gtk_label_new("--");
  gtk_box_pack_start(GTK_BOX(hbox),uinfo->current_track_label,FALSE,FALSE,0);
  gtk_widget_show(uinfo->current_track_label);

  button=gtk_button_new();
  gtk_widget_set_style(button,uinfo->style_LCD);
#if (GTK_MINOR_VERSION != 0)
  gtk_button_set_relief(GTK_BUTTON(button),GTK_RELIEF_NONE);
#endif
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(ChangeTimeMode),(gpointer)ginfo);

  uinfo->play_time_label=gtk_label_new("--:--");
  gtk_container_add(GTK_CONTAINER(button),uinfo->play_time_label);
  gtk_widget_show(uinfo->play_time_label);

  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  indicator_box=gtk_hbox_new(TRUE,0);

#ifndef GRIPCD
  uinfo->rip_indicator=NewBlankPixmap(GTK_WIDGET(uinfo->app));
#if (GTK_MINOR_VERSION != 0)
  gtk_box_pack_start(GTK_BOX(indicator_box),uinfo->rip_indicator,TRUE,TRUE,0);
  gtk_widget_show(uinfo->rip_indicator);
#endif

#ifndef GRIPCD
  uinfo->lcd_smile_indicator=NewBlankPixmap(GTK_WIDGET(uinfo->app));
  gtk_tooltips_set_tip(MakeToolTip(),uinfo->lcd_smile_indicator,
		       _("Rip status"),NULL);
  gtk_box_pack_start(GTK_BOX(indicator_box),uinfo->lcd_smile_indicator,
		     TRUE,TRUE,0);
  gtk_widget_show(uinfo->lcd_smile_indicator);
#endif

  for(mycpu=0;mycpu<ginfo->num_cpu;mycpu++){
    uinfo->mp3_indicator[mycpu]=NewBlankPixmap(GTK_WIDGET(uinfo->app));
#if (GTK_MINOR_VERSION != 0)
    gtk_box_pack_start(GTK_BOX(indicator_box),
		       uinfo->mp3_indicator[mycpu],TRUE,TRUE,0);
#endif
    gtk_widget_show(uinfo->mp3_indicator[mycpu]);
  }
#endif
  
  uinfo->discdb_indicator=NewBlankPixmap(GTK_WIDGET(uinfo->app));
#if (GTK_MINOR_VERSION != 0)
  gtk_box_pack_start(GTK_BOX(indicator_box),uinfo->discdb_indicator,
		     TRUE,TRUE,0);
  gtk_widget_show(uinfo->discdb_indicator);
#endif  
  gtk_box_pack_start(GTK_BOX(hbox),indicator_box,TRUE,TRUE,0);
  gtk_widget_show(indicator_box);

  gtk_container_add(GTK_CONTAINER(hbox2),hbox);
  gtk_widget_show(hbox);

  imagebox=gtk_vbox_new(FALSE,0);

  gtk_box_pack_start(GTK_BOX(imagebox),uinfo->upright_image,FALSE,FALSE,0);
  gtk_widget_show(uinfo->upright_image);

  gtk_box_pack_end(GTK_BOX(imagebox),uinfo->lowright_image,FALSE,FALSE,0);
  gtk_widget_show(uinfo->lowright_image);

  gtk_box_pack_start(GTK_BOX(hbox2),imagebox,FALSE,FALSE,0);
  gtk_widget_show(imagebox);
  
  gtk_container_add(GTK_CONTAINER(lcdbox),hbox2);
  gtk_widget_show(hbox2);

  gtk_box_pack_start(GTK_BOX(vbox3),lcdbox,FALSE,FALSE,0);
  gtk_widget_show(lcdbox);

  gtk_box_pack_start(GTK_BOX(vbox),vbox3,FALSE,FALSE,0);
  gtk_widget_show(vbox3);

  adj=gtk_adjustment_new((gfloat)ginfo->volume,0.0,255.0,1.0,1.0,0.0);
  gtk_signal_connect(adj,"value_changed",
  		     GTK_SIGNAL_FUNC(SetVolume),(gpointer)ginfo);
  uinfo->volume_control=gtk_hscale_new(GTK_ADJUSTMENT(adj));

  gtk_scale_set_draw_value(GTK_SCALE(uinfo->volume_control),FALSE);
  gtk_widget_set_name(uinfo->volume_control,"darkgrey");
  gtk_box_pack_start(GTK_BOX(vbox),uinfo->volume_control,FALSE,FALSE,0);

  /*  CDGetVolume(cd_desc,&vol);
  gtk_adjustment_set_value(GTK_ADJUSTMENT(adj),(vol.vol_front.left+
  vol.vol_front.right)/2);*/

  if(uinfo->volvis) gtk_widget_show(uinfo->volume_control);

  uinfo->control_button_box=gtk_vbox_new(TRUE,0);

  hbox=gtk_hbox_new(TRUE,0);

  button=ImageButton(GTK_WIDGET(uinfo->app),uinfo->playpaus_image);
  gtk_widget_set_style(button,uinfo->style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
  		     GTK_SIGNAL_FUNC(PlayTrackCB),(gpointer)ginfo);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Play track / Pause play"),NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  button=ImageButton(GTK_WIDGET(uinfo->app),uinfo->rew_image);
  gtk_widget_set_style(button,uinfo->style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"pressed",
  		     GTK_SIGNAL_FUNC(RewindCB),(gpointer)ginfo);
  gtk_signal_connect(GTK_OBJECT(button),"released",
  		     GTK_SIGNAL_FUNC(RewindCB),(gpointer)ginfo);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Rewind"),NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  button=ImageButton(GTK_WIDGET(uinfo->app),uinfo->ff_image);
  gtk_widget_set_style(button,uinfo->style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"pressed",
  		     GTK_SIGNAL_FUNC(FastFwdCB),(gpointer)ginfo);
  gtk_signal_connect(GTK_OBJECT(button),"released",
  		     GTK_SIGNAL_FUNC(FastFwdCB),(gpointer)ginfo);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("FastForward"),NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  button=ImageButton(GTK_WIDGET(uinfo->app),uinfo->prevtrk_image);
  gtk_widget_set_style(button,uinfo->style_dark_grey);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
  		     GTK_SIGNAL_FUNC(PrevTrackCB),(gpointer)ginfo);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Go to previous track"),NULL);
  gtk_widget_show(button);

  button=ImageButton(GTK_WIDGET(uinfo->app),uinfo->nexttrk_image);
  gtk_widget_set_style(button,uinfo->style_dark_grey);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
  		     GTK_SIGNAL_FUNC(NextTrackCB),(gpointer)ginfo);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Go to next track"),NULL);
  gtk_widget_show(button);

  button=ImageButton(GTK_WIDGET(uinfo->app),uinfo->progtrack_image);
  gtk_widget_set_style(button,uinfo->style_dark_grey);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
    		     GTK_SIGNAL_FUNC(ToggleProg),(gpointer)uinfo);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Toggle play mode options"),NULL);
  gtk_widget_show(button);

  if(ginfo->changer_slots>1) {
    button=ImageButton(GTK_WIDGET(uinfo->app),uinfo->rotate_image);
    gtk_widget_set_style(button,uinfo->style_dark_grey);
    gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
    gtk_signal_connect(GTK_OBJECT(button),"clicked",
    		       GTK_SIGNAL_FUNC(NextDisc),(gpointer)ginfo);
    gtk_tooltips_set_tip(MakeToolTip(),button,
			 _("Next Disc"),NULL);
    gtk_widget_show(button);
  }

  gtk_box_pack_start(GTK_BOX(uinfo->control_button_box),hbox,TRUE,TRUE,0);
  gtk_widget_show(hbox);

  hbox=gtk_hbox_new(TRUE,0);

  button=ImageButton(GTK_WIDGET(uinfo->app),uinfo->stop_image);
  gtk_widget_set_style(button,uinfo->style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
  		     GTK_SIGNAL_FUNC(StopPlayCB),(gpointer)ginfo);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Stop play"),NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  button=ImageButton(GTK_WIDGET(uinfo->app),uinfo->eject_image);
  gtk_widget_set_style(button,uinfo->style_dark_grey);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
  		     GTK_SIGNAL_FUNC(EjectDisc),(gpointer)ginfo);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Eject disc"),NULL);
  gtk_widget_show(button);

  button=ImageButton(GTK_WIDGET(uinfo->app),uinfo->cdscan_image);
  gtk_widget_set_style(button,uinfo->style_dark_grey);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
  		     GTK_SIGNAL_FUNC(ScanDisc),(gpointer)ginfo);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Scan Disc Contents"),NULL);
  gtk_widget_show(button);

  button=ImageButton(GTK_WIDGET(uinfo->app),uinfo->vol_image);
  gtk_widget_set_style(button,uinfo->style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
  		     GTK_SIGNAL_FUNC(ToggleVol),(gpointer)uinfo);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Toggle Volume Control"),NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  button=ImageButton(GTK_WIDGET(uinfo->app),uinfo->edit_image);
  gtk_widget_set_style(button,uinfo->style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
  		     GTK_SIGNAL_FUNC(ToggleTrackEdit),(gpointer)ginfo);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Toggle disc editor"),NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  if(!ginfo->local_mode) {
    button=ImageButton(GTK_WIDGET(uinfo->app),uinfo->discdbwht_image);
    gtk_widget_set_style(button,uinfo->style_dark_grey);
    gtk_signal_connect(GTK_OBJECT(button),"clicked",
    		       GTK_SIGNAL_FUNC(DiscDBToggle),(gpointer)ginfo);
    gtk_tooltips_set_tip(MakeToolTip(),button,
			 _("Initiate/abort DiscDB lookup"),NULL);
    gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
    gtk_widget_show(button);
  }

  button=ImageButton(GTK_WIDGET(uinfo->app),uinfo->minmax_image);
  gtk_widget_set_style(button,uinfo->style_dark_grey);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
  		     GTK_SIGNAL_FUNC(MinMax),(gpointer)uinfo);
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Toggle track display"),NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_widget_show(button);

  button=ImageButton(GTK_WIDGET(uinfo->app),uinfo->quit_image);
  gtk_widget_set_style(button,uinfo->style_dark_grey);
#ifndef GRIPCD
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Exit Grip"),NULL);
#else
  gtk_tooltips_set_tip(MakeToolTip(),button,
		       _("Exit GCD"),NULL);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
  		     GTK_SIGNAL_FUNC(ShutDownCB),(gpointer)ginfo);
  gtk_widget_show(button);
  
  gtk_box_pack_start(GTK_BOX(uinfo->control_button_box),hbox,TRUE,TRUE,0);
  gtk_widget_show(hbox);

  gtk_box_pack_start(GTK_BOX(vbox),uinfo->control_button_box,TRUE,TRUE,0);
  gtk_widget_show(uinfo->control_button_box);


  gtk_container_add(GTK_CONTAINER(ebox),vbox);
  gtk_widget_show(vbox);

  return ebox;
}

static void ChangeTimeMode(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  ginfo->gui_info.time_display_mode=(ginfo->gui_info.time_display_mode+1)%4;
  UpdateDisplay(ginfo);
}

static void MinMax(GtkWidget *widget,gpointer data)
{
  GripGUI *uinfo;

  uinfo=(GripGUI *)data;

  if(uinfo->minimized) {
    gtk_container_border_width(GTK_CONTAINER(uinfo->winbox),3);
    gtk_widget_show(uinfo->notebook);

#ifndef GRIPCD
    CopyPixmap(GTK_PIXMAP(uinfo->lcd_smile_indicator),
	       GTK_PIXMAP(uinfo->smile_indicator));
    CopyPixmap(GTK_PIXMAP(uinfo->empty_image),
    GTK_PIXMAP(uinfo->lcd_smile_indicator));
#endif
  }
  else {
    gtk_window_set_policy(GTK_WINDOW(uinfo->app),FALSE,TRUE,TRUE);
    gtk_container_border_width(GTK_CONTAINER(uinfo->winbox),0);
    gtk_widget_hide(uinfo->notebook);

#ifndef GRIPCD
    CopyPixmap(GTK_PIXMAP(uinfo->smile_indicator),
	       GTK_PIXMAP(uinfo->lcd_smile_indicator));
#endif

    UpdateGTK();
    gtk_window_set_policy(GTK_WINDOW(uinfo->app),FALSE,TRUE,FALSE);
  }

  uinfo->minimized=!uinfo->minimized;
}

static void ToggleProg(GtkWidget *widget,gpointer data)
{
  GripGUI *uinfo;

  uinfo=(GripGUI *)data;

  if(uinfo->track_prog_visible) {
    gtk_window_set_policy(GTK_WINDOW(uinfo->app),FALSE,TRUE,
			  uinfo->minimized||uinfo->keep_min_size);
    gtk_widget_hide(uinfo->playopts);
    UpdateGTK();
    gtk_window_set_policy(GTK_WINDOW(uinfo->app),FALSE,TRUE,FALSE);
  }
  else {
    gtk_widget_show(uinfo->playopts);
  }

  uinfo->track_prog_visible=!uinfo->track_prog_visible;
}

static void ToggleControlButtons(GtkWidget *widget,GdkEventButton *event,
				 gpointer data)
{
  GripGUI *uinfo;

  uinfo=&((GripInfo *)data)->gui_info;

  if(uinfo->control_buttons_visible) {
    gtk_window_set_policy(GTK_WINDOW(uinfo->app),
			  FALSE,TRUE,TRUE);
    gtk_widget_hide(uinfo->control_button_box);

    UpdateGTK();
    gtk_window_set_policy(GTK_WINDOW(uinfo->app),
			  FALSE,TRUE,FALSE);
  }
  else {
    gtk_widget_show(uinfo->control_button_box);
  }

  uinfo->control_buttons_visible=!uinfo->control_buttons_visible;
}

static void ToggleVol(GtkWidget *widget,gpointer data)
{
  GripGUI *uinfo;

  uinfo=(GripGUI *)data;

  if(uinfo->volvis) {
    gtk_window_set_policy(GTK_WINDOW(uinfo->app),FALSE,TRUE,
			  uinfo->minimized||uinfo->keep_min_size);
    gtk_widget_hide(uinfo->volume_control);
    UpdateGTK();
    gtk_window_set_policy(GTK_WINDOW(uinfo->app),FALSE,TRUE,FALSE);
  }
  else {
    gtk_widget_show(uinfo->volume_control);
  }

  uinfo->volvis=!uinfo->volvis;
}

static void SetVolume(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;
  DiscVolume vol;

  ginfo=(GripInfo *)data;

  ginfo->volume=vol.vol_front.left=vol.vol_front.right=
    vol.vol_back.left=vol.vol_back.right=GTK_ADJUSTMENT(widget)->value;

  CDSetVolume(&(ginfo->disc),&vol);
}

static void FastFwdCB(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

#ifndef GRIPCD
  if(ginfo->ripping_a_disc) {
    DisplayMsg(_("Cannot fast forward while ripping"));

    return;
  }
#endif

  ginfo->ffwding=!ginfo->ffwding;

  if(ginfo->ffwding) FastFwd(ginfo);
}

void FastFwd(GripInfo *ginfo)
{
  DiscTime tv;

  tv.mins=0;
  tv.secs=5;

  if((ginfo->disc.disc_mode==CDAUDIO_PLAYING)||
     (ginfo->disc.disc_mode==CDAUDIO_PAUSED)) {
    CDAdvance(&(ginfo->disc),&tv);
  }
}

static void RewindCB(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

#ifndef GRIPCD
  if(ginfo->ripping_a_disc) {
    DisplayMsg(_("Cannot rewind while ripping"));

    return;
  }
#endif

  ginfo->rewinding=!ginfo->rewinding;

  if(ginfo->rewinding) Rewind(ginfo);
}

void Rewind(GripInfo *ginfo)
{
  DiscTime tv;

  tv.mins=0;
  tv.secs=-5;

  if((ginfo->disc.disc_mode==CDAUDIO_PLAYING)||
     (ginfo->disc.disc_mode==CDAUDIO_PAUSED)) {
    CDAdvance(&(ginfo->disc),&tv);
  }
}

static void NextDisc(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

#ifndef GRIPCD
  if(ginfo->ripping_a_disc) {
    DisplayMsg(_("Cannot switch discs while ripping"));

    return;
  }
#endif

  if(ginfo->changer_slots>1) {
    ginfo->current_disc=(ginfo->current_disc+1)%ginfo->changer_slots;
    CDChangerSelectDisc(&(ginfo->disc),ginfo->current_disc);
    ginfo->have_disc=FALSE;
  }
}

void EjectDisc(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  Debug(_("Eject disc\n"));

#ifndef GRIPCD
  if(ginfo->ripping_a_disc) {
    DisplayMsg(_("Cannot eject while ripping"));

    return;
  }
#endif

  if(ginfo->auto_eject_countdown) return;

  Busy(&(ginfo->gui_info));

  if(ginfo->have_disc) {
    Debug(_("Have disc -- ejecting\n"));

    CDStop(&(ginfo->disc));
    CDEject(&(ginfo->disc));
    ginfo->playing=FALSE;
    ginfo->have_disc=FALSE;
    ginfo->update_required=TRUE;
    ginfo->current_discid=0;
    ginfo->tray_open=TRUE;
  }
  else {
    if(ginfo->faulty_eject) {
      if(ginfo->tray_open) CDClose(&(ginfo->disc));
      else CDEject(&(ginfo->disc));
    }
    else {
      if(TrayOpen(&(ginfo->disc))!=0) CDClose(&(ginfo->disc));
      else CDEject(&(ginfo->disc));
    }

    ginfo->tray_open=!ginfo->tray_open;

    if(!ginfo->tray_open)
      CheckNewDisc(ginfo,FALSE);
  }

  UnBusy(&(ginfo->gui_info));
}

static void StopPlayCB(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

#ifndef GRIPCD
  if(ginfo->ripping_a_disc) return;
#endif

  CDStop(&(ginfo->disc));
  CDStat(&(ginfo->disc),FALSE);
  ginfo->stopped=TRUE;

  if(ginfo->stop_first)
    gtk_clist_select_row(GTK_CLIST(ginfo->gui_info.trackclist),0,0);
}

void PlaySegment(GripInfo *ginfo,int track)
{
  CDPlayFrames(&(ginfo->disc),
	       ginfo->disc.track[track].start_frame+ginfo->start_sector,
	       ginfo->disc.track[track].start_frame+ginfo->end_sector);
}



static void PlayTrackCB(GtkWidget *widget,gpointer data)
{
  int track;
  GripInfo *ginfo;
  DiscInfo *disc;

  ginfo=(GripInfo *)data;
  disc=&(ginfo->disc);

#ifndef GRIPCD
  if(ginfo->ripping_a_disc) {
    DisplayMsg(_("Cannot play while ripping"));
   
    return;
  }
#endif

  CDStat(disc,FALSE);

  if(ginfo->play_mode!=PM_NORMAL&&!((disc->disc_mode==CDAUDIO_PLAYING)||
				    disc->disc_mode==CDAUDIO_PAUSED)) {
    if(ginfo->play_mode==PM_SHUFFLE && ginfo->automatic_reshuffle)
      ShuffleTracks(ginfo);
    ginfo->current_track_index=0;
    gtk_clist_select_row(GTK_CLIST(ginfo->gui_info.trackclist),
			 CURRENT_TRACK,0);
  }

  track=CURRENT_TRACK;

  if(track==(disc->curr_track-1)) {
    switch(disc->disc_mode) {
    case CDAUDIO_PLAYING:
      CDPause(disc);
      return;
      break;
    case CDAUDIO_PAUSED:
      CDResume(disc);
      return;
      break;
    default:
      PlayTrack(ginfo,track);
      break;
    }
  }
  else PlayTrack(ginfo,track);
}

static void PlayTrack(GripInfo *ginfo,int track)
{
  Busy(&(ginfo->gui_info));
  
  if(ginfo->play_mode==PM_NORMAL)
    CDPlayTrack(&(ginfo->disc),track+1,ginfo->disc.num_tracks);
  else CDPlayTrack(&(ginfo->disc),track+1,track+1);

  UnBusy(&(ginfo->gui_info));

  ginfo->playing=TRUE;
}

static void NextTrackCB(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  NextTrack(ginfo);
}

void NextTrack(GripInfo *ginfo)
{
#ifndef GRIPCD
  if(ginfo->ripping_a_disc) {
    DisplayMsg(_("Cannot switch tracks while ripping"));
    return;
  }
#endif
  
  CDStat(&(ginfo->disc),FALSE);

  if(ginfo->current_track_index<(ginfo->prog_totaltracks-1)) {
    gtk_clist_select_row(GTK_CLIST(ginfo->gui_info.trackclist),
			 NEXT_TRACK,0);
  }
  else {
    if(!ginfo->playloop) {
      ginfo->stopped=TRUE;
    }

    gtk_clist_select_row(GTK_CLIST(ginfo->gui_info.trackclist),
			 ginfo->tracks_prog[0],0);
  }
}

static void PrevTrackCB(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  PrevTrack(ginfo);
}

static void PrevTrack(GripInfo *ginfo)
{
#ifndef GRIPCD
  if(ginfo->ripping_a_disc) {
    DisplayMsg(_("Cannot switch tracks while ripping"));
    return;
  }
#endif

  CDStat(&(ginfo->disc),FALSE);

  if((ginfo->disc.disc_mode==CDAUDIO_PLAYING) &&
     ((ginfo->disc.curr_frame-
       ginfo->disc.track[ginfo->disc.curr_track-1].start_frame) > 100))
    PlayTrack(ginfo,CURRENT_TRACK);
  else {
    if(ginfo->current_track_index) {
      gtk_clist_select_row(GTK_CLIST(ginfo->gui_info.trackclist),
			   PREV_TRACK,0);
    }
    else {
      if(ginfo->playloop) {
	gtk_clist_select_row(GTK_CLIST(ginfo->gui_info.trackclist),
			     ginfo->tracks_prog[ginfo->prog_totaltracks-1]
			     ,0);
      }
    }
  }
}

static void InitProgram(GripInfo *ginfo)
{
  int track;
  char *tok;
  int mode;

  mode=ginfo->play_mode;

  if((mode==PM_PLAYLIST)) {
    tok=gtk_entry_get_text(GTK_ENTRY(ginfo->gui_info.playlist_entry));
    if(!tok||!*tok) mode=PM_NORMAL;
  }

  if(mode==PM_PLAYLIST) {
    ginfo->prog_totaltracks=0;

    tok=strtok(gtk_entry_get_text(GTK_ENTRY(ginfo->gui_info.playlist_entry)),
	       ",");

    while(tok) {
      ginfo->tracks_prog[ginfo->prog_totaltracks++]=atoi(tok)-1;

      tok=strtok(NULL,",");
    }
  }
  else {
    ginfo->prog_totaltracks=ginfo->disc.num_tracks;
    
    for(track=0;track<ginfo->prog_totaltracks;track++) {
      ginfo->tracks_prog[track]=track;
    }
    
    if(mode==PM_SHUFFLE)
      ShuffleTracks(ginfo);
  }
}

 /* Shuffle the tracks around a bit */
static void ShuffleTracks(GripInfo *ginfo)
{
  int t1,t2,tmp,shuffle;

  for(shuffle=0;shuffle<(ginfo->prog_totaltracks*10);shuffle++) {
    t1=RRand(ginfo->prog_totaltracks);
    t2=RRand(ginfo->prog_totaltracks);
    
    tmp=ginfo->tracks_prog[t1];
    ginfo->tracks_prog[t1]=ginfo->tracks_prog[t2];
    ginfo->tracks_prog[t2]=tmp;
  }
}

void CheckNewDisc(GripInfo *ginfo,gboolean force)
{
  int new_id;
  DiscInfo *disc;

  disc=&(ginfo->disc);

  if(!ginfo->looking_up) {
    Debug(_("Checking for a new disc\n"));

    if(CDStat(disc,FALSE)
       && disc->disc_present
       && CDStat(disc,TRUE)) {
      Debug(_("CDStat found a disc, checking tracks\n"));
      
      if(CheckTracks(disc)) {
	Debug(_("We have a valid disc!\n"));
	
	new_id=DiscDBDiscid(disc);

	InitProgram(ginfo);

        if(ginfo->play_first)
          if(disc->disc_mode == CDAUDIO_COMPLETED ||
	     disc->disc_mode == CDAUDIO_NOSTATUS) {
            gtk_clist_select_row(GTK_CLIST(ginfo->gui_info.trackclist),0,0);
	    disc->curr_track = 1;
          }
	
	if(new_id || force) {
	  ginfo->have_disc=TRUE;

	  if(ginfo->play_on_insert) PlayTrackCB(NULL,(gpointer)ginfo);

	  LookupDisc(ginfo,FALSE);
	}
      }
      else {
	if(ginfo->have_disc)
	  ginfo->update_required=TRUE;
	
	ginfo->have_disc=FALSE;
	Debug(_("No non-zero length tracks\n"));
      }
    }
    else {
      if(ginfo->have_disc) {
	ginfo->update_required=TRUE;
      }

      ginfo->have_disc=FALSE;
      Debug(_("CDStat said no disc\n"));
    }
  }
}

/* Check to make sure we didn't get a false alarm from the cdrom device */

static gboolean CheckTracks(DiscInfo *disc)
{
  int track;
  gboolean have_track=FALSE;

  for(track=0;track<disc->num_tracks;track++)
    if(disc->track[track].length.mins||
       disc->track[track].length.secs) have_track=TRUE;

  return have_track;
}

/* Scan the disc */
void ScanDisc(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  ginfo->update_required=TRUE;

  CheckNewDisc(ginfo,TRUE);
}

void UpdateDisplay(GripInfo *ginfo)
{
  /* Note: need another solution other than statics if we ever want to be
     reentrant */
  static int play_counter=0;
  static int discdb_counter=0;
  char buf[80]="";
  char icon_buf[80];
  static int frames;
  static int secs;
  static int mins;
  int totsecs;
  GripGUI *uinfo;
  DiscInfo *disc;

  uinfo=&(ginfo->gui_info);
  disc=&(ginfo->disc);

  if(!ginfo->looking_up) {
    if(discdb_counter%2)
      discdb_counter++;
  }
  else
    CopyPixmap(GTK_PIXMAP(uinfo->discdb_pix[discdb_counter++%2]),
	       GTK_PIXMAP(uinfo->discdb_indicator));

  if(!ginfo->update_required) {
    if(ginfo->have_disc) {
      /* Allow disc time to spin down after ripping before checking for a new
	 disc. Some drives report no disc when spinning down. */
      if(ginfo->rip_finished) {
	if((time(NULL)-ginfo->rip_finished)>5) {
	  ginfo->rip_finished=0;
	}
      }

      if(!ginfo->rip_finished) {
	CDStat(disc,FALSE);
	
	if(!disc->disc_present) {
	  ginfo->have_disc=FALSE;
	  ginfo->update_required=TRUE;
	}
      }
    }
  }

  if(!ginfo->update_required) {
    if(ginfo->have_disc) {
      if((disc->disc_mode==CDAUDIO_PLAYING)||
	 (disc->disc_mode==CDAUDIO_PAUSED)) {
	if(disc->disc_mode==CDAUDIO_PAUSED) {
	  if((play_counter++%2)==0) {
	    strcpy(buf,"");
	  }
	  else {
	    g_snprintf(buf,80,"%02d:%02d",mins,secs);
	  }
	}
	else {
	  if((disc->curr_track-1)!=CURRENT_TRACK) {
	    gtk_clist_select_row(GTK_CLIST(uinfo->trackclist),
				 disc->curr_track-1,0);
	  }

	  frames=disc->curr_frame-disc->track[disc->curr_track-1].start_frame;

	  switch(uinfo->time_display_mode) {
	  case TIME_MODE_TRACK:
	    mins=disc->track_time.mins;
	    secs=disc->track_time.secs;
	    break;
	  case TIME_MODE_DISC:
	    mins=disc->disc_time.mins;
	    secs=disc->disc_time.secs;
	    break;
	  case TIME_MODE_LEFT_TRACK:
	    secs=(disc->track_time.mins*60)+disc->track_time.secs;
	    totsecs=(disc->track[CURRENT_TRACK].length.mins*60)+
	      disc->track[CURRENT_TRACK].length.secs;
	    
	    totsecs-=secs;
	    
	    mins=totsecs/60;
	    secs=totsecs%60;
	    break;
	  case TIME_MODE_LEFT_DISC:
	    secs=(disc->disc_time.mins*60)+disc->disc_time.secs;
	    totsecs=(disc->length.mins*60)+disc->length.secs;
	    
	    totsecs-=secs;
	    
	    mins=totsecs/60;
	    secs=totsecs%60;
	    break;
	  }
	  
#ifndef GRIPCD
	  g_snprintf(buf,80,_("Current sector: %6d"),frames);
	  gtk_label_set(GTK_LABEL(uinfo->play_sector_label),buf);
#endif
          if(uinfo->time_display_mode == TIME_MODE_LEFT_TRACK ||
	     uinfo->time_display_mode == TIME_MODE_LEFT_DISC)
            g_snprintf(buf,80,"-%02d:%02d",mins,secs);
          else
	    g_snprintf(buf,80,"%02d:%02d",mins,secs);
	}
      }
      else {
	if(ginfo->playing&&((disc->disc_mode==CDAUDIO_COMPLETED)||
			    ((disc->disc_mode==CDAUDIO_NOSTATUS)&&
			     !ginfo->stopped))) {
	  NextTrack(ginfo);
	  strcpy(buf,"00:00");
	  if(!ginfo->stopped) PlayTrack(ginfo,CURRENT_TRACK);
	}
	else if(ginfo->stopped) {
	  CDStop(disc);
#ifndef GRIPCD
	  frames=secs=mins=0;
	  g_snprintf(buf,80,_("Current sector: %6d"),frames);
	  gtk_label_set(GTK_LABEL(uinfo->play_sector_label),buf);
#endif
	  
	  strcpy(buf,"00:00");
	  
	  ginfo->stopped=FALSE;
	  ginfo->playing=FALSE;
	}
	else return;
      }
      
      gtk_label_set(GTK_LABEL(uinfo->play_time_label),buf);
      g_snprintf(icon_buf,sizeof(icon_buf),"%02d %s %s",
		 disc->curr_track,buf,PACKAGE);
      gdk_window_set_icon_name(uinfo->app->window,icon_buf);
    }
  }

  if(ginfo->update_required) {
    UpdateTracks(ginfo);

    ginfo->update_required=FALSE;

    if(ginfo->have_disc) {
      g_snprintf(buf,80,"%02d:%02d",disc->length.mins,
		 disc->length.secs);
      g_snprintf(icon_buf, sizeof(icon_buf),"%02d %s %s",
		 disc->curr_track,buf,PACKAGE);
	       
      gtk_label_set(GTK_LABEL(uinfo->play_time_label),buf);
      
      if(!ginfo->looking_up) {
	CopyPixmap(GTK_PIXMAP(uinfo->empty_image),
		   GTK_PIXMAP(uinfo->discdb_indicator));

#ifndef GRIPCD
	if(ginfo->auto_rip&&ginfo->is_new_disc) {
	  ClickColumn(NULL,2,ginfo);
	  DoRip(NULL,ginfo);
	}

	ginfo->is_new_disc=FALSE;
#endif
      }
      
      if(!ginfo->no_interrupt)
      	gtk_clist_select_row(GTK_CLIST(uinfo->trackclist),0,0);
      else
	gtk_clist_select_row(GTK_CLIST(uinfo->trackclist),
			     disc->curr_track-1,0);
    }
    else {
      gtk_label_set(GTK_LABEL(uinfo->play_time_label),"--:--");
      strncpy(icon_buf,PACKAGE,sizeof(icon_buf));
      
      SetCurrentTrack(ginfo,-1);
    }

    gdk_window_set_icon_name(uinfo->app->window,icon_buf);
  }
}

void UpdateTracks(GripInfo *ginfo)
{
  int track;
  char *col_strings[3];
  gboolean multi_artist_backup;
  GripGUI *uinfo;
  DiscInfo *disc;
  DiscData *ddata;
  EncodeTrack enc_track;

  uinfo=&(ginfo->gui_info);
  disc=&(ginfo->disc);
  ddata=&(ginfo->ddata);

  if(ginfo->have_disc) {
    /* Reset to make sure we don't eject twice */
    ginfo->auto_eject_countdown=0;

    ginfo->current_discid=DiscDBDiscid(disc);

    SetTitle(ginfo,ddata->data_title);
    SetArtist(ginfo,ddata->data_artist);
    SetYear(ginfo,ddata->data_year);
    SetID3Genre(ginfo,ddata->data_id3genre);

    multi_artist_backup=ddata->data_multi_artist;

#ifdef GTK_HAVE_FEATURES_1_1_13
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(uinfo->multi_artist_button),
				 ginfo->ddata.data_multi_artist);
#else
    gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(uinfo->multi_artist_button),
				ginfo->ddata.data_multi_artist);
#endif

    ddata->data_multi_artist=multi_artist_backup;
    UpdateMultiArtist(NULL,(gpointer)ginfo);

    if(*(ginfo->cdupdate)) {
      FillInTrackInfo(ginfo,0,&enc_track);

      TranslateAndLaunch(ginfo->cdupdate,TranslateSwitch,&enc_track,
			 FALSE,&(ginfo->sprefs),CloseStuff,(void *)ginfo);
    }
  }
  else {
    SetTitle(ginfo,_("No Disc"));
    SetArtist(ginfo,"");
    SetYear(ginfo,0);
    SetID3Genre(ginfo,17);
  }

  gtk_entry_set_text(GTK_ENTRY(uinfo->playlist_entry),
		     ddata->data_playlist);

  if(!ginfo->first_time)
    gtk_clist_clear(GTK_CLIST(uinfo->trackclist));
  else {
    SetCurrentTrackIndex(ginfo,disc->curr_track - 1);
  }

  if(ginfo->have_disc) {
    /* Block the select row callback so we don't mess up the current track
       while reconstructing the clist */

    gtk_signal_handler_block_by_func(GTK_OBJECT(uinfo->trackclist),
				     GTK_SIGNAL_FUNC(SelectRow),ginfo);

    col_strings[0]=(char *)malloc(260);
    col_strings[1]=(char *)malloc(6);
    col_strings[2]=NULL;

    for(track=0;track<disc->num_tracks;track++) {
      if(*ddata->data_track[track].track_artist)
	g_snprintf(col_strings[0],260,"%02d  %s (%s)",track+1,
		   ddata->data_track[track].track_name,
		   ddata->data_track[track].track_artist);
      else
	g_snprintf(col_strings[0],260,"%02d  %s",track+1,
		   ddata->data_track[track].track_name);

      g_snprintf(col_strings[1],6,"%2d:%02d",
	       disc->track[track].length.mins,
	       disc->track[track].length.secs);

      gtk_clist_append(GTK_CLIST(uinfo->trackclist),col_strings);
    }

    free(col_strings[0]);
    free(col_strings[1]);

    gtk_clist_select_row(GTK_CLIST(uinfo->trackclist),CURRENT_TRACK,0);

    gtk_signal_handler_unblock_by_func(GTK_OBJECT(uinfo->trackclist),
				       GTK_SIGNAL_FUNC(SelectRow),ginfo);

  }

  if(ginfo->ask_submit) {
    gnome_app_ok_cancel_modal
      ((GnomeApp *)uinfo->app,
       _("This disc has been found on your secondary server,\n"
       "but not on your primary server.\n\n"
       "Do you wish to submit this disc information?"),
       SubmitEntry,(gpointer)ginfo);
    
    ginfo->ask_submit=FALSE;
  }

  ginfo->first_time=0;
}

void SubmitEntry(gint reply,gpointer data)
{
  GripInfo *ginfo;
  int fd;
  FILE *efp;
  char mailcmd[256];
  char filename[256];

  if(reply) return;

  ginfo=(GripInfo *)data;

  sprintf(filename,"/tmp/grip.XXXXXX");
  fd = mkstemp(filename);

  if(fd == -1) {
    DisplayMsg(_("Error: Unable to create temporary file"));
    return;
  }

  efp=fdopen(fd,"w");

  if(!efp) {
    close(fd);
    DisplayMsg(_("Error: Unable to create temporary file"));
  }
  else {
    fprintf(efp,"To: %s\nFrom: %s\nSubject: cddb %s %02x\n\n",
	    ginfo->discdb_submit_email,ginfo->user_email,
	    DiscDBGenre(ginfo->ddata.data_genre),
	    ginfo->ddata.data_id);

    if(DiscDBWriteDiscData(&(ginfo->disc),&(ginfo->ddata),efp,FALSE,
			   ginfo->db_use_freedb)<0) {
      DisplayMsg(_("Error: Unable to write disc data"));
      fclose(efp);
    }
    else {
      fclose(efp);
      close(fd);

      g_snprintf(mailcmd,256,"%s < %s",MAILER,filename);

      Debug(_("Mailing entry to %s\n"),ginfo->discdb_submit_email);

      system(mailcmd);

      remove(filename);
    }
  }
}
