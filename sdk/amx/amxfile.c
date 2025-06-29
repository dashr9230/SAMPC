/* Text file I/O module for the Pawn Abstract Machine
 *
 *  Copyright (c) ITB CompuPhase, 2003-2005
 *
 *  This software is provided "as-is", without any express or implied warranty.
 *  In no event will the authors be held liable for any damages arising from
 *  the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1.  The origin of this software must not be misrepresented; you must not
 *      claim that you wrote the original software. If you use this software in
 *      a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *  2.  Altered source versions must be plainly marked as such, and must not be
 *      misrepresented as being the original software.
 *  3.  This notice may not be removed or altered from any source distribution.
 *
 *  Version: $Id: amxfile.c,v 1.7 2006/04/19 12:00:58 spookie Exp $
 */
#if defined _UNICODE || defined __UNICODE__ || defined UNICODE
# if !defined UNICODE   /* for Windows */
#   define UNICODE
# endif
# if !defined _UNICODE  /* for C library */
#   define _UNICODE
# endif
#endif

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if defined __WIN32__ || defined _WIN32 || defined WIN32 || defined __MSDOS__
  #include <io.h>
  #include <malloc.h>
#endif
#if defined __WIN32__ || defined _WIN32 || defined WIN32 || defined _Windows
  #include <windows.h>
#endif
#include "osdefs.h"
#include "amx.h"

#if !defined AMXFILE_VAR
  #define AMXFILE_VAR   "AMXFILE"
#elif AMXFILE_VAR==""
  #undef AMXFILE_VAR
#endif

#if defined _UNICODE
# include <tchar.h>
#elif !defined __T
  typedef char          TCHAR;
# define __T(string)    string
# define _tfopen        fopen
# define _tgetenv       getenv
# define _tfputs        fputs
# define _tcscat        strcat
# define _tcschr        strchr
# define _tcscpy        strcpy
# define _tcsdup        strdup
# define _tcslen        strlen
# define _tcspbrk       strpbrk
# define _tcsrchr       strrchr
#endif

#if !defined UNUSED_PARAM
  #define UNUSED_PARAM(p) ((void)(p))
#endif

#define MAX_FILE_STREAMS (101)

enum filemode {
  io_read,      /* file must exist */
  io_write,     /* creates a new file */
  io_readwrite, /* file must exist */
  io_append,    /* file must exist, opened for writing only and seek to the end */
};

enum seek_whence {
  seek_start,
  seek_current,
  seek_end,
};

typedef struct file_stream file_stream;

struct file_stream {
  AMX* amx;
  FILE* file;
  file_stream* next;
};

file_stream* filestream=NULL;

int verify_stream(AMX* amx, FILE* file)
{
  file_stream* curr=filestream;

  while (curr!=NULL) {
    if (curr->file==file && (curr->amx==amx || curr->amx==NULL)) {
      return 1;
    }
    curr=curr->next;
  }
  return 0;
}

int add_stream(AMX* amx, FILE* file)
{
  file_stream* stream;

  if ((stream=(file_stream *)malloc(sizeof(file_stream)))==NULL)
    return 0;

  stream->amx=amx;
  stream->file=file;
  stream->next=NULL;

  if (filestream==NULL) {
    filestream=stream;
  } else {
    stream->next=filestream;
    filestream=stream;
  }
  return 1;
}

int delete_stream(AMX* amx, FILE* file)
{
  file_stream* curr=filestream;
  file_stream* prev=NULL;

  while (curr!=NULL) {
    if (curr->amx==amx && curr->file==file) {
      if (prev)
        prev->next=curr->next;
      curr->next=NULL;
      curr->file=NULL;
      curr->amx=NULL;
      free(curr);
      return 1;
    } /* if */
    prev=curr;
    curr=curr->next;
  } /* while */
  return 0;
}

/* This function only stores unpacked strings. UTF-8 is used for
 * Unicode, and packed strings can only store 7-bit and 8-bit
 * character sets (ASCII, Latin-1).
 */
