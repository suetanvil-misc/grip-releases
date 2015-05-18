/* gripcfg.c
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
#include <unistd.h>
#include "grip.h"
#include "gripcfg.h"
#include "dialog.h"

static void UseProxyChanged(GtkWidget *widget,gpointer data);
static void RipperSelected(GtkWidget *widget,gpointer data);
static void EncoderSelected(GtkWidget *widget,gpointer data);

static char *bin_search_paths[]={
  "/usr/bin","/usr/local/bin","/cpd/misc/bin",NULL
};

static Ripper ripper_defaults[]={
#ifdef CDPAR
  {"grip (cdparanoia)",""},
#endif
  {"cdparanoia","-d %*c %t:[.%s]-%t:[.%e] %*w"},
#ifdef SOLARIS
  {"cdda2wav","-x -H -t %t -O wav %*w"},
#else
  {"cdda2wav","-D %*c -x -H -t %t -O wav %*w"},
#endif
  {"other",""},
  {"",""}
};

static MP3Encoder encoder_defaults[]={{"bladeenc","-%b -QUIT %w %m"},
				      {"lame","-h -b %b %w %m"},
				      {"l3enc","-br %b %w %m"},
				      {"xingmp3enc","-B %b -Q %w"},
				      {"mp3encode","-p 2 -l 3 -b %b %w %m"},
				      {"gogo","-b %b %w %m"},
				      {"oggenc","-o %m -a \"%a\" -l \"%d\" -t \"%n\" %w"},
				      {"flac","-V -o %o %f"},
				      {"other",""},
				      {"",""}
};

static void UseProxyChanged(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  ginfo->dbserver2.use_proxy=ginfo->dbserver.use_proxy=ginfo->use_proxy;
}

void MakeConfigPage(GripInfo *ginfo)
{
  GripGUI *uinfo;
  GtkWidget *vbox,*vbox2,*dbvbox;
  GtkWidget *entry;
  GtkWidget *realentry;
  GtkWidget *label;
  GtkWidget *page,*page2;
  GtkWidget *check;
  GtkWidget *notebook;
  GtkWidget *config_notebook;
  GtkWidget *configpage;
  GtkWidget *button;
#ifndef GRIPCD
  GtkWidget *hsep;
  GtkWidget *hbox;
  GtkWidget *menu,*optmenu;
  GtkWidget *item;
  MP3Encoder *enc;
  Ripper *rip;
#endif

  uinfo=&(ginfo->gui_info);

  configpage=MakeNewPage(uinfo->notebook,"Config");

  vbox2=gtk_vbox_new(FALSE,0);
  config_notebook=gtk_notebook_new();

  page=gtk_frame_new(NULL);
  vbox=gtk_vbox_new(FALSE,2);

  entry=MakeStrEntry(NULL,ginfo->cd_device,"CDRom device",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  check=MakeCheckButton(NULL,&ginfo->no_interrupt,
			"Don't interrupt playback on exit/startup");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);
 
  check=MakeCheckButton(NULL,&ginfo->stop_first,"Rewind when stopped");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&ginfo->play_first,
			"Startup with first track if not playing");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&ginfo->automatic_reshuffle,
			"Reshuffle before each playback");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&ginfo->faulty_eject,"Work around faulty eject");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
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

  vbox=gtk_vbox_new(FALSE,4);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  hbox=gtk_hbox_new(FALSE,3);

  label=gtk_label_new("Ripper:");
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
  gtk_widget_show(label);

  menu=gtk_menu_new();

  rip=ripper_defaults;

  while(*(rip->name)) {
    item=gtk_menu_item_new_with_label(rip->name);
    gtk_object_set_user_data(GTK_OBJECT(item),(gpointer)rip);
    gtk_signal_connect(GTK_OBJECT(item),"activate",
    		       GTK_SIGNAL_FUNC(RipperSelected),(gpointer)ginfo);
    gtk_menu_append(GTK_MENU(menu),item);
    gtk_widget_show(item);

    rip++;
  }

  gtk_menu_set_active(GTK_MENU(menu),ginfo->selected_ripper);

  optmenu=gtk_option_menu_new();
  gtk_option_menu_set_menu(GTK_OPTION_MENU(optmenu),menu);
  gtk_widget_show(menu);
  gtk_box_pack_start(GTK_BOX(hbox),optmenu,TRUE,TRUE,0);
  gtk_widget_show(optmenu);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  hsep=gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(vbox),hsep,FALSE,FALSE,0);
  gtk_widget_show(hsep);

  uinfo->rip_exe_box=gtk_vbox_new(FALSE,2);

  entry=MakeStrEntry(&(uinfo->ripexename_entry),ginfo->ripexename,
		     "Ripping executable",255,TRUE);
  gtk_box_pack_start(GTK_BOX(uinfo->rip_exe_box),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  entry=MakeStrEntry(&(uinfo->ripcmdline_entry),ginfo->ripcmdline,
		     "Rip command-line",255,TRUE);
  gtk_box_pack_start(GTK_BOX(uinfo->rip_exe_box),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  gtk_box_pack_start(GTK_BOX(vbox),uinfo->rip_exe_box,FALSE,FALSE,0);
  if(!ginfo->using_builtin_cdp) gtk_widget_show(uinfo->rip_exe_box);

#ifdef CDPAR
  uinfo->rip_builtin_box=gtk_vbox_new(FALSE,2);

  check=MakeCheckButton(NULL,&ginfo->disable_paranoia,"Disable paranoia");
  gtk_box_pack_start(GTK_BOX(uinfo->rip_builtin_box),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&ginfo->disable_extra_paranoia,
			"Disable extra paranoia");
  gtk_box_pack_start(GTK_BOX(uinfo->rip_builtin_box),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  hbox=gtk_hbox_new(FALSE,3);

  label=gtk_label_new("Disable scratch");
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
  gtk_widget_show(label);
  
  check=MakeCheckButton(NULL,&ginfo->disable_scratch_detect,"detection");
  gtk_box_pack_start(GTK_BOX(hbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&ginfo->disable_scratch_repair,"repair");
  gtk_box_pack_start(GTK_BOX(hbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);

  gtk_box_pack_start(GTK_BOX(uinfo->rip_builtin_box),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);
  
  entry=MakeStrEntry(NULL,ginfo->force_scsi,"Generic SCSI device",
		     255,TRUE);
  gtk_box_pack_start(GTK_BOX(uinfo->rip_builtin_box),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  gtk_box_pack_start(GTK_BOX(vbox),uinfo->rip_builtin_box,FALSE,FALSE,0);
  if(ginfo->using_builtin_cdp) gtk_widget_show(uinfo->rip_builtin_box);
#endif

  entry=MakeStrEntry(NULL,ginfo->ripfileformat,"Rip file format",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  gtk_container_add(GTK_CONTAINER(page2),vbox);
  gtk_widget_show(vbox);

  label=gtk_label_new("Ripper");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),page2,label);
  gtk_widget_show(page2);

  page2=gtk_frame_new(NULL);

  vbox=gtk_vbox_new(FALSE,2);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  entry=MakeNumEntry(NULL,&ginfo->ripnice,"Rip 'nice' value",3);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);
  
  entry=MakeNumEntry(NULL,&ginfo->max_wavs,"Max non-encoded .wav's",3);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  check=MakeCheckButton(NULL,&ginfo->auto_rip,"Auto-rip on insert");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&ginfo->beep_after_rip,"Beep after rip");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&ginfo->eject_after_rip,"Auto-eject after rip");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  entry=MakeNumEntry(NULL,&ginfo->eject_delay,"Auto-eject delay",3);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  entry=MakeStrEntry(NULL,ginfo->wav_filter_cmd,"Wav filter command",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
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

  vbox=gtk_vbox_new(FALSE,4);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  hbox=gtk_hbox_new(FALSE,3);

  label=gtk_label_new("Encoder:");
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
  gtk_widget_show(label);

  menu=gtk_menu_new();

  enc=encoder_defaults;

  while(*(enc->name)) {
    item=gtk_menu_item_new_with_label(enc->name);
    gtk_object_set_user_data(GTK_OBJECT(item),(gpointer)enc);
    gtk_signal_connect(GTK_OBJECT(item),"activate",
    		       GTK_SIGNAL_FUNC(EncoderSelected),(gpointer)ginfo);
    gtk_menu_append(GTK_MENU(menu),item);
    gtk_widget_show(item);

    enc++;
  }

  gtk_menu_set_active(GTK_MENU(menu),ginfo->selected_encoder);

  optmenu=gtk_option_menu_new();
  gtk_option_menu_set_menu(GTK_OPTION_MENU(optmenu),menu);
  gtk_widget_show(menu);
  gtk_box_pack_start(GTK_BOX(hbox),optmenu,TRUE,TRUE,0);
  gtk_widget_show(optmenu);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  hsep=gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(vbox),hsep,FALSE,FALSE,0);
  gtk_widget_show(hsep);

  entry=MakeStrEntry(&(uinfo->mp3exename_entry),ginfo->mp3exename,
		     "MP3 executable",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  entry=MakeStrEntry(&(uinfo->mp3cmdline_entry),ginfo->mp3cmdline,
		     "MP3 command-line",
		     255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  entry=MakeStrEntry(NULL,ginfo->mp3fileformat,"MP3 file format",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  gtk_container_add(GTK_CONTAINER(page2),vbox);
  gtk_widget_show(vbox);

  label=gtk_label_new("Encoder");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),page2,label);
  gtk_widget_show(page2);

  page2=gtk_frame_new(NULL);

  vbox=gtk_vbox_new(FALSE,0);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  check=MakeCheckButton(NULL,&ginfo->delete_wavs,
			"Delete .wav after encoding");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&ginfo->add_to_db,
			"Insert info into SQL database");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&ginfo->add_m3u,"Create .m3u files");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&ginfo->rel_m3u,
			"Use relative paths in .m3u files");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  entry=MakeStrEntry(NULL,ginfo->m3ufileformat,"M3U file format",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  entry=MakeNumEntry(NULL,&ginfo->kbits_per_sec,
		     "Encoding bitrate (kbits/sec)",3);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);
  
  entry=MakeNumEntry(NULL,&ginfo->edit_num_cpu,"Number of CPUs to use",3);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);
  
  entry=MakeNumEntry(NULL,&ginfo->mp3nice,"Mp3 'nice' value",3);
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

  vbox=gtk_vbox_new(FALSE,2);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  check=MakeCheckButton(NULL,&ginfo->doid3,"Add ID3 tags to MP3 files");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

#ifdef HAVE_ID3LIB
  check=MakeCheckButton(NULL,&ginfo->doid3v2,"Add ID3v2 tags to MP3 files");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);
#endif

  entry=MakeStrEntry(NULL,ginfo->id3_comment,"ID3 comment field",29,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  gtk_container_add(GTK_CONTAINER(page),vbox);
  gtk_widget_show(vbox);

  label = gtk_label_new("ID3");
  gtk_notebook_append_page(GTK_NOTEBOOK(config_notebook),page,label);
  gtk_widget_show(page);
#endif

  page=gtk_frame_new(NULL);

  dbvbox=gtk_vbox_new(FALSE,4);
  gtk_container_border_width(GTK_CONTAINER(dbvbox),3);

  notebook=gtk_notebook_new();

  page2=gtk_frame_new(NULL);

  vbox=gtk_vbox_new(FALSE,2);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  entry=MakeStrEntry(NULL,ginfo->dbserver.name,"DB server",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  entry=MakeStrEntry(NULL,ginfo->dbserver.cgi_prog,"CGI path",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  gtk_container_add(GTK_CONTAINER(page2),vbox);
  gtk_widget_show(vbox);

  label=gtk_label_new("Primary Server");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),page2,label);
  gtk_widget_show(page2);

  page2=gtk_frame_new(NULL);

  vbox=gtk_vbox_new(FALSE,2);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  entry=MakeStrEntry(NULL,ginfo->dbserver2.name,"DB server",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  entry=MakeStrEntry(NULL,ginfo->dbserver2.cgi_prog,"CGI path",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  gtk_container_add(GTK_CONTAINER(page2),vbox);
  gtk_widget_show(vbox);

  label=gtk_label_new("Secondary Server");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),page2,label);
  gtk_widget_show(page2);


  gtk_box_pack_start(GTK_BOX(dbvbox),notebook,FALSE,FALSE,0);
  gtk_widget_show(notebook);


  entry=MakeStrEntry(NULL,ginfo->discdb_submit_email,
		     "DB Submit email",255,TRUE);
  gtk_box_pack_start(GTK_BOX(dbvbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  check=MakeCheckButton(NULL,&ginfo->db_use_freedb,
			"Use freedb extensions");
  gtk_box_pack_start(GTK_BOX(dbvbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&ginfo->automatic_discdb,
			"Perform disc lookups automatically");
  gtk_box_pack_start(GTK_BOX(dbvbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  gtk_container_add(GTK_CONTAINER(page),dbvbox);
  gtk_widget_show(dbvbox);


  label=gtk_label_new("DiscDB");
  gtk_notebook_append_page(GTK_NOTEBOOK(config_notebook),page,label);
  gtk_widget_show(page);

  page=gtk_frame_new(NULL);

  vbox=gtk_vbox_new(FALSE,2);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  check=MakeCheckButton(&button,&ginfo->use_proxy,"Use proxy server");
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
  		     GTK_SIGNAL_FUNC(UseProxyChanged),(gpointer)ginfo);
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&ginfo->use_proxy_env,
			"Get server from 'http_proxy' env. var");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  entry=MakeStrEntry(NULL,ginfo->proxy_server.name,"Proxy server",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  entry=MakeNumEntry(NULL,&(ginfo->proxy_server.port),"Proxy port",5);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  entry=MakeStrEntry(NULL,ginfo->proxy_server.username,"Proxy username",
		     80,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  entry=MakeStrEntry(&realentry,ginfo->proxy_server.pswd,
		     "Proxy password",80,TRUE);
  gtk_entry_set_visibility(GTK_ENTRY(realentry),FALSE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  gtk_container_add(GTK_CONTAINER(page),vbox);
  gtk_widget_show(vbox);

  label=gtk_label_new("Proxy");
  gtk_notebook_append_page(GTK_NOTEBOOK(config_notebook),page,label);
  gtk_widget_show(page);

  page=gtk_frame_new(NULL);

  vbox=gtk_vbox_new(FALSE,2);
  gtk_container_border_width(GTK_CONTAINER(vbox),3);

  entry=MakeStrEntry(NULL,ginfo->user_email,"Email address",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  entry=MakeStrEntry(NULL,ginfo->cdupdate,"CD update program",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  check=MakeCheckButton(NULL,&ginfo->sprefs.no_lower_case,
			"Do not lowercase filenames");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);
   
  check=MakeCheckButton(NULL,&ginfo->sprefs.allow_high_bits,
			"Allow high bits in filenames");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  check=MakeCheckButton(NULL,&ginfo->sprefs.no_underscore,
			"Do not change spaces to underscores");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  entry=MakeStrEntry(NULL,ginfo->sprefs.allow_these_chars,
		     "Characters to not strip\nin filenames",255,TRUE);
  gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
  gtk_widget_show(entry);

  check=MakeCheckButton(NULL,&(ginfo->gui_info.keep_min_size),
			"Keep application minimum size");
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_widget_show(check);

  gtk_container_add(GTK_CONTAINER(page),vbox);
  gtk_widget_show(vbox);

  label=gtk_label_new("Misc");
  gtk_notebook_append_page(GTK_NOTEBOOK(config_notebook),page,label);
  gtk_widget_show(page);

  gtk_box_pack_start(GTK_BOX(vbox2),config_notebook,FALSE,FALSE,0);
  gtk_widget_show(config_notebook);

  gtk_container_add(GTK_CONTAINER(configpage),vbox2);
  gtk_widget_show(vbox2);
}

static void RipperSelected(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;
  GripGUI *uinfo;
  Ripper *rip;
  char buf[256];
  char *path=NULL;

  ginfo=(GripInfo *)data;
  uinfo=&(ginfo->gui_info);
  rip=(Ripper *)gtk_object_get_user_data(GTK_OBJECT(widget));

  ginfo->selected_ripper=rip-ripper_defaults;

#ifdef CDPAR
  if(ginfo->selected_ripper==0) {
    ginfo->using_builtin_cdp=TRUE;

    gtk_widget_hide(uinfo->rip_exe_box);
    gtk_widget_show(uinfo->rip_builtin_box);
  }
  else {
    ginfo->using_builtin_cdp=FALSE;

    gtk_widget_show(uinfo->rip_exe_box);
    gtk_widget_hide(uinfo->rip_builtin_box);
  }
#endif

  if(!ginfo->using_builtin_cdp) {
    if(strcmp(rip->name,"other")) {
      path=FindExe(rip->name,bin_search_paths);
      
      if(!path) path=bin_search_paths[0];
      
      g_snprintf(buf,256,"%s/%s",path,rip->name);
      
      gtk_entry_set_text(GTK_ENTRY(uinfo->ripexename_entry),buf);
    }
    else gtk_entry_set_text(GTK_ENTRY(uinfo->ripexename_entry),"");
    
    gtk_entry_set_text(GTK_ENTRY(uinfo->ripcmdline_entry),rip->cmdline);
  }
}

static void EncoderSelected(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;
  GripGUI *uinfo;
  MP3Encoder *enc;
  char buf[256];
  char *path=NULL;

  ginfo=(GripInfo *)data;
  uinfo=&(ginfo->gui_info);
  enc=(MP3Encoder *)gtk_object_get_user_data(GTK_OBJECT(widget));

  if(strcmp(enc->name,"other")) {
    path=FindExe(enc->name,bin_search_paths);

    if(!path) path=bin_search_paths[0];
    
    g_snprintf(buf,256,"%s/%s",path,enc->name);

    gtk_entry_set_text(GTK_ENTRY(uinfo->mp3exename_entry),buf);
  }
  else gtk_entry_set_text(GTK_ENTRY(uinfo->mp3exename_entry),"");

  gtk_entry_set_text(GTK_ENTRY(uinfo->mp3cmdline_entry),enc->cmdline);

  ginfo->selected_encoder=enc-encoder_defaults;
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

gboolean FileExists(char *filename)
{
  struct stat mystat;

  return (stat(filename,&mystat)>=0);
}

