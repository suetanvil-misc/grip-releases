/* grip.c
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

#include <pthread.h>
#include <config.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/param.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include "grip.h"
#include <libgnomeui/gnome-window-icon.h>
#include "discdb.h"
#include "cdplay.h"
#include "discedit.h"
#include "rip.h"
#include "common.h"
#include "dialog.h"
#include "gripcfg.h"
#include "xpm.h"
#include "parsecfg.h"

static void ReallyDie(gint reply,gpointer data);
static void DoHelp(GtkWidget *widget,gpointer data);
static void MakeHelpPage(GripInfo *ginfo);
static void MakeAboutPage(GripGUI *uinfo);
static void MakeStyles(GripGUI *uinfo);
static void Homepage(void);
static void LoadImages(GripGUI *uinfo);
static void DoLoadConfig(GripInfo *ginfo);
void DoSaveConfig(GripInfo *ginfo);

static GnomeHelpMenuEntry main_help_entry={"grip","grip.html"};
static GnomeHelpMenuEntry cdplay_help_entry={"grip","cdplayer.html"};
static GnomeHelpMenuEntry rip_help_entry={"grip","ripping.html"};
static GnomeHelpMenuEntry configure_help_entry={"grip","configure.html"};
static GnomeHelpMenuEntry faq_help_entry={"grip","faq.html"};
static GnomeHelpMenuEntry morehelp_help_entry={"grip","morehelp.html"};
static GnomeHelpMenuEntry bug_help_entry={"grip","bugs.html"};

#define BASE_CFG_ENTRIES \
{"cd_device",CFG_ENTRY_STRING,256,ginfo->cd_device},\
{"ripexename",CFG_ENTRY_STRING,256,ginfo->ripexename},\
{"ripcmdline",CFG_ENTRY_STRING,256,ginfo->ripcmdline},\
{"wav_filter_cmd",CFG_ENTRY_STRING,256,ginfo->wav_filter_cmd},\
{"disc_filter_cmd",CFG_ENTRY_STRING,256,ginfo->disc_filter_cmd},\
{"mp3exename",CFG_ENTRY_STRING,256,ginfo->mp3exename},\
{"mp3cmdline",CFG_ENTRY_STRING,256,ginfo->mp3cmdline},\
{"dbserver",CFG_ENTRY_STRING,256,ginfo->dbserver.name},\
{"ripfileformat",CFG_ENTRY_STRING,256,ginfo->ripfileformat},\
{"mp3fileformat",CFG_ENTRY_STRING,256,ginfo->mp3fileformat},\
{"m3ufileformat",CFG_ENTRY_STRING,256,ginfo->m3ufileformat},\
{"delete_wavs",CFG_ENTRY_BOOL,0,&ginfo->delete_wavs},\
{"add_m3u",CFG_ENTRY_BOOL,0,&ginfo->add_m3u},\
{"rel_m3u",CFG_ENTRY_BOOL,0,&ginfo->rel_m3u},\
{"add_to_db",CFG_ENTRY_BOOL,0,&ginfo->add_to_db},\
{"use_proxy",CFG_ENTRY_BOOL,0,&ginfo->use_proxy},\
{"proxy_name",CFG_ENTRY_STRING,256,ginfo->proxy_server.name},\
{"proxy_port",CFG_ENTRY_INT,0,&(ginfo->proxy_server.port)},\
{"proxy_user",CFG_ENTRY_STRING,80,ginfo->proxy_server.username},\
{"proxy_pswd",CFG_ENTRY_STRING,80,ginfo->proxy_server.pswd},\
{"cdupdate",CFG_ENTRY_STRING,256,ginfo->cdupdate},\
{"user_email",CFG_ENTRY_STRING,256,ginfo->user_email},\
{"ripnice",CFG_ENTRY_INT,0,&ginfo->ripnice},\
{"mp3nice",CFG_ENTRY_INT,0,&ginfo->mp3nice},\
{"mp3_filter_cmd",CFG_ENTRY_STRING,256,ginfo->mp3_filter_cmd},\
{"doid3",CFG_ENTRY_BOOL,0,&ginfo->doid3},\
{"doid3v2",CFG_ENTRY_BOOL,0,&ginfo->doid3v2},\
{"id3_comment",CFG_ENTRY_STRING,30,ginfo->id3_comment},\
{"max_wavs",CFG_ENTRY_INT,0,&ginfo->max_wavs},\
{"auto_rip",CFG_ENTRY_BOOL,0,&ginfo->auto_rip},\
{"eject_after_rip",CFG_ENTRY_BOOL,0,&ginfo->eject_after_rip},\
{"eject_delay",CFG_ENTRY_INT,0,&ginfo->eject_delay},\
{"beep_after_rip",CFG_ENTRY_BOOL,0,&ginfo->beep_after_rip},\
{"faulty_eject",CFG_ENTRY_BOOL,0,&ginfo->faulty_eject},\
{"use_proxy_env",CFG_ENTRY_BOOL,0,&ginfo->use_proxy_env},\
{"db_cgi",CFG_ENTRY_STRING,256,ginfo->dbserver.cgi_prog},\
{"cddb_submit_email",CFG_ENTRY_STRING,256,ginfo->discdb_submit_email},\
{"db_use_freedb",CFG_ENTRY_BOOL,0,&ginfo->db_use_freedb},\
{"dbserver2",CFG_ENTRY_STRING,256,ginfo->dbserver2.name},\
{"db2_cgi",CFG_ENTRY_STRING,256,ginfo->dbserver2.cgi_prog},\
{"no_interrupt",CFG_ENTRY_BOOL,0,&ginfo->no_interrupt},\
{"stop_first",CFG_ENTRY_BOOL,0,&ginfo->stop_first},\
{"play_first",CFG_ENTRY_BOOL,0,&ginfo->play_first},\
{"play_on_insert",CFG_ENTRY_BOOL,0,&ginfo->play_on_insert},\
{"automatic_cddb",CFG_ENTRY_BOOL,0,&ginfo->automatic_discdb},\
{"automatic_reshuffle",CFG_ENTRY_BOOL,0,&ginfo->automatic_reshuffle},\
{"no_lower_case",CFG_ENTRY_BOOL,0,&ginfo->sprefs.no_lower_case},\
{"no_underscore",CFG_ENTRY_BOOL,0,&ginfo->sprefs.no_underscore},\
{"allow_high_bits",CFG_ENTRY_BOOL,0,&ginfo->sprefs.allow_high_bits},\
{"allow_these_chars",CFG_ENTRY_STRING,256,ginfo->sprefs.allow_these_chars},\
{"keep_min_size",CFG_ENTRY_BOOL,0,&uinfo->keep_min_size},\
{"num_cpu",CFG_ENTRY_INT,0,&ginfo->edit_num_cpu},\
{"kbits_per_sec",CFG_ENTRY_INT,0,&ginfo->kbits_per_sec},\
{"selected_encoder",CFG_ENTRY_INT,0,&ginfo->selected_encoder},\
{"selected_ripper",CFG_ENTRY_INT,0,&ginfo->selected_ripper},\
{"play_mode",CFG_ENTRY_INT,0,&ginfo->play_mode},\
{"playloop",CFG_ENTRY_BOOL,0,&ginfo->playloop},\
{"volume",CFG_ENTRY_INT,0,&ginfo->volume},

#define CDPAR_CFG_ENTRIES \
{"disable_paranoia",CFG_ENTRY_BOOL,0,&ginfo->disable_paranoia},\
{"disable_extra_paranoia",CFG_ENTRY_BOOL,0,&ginfo->disable_extra_paranoia},\
{"disable_scratch_detect",CFG_ENTRY_BOOL,0,&ginfo->disable_scratch_detect},\
{"disable_scratch_repair",CFG_ENTRY_BOOL,0,&ginfo->disable_scratch_repair},\
{"calc_gain",CFG_ENTRY_BOOL,0,&ginfo->calc_gain},\
{"force_scsi",CFG_ENTRY_STRING,256,ginfo->force_scsi},

#ifdef CDPAR
#define CFG_ENTRIES BASE_CFG_ENTRIES CDPAR_CFG_ENTRIES
#else
#define CFG_ENTRIES BASE_CFG_ENTRIES
#endif

GtkWidget *GripNew(const gchar* geometry,char *device,char *scsi_device,
		   gboolean force_small,
		   gboolean local_mode,gboolean no_redirect)
{
  GtkWidget *app;
  GripInfo *ginfo;
  GripGUI *uinfo;

  gnome_window_icon_set_default_from_file(GNOME_ICONDIR"/gripicon.png");

  app=gnome_app_new(PACKAGE,_("Grip"));

  ginfo=g_new(GripInfo,1);

  gtk_object_set_user_data(GTK_OBJECT(app),(gpointer)ginfo);

  uinfo=&(ginfo->gui_info);
  uinfo->app=app;

  DoLoadConfig(ginfo);

  if(device) g_snprintf(ginfo->cd_device,256,"%s",device);
  if(scsi_device) g_snprintf(ginfo->force_scsi,256,"%s",scsi_device);

  uinfo->minimized=force_small;
  ginfo->local_mode=local_mode;
  ginfo->do_redirect=!no_redirect;

  if(!CDInitDevice(ginfo->cd_device,&(ginfo->disc))) {
    printf(_("Error: Unable to initialize [%s]\n"),ginfo->cd_device);
    exit(0);
  }

  CDStat(&(ginfo->disc),TRUE);

  gtk_window_set_policy(GTK_WINDOW(app),FALSE,TRUE,FALSE);
  gtk_window_set_wmclass(GTK_WINDOW(app),"grip","Grip");
  gtk_signal_connect(GTK_OBJECT(app),"delete_event",
                     GTK_SIGNAL_FUNC(GripDie),NULL);

  /* geometry */
  if(geometry != NULL) {
    gint x,y,w,h;
    
    if(gnome_parse_geometry(geometry, 
			    &x,&y,&w,&h)) {
      if(x != -1) {
	gtk_widget_set_uposition(app,x,y);
      }
      
      if(w != -1) {
	gtk_window_set_default_size(GTK_WINDOW(app),w,h);
      }
    }
    else {
      g_error(_("Could not parse geometry string `%s'"), geometry);
    }
  }

  gtk_widget_realize(app);

  uinfo->winbox=gtk_vbox_new(FALSE,3);
  if(!uinfo->minimized)
    gtk_container_border_width(GTK_CONTAINER(uinfo->winbox),3);

  uinfo->notebook=gtk_notebook_new();

  LoadImages(uinfo);
  MakeStyles(uinfo);
  MakeTrackPage(ginfo);
  MakeRipPage(ginfo);
  MakeConfigPage(ginfo);
  MakeHelpPage(ginfo);
  MakeAboutPage(uinfo);

  gtk_box_pack_start(GTK_BOX(uinfo->winbox),uinfo->notebook,TRUE,TRUE,0);
  if(!uinfo->minimized) gtk_widget_show(uinfo->notebook);

  uinfo->track_edit_box=MakeEditBox(ginfo);
  gtk_box_pack_start(GTK_BOX(uinfo->winbox),uinfo->track_edit_box,
		     FALSE,FALSE,0);

  uinfo->playopts=MakePlayOpts(ginfo);
  gtk_box_pack_start(GTK_BOX(uinfo->winbox),uinfo->playopts,FALSE,FALSE,0);
  if(uinfo->track_prog_visible) gtk_widget_show(uinfo->playopts);
 
  uinfo->controls=MakeControls(ginfo);
  gtk_box_pack_start(GTK_BOX(uinfo->winbox),uinfo->controls,FALSE,FALSE,0);
  gtk_widget_show(uinfo->controls);
  
  gnome_app_set_contents(GNOME_APP(app),uinfo->winbox);
  gtk_widget_show(uinfo->winbox);

  return app;
}