static size_t fgets_cell(FILE *fp,cell *string,size_t max,int utf8mode)
{
  size_t index;
  fpos_t pos;
  cell c;
  int follow,lastcr;
  cell lowmark;

  assert(sizeof(cell)>=4);
  assert(fp!=NULL);
  assert(string!=NULL);
  if (max<=0)
    return 0;

  /* get the position, in case we have to back up */
  fgetpos(fp, &pos);

  index=0;
  follow=0;
  lowmark=0;
  lastcr=0;
  for ( ;; ) {
    assert(index<max);
    if (index==max-1)
      break;                    /* string fully filled */
    if ((c=fgetc(fp))==EOF) {
      if (!utf8mode || follow==0)
        break;                  /* no more characters */
      /* If an EOF happened halfway an UTF-8 code, the string cannot be
       * UTF-8 mode, and we must restart.
       */
      index=0;
      fsetpos(fp, &pos);
      continue;
    } /* if */

    /* 8-bit characters are unsigned */
    if (c<0)
      c=-c;

    if (utf8mode) {
      if (follow>0 && (c & 0xc0)==0x80) {
        /* leader code is active, combine with earlier code */
        string[index]=(string[index] << 6) | ((unsigned char)c & 0x3f);
        if (--follow==0) {
          /* encoding a character in more bytes than is strictly needed,
           * is not really valid UTF-8; we are strict here to increase
           * the chance of heuristic dectection of non-UTF-8 text
           * (JAVA writes zero bytes as a 2-byte code UTF-8, which is invalid)
           */
          if (string[index]<lowmark)
            utf8mode=0;
          /* the code positions 0xd800--0xdfff and 0xfffe & 0xffff do not
           * exist in UCS-4 (and hence, they do not exist in Unicode)
           */
          if (string[index]>=0xd800 && string[index]<=0xdfff
              || string[index]==0xfffe || string[index]==0xffff)
            utf8mode=0;
          index++;
        } /* if */
      } else if (follow==0 && (c & 0x80)==0x80) {
        /* UTF-8 leader code */
        if ((c & 0xe0)==0xc0) {
          /* 110xxxxx 10xxxxxx */
          follow=1;
          lowmark=0x80;
          string[index]=c & 0x1f;
        } else if ((c & 0xf0)==0xe0) {
          /* 1110xxxx 10xxxxxx 10xxxxxx (16 bits, BMP plane) */
          follow=2;
          lowmark=0x800;
          string[index]=c & 0x0f;
        } else if ((c & 0xf8)==0xf0) {
          /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
          follow=3;
          lowmark=0x10000;
          string[index]=c & 0x07;
        } else if ((c & 0xfc)==0xf8) {
          /* 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
          follow=4;
          lowmark=0x200000;
          string[index]=c & 0x03;
        } else if ((c & 0xfe)==0xfc) {
          /* 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx (31 bits) */
          follow=5;
          lowmark=0x4000000;
          string[index]=c & 0x01;
        } else {
          /* this is invalid UTF-8 */
          utf8mode=0;
        } /* if */
      } else if (follow==0 && (c & 0x80)==0x00) {
        /* 0xxxxxxx (US-ASCII) */
        string[index++]=c;
        if (c==__T('\n'))
          break;        /* read newline, done */
      } else {
        /* this is invalid UTF-8 */
        utf8mode=0;
      } /* if */
      if (!utf8mode) {
        /* UTF-8 mode was switched just off, which means that non-conforming
         * UTF-8 codes were found, which means in turn that the string is
         * probably not intended as UTF-8; start over again
         */
        index=0;
        fsetpos(fp, &pos);
      } /* if */
    } else {
      string[index++]=c;
      if (c==__T('\n')) {
        break;                  /* read newline, done */
      } else if (lastcr) {
        ungetc(c,fp);           /* carriage return was read, no newline follows */
        break;
      } /* if */
      lastcr=(c==__T('\r'));
    } /* if */
  } /* for */
  assert(index<max);
  string[index]=__T('\0');

  return index;
}

