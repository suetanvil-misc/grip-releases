/*****************************************************************

  parsecfg.c -- routines for parsing a key-based config file

  Copyright (c) 1999 by Mike Oliphant - oliphant@gtk.org

    http://www.ling.ed.ac.uk/~oliphant

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.

*****************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <glib.h>
#include "parsecfg.h"

static gboolean ParseLine(char *buf,CFGEntry *cfg);
static char *ChopWhite(char *buf);


/* Get rid of whitespace at the beginning or end of a string */

static char *ChopWhite(char *buf)
{
  int pos;

  for(pos=strlen(buf)-1;(pos>=0)&&isspace(buf[pos]);pos--);

  buf[pos+1]='\0';

  for(;isspace(*buf);buf++);

  return buf;
}

static gboolean ParseLine(char *buf,CFGEntry *cfg)
{
  int cfgent;
  gboolean found=FALSE;
  char *tok;

  tok=strtok(buf," ");

  if(tok)
    for(cfgent=0;cfg[cfgent].type!=CFG_ENTRY_LAST;cfgent++) {
      if(!strcasecmp(tok,cfg[cfgent].name)) {
	tok=strtok(NULL,"");

	if(tok)
	  switch(cfg[cfgent].type) {
	  case CFG_ENTRY_STRING:
	    strncpy((char *)cfg[cfgent].destvar,
		    ChopWhite(tok),cfg[cfgent].length);
	    break;
	  case CFG_ENTRY_BOOL:
	    *((gboolean *)cfg[cfgent].destvar)=(atoi(tok)==1);
	    break;
	  case CFG_ENTRY_INT:
	    *((int *)cfg[cfgent].destvar)=atoi(tok);
	    break;
	  default:
	    printf("Error: Bad entry type\n");
	    break;
	  }
	
	found=TRUE;
	break;
      }
    }

  /*  if(!found) printf("Error: Unrecognized line: %s\n",buf);*/

  return found;
}

gboolean LoadConfig(char *filename,char *name,int ver,int reqver,CFGEntry *cfg)
{
  char buf[1024];
  FILE *cfp;
  char *tok;

  cfp=fopen(filename,"r");
  if(!cfp) return FALSE;

  fgets(buf,1024,cfp);

  tok=strtok(buf," ");

  if(!tok||(strcasecmp(tok,name))) {
    printf("Error: Invalid config file\n");

    return FALSE;
  }

  tok=strtok(NULL,"");

  if(!tok||(atoi(tok)<reqver)) {
    printf("Error: Config file out of date\n");

    return FALSE;
  }

  while(fgets(buf,1024,cfp)) {
    ParseLine(buf,cfg);
  }

  fclose(cfp);

  return TRUE;
}

gboolean SaveConfig(char *filename,char *name,int ver,CFGEntry *cfg)
{
  FILE *cfp;
  int cfgent;

  cfp=fopen(filename,"w");

  if(!cfp) return FALSE;

  fprintf(cfp,"%s %d\n",name,ver);

  for(cfgent=0;cfg[cfgent].type!=CFG_ENTRY_LAST;cfgent++) {
    fprintf(cfp,"%s ",cfg[cfgent].name);

    switch(cfg[cfgent].type) {
    case CFG_ENTRY_STRING:
      fprintf(cfp,"%s\n",(char *)cfg[cfgent].destvar);
      break;
    case CFG_ENTRY_INT:
      fprintf(cfp,"%d\n",*((int *)cfg[cfgent].destvar));
      break;
    case CFG_ENTRY_BOOL:
      fprintf(cfp,"%d\n",*((int *)cfg[cfgent].destvar)==1);
      break;
    default:
      break;
    }
  }

  fclose(cfp);

  return TRUE;
}
