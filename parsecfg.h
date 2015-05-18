/*****************************************************************

  parsecfg.h

  Copyright (c) 1998 by Mike Oliphant - oliphant@ling.ed.ac.uk

    http://www.ling.ed.ac.uk/~oliphant

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.

*****************************************************************/

/* Config entry types */

typedef enum
{
  CFG_ENTRY_STRING,
  CFG_ENTRY_BOOL,
  CFG_ENTRY_INT,
  CFG_ENTRY_LAST
} CFGEntryType;

typedef struct _cfg_entry
{
  char name[80];
  CFGEntryType type;
  int length;
  void *destvar;
} CFGEntry;


gboolean LoadConfig(char *filename,char *name,int ver,int reqver,
		    CFGEntry *cfg);
gboolean SaveConfig(char *filename,char *name,int ver,CFGEntry *cfg);