static size_t fputs_cell(FILE *fp,cell *string,int utf8mode)
{
  size_t count=0;

  assert(sizeof(cell)>=4);
  assert(fp!=NULL);
  assert(string!=NULL);

  while (*string!=0) {
    if (utf8mode) {
      cell c=*string;
      if (c<0x80) {
        /* 0xxxxxxx */
        fputc((unsigned char)c,fp);
      } else if (c<0x800) {
        /* 110xxxxx 10xxxxxx */
        fputc((unsigned char)((c>>6) & 0x1f | 0xc0),fp);
        fputc((unsigned char)(c & 0x3f | 0x80),fp);
      } else if (c<0x10000) {
        /* 1110xxxx 10xxxxxx 10xxxxxx (16 bits, BMP plane) */
        fputc((unsigned char)((c>>12) & 0x0f | 0xe0),fp);
        fputc((unsigned char)((c>>6) & 0x3f | 0x80),fp);
        fputc((unsigned char)(c & 0x3f | 0x80),fp);
      } else if (c<0x200000) {
        /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
        fputc((unsigned char)((c>>18) & 0x07 | 0xf0),fp);
        fputc((unsigned char)((c>>12) & 0x3f | 0x80),fp);
        fputc((unsigned char)((c>>6) & 0x3f | 0x80),fp);
        fputc((unsigned char)(c & 0x3f | 0x80),fp);
      } else if (c<0x4000000) {
        /* 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
        fputc((unsigned char)((c>>24) & 0x03 | 0xf8),fp);
        fputc((unsigned char)((c>>18) & 0x3f | 0x80),fp);
        fputc((unsigned char)((c>>12) & 0x3f | 0x80),fp);
        fputc((unsigned char)((c>>6) & 0x3f | 0x80),fp);
        fputc((unsigned char)(c & 0x3f | 0x80),fp);
      } else {
        /* 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx (31 bits) */
        fputc((unsigned char)((c>>30) & 0x01 | 0xfc),fp);
        fputc((unsigned char)((c>>24) & 0x3f | 0x80),fp);
        fputc((unsigned char)((c>>18) & 0x3f | 0x80),fp);
        fputc((unsigned char)((c>>12) & 0x3f | 0x80),fp);
        fputc((unsigned char)((c>>6) & 0x3f | 0x80),fp);
        fputc((unsigned char)(c & 0x3f | 0x80),fp);
      } /* if */
    } else {
      /* not UTF-8 mode */
      fputc((unsigned char)*string,fp);
    } /* if */
    string++;
    count++;
  } /* while */
  return count;
}

static size_t fgets_char(FILE *fp, char *string, size_t max)
{
  size_t index;
  int c,lastcr;

  index=0;
  lastcr=0;
  for ( ;; ) {
    assert(index<max);
    if (index==max-1)
      break;                    /* string fully filled */
    if ((c=fgetc(fp))==EOF)
      break;                    /* no more characters */
    string[index++]=(char)c;
    if (c==__T('\n')) {
      break;                    /* read newline, done */
    } else if (lastcr) {
      ungetc(c,fp);             /* carriage return was read, no newline follows */
      break;
    } /* if */
    lastcr=(c==__T('\r'));
  } /* for */
  assert(index<max);
  string[index]=__T('\0');

  return index;
}

#if defined __WIN32__ || defined _WIN32 || defined WIN32
#if defined _UNICODE
wchar_t *_wgetenv(wchar_t *name)
{
static wchar_t buffer[AMX_MAX_PATH];
  buffer[0]=L'\0';
  GetEnvironmentVariable(name,buffer,sizeof buffer/sizeof(wchar_t));
  return buffer[0]!=L'\0' ? buffer : NULL;
}
#else
char *amx_getenv(const char *name)
{
static char buffer[AMX_MAX_PATH];
  buffer[0]='\0';
  GetEnvironmentVariable(name,buffer,sizeof buffer);
  return buffer[0]!='\0' ? buffer : NULL;
}
#endif
#else
	#define amx_getenv getenv
