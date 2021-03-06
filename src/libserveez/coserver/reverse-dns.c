/*
 * reverse-dns.c - reverse DNS lookup coserver implementation
 *
 * Copyright (C) 2000, 2001, 2003 Stefan Jahn <stefan@lkcc.org>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "util.h"
#include "coserver/coserver.h"
#include "coserver/reverse-dns.h"

#define MAX_CACHE_ENTRIES 1024 /* nslookup cache entries */

/*
 * Reverse DNS lookup cache structure.
 */
typedef struct
{
  int entries;
  unsigned long ip[MAX_CACHE_ENTRIES];
  char resolved[COSERVER_BUFSIZE][MAX_CACHE_ENTRIES];
}
reverse_dns_cache_t;

reverse_dns_cache_t reverse_dns_cache;

/*
 * Initialize the cache structure.
 */
void
reverse_dns_init (void)
{
  reverse_dns_cache.entries = 0;
}

/*
 * Proceed a reverse DNS lookup.
 */
char *
reverse_dns_handle_request (char *inbuf)
{
  char ip[16];
  unsigned long addr[2];
  struct hostent *host;
  static char resolved[COSERVER_BUFSIZE];
  int n;

  if ((1 == sscanf (inbuf, "%s", ip)))
    {
      addr[0] = inet_addr (ip);
      addr[1] = 0;

      /*
       * look up the ip->host cache first
       */
      for (n = 0; n < reverse_dns_cache.entries; n++)
        {
          if (reverse_dns_cache.ip[n] == addr[0])
            {
              sprintf (resolved, "%s", reverse_dns_cache.resolved[n]);
              return resolved;
            }
        }

      if ((host = gethostbyaddr ((char *) addr, sizeof (addr[0]), AF_INET))
          == NULL)
        {
          svz_log (LOG_ERROR, "reverse dns: gethostbyaddr: %s (%s)\n",
                   strerror (errno), ip);
          return NULL;
        }
      else
        {
          if (n < MAX_CACHE_ENTRIES)
            {
              strcpy (reverse_dns_cache.resolved[n], host->h_name);
              reverse_dns_cache.ip[n] = addr[0];
              reverse_dns_cache.entries++;
            }

          svz_log (LOG_DEBUG, "reverse dns: %s is %s\n", ip, host->h_name);
          sprintf (resolved, "%s", host->h_name);
          return resolved;
        }
    }
  else
    {
      svz_log (LOG_ERROR, "reverse dns: protocol error\n");
      return NULL;
    }
}
