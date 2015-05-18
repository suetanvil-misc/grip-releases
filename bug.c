/*****************************************************************

  bug.c  -- bug report form for Grip

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

#include <stdlib.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include "grip.h"
#include "config.h"

extern char *Program;
extern char *Version;

void SubmitBugReport(void);

extern char user_email[256];	// This is an option field defined in grip.c 

GtkWidget *bug_dialog;
GtkWidget *sys_info_entry;
GtkWidget *prob_summ_entry;
GtkWidget *bug_text;

void BugReport(void)
{
  GtkWidget *label;
  GtkWidget *button;

  bug_dialog=gtk_dialog_new();
  gtk_container_border_width(GTK_CONTAINER(GTK_DIALOG(bug_dialog)->vbox),5);
  gtk_window_set_title(GTK_WINDOW(bug_dialog),"Bug Report");

  label=gtk_label_new("System info (CPU,OS,GTK+ ver,X11 ver, etc.)");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(bug_dialog)->vbox),label,
		     FALSE,FALSE,0);
  gtk_widget_show(label);
  
  sys_info_entry=gtk_entry_new_with_max_length(256);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(bug_dialog)->vbox),sys_info_entry,
		     FALSE,FALSE,0);
  gtk_widget_show(sys_info_entry);


  label=gtk_label_new("Short summary of problem");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(bug_dialog)->vbox),label,
		     FALSE,FALSE,0);
  gtk_widget_show(label);
  
  prob_summ_entry=gtk_entry_new_with_max_length(256);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(bug_dialog)->vbox),prob_summ_entry,
		     FALSE,FALSE,0);
  gtk_widget_show(prob_summ_entry);


  label=gtk_label_new("Bug report");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(bug_dialog)->vbox),label,
		     FALSE,FALSE,5);
  gtk_widget_show(label);

  bug_text=gtk_text_new(NULL,NULL);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(bug_dialog)->vbox),bug_text,
		     TRUE,TRUE,0);
  gtk_text_set_editable(GTK_TEXT(bug_text),TRUE);
  gtk_text_set_word_wrap(GTK_TEXT(bug_text),TRUE);
  gtk_widget_show(bug_text);


  button=gtk_button_new_with_label("Submit");
  gtk_signal_connect(GTK_OBJECT(button),"clicked",
		     SubmitBugReport,NULL);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(bug_dialog)->action_area),button,
		     TRUE,TRUE,0);
  gtk_widget_show(button);

  button=gtk_button_new_with_label("Cancel");
  gtk_signal_connect_object(GTK_OBJECT(button),"clicked",
			    GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    GTK_OBJECT(bug_dialog));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(bug_dialog)->action_area),button,
		     TRUE,TRUE,0);
  gtk_widget_show(button);


  gtk_widget_show(bug_dialog);
  gtk_grab_add(bug_dialog);
}

void SubmitBugReport(void)
{
  char mailcmd[256];
  int fd;
  FILE *bfp;
  char filename[256];

  if(!*gtk_entry_get_text(GTK_ENTRY(sys_info_entry))) {
    BoolDialog("Please provide your system information","OK",NULL,NULL,NULL);

    return;
  }

  if(!*gtk_entry_get_text(GTK_ENTRY(prob_summ_entry))) {
    BoolDialog("Please provide a summary of your problem","OK",NULL,NULL,NULL);

    return;
  }

  if(!*gtk_editable_get_chars(GTK_EDITABLE(bug_text),0,-1)) {
    BoolDialog("That wasn't much of a bug report!","OK",NULL,NULL,NULL);

    return;
  }

  sprintf (filename, "/tmp/grip.XXXXXX");
  fd = mkstemp (filename);
  if (fd == -1) {
    printf("Error: Unable to create temporary file\n\n");
    return;
  }
  
  bfp=fdopen(fd,"w");
  if(!bfp) {
    close (fd);
    printf("Error: Unable to create temporary file\n");
    return;
  }

/* File header contains sender, recipient and subject */

  fprintf(bfp,"To: %s\nFrom: %s\nSubject: %s bug report\n\n", 	
  	MAINTAINER,user_email,Program);	
  fprintf(bfp,"%s %s\n",Program,Version);
  fprintf(bfp,"Run on GTK+ version %d.%d.%d\n",gtk_major_version,
	  gtk_minor_version,gtk_micro_version);
  fprintf(bfp,"Compiled on GTK+ versions %d.%d.%d\n",GTK_MAJOR_VERSION,
	  GTK_MINOR_VERSION,GTK_MICRO_VERSION);
  fprintf(bfp,"%s/%s\n",(getenv("OSTYPE"))?getenv("OSTYPE"):"OS?",
	  (getenv("HOSTTYPE"))?getenv("HOSTTYPE"):"Platform?");
  fprintf(bfp,"User suppiled sysinfo: %s\n",
	  gtk_entry_get_text(GTK_ENTRY(sys_info_entry)));
  fprintf(bfp,"\n");
  fprintf(bfp,"Summary of problem: %s\n",
	  gtk_entry_get_text(GTK_ENTRY(prob_summ_entry)));
  fprintf(bfp,"\n");
  fprintf(bfp,"%s\n",gtk_editable_get_chars(GTK_EDITABLE(bug_text),0,-1));
	  
  fclose(bfp);
  close (fd);

/* We're using sendmail -t, so we just pipe in the file we created and
	let it take care of the addresses and subject */

  snprintf(mailcmd,256,"%s < %s",MAILER,filename);

  system(mailcmd);

  gtk_widget_destroy(bug_dialog);
}