#endif

static char *completename(TCHAR *dest, TCHAR *src, size_t size)
{
  #if defined AMXFILE_VAR
    TCHAR *prefix,*ptr;
    size_t len;

	
    /* only files below a specific path are accessible */
    prefix=amx_getenv(AMXFILE_VAR);

    /* if no specific path for files is present, use the "temporary" path */
    if (prefix==NULL)
      prefix=amx_getenv(__T("tmp"));    /* common under Windows and Unix */
    if (prefix==NULL)
      prefix=amx_getenv(__T("temp"));   /* common under Windows */
    if (prefix==NULL)
      prefix=amx_getenv(__T("tmpdir")); /* common under Unix */

    /* if no path for files is defined, and no temporary directory exists,
     * fail the function; this is for security reasons.
     */
    if (prefix==NULL)
      return NULL;

    if (_tcslen(prefix)+1>=size) /* +1 because directory separator is appended */
      return NULL;
    _tcscpy(dest,prefix);
    /* append a directory separator (if not already present) */
    len=_tcslen(dest);
    if (len==0)
      return NULL;              /* empty start directory is not allowed */
    if (dest[len-1]!=__T(DIRSEP_CHAR) && dest[len-1]!=__T('/') && len+1<size) {
      dest[len]=__T(DIRSEP_CHAR);
      dest[len+1]=__T('\0');
    } /* if */
    assert(_tcslen(dest)<size);

    /* for DOS/Windows and Unix/Linux, skip everyting up to a comma, because
     * this is used to indicate a protocol (e.g. file://C:/myfile.txt)
     */
    #if DIRSEP_CHAR!=':'
      if ((ptr=_tcsrchr(src,__T(':')))!=NULL) {
        src=ptr+1;              /* skip protocol/drive and colon */
        /* a "drive" specifier is sometimes ended with a vertical bar instead
         * of a colon in URL specifications
         */
        if ((ptr=_tcschr(src,__T('|')))!=NULL)
          src=ptr+1;            /* skip drive and vertical bar */
        while (src[0]==__T(DIRSEP_CHAR) || src[0]==__T('/'))
          src++;                /* skip slashes behind the protocol/drive letter */
      } /* if */
    #endif

    /* skip an initial backslash or a drive specifier in the source */
    if ((src[0]==__T(DIRSEP_CHAR) || src[0]==__T('/')) && (src[1]==__T(DIRSEP_CHAR) || src[1]==__T('/'))) {
      /* UNC path */
      char separators[]={__T(DIRSEP_CHAR),__T('/'),__T('\0')};
      src+=2;
      ptr=_tcspbrk(src,separators);
      if (ptr!=NULL)
        src=ptr+1;
    } else if (src[0]==__T(DIRSEP_CHAR) || src[0]==__T('/')) {
      /* simple path starting from the root directory */
      src++;
    } /* if */

    /* disallow any "../" specifications in the source path
     * (the check below should be stricter, but directory names with
     * trailing periods are rare anyway)
     */
    for (ptr=src; *ptr!=__T('\0'); ptr++)
      if (ptr[0]==__T('.') && (ptr[1]==__T(DIRSEP_CHAR) || ptr[1]==__T('/')))
        return NULL;            /* path name is not allowed */

    /* concatenate the drive letter to the destination path */
    if (_tcslen(dest)+_tcslen(src)>=size)
      return NULL;
    _tcscat(dest,src);

    /* change forward slashes into proper directory separators */
    #if DIRSEP_CHAR!='/'
      while ((ptr=_tcschr(dest,__T('/')))!=NULL)
        *ptr=__T(DIRSEP_CHAR);
    #endif
    return dest;

  #else
    if (_tcslen(src)>=size)
      return NULL;
    _tcscpy(dest,src);
    /* change forward slashes into proper directory separators */
    #if DIRSEP_CHAR!='/'
      while ((ptr=_tcschr(dest,__T('/')))!=NULL)
        *ptr=__T(DIRSEP_CHAR);
    #endif
    return dest;
  #endif
}

