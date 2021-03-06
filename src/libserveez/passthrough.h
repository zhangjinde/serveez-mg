/*
 * passthrough.h - pass through declarations
 *
 * Copyright (C) 2001, 2003 Stefan Jahn <stefan@lkcc.org>
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PASSTHROUGH_H__
#define __PASSTHROUGH_H__ 1

#define _GNU_SOURCE 1

#include "defines.h"

typedef char ** svz_envp_t;

/* Structure containing a system independent environment.  */
typedef struct
{
  int size;     /* Number of environment entries.  */
  char **entry; /* Environment entries in the format "VAR=VALUE".  */
  char *block;  /* Temporary environment block.  */
}
svz_envblock_t;

/* Internally used to pass lots of arguments.  */
typedef struct
{
  svz_socket_t *sock;   /* Socket structure to pass through.  */
  char *bin;            /* Fully qualified program name.  */
  char *dir;            /* Working directory.  */
  char **argv;          /* Program arguments including argv[0].  */
  svz_envblock_t *envp; /* Environment block.  */
  char *user;           /* User and group.  */
  char *app;            /* Additional @var{bin} interpreter application.  */
  svz_t_handle in, out; /* New stdin and stdout of child process.  */
  int flag;             /* Passthrough method flag.  */
}
svz_process_t;

/* Definition for the @var{flag} argument of @code{svz_sock_process()}.  */
#define SVZ_PROCESS_FORK         1
#define SVZ_PROCESS_SHUFFLE_SOCK 2
#define SVZ_PROCESS_SHUFFLE_PIPE 3

/* Definitions for the @var{user} argument of @code{svz_sock_process()}.  */
#define SVZ_PROCESS_NONE  ((char *) 0L)
#define SVZ_PROCESS_OWNER ((char *) ~0L)

/* Envrionment variables used to pass the receive and send sockets to
   the child process on Win32.  */
#define SVZ_PROCESS_RECV_HANDLE "RECV_HANDLE"
#define SVZ_PROCESS_SEND_HANDLE "SEND_HANDLE"

/* Extern declaration of the process environment pointer.  */
#if !defined(__MINGW32__) && !defined(__CYGWIN__)
extern char **environ;
#endif

/*
 * This macro must be called once after @code{svz_boot()} for setting up the
 * @code{svz_environ} variable.  It simply passes the @code{environ} variable
 * of the calling application to the underlying Serveez core API.  This is
 * necessary to make the @code{svz_envblock_default()} function working
 * correctly.
 */
#define svz_envblock_setup() do { svz_environ = environ; } while (0)

__BEGIN_DECLS

SERVEEZ_API int svz_sock_process (svz_socket_t *, char *, char *,
                                  char **, svz_envblock_t *, int,
                                  char *);

SERVEEZ_API int svz_process_disconnect (svz_socket_t *);
SERVEEZ_API int svz_process_disconnect_passthrough (svz_socket_t *);
SERVEEZ_API int svz_process_check_request (svz_socket_t *);
SERVEEZ_API int svz_process_idle (svz_socket_t *);
SERVEEZ_API int svz_process_send_pipe (svz_socket_t *);
SERVEEZ_API int svz_process_recv_pipe (svz_socket_t *);
SERVEEZ_API int svz_process_send_socket (svz_socket_t *);
SERVEEZ_API int svz_process_recv_socket (svz_socket_t *);

SERVEEZ_API int svz_process_create_child (svz_process_t *);
SERVEEZ_API int svz_process_shuffle (svz_process_t *);
SERVEEZ_API int svz_process_fork (svz_process_t *);

SERVEEZ_API int svz_process_check_executable (char *, char **);
SERVEEZ_API int svz_process_split_usergroup (char *, char **, 
                                             char **);
SERVEEZ_API int svz_process_check_access (char *, char *);

SERVEEZ_API char **svz_environ;
SERVEEZ_API svz_envblock_t *svz_envblock_create (void);
SERVEEZ_API int svz_envblock_default (svz_envblock_t *);
SERVEEZ_API int svz_envblock_add (svz_envblock_t *, char *, ...);
SERVEEZ_API int svz_envblock_free (svz_envblock_t *);
SERVEEZ_API void svz_envblock_destroy (svz_envblock_t *);
SERVEEZ_API svz_envp_t svz_envblock_get (svz_envblock_t *);

__END_DECLS

#endif /* __PASSTHROUGH_H__ */
