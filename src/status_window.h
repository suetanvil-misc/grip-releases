/* status_window.h
 *
 * Copyright (c) 1998-2003  Mike Oliphant <oliphant@gtk.org>
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

#include <gtk/gtk.h>
/*#include <zvt/zvtterm.h>*/

typedef struct {
  gboolean embedded;
  GtkWidget *term_widget;
  int pipe[2];
} StatusWindow;

StatusWindow *NewStatusWindow(GtkWidget *box);
void StatusWindowWrite(StatusWindow *sw,char *msg);
int GetStatusWindowPipe(StatusWindow *sw);