/* File: fopen(const name[], filemode: mode) */
static cell AMX_NATIVE_CALL n_fopen(AMX *amx, cell *params)
{
  TCHAR *attrib,*altattrib;
  TCHAR *name,fullname[AMX_MAX_PATH];
  FILE *f = NULL;

  altattrib=NULL;
  switch (params[2] & 0x7fff) {
  case io_read:
    attrib=__T("rb");
    break;
  case io_write:
    attrib=__T("wb");
    break;
  case io_readwrite:
    attrib=__T("r+b");
    altattrib=__T("w+b");
    break;
  case io_append:
    attrib=__T("ab");
    break;
  default:
    return 0;
  } /* switch */
  
  /* get the filename */
  amx_StrParam(amx,params[1],name);
  if (name!=NULL && completename(fullname,name,sizeof fullname)!=NULL) {
    f=_tfopen(fullname,attrib);
    if (f==NULL && altattrib!=NULL)
      f=_tfopen(fullname,altattrib);
  } /* if */
  if (f!=NULL) {
    /* add file stream to the map */
    if (add_stream(amx, f) == 0) {
      /* allocation failed, close the opened file and return 0 (null) */
      fclose(f);
      f = NULL;
    }
  } /* if */
  return (cell)f;
}

/* fclose(File: handle) */
static cell AMX_NATIVE_CALL n_fclose(AMX *amx, cell *params)
{
  /* check if passed handle is added to the map */
  if (verify_stream(amx,(FILE*)params[1])==0) {
      return 0;
  } /* if */
  return (cell)fclose((FILE*)params[1]) == 0;
}

/* fwrite(File: handle, const string[]) */
static cell AMX_NATIVE_CALL n_fwrite(AMX *amx, cell *params)
{
  int r = 0;
  cell *cptr;
  char *str;
  int len;

  if (verify_stream(amx,(FILE*)params[1])==0)
      return 0;

  amx_GetAddr(amx,params[2],&cptr);
  amx_StrLen(cptr,&len);
  if (len==0)
    return 0;

  if ((ucell)*cptr>UNPACKEDMAX) {
    /* the string is packed, write it as an ASCII/ANSI string */
    if ((str=(char*)alloca(len+1))!=NULL)
      r=_tfputs(str, (FILE*)params[1]);
  } else {
    /* the string is unpacked, write it as UTF-8 */
    r=fputs_cell((FILE*)params[1],cptr,1);
  } /* if */
  return r;
}

/* fread(File: handle, string[], size=sizeof string, bool:pack=false) */
static cell AMX_NATIVE_CALL n_fread(AMX *amx, cell *params)
{
  int chars=0,max;
  char *str;
  cell *cptr;
  
  if (verify_stream(amx,(FILE*)params[1])==0)
    return 0;
  
  max=(int)params[3];
  if (max<=0)
    return 0;
  if (params[4])
    max*=sizeof(cell);

  amx_GetAddr(amx,params[2],&cptr);
  str=(char *)alloca(max);
  if (str==NULL || cptr==NULL) {
    amx_RaiseError(amx, AMX_ERR_NATIVE);
    return 0;
  } /* if */

  if (params[4]) {
    /* store as packed string, read an ASCII/ANSI string */
    chars=fgets_char((FILE*)params[1],str,max);
    assert(chars<max);
    amx_SetString(cptr,str,(int)params[4],0,max);
  } else {
    /* store and unpacked string, interpret UTF-8 */
    chars=fgets_cell((FILE*)params[1],cptr,max,1);
  } /* if */

  assert(chars<max);
  return chars;
}

