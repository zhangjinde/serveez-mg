/*
 * http-dirlist.c - http protocol directory listing
 *
 * Copyright (C) 2000, 2003 Stefan Jahn <stefan@lkcc.org>
 * Copyright (C) 2000 Raimund Jacob <raimi@lkcc.org>
 * Copyright (C) 2010 Michael Gran <spk121@yahoo.com>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)

#include <unistd.h>

#include "libserveez.h"
#include "http-proto.h"
#include "http-core.h"
#include "http-dirlist.h"

#define FILENAME dir[n]->d_name

/* Size of last buffer allocated.  */
int http_dirlist_size = 0;

/*
 * Convert a given filename to an appropriate http request URI.
 * This is necessary because of special characters within
 * these.
 */
static char *
http_create_uri (char *file)
{
  static char uri[DIRLIST_SPACE_NAME];
  char *p, *dst;

  p = file;
  dst = uri;

  /* go throughout the filename */
  while (*p)
    {
      /* check if the current character is valid */
      while (*p &&
             ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9') ||
              *p == ' ' || *p == '/' || *p == '.' || *p == '_'))
        *dst++ = *p++;
      if (!*p)
        break;
      *dst++ = '%';
      sprintf (dst, "%02X", (unsigned char) *p++);
      dst += 2;
    }
  *dst = '\0';
  return uri;
}

/*
 * Return a buffer to a http directory listing referring to DIRNAME
 * and being part of the document root DOCROOT.  Do not to forget to
 * svz_free() the return buffer.  Return NULL on errors.
 */
char *
http_dirlist (char *dirname, char *docroot, char *userdir)
{
  char *dirdata = NULL;
  int datasize = 0;
  char *relpath = NULL;
  int i = 0;
  struct stat buf;
  char filename[DIRLIST_SPACE_NAME];
  char entrystr[DIRLIST_SPACE_ENTRY];
  char *timestr = NULL;
  char postamble[DIRLIST_SPACE_POST];
  int files = 0;
  struct dirent **dir;
  int n;

  /* Initialize data fields */
  memset (filename, 0, DIRLIST_SPACE_NAME);
  memset (entrystr, 0, DIRLIST_SPACE_ENTRY);
  memset (postamble, 0, DIRLIST_SPACE_POST);

  /* Remove trailing slash of dirname */
  if (strlen (dirname) != 1 &&
      (dirname[strlen (dirname) - 1] == '/' ||
       dirname[strlen (dirname) - 1] == '\\'))
    {
      dirname[strlen (dirname) - 1] = 0;
    }

  /* Open the directory */
  if ((files = scandir (dirname, &dir, 0, alphasort)) == -1)
    {
      return NULL;
    }

  dirdata = svz_malloc (DIRLIST_SPACE);
  datasize = DIRLIST_SPACE;

  /* Calculate relative path */
  if (!userdir)
    {
      i = 0;
      while (dirname[i] == docroot[i] && docroot[i] != 0)
        i++;
      relpath = &dirname[i];
      if (!strcmp (relpath, "/"))
        {
          relpath++;
          dirname++;
        }
    }
  else
    relpath = userdir + 1;

  /* Output preamble */
  while (-1 == snprintf (dirdata, datasize,
                         "%sContent-Type: text/html\r\n\r\n"
                         "<html><head>\n"
                         "<title>Directory listing of %s%s</title></head>"
                         "\n<body bgcolor=white text=black link=blue>\n"
                         "<h1>Directory listing of %s%s</h1>\n"
                         "<hr noshade>\n"
                         "<pre>\n",
                         HTTP_OK, relpath, userdir ? "" : "/",
                         relpath, userdir ? "" : "/"))
    {
      dirdata = svz_realloc (dirdata, datasize + DIRLIST_SPACE_GROW);
      datasize += DIRLIST_SPACE_GROW;
    }

  /* Iterate directory */
  for (n = 0; n < files; n++)
    {
      /* Create fully qualified filename */
      snprintf (filename, DIRLIST_SPACE_NAME - 1, "%s/%s",
                dirname, FILENAME);

      /* Stat the given file */
      if (-1 == stat (filename, &buf)) 
	{
	  /* Something is wrong with this file... */
          snprintf (entrystr, DIRLIST_SPACE_ENTRY - 1,
                    "<font color=red>%s -- %s</font>\n",
                    FILENAME, SYS_ERROR);
	} 
      else 
	{
	  /* Get file time and remove trailing newline */
	  if (localtime (&buf.st_mtime))
	    timestr = asctime (localtime (&buf.st_mtime));
	  else
	    {
	      buf.st_mtime = 0;
	      timestr = asctime (localtime (&buf.st_mtime));
	    }
	  if (timestr[strlen (timestr) - 1] == '\n') 
	    {
	      timestr[strlen (timestr) - 1] = 0;
	    }

	  /* Emit beautiful description */
	  if (S_ISDIR (buf.st_mode)) 
	    {
	      /* This is a directory... */
              snprintf (entrystr, DIRLIST_SPACE_ENTRY - 1,
                        "<img border=0 src=internal-gopher-menu> "
                        "<a href=\"%s/\">%-40s</a> "
                        "&lt;directory&gt; "
                        "%s\n",
                        http_create_uri (FILENAME), FILENAME, timestr);
	    }
	  else 
	    {
	      /* Let's treat this as a normal file */
              snprintf (entrystr, DIRLIST_SPACE_ENTRY - 1,
                        "<img border=0 src=internal-gopher-text> "
                        "<a href=\"%s\">%-40s</a> "
                        "<b>%11d</b> "
                        "%s\n",
                        http_create_uri (FILENAME),
                        FILENAME, (int) buf.st_size, timestr);
	    }
	}

      /* Append this entry's data to output buffer */
      while (datasize - strlen (dirdata) < strlen (entrystr) + 1)
        {
          dirdata = svz_realloc (dirdata, datasize + DIRLIST_SPACE_GROW);
          datasize += DIRLIST_SPACE_GROW;
        }
      strcat (dirdata, entrystr);
    }

  /* Output postamble */
  snprintf (postamble, DIRLIST_SPACE_POST - 1,
            "\n</pre><hr noshade>\n"
            "%d entries\n</body>\n</html>", files);

  if (datasize - strlen (dirdata) < strlen (postamble) + 1) 
    {
      dirdata = svz_realloc (dirdata, datasize + strlen (postamble) + 1);
      datasize += strlen (postamble) + 1;
    }
  strcat (dirdata, postamble);

  /* Remember buffer size for caller */
  http_dirlist_size = datasize;

  /* Close the directory */
  free (dir);

  return dirdata;
}

