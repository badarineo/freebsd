/*
 *	      PPP Memory handling module
 *
 *	    Written by Toshiharu OHNO (tony-o@iij.ad.jp)
 *
 *   Copyright (C) 1993, Internet Initiative Japan, Inc. All rights reserverd.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the Internet Initiative Japan, Inc.  The name of the
 * IIJ may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Id: mbuf.c,v 1.13.2.6 1998/04/03 19:21:38 brian Exp $
 *
 */
#include <sys/param.h>
#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <termios.h>

#include "command.h"
#include "mbuf.h"
#include "log.h"
#include "defs.h"
#include "loadalias.h"
#include "vars.h"
#include "descriptor.h"
#include "prompt.h"
#include "main.h"

static struct memmap {
  struct mbuf *queue;
  int count;
} MemMap[MB_MAX + 2];

static int totalalloced;

int
plength(struct mbuf * bp)
{
  int len;

  for (len = 0; bp; bp = bp->next)
    len += bp->cnt;
  return (len);
}

struct mbuf *
mballoc(int cnt, int type)
{
  u_char *p;
  struct mbuf *bp;

  if (type > MB_MAX)
    LogPrintf(LogERROR, "Bad mbuf type %d\n", type);
  bp = (struct mbuf *) malloc(sizeof(struct mbuf));
  if (bp == NULL) {
    LogPrintf(LogALERT, "failed to allocate memory: %u\n", sizeof(struct mbuf));
    AbortProgram(EX_OSERR);
  }
  memset(bp, '\0', sizeof(struct mbuf));
  p = (u_char *) malloc(cnt);
  if (p == NULL) {
    LogPrintf(LogALERT, "failed to allocate memory: %d\n", cnt);
    AbortProgram(EX_OSERR);
  }
  MemMap[type].count += cnt;
  totalalloced += cnt;
  bp->base = p;
  bp->size = bp->cnt = cnt;
  bp->type = type;
  bp->pnext = NULL;
  return (bp);
}

struct mbuf *
mbfree(struct mbuf * bp)
{
  struct mbuf *nbp;

  if (bp) {
    nbp = bp->next;
    MemMap[bp->type].count -= bp->size;
    totalalloced -= bp->size;
    free(bp->base);
    free(bp);
    return (nbp);
  }
  return (bp);
}

void
pfree(struct mbuf * bp)
{
  while (bp)
    bp = mbfree(bp);
}

struct mbuf *
mbread(struct mbuf * bp, u_char * ptr, int len)
{
  int nb;

  while (bp && len > 0) {
    if (len > bp->cnt)
      nb = bp->cnt;
    else
      nb = len;
    memcpy(ptr, MBUF_CTOP(bp), nb);
    ptr += nb;
    bp->cnt -= nb;
    len -= nb;
    bp->offset += nb;
    if (bp->cnt == 0) {
#ifdef notdef
      bp = bp->next;
#else
      bp = mbfree(bp);
#endif
    }
  }
  return (bp);
}

void
mbwrite(struct mbuf * bp, u_char * ptr, int cnt)
{
  int plen;
  int nb;

  plen = plength(bp);
  if (plen < cnt)
    cnt = plen;

  while (cnt > 0) {
    nb = (cnt < bp->cnt) ? cnt : bp->cnt;
    memcpy(MBUF_CTOP(bp), ptr, nb);
    cnt -= bp->cnt;
    bp = bp->next;
  }
}

int
ShowMemMap(struct cmdargs const *arg)
{
  /* Watch it - ~m calls us with arg == NULL */
  int i;

  for (i = 0; i <= MB_MAX; i += 2)
    prompt_Printf(arg->prompt, "%d: %d   %d: %d\n",
	    i, MemMap[i].count, i + 1, MemMap[i + 1].count);

  return 0;
}

void
LogMemory()
{
  LogPrintf(LogDEBUG, "LogMemory: mem alloced: %d\n", totalalloced);
  LogPrintf(LogDEBUG, "LogMemory:  1: %d  2: %d   3: %d   4: %d\n",
	MemMap[1].count, MemMap[2].count, MemMap[3].count, MemMap[4].count);
  LogPrintf(LogDEBUG, "LogMemory:  5: %d  6: %d   7: %d   8: %d\n",
	MemMap[5].count, MemMap[6].count, MemMap[7].count, MemMap[8].count);
}

struct mbuf *
Dequeue(struct mqueue *q)
{
  struct mbuf *bp;
  
  LogPrintf(LogDEBUG, "Dequeue: queue len = %d\n", q->qlen);
  bp = q->top;
  if (bp) {
    q->top = q->top->pnext;
    q->qlen--;
    if (q->top == NULL) {
      q->last = q->top;
      if (q->qlen)
	LogPrintf(LogERROR, "Dequeue: Not zero (%d)!!!\n", q->qlen);
    }
  }

  return bp;
}

void
Enqueue(struct mqueue *queue, struct mbuf *bp)
{
  if (queue->last) {
    queue->last->pnext = bp;
    queue->last = bp;
  } else
    queue->last = queue->top = bp;
  queue->qlen++;
  LogPrintf(LogDEBUG, "Enqueue: len = %d\n", queue->qlen);
}