/* fputchar(File: handle, value, bool:utf8 = true) */
static cell AMX_NATIVE_CALL n_fputchar(AMX *amx, cell *params)
{
  size_t result=0;

  UNUSED_PARAM(amx);

  if (verify_stream(amx,(FILE*)params[1])==0)
    return 0;

  if (params[3]) {
    cell str[2];
    str[0]=params[2];
    str[1]=0;
    result=fputs_cell((FILE*)params[1],str,1);
    assert(result==0 || result==1);
  } else {
    fputc((int)params[2], (FILE*)params[1]);
  } /* if */
  return result;
}

/* fgetchar(File: handle, value, bool:utf8 = true) */
static cell AMX_NATIVE_CALL n_fgetchar(AMX *amx, cell *params)
{
  cell str[2];
  size_t result=0;

  UNUSED_PARAM(amx);

  if (verify_stream(amx,(FILE*)params[1])==0)
    return 0;

  if (params[3]) {
    result=fgets_cell((FILE*)params[1],str,2,1);
  } else {
    str[0]=fgetc((FILE*)params[1]);
    result=(str[0]!=EOF);
  } /* if */
  assert(result==0 || result==1);
  if (result==0)
    return EOF;
  else
    return str[0];
}

#if PAWN_CELL_SIZE==16
  #define aligncell amx_Align16
#elif PAWN_CELL_SIZE==32
  #define aligncell amx_Align32
#elif PAWN_CELL_SIZE==64 && (defined _I64_MAX || defined HAVE_I64)
  #define aligncell amx_Align64
#else
  #error Unsupported cell size
#endif

/* fblockwrite(File: handle, buffer[], size=sizeof buffer) */
static cell AMX_NATIVE_CALL n_fblockwrite(AMX *amx, cell *params)
{
  cell *cptr;
  cell count=0;

  if (verify_stream(amx,(FILE*)params[1])==0)
    return 0;

  amx_GetAddr(amx,params[2],&cptr);
  if (cptr!=NULL) {
    cell max=params[3];
    ucell v;
    for (count=0; count<max; count++) {
      v=(ucell)*cptr++;
      if (fwrite(aligncell(&v),sizeof(cell),1,(FILE*)params[1])!= 1)
        break; /* write error */
    } /* for */
  } /* if */
  return count;
}

/* fblockread(File: handle, buffer[], size=sizeof buffer) */
static cell AMX_NATIVE_CALL n_fblockread(AMX *amx, cell *params)
{
  cell *cptr;
  cell count=0;

  if (verify_stream(amx,(FILE*)params[1])==0)
    return 0;

  amx_GetAddr(amx,params[2],&cptr);
  if (cptr!=NULL) {
    cell max=params[3];
    ucell v;
    for (count=0; count<max; count++) {
      if (fread(&v,sizeof(cell),1,(FILE*)params[1])!=1)
        break; /* write error */
      *cptr++=(cell)*aligncell(&v);
    } /* for */
  } /* if */
  return count;
}

/* File: ftemp() */
static cell AMX_NATIVE_CALL n_ftemp(AMX *amx, cell *params)
{
  FILE* f=NULL;
  //UNUSED_PARAM(amx);
  UNUSED_PARAM(params);
  if ((f=tmpfile())!=NULL) {
    if (add_stream(amx,f)==0) {
      fclose(f);
      f=NULL;
    } /* if */
  } /* if */
  return (cell)f;
}

/* fseek(File: handle, position, seek_whence: whence=seek_start) */
static cell AMX_NATIVE_CALL n_fseek(AMX *amx, cell *params)
{
  int whence;

  if (verify_stream(amx,(FILE*)params[1])==0)
    return 0;

  switch (params[3]) {
  case seek_start:
    whence=SEEK_SET;
    break;
  case seek_current:
    whence=SEEK_CUR;
    break;
  case seek_end:
    whence=SEEK_END;
    //if (params[2]>0)
    //  params[2]=-params[2];
    break;
  default:
    return 0;
  } /* switch */
  UNUSED_PARAM(amx);
  return lseek(fileno((FILE*)params[1]),params[2],whence);
}