void GripDie(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)gtk_object_get_user_data(GTK_OBJECT(widget));
  
#ifndef GRIPCD
  if(ginfo->ripping || ginfo->encoding)
    gnome_app_ok_cancel_modal((GnomeApp *)ginfo->gui_info.app,
			      _("Work is in progress.\nReally shut down?"),
			      ReallyDie,(gpointer)ginfo);
  else ReallyDie(0,ginfo);
#else
  ReallyDie(0,ginfo);
#endif
}

static void ReallyDie(gint reply,gpointer data)
{
  GripInfo *ginfo;

  if(reply) return;

  ginfo=(GripInfo *)data;

#ifndef GRIPCD
  if(ginfo->ripping) KillRip(NULL,ginfo);
  if(ginfo->encoding) KillEncode(NULL,ginfo);
#endif

  if(!ginfo->no_interrupt)
    CDStop(&(ginfo->disc));

  DoSaveConfig(ginfo);

  gtk_main_quit();
}

GtkWidget *MakeNewPage(GtkWidget *notebook,char *name)
{
  GtkWidget *page;
  GtkWidget *label;

  page=gtk_frame_new(NULL);
  gtk_widget_show(page);

  label=gtk_label_new(name);
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),page,label);

  return page;
}

static void DoHelp(GtkWidget *widget,gpointer data)
{
  GnomeHelpMenuEntry *entry;

  entry=(GnomeHelpMenuEntry *)data;

  gnome_help_display(NULL,entry);
}

