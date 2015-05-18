/* main.c
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

#include <config.h>
#include <gnome.h>

#include "grip.h"

static gint KillSession(GnomeClient* client, gpointer client_data);
static gint SaveSession(GnomeClient *client, gint phase, 
			GnomeSaveStyle save_style,
			gint is_shutdown, GnomeInteractStyle interact_style,
			gint is_fast, gpointer client_data);
static gint TimeOut(gpointer data);

gboolean do_debug=TRUE;
GtkWidget* grip_app;

/* popt table */
static char *geometry=NULL;
static char *device=NULL;
static int force_small=FALSE;
static int local_mode=FALSE;
static int no_redirect=FALSE;
static int verbose=FALSE;

struct poptOption options[] = {
  { 
    "geometry",
    '\0',
    POPT_ARG_STRING,
    &geometry,
    0,
    N_("Specify the geometry of the main window"),
    N_("GEOMETRY")
  },
  { 
    "device",
    '\0',
    POPT_ARG_STRING,
    &device,
    0,
    N_("Specify the cdrom device to use"),
    N_("DEVICE")
  },
  { 
    "small",
    '\0',
    POPT_ARG_NONE,
    &force_small,
    0,
    N_("Launch in \"small\" (cd-only) mode"),
    NULL
  },
  { 
    "local",
    '\0',
    POPT_ARG_NONE,
    &local_mode,
    0,
    N_("\"Local\" mode -- do not look up disc info on the net"),
    NULL
  },
  { 
    "no-redirect",
    '\0',
    POPT_ARG_NONE,
    &no_redirect,
    0,
    N_("Do not do I/O redirection"),
    NULL
  },
  { 
    "verbose",
    '\0',
    POPT_ARG_NONE,
    &verbose,
    0,
    N_("Run in verbose (debug) mode"),
    NULL
  },
  {
    NULL,
    '\0',
    0,
    NULL,
    0,
    NULL,
    NULL
  }
};

void Debug(char *fmt,...)
{
  va_list args;

  if(do_debug) {
    va_start(args,fmt);

    vprintf(fmt,args);
  }

  va_end(args);
}

int main(int argc, char* argv[])
{
  poptContext pctx;

  char** args;

  GnomeClient *client;

  bindtextdomain(PACKAGE,GNOMELOCALEDIR);  
  textdomain(PACKAGE);

  gnome_init_with_popt_table(PACKAGE,VERSION,argc,argv, 
                             options,0,&pctx);  

  /* Parse args */

  args=(char **)poptGetArgs(pctx);

  poptFreeContext(pctx);

  /* Session Management */
  
  client=gnome_master_client();
  gtk_signal_connect(GTK_OBJECT(client),"save_yourself",
		     GTK_SIGNAL_FUNC(SaveSession),argv[0]);
  gtk_signal_connect(GTK_OBJECT(client),"die",
		     GTK_SIGNAL_FUNC(KillSession),NULL);
  

  do_debug=verbose;

  /* Start a new Grip app */
  grip_app=GripNew(geometry,device,force_small,local_mode,no_redirect);

  gtk_widget_show(grip_app);

  gtk_timeout_add(1000,TimeOut,0);

  gtk_main();

  return 0;
}

/* Save the session */
static gint SaveSession(GnomeClient *client, gint phase,
			GnomeSaveStyle save_style,
			gint is_shutdown, GnomeInteractStyle interact_style,
			gint is_fast, gpointer client_data)
{
  gchar** argv;
  guint argc;

  /* allocate 0-filled, so it will be NULL-terminated */
  argv = g_malloc0(sizeof(gchar*)*4);
  argc = 1;

  argv[0] = client_data;

  gnome_client_set_clone_command(client, argc, argv);
  gnome_client_set_restart_command(client, argc, argv);

  return TRUE;
}

/* Kill Session */
static gint KillSession(GnomeClient* client, gpointer client_data)
{
  gtk_main_quit();

  return TRUE;
}

static gint TimeOut(gpointer data)
{
  GripUpdate(grip_app);

  return TRUE;
}