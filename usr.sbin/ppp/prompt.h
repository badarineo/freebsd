/*-
 * Copyright (c) 1998 Brian Somers <brian@Awfulhak.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$Id: prompt.h,v 1.1.2.3 1998/02/17 19:28:13 brian Exp $
 */

#define LOCAL_AUTH	0x01
#define LOCAL_NO_AUTH	0x02
#define LOCAL_DENY	0x03
#define LOCAL_CX	0x04	/* OR'd value - require a context */
#define LOCAL_CX_OPT	0x08	/* OR'd value - optional context */

struct server;
struct bundle;

struct prompt {
  struct descriptor desc;
  int fd_in, fd_out;
  struct datalink *TermMode;	/* The modem we're talking directly to */
  FILE *Term;			/* sits on top of fd_out */
  u_char auth;			/* Local Authorized status */
  struct server *owner;         /* who created me */
  struct bundle *bundle;	/* who I'm controlling */
  unsigned nonewline : 1;	/* need a newline before our prompt ? */
  unsigned needprompt : 1;	/* Show a prompt at the next UpdateSet() */

  char who[40];			/* Where do I come from */

  struct prompt *lognext;	/* Maintained in log.c */
  u_long logmask;		/* Maintained in log.c */

  struct termios oldtio;	/* Original tty mode */
  struct termios comtio;	/* Command level tty mode */
};

#define prompt2descriptor(p) (&(p)->desc)
#define descriptor2prompt(d) \
  ((d)->type == PROMPT_DESCRIPTOR ? (struct prompt *)(d) : NULL)

#define PROMPT_STD (-1)
extern struct prompt *prompt_Create(struct server *, struct bundle *, int);
extern void prompt_Destroy(struct prompt *, int);
extern void prompt_DestroyUnclean(struct prompt *);
extern void prompt_Required(struct prompt *);
extern void prompt_Printf(struct prompt *, const char *, ...);
extern void prompt_vPrintf(struct prompt *, const char *, _BSD_VA_LIST_);
#define PROMPT_DONT_WANT_INT 1
#define PROMPT_WANT_INT 0
extern void prompt_TtyInit(struct prompt *, int);
extern void prompt_TtyCommandMode(struct prompt *);
extern void prompt_TtyTermMode(struct prompt *, struct datalink *);
extern void prompt_TtyOldMode(struct prompt *);
extern pid_t prompt_pgrp(struct prompt *);
extern int PasswdCommand(struct cmdargs const *);
#define prompt_IsTermMode(p, dl) ((p)->TermMode == (dl) ? 1 : 0)
#define prompt_IsController(p) (!(p) || (p)->owner ? 0 : 1)
#define prompt_Required(p) ((p)->needprompt = 1)