static void MakeHelpPage(GripInfo *ginfo)
{
  GtkWidget *help_page;
  GtkWidget *button;
  GtkWidget *vbox;

  help_page=MakeNewPage(ginfo->gui_info.notebook,_("Help"));

  vbox=gtk_vbox_new(FALSE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  button=gtk_button_new_with_label(_("Table Of Contents"));
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(DoHelp),(gpointer)&main_help_entry);
  gtk_box_pack_start(GTK_BOX(vbox),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  button=gtk_button_new_with_label(_("Playing CDs"));
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(DoHelp),(gpointer)&cdplay_help_entry);
  gtk_box_pack_start(GTK_BOX(vbox),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  button=gtk_button_new_with_label(_("Ripping CDs"));
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(DoHelp),(gpointer)&rip_help_entry);
  gtk_box_pack_start(GTK_BOX(vbox),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  button=gtk_button_new_with_label(_("Configuring Grip"));
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(DoHelp),(gpointer)&configure_help_entry);
  gtk_box_pack_start(GTK_BOX(vbox),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  button=gtk_button_new_with_label(_("FAQ"));
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(DoHelp),(gpointer)&faq_help_entry);
  gtk_box_pack_start(GTK_BOX(vbox),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  button=gtk_button_new_with_label(_("Getting More Help"));
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(DoHelp),(gpointer)&morehelp_help_entry);
  gtk_box_pack_start(GTK_BOX(vbox),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  button=gtk_button_new_with_label(_("Reporting Bugs"));
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     GTK_SIGNAL_FUNC(DoHelp),(gpointer)&bug_help_entry);
  gtk_box_pack_start(GTK_BOX(vbox),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  gtk_container_add(GTK_CONTAINER(help_page),vbox);
  gtk_widget_show(vbox);
}

void MakeAboutPage(GripGUI *uinfo)
{
  GtkWidget *aboutpage;
  GtkWidget *vbox,*vbox2,*hbox;
  GtkWidget *label;
  GtkWidget *logo;
  GtkWidget *ebox;
  GtkWidget *button;
  char versionbuf[20];

  aboutpage=MakeNewPage(uinfo->notebook,_("About"));

  ebox=gtk_event_box_new();
  gtk_widget_set_style(ebox,uinfo->style_wb);

  vbox=gtk_vbox_new(TRUE,5);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

#ifndef GRIPCD
  logo=Loadxpm(GTK_WIDGET(uinfo->app),grip_xpm);
#else
  logo=Loadxpm(GTK_WIDGET(uinfo->app),gcd_xpm);
#endif

  gtk_box_pack_start(GTK_BOX(vbox),logo,FALSE,FALSE,0);
  gtk_widget_show(logo);

  vbox2=gtk_vbox_new(TRUE,0);

  sprintf(versionbuf,"Version %s",VERSION);
  label=gtk_label_new(versionbuf);
  gtk_widget_set_style(label,uinfo->style_wb);
  gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  label=gtk_label_new("Copyright (c) 1998-2002, Mike Oliphant");
  gtk_widget_set_style(label,uinfo->style_wb);
  gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,FALSE,0);
  gtk_widget_show(label);

#if defined(__sun__)
  label=gtk_label_new("Solaris Port, David Meleedy");
  gtk_widget_set_style(label,uinfo->style_wb);
  gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,FALSE,0);
  gtk_widget_show(label);