/* bool: fexist(const name[]) */
static cell AMX_NATIVE_CALL n_fexist(AMX *amx, cell *params)
{
  int r=1;
  TCHAR *name,fullname[AMX_MAX_PATH];

  amx_StrParam(amx,params[1],name);
  if (name!=NULL && completename(fullname,name,sizeof fullname)!=NULL)
    r=access(fullname,0);
  return r==0;
}

/* bool: fremove(const name[]) */
static cell AMX_NATIVE_CALL n_fremove(AMX *amx, cell *params)
{
  int r=1;
  TCHAR *name,fullname[AMX_MAX_PATH];

  amx_StrParam(amx,params[1],name);
  if (name!=NULL && completename(fullname,name,sizeof fullname)!=NULL)
    r=remove(fullname);
  return r==0;
}

/* flength(File: handle) */
static cell AMX_NATIVE_CALL n_flength(AMX *amx, cell *params)
{
  long l=0,c;

  if (verify_stream(amx,(FILE*)params[1])==0)
    return l;

  int fn=fileno((FILE*)params[1]);
  c=lseek(fn,0,SEEK_CUR); /* save the current position */
  l=lseek(fn,0,SEEK_END); /* return the file position at its end */
  lseek(fn,c,SEEK_SET);   /* restore the file pointer */
  UNUSED_PARAM(amx);
  return l;
}

/* bool: fcopy(const source[], const target[]) */
static cell AMX_NATIVE_CALL n_fcopy(AMX *amx, const cell *params)
{
  int c;
  TCHAR *name,oldname[_MAX_PATH],newname[_MAX_PATH];
  FILE *fr,* fw;

  UNUSED_PARAM(amx);
  amx_StrParam(amx,params[1],name);
  if (name!=NULL && completename(oldname,name,sizeof oldname)!=NULL) {
    amx_StrParam(amx,params[2],name);
    if (name!=NULL && completename(newname,name,sizeof newname)!=NULL) {
      fr=fopen(oldname,"rb");
      if (fr==NULL)
        return 0;
      fw=fopen(newname,"wb");
      if (fw==NULL) {
        fclose(fr);
        return 0;
      } /* if */
      while ((c=fgetc(fr))!=EOF)
        fputc(c,fw);
      fclose(fr);
      fclose(fw);
    } /* if */
  } /* if */
  return 1;
}

#if defined __cplusplus
  extern "C"
#endif
AMX_NATIVE_INFO file_Natives[] = {
  { "fopen",       n_fopen },
  { "fclose",      n_fclose },
  { "fwrite",      n_fwrite },
  { "fread",       n_fread },
  { "fputchar",    n_fputchar },
  { "fgetchar",    n_fgetchar },
  { "fblockwrite", n_fblockwrite },
  { "fblockread",  n_fblockread },
  { "ftemp",       n_ftemp },
  { "fseek",       n_fseek },
  { "fexist",      n_fexist },
  { "flength",     n_flength },
  { "fremove",     n_fremove },
  { "fcopy",       n_fcopy },
  { NULL, NULL }        /* terminator */
};

int AMXEXPORT amx_FileInit(AMX *amx)
{
  return amx_Register(amx, file_Natives, -1);
}

int AMXEXPORT amx_FileCleanup(AMX *amx)
{
  file_stream* curr = filestream;
  file_stream* prev = NULL;
  while(curr != NULL) {
    if (curr->amx == amx) {
      fclose(curr->file);
      curr->file = NULL;
      if(prev)
        prev->next = curr->next;
      curr->next = NULL;
      curr->amx = NULL;
      free(curr);
    } /* if */
    prev = curr;
    curr = curr->next;
  } /* while */
  return AMX_ERR_NONE;
}
