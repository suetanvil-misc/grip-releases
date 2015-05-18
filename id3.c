#include <stdio.h>
#include "grip.h"

void ID3Put(char *dest,char *src,int len);

/* ID3 tag structure */

typedef struct _id3_tag {
  char tag[3];
  char title[30];
  char artist[30];
  char album[30];
  char year[4];
  char comment[29];
  unsigned char tracknum;
  unsigned char genre;
} ID3Tag;

/* Add and ID3v1 tag to a file */

gboolean ID3TagFile(char *filename,char *title,char *artist,char *album,
		    char *year,char *comment,unsigned char genre,
		    unsigned char tracknum)
{
  FILE *fp;
  ID3Tag tag;

  fp=fopen(filename,"a");

  ID3Put(tag.tag,"TAG",3);
  ID3Put(tag.title,title,30);
  ID3Put(tag.artist,artist,30);
  ID3Put(tag.album,album,30);
  ID3Put(tag.year,year,4);
  ID3Put(tag.comment,comment,29);
  tag.tracknum=tracknum;
  tag.genre=genre;

  fwrite(&tag,sizeof(ID3Tag),1,fp);

  fclose(fp);

  return TRUE;
}

/* Copy a string padding with zeros */

void ID3Put(char *dest,char *src,int len)
{
  int pos;
  int srclen;

  srclen=strlen(src);

  for(pos=0;pos<len;pos++) {
    if(pos<srclen) dest[pos]=src[pos];
    else dest[pos]=0;
  }
}