#endif

  hbox=gtk_hbox_new(TRUE,0);

  button=gtk_button_new_with_label("http://www.nostatic.org/grip");
  gtk_widget_set_style(button,uinfo->style_dark_grey);
  gtk_widget_set_style(GTK_BUTTON(button)->child,
		       uinfo->style_dark_grey);
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

static void MakeStyles(GripGUI *uinfo)
{
  GdkColor gdkblack;
  GdkColor gdkwhite;
  GdkColor *color_LCD;
  GdkColor *color_dark_grey;

  gdk_color_white(gdk_colormap_get_system(),&gdkwhite);
  gdk_color_black(gdk_colormap_get_system(),&gdkblack);
  
  color_LCD=MakeColor(33686,38273,29557);
  color_dark_grey=MakeColor(0x4444,0x4444,0x4444);
  
  uinfo->style_wb=MakeStyle(&gdkwhite,&gdkblack,FALSE);
  uinfo->style_LCD=MakeStyle(color_LCD,color_LCD,FALSE);
  uinfo->style_dark_grey=MakeStyle(&gdkwhite,color_dark_grey,TRUE);
}

static void Homepage(void)
{
  system("gnome-moz-remote http://www.nostatic.org/grip");
}

static void LoadImages(GripGUI *uinfo)
{
  uinfo->check_image=Loadxpm(uinfo->app,check_xpm);
  uinfo->eject_image=Loadxpm(uinfo->app,eject_xpm);
  uinfo->ff_image=Loadxpm(uinfo->app,ff_xpm);
  uinfo->lowleft_image=Loadxpm(uinfo->app,lowleft_xpm);
  uinfo->lowright_image=Loadxpm(uinfo->app,lowright_xpm);
  uinfo->minmax_image=Loadxpm(uinfo->app,minmax_xpm);
  uinfo->nexttrk_image=Loadxpm(uinfo->app,nexttrk_xpm);
  uinfo->playpaus_image=Loadxpm(uinfo->app,playpaus_xpm);
  uinfo->prevtrk_image=Loadxpm(uinfo->app,prevtrk_xpm);
  uinfo->loop_image=Loadxpm(uinfo->app,loop_xpm);
  uinfo->noloop_image=Loadxpm(uinfo->app,noloop_xpm);
  uinfo->random_image=Loadxpm(uinfo->app,random_xpm);
  uinfo->playlist_image=Loadxpm(uinfo->app,playlist_xpm);
  uinfo->playnorm_image=Loadxpm(uinfo->app,playnorm_xpm);
  uinfo->quit_image=Loadxpm(uinfo->app,quit_xpm);
  uinfo->rew_image=Loadxpm(uinfo->app,rew_xpm);
  uinfo->stop_image=Loadxpm(uinfo->app,stop_xpm);
  uinfo->upleft_image=Loadxpm(uinfo->app,upleft_xpm);
  uinfo->upright_image=Loadxpm(uinfo->app,upright_xpm);
  uinfo->vol_image=Loadxpm(uinfo->app,vol_xpm);
  uinfo->discdbwht_image=Loadxpm(uinfo->app,discdbwht_xpm);
  uinfo->rotate_image=Loadxpm(uinfo->app,rotate_xpm);
  uinfo->edit_image=Loadxpm(uinfo->app,edit_xpm);
  uinfo->progtrack_image=Loadxpm(uinfo->app,progtrack_xpm);
  uinfo->mail_image=Loadxpm(uinfo->app,mail_xpm);
  uinfo->save_image=Loadxpm(uinfo->app,save_xpm);

  uinfo->empty_image=NewBlankPixmap(uinfo->app);

  uinfo->discdb_pix[0]=Loadxpm(uinfo->app,discdb0_xpm);
  uinfo->discdb_pix[1]=Loadxpm(uinfo->app,discdb1_xpm);

  uinfo->play_pix[0]=Loadxpm(uinfo->app,playnorm_xpm);
  uinfo->play_pix[1]=Loadxpm(uinfo->app,random_xpm);
  uinfo->play_pix[2]=Loadxpm(uinfo->app,playlist_xpm);

#ifndef GRIPCD
  uinfo->rip_pix[0]=Loadxpm(uinfo->app,rip0_xpm);
  uinfo->rip_pix[1]=Loadxpm(uinfo->app,rip1_xpm);
  uinfo->rip_pix[2]=Loadxpm(uinfo->app,rip2_xpm);
  uinfo->rip_pix[3]=Loadxpm(uinfo->app,rip3_xpm);

  uinfo->mp3_pix[0]=Loadxpm(uinfo->app,enc0_xpm);
  uinfo->mp3_pix[1]=Loadxpm(uinfo->app,enc1_xpm);
  uinfo->mp3_pix[2]=Loadxpm(uinfo->app,enc2_xpm);
  uinfo->mp3_pix[3]=Loadxpm(uinfo->app,enc3_xpm);

  uinfo->smile_pix[0]=Loadxpm(uinfo->app,smile1_xpm);
  uinfo->smile_pix[1]=Loadxpm(uinfo->app,smile2_xpm);
  uinfo->smile_pix[2]=Loadxpm(uinfo->app,smile3_xpm);
  uinfo->smile_pix[3]=Loadxpm(uinfo->app,smile4_xpm);
  uinfo->smile_pix[4]=Loadxpm(uinfo->app,smile5_xpm);
  uinfo->smile_pix[5]=Loadxpm(uinfo->app,smile6_xpm);
  uinfo->smile_pix[6]=Loadxpm(uinfo->app,smile7_xpm);
  uinfo->smile_pix[7]=Loadxpm(uinfo->app,smile8_xpm);
#endif
}

