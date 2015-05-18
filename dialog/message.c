/*****************************************************************

  message.c -- message display dialog routines

  Copyright (c) 1999 by Mike Oliphant - oliphant@gtk.org

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
#include <gtk/gtk.h>
#include "dialog.h"

void DisplayMsg(char *msg)
{
  BoolDialog(msg,"OK",NULL,NULL,NULL);
}

void BoolDialog(char *question,char *yes,GtkSignalFunc yesfunc,
		char *no,GtkSignalFunc nofunc)
{
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *yesbutton;
  GtkWidget *nobutton;

  dialog=gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(dialog),"System Message");

  gtk_container_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),5);

  label=gtk_label_new(question);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),label,TRUE,TRUE,0);
  gtk_widget_show(label);

  yesbutton=gtk_button_new_with_label(yes);
  if(yesfunc)
    gtk_signal_connect(GTK_OBJECT(yesbutton),"clicked",
		       yesfunc,NULL);
  gtk_signal_connect_object(GTK_OBJECT(yesbutton),"clicked",
			    GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    GTK_OBJECT(dialog));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),yesbutton,
		     TRUE,TRUE,0);
  gtk_widget_show(yesbutton);

  if(no) {
    nobutton=gtk_button_new_with_label(no);
    if(nofunc)
      gtk_signal_connect(GTK_OBJECT(nobutton),"clicked",
			 nofunc,NULL);
    gtk_signal_connect_object(GTK_OBJECT(nobutton),"clicked",
			      GTK_SIGNAL_FUNC(gtk_widget_destroy),
			      GTK_OBJECT(dialog));
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),nobutton,
		       TRUE,TRUE,0);
    gtk_widget_show(nobutton);
  }

  gtk_widget_show(dialog);

  gtk_grab_add(dialog);
}
