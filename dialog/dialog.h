/*****************************************************************

  dialog.h -- header file for dialog routines

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

/* Routines from fileselect.c */

GtkWidget *MakeFileSelection(GtkWidget **entry,int len,char *var,char *name);
void FileDialog(char *winname,char *filename,char *doit,GtkSignalFunc doitfunc,
		char *cancel,GtkSignalFunc cancelfunc);


/* Routines from message.c */

void DisplayMsg(char *msg);
void BoolDialog(char *question,char *yes,GtkSignalFunc yesfunc,
		char *no,GtkSignalFunc nofunc);


/* Routines from input.c */

void InputDialog(char *prompt,char *default_str,int len,char *doit,
		 GtkSignalFunc doitfunc,
		 char *cancel,GtkSignalFunc cancelfunc);
void ChangeStrVal(GtkWidget *widget,gpointer data);
GtkWidget *MakeStrEntry(GtkWidget **entry,char *var,char *name,
			int len,gboolean editable);
void ChangeIntVal(GtkWidget *widget,gpointer data);
GtkWidget *MakeNumEntry(GtkWidget **entry,int *var,char *name,int len);
void ChangeDoubleVal(GtkWidget *widget,gpointer data);
GtkWidget *MakeDoubleEntry(GtkWidget **entry,gdouble *var,char *name);
void ChangeBoolVal(GtkWidget *widget,gpointer data);
GtkWidget *MakeCheckButton(GtkWidget **button,gboolean *var,char *name);



/* Routines from progress.c */

GtkWidget *ProgressDialog(char *winname,char *cancel,GtkSignalFunc cancelfunc);
void UpdateProgress(GtkWidget *progdialog,gfloat value);