void GripUpdate(GtkWidget *app)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)gtk_object_get_user_data(GTK_OBJECT(app));

  if(ginfo->ffwding) FastFwd(ginfo);
  if(ginfo->rewinding) Rewind(ginfo);

#ifdef GRIPCD
  if(!ginfo->have_disc)
    CheckNewDisc(ginfo);

  if(ginfo->auto_eject_countdown && !(--ginfo->auto_eject_countdown))
    EjectDisc(&(ginfo->disc));

  UpdateDisplay(ginfo);
#else
  if(ginfo->ripping|ginfo->encoding) UpdateRipProgress(ginfo);

  if(!ginfo->ripping) {
    if(!ginfo->have_disc)
      CheckNewDisc(ginfo);
    
    UpdateDisplay(ginfo);
  }
#endif
}

void Busy(GripGUI *uinfo)
{
  gdk_window_set_cursor(uinfo->app->window,uinfo->wait_cursor);

  UpdateGTK();
}

void UnBusy(GripGUI *uinfo)
{
  gdk_window_set_cursor(uinfo->app->window,NULL);

  UpdateGTK();
}

static void DoLoadConfig(GripInfo *ginfo)
{
  GripGUI *uinfo=&(ginfo->gui_info);
  char filename[256];
  char renamefile[256];
  char *proxy_env,*tok;
  char outputdir[256];
  int confret;
  CFGEntry cfg_entries[]={
    CFG_ENTRIES
    {"outputdir",CFG_ENTRY_STRING,256,outputdir},
    {"",CFG_ENTRY_LAST,0,NULL}
  };

  outputdir[0]='\0';

  uinfo->minimized=FALSE;
  uinfo->keep_min_size=TRUE;
  uinfo->volvis=FALSE;
  uinfo->track_prog_visible=FALSE;
  uinfo->track_edit_visible=FALSE;

  uinfo->wait_cursor=gdk_cursor_new(GDK_WATCH);

  uinfo->id3_genre_item_list=NULL;

  strcpy(ginfo->cd_device,"/dev/cdrom");
  *ginfo->force_scsi='\0';

  ginfo->local_mode=FALSE;
  ginfo->have_disc=FALSE;
  ginfo->tray_open=FALSE;
  ginfo->faulty_eject=FALSE;
  ginfo->looking_up=FALSE;
  ginfo->play_mode=PM_NORMAL;
  ginfo->playloop=TRUE;
  ginfo->automatic_reshuffle=TRUE;
  ginfo->ask_submit=FALSE;
  ginfo->is_new_disc=FALSE;
  ginfo->first_time=TRUE;
  ginfo->automatic_discdb=TRUE;
  ginfo->auto_eject_countdown=0;
  ginfo->current_discid=0;
  ginfo->volume=255;

  ginfo->changer_slots=0;
  ginfo->current_disc=0;

  ginfo->proxy_server.name[0]='\0';
  ginfo->proxy_server.port=8000;
  ginfo->use_proxy=FALSE;
  ginfo->use_proxy_env=FALSE;

  strcpy(ginfo->dbserver.name,"freedb.freedb.org");
  strcpy(ginfo->dbserver.cgi_prog,"~cddb/cddb.cgi");
  ginfo->dbserver.port=80;
  ginfo->dbserver.use_proxy=0;
  ginfo->dbserver.proxy=&(ginfo->proxy_server);

  strcpy(ginfo->dbserver2.name,"");
  strcpy(ginfo->dbserver2.cgi_prog,"~cddb/cddb.cgi");
  ginfo->dbserver2.port=80;
  ginfo->dbserver2.use_proxy=0;
  ginfo->dbserver2.proxy=&(ginfo->proxy_server);

  strcpy(ginfo->discdb_submit_email,"freedb-submit@freedb.org");
  ginfo->db_use_freedb=TRUE;
  *ginfo->user_email='\0';

  ginfo->local_mode=FALSE;
  ginfo->update_required=FALSE;
  ginfo->looking_up=FALSE;
  ginfo->ask_submit=FALSE;
  ginfo->is_new_disc=FALSE;
  ginfo->automatic_discdb=TRUE;
  ginfo->play_first=TRUE;
  ginfo->play_on_insert=FALSE;
  ginfo->stop_first=FALSE;
  ginfo->no_interrupt=FALSE;
  ginfo->playing=FALSE;
  ginfo->stopped=FALSE;
  ginfo->ffwding=FALSE;
  ginfo->rewinding=FALSE;

  strcpy(ginfo->title_split_chars,"/");

  ginfo->num_cpu=1;
  ginfo->ripping=FALSE;
  ginfo->ripping_a_disc=FALSE;
  ginfo->encoding=FALSE;
  ginfo->rip_partial=FALSE;
  ginfo->stop_rip=FALSE;
  ginfo->stop_encode=FALSE;
  ginfo->num_wavs=0;
  ginfo->doencode=FALSE;
  ginfo->encode_list=NULL;
  ginfo->do_redirect=TRUE;
  ginfo->selected_ripper=0;
#ifdef CDPAR
  ginfo->stop_thread_rip_now=FALSE;
  ginfo->using_builtin_cdp=TRUE;
  ginfo->disable_paranoia=FALSE;
  ginfo->disable_extra_paranoia=FALSE;
  ginfo->disable_scratch_detect=FALSE;
  ginfo->disable_scratch_repair=FALSE;
  ginfo->calc_gain=FALSE;
#else
  ginfo->using_builtin_cdp=FALSE;
#endif
  ginfo->in_rip_thread=FALSE;
  strcpy(ginfo->ripfileformat,"~/mp3/%A/%d/%n.wav");
#ifdef __linux__
  FindExeInPath("cdparanoia", ginfo->ripexename, sizeof(ginfo->ripexename));
  strcpy(ginfo->ripcmdline,"-d %c %t:[.%s]-%t:[.%e] %w");
#else
  FindExeInPath("cdda2wav", ginfo->ripexename, sizeof(ginfo->ripexename));
#ifdef __sun__
  strcpy(ginfo->ripcmdline,"-x -H -t %t -O wav %w");
#else
  strcpy(ginfo->ripcmdline,"-D %C -x -H -t %t -O wav %w");
#endif /* not sun */
#endif /* not linux */

  ginfo->ripnice=0;
  ginfo->max_wavs=99;
  ginfo->auto_rip=FALSE;
  ginfo->beep_after_rip=TRUE;
  ginfo->eject_after_rip=TRUE;
  ginfo->eject_delay=0;
  *ginfo->wav_filter_cmd='\0';
  *ginfo->disc_filter_cmd='\0';
  ginfo->selected_encoder=1;
  strcpy(ginfo->mp3cmdline,"-h -b %b %w %m");
  FindExeInPath("lame", ginfo->mp3exename, sizeof(ginfo->mp3exename));
  strcpy(ginfo->mp3fileformat,"~/mp3/%A/%d/%n.mp3");
  ginfo->mp3nice=0;
  *ginfo->mp3_filter_cmd='\0';
  ginfo->delete_wavs=TRUE;
  ginfo->add_to_db=FALSE;
  ginfo->add_m3u=TRUE;
  ginfo->rel_m3u=TRUE;
  strcpy(ginfo->m3ufileformat,"~/mp3/%A-%d.m3u");
  ginfo->kbits_per_sec=128;
  ginfo->edit_num_cpu=1;
  ginfo->doid3=TRUE;
  ginfo->doid3=FALSE;
  strcpy(ginfo->id3_comment,"Created by Grip");
  *ginfo->cdupdate='\0';
  ginfo->sprefs.no_lower_case=FALSE;
  ginfo->sprefs.allow_high_bits=FALSE;
  ginfo->sprefs.no_underscore=FALSE;
  *ginfo->sprefs.allow_these_chars='\0';

  sprintf(filename,"%s/.grip",getenv("HOME"));

  confret=LoadConfig(filename,"GRIP",2,2,cfg_entries);

  if(confret<0) {
    /* Check if the config is out of date */
    if(confret==-2) {
      DisplayMsg(_("Your config file is out of date -- "
		   "resetting to defaults.\n"
		   "You will need to re-configure Grip.\n"
		   "Your old config file has been saved in ~/.grip-old."));

      sprintf(renamefile,"%s-old",filename);

      rename(filename,renamefile);
    }

    DoSaveConfig(ginfo);
  }

#ifndef GRIPCD
  /* Phase out 'outputdir' variable */

  if(*outputdir) {
    strcpy(filename,outputdir);
    MakePath(filename);
    strcat(filename,ginfo->mp3fileformat);
    strcpy(ginfo->mp3fileformat,filename);

    strcpy(filename,outputdir);
    MakePath(filename);
    strcat(filename,ginfo->ripfileformat);
    strcpy(ginfo->ripfileformat,filename);

    *outputdir='\0';
  }
#endif

  ginfo->dbserver2.use_proxy=ginfo->dbserver.use_proxy=ginfo->use_proxy;
  ginfo->dbserver2.proxy=ginfo->dbserver.proxy;

  ginfo->num_cpu=ginfo->edit_num_cpu;

  if(!*ginfo->user_email)
#if defined(__sun__)
    g_snprintf(ginfo->user_email,256,"%s@%s",getenv("USER"),getenv("HOST"));
#else
    g_snprintf(ginfo->user_email,256,"%s@%s",getenv("USER"),
	       getenv("HOSTNAME"));
#endif

  if(ginfo->use_proxy_env) {   /* Get proxy info from "http_proxy" */
    proxy_env=getenv("http_proxy");

    if(proxy_env) {
      
      /* Skip the "http://" if it's present */
      
      if(!strncasecmp(proxy_env,"http://",7)) proxy_env+=7;
      
      tok=strtok(proxy_env,":");
      if(tok) strncpy(ginfo->proxy_server.name,tok,256);
      
      tok=strtok(NULL,"/");
      if(tok) ginfo->proxy_server.port=atoi(tok);
      
      Debug(_("server is %s, port %d\n"),ginfo->proxy_server.name,
	    ginfo->proxy_server.port);
    }
  }
}

void DoSaveConfig(GripInfo *ginfo)
{
  char filename[256];
  GripGUI *uinfo=&(ginfo->gui_info);
  CFGEntry cfg_entries[]={
    CFG_ENTRIES
    {"",CFG_ENTRY_LAST,0,NULL}
  };

  if(ginfo->edit_num_cpu>MAX_NUM_CPU) ginfo->edit_num_cpu=MAX_NUM_CPU;

  g_snprintf(filename,256,"%s/.grip",getenv("HOME"));

  if(!SaveConfig(filename,"GRIP",2,cfg_entries))
    DisplayMsg(_("Error: Unable to save config file"));
}

/* Shut down stuff (generally before an exec) */
void CloseStuff(void *user_data)
{
  GripInfo *ginfo;
  int fd;

  ginfo=(GripInfo *)user_data;

  close(ConnectionNumber(GDK_DISPLAY()));
  close(ginfo->disc.cd_desc);

  fd=open("/dev/null",O_RDWR);
  dup2(fd,0);

  if(ginfo->do_redirect) {
    dup2(fd,1);
    dup2(fd,2);
  }

  /* Close any other filehandles that might be around */
  for(fd=3;fd<NOFILE;fd++) {
    close(fd);
  }
}
