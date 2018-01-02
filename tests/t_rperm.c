/* $NetBSD$ */

/*-
 * Copyright (c) 2015, 2016 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Santhosh N. Raju <santhosh.raju@gmail.com>
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
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__RCSID("$NetBSD$");

/* Testing API - assumes userland */
/* Provide Kernel API equivalents */
typedef  unsigned char           __cpu_simple_lock_nv_t;

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h> /* memset(3) et. al */
#include <stdio.h> /* printf(3) */
#include <stdlib.h> /* malloc(3) */
#include <stdarg.h>
#include <stddef.h>
#include <time.h> /* time(3) */
#include <fcntl.h>

#define	PRIxPADDR	"lx"
#define	PRIxPSIZE	"lx"
#define	PRIuPSIZE	"lu"
#define	PRIxVADDR	"lx"
#define	PRIxVSIZE	"lx"
#define	PRIuVSIZE	"lu"

#define DEBUG /* Enable debug functionality. */

#define DEV_CMAJOR 420
#define DEV_CMINOR 0

typedef unsigned long vaddr_t;
typedef unsigned long paddr_t;
typedef unsigned long psize_t;
typedef unsigned long vsize_t;

#ifndef DIAGNOSTIC
#define	KASSERTMSG(e, msg, ...)	/* NOTHING */
#define	KASSERT(e)		/* NOTHING */
#else
#define	KASSERT(a)		assert(a)
#define KASSERTMSG(exp, ...)    printf(__VA_ARGS__); assert((exp))
#endif

#define atop(x)         (((paddr_t)(x)) >> PAGE_SHIFT)
#define ptoa(x)         (((paddr_t)(x)) << PAGE_SHIFT)

#define	mutex_enter(l)
#define	mutex_exit(l)

#define dev_type_open(n)        int n (dev_t, int, int, struct lwp *)
#define dev_type_close(n)       int n (dev_t, int, int, struct lwp *)
#define dev_type_read(n)        int n (dev_t, struct uio *, int)
#define dev_type_write(n)       int n (dev_t, struct uio *, int)
#define dev_type_ioctl(n) \
                int n (dev_t, u_long, void *, int, struct lwp *)
#define dev_type_stop(n)        void n (struct tty *, int)
#define dev_type_tty(n)         struct tty * n (dev_t)
#define dev_type_poll(n)        int n (dev_t, int, struct lwp *)
#define dev_type_mmap(n)        paddr_t n (dev_t, off_t, int)
#define dev_type_strategy(n)    void n (struct buf *)
#define dev_type_dump(n)        int n (dev_t, daddr_t, void *, size_t)
#define dev_type_size(n)        int n (dev_t)
#define dev_type_kqfilter(n)    int n (dev_t, struct knote *)
#define dev_type_discard(n)     int n (dev_t, off_t, off_t)

#define noopen          ((dev_type_open((*)))enodev)
#define noclose         ((dev_type_close((*)))enodev)
#define noread          ((dev_type_read((*)))enodev)
#define nowrite         ((dev_type_write((*)))enodev)
#define noioctl         ((dev_type_ioctl((*)))enodev)
#define nostop          ((dev_type_stop((*)))enodev)
#define notty           NULL
#define nopoll          seltrue
paddr_t nommap(dev_t, off_t, int);
#define nodump          ((dev_type_dump((*)))enodev)
#define nosize          NULL
#define nokqfilter      seltrue_kqfilter
#define nodiscard       ((dev_type_discard((*)))enodev)

#define seltrue			0
#define seltrue_kqfilter	0

#include <sys/kmem.h>

void *
kmem_alloc(size_t size, km_flag_t flags)
{
	return malloc(size);
}

void *
kmem_zalloc(size_t size, km_flag_t flags)
{
	void *ptr;
	ptr = malloc(size);

	memset(ptr, 0, size);

	return ptr;
}

void
kmem_free(void *mem, size_t size)
{
	free(mem);
}

static void
panic(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	printf("\n");
	va_end(ap);
	KASSERT(false);

	/*NOTREACHED*/
}

#define MODULE(MODULE_CLASS_DRIVER, rperm, NULL);

#include <sys/uio.h>

struct uio {
        struct  iovec *uio_iov; /* pointer to array of iovecs */
        int     uio_iovcnt;     /* number of iovecs in array */
        off_t   uio_offset;     /* offset into file this uio corresponds to */
        size_t  uio_resid;      /* residual i/o count */
        enum    uio_rw uio_rw;  /* see above */
};

#include <sys/conf.h>

static int
devsw_attach(const char *devname, const struct bdevsw *bev, devmajor_t *bmajor, const struct cdevsw *cdev, devmajor_t *cmajor)
{
	return 0;
}

static int
devsw_detach(const struct bdevsw *bdev, const struct cdevsw *cdev)
{
	return 0;
}

static int
uiomove(void *buf, size_t n, struct uio *uio)
{
	switch(uio->uio_rw) {
	case UIO_READ:
		memcpy(buf, uio->uio_iov->iov_base, n);
		break;

	case UIO_WRITE:
		memcpy(uio->uio_iov->iov_base, buf, n);
		break;

	default:
		panic("Unknown operation %d.\n", (int)uio->uio_rw);
	}
	return 0;
}

static uint32_t
cprng_strong32(void)
{
  srandom((uint32_t)time(NULL));
  return (uint32_t) random();
}

uint32_t rand_n(uint32_t low, uint32_t high);

/* end - Provide Kernel API equivalents */


#include "../rperm.c"

#include <atf-c.h>

/*
 * Test Fixture SetUp().
 */
static void
setup(void)
{
	/* Prerequisites for running certain calls in rperm */
	rperm_modcmd(0, NULL);
}

ATF_TC(rperm_get_open);
ATF_TC_HEAD(rperm_get_open, tc)
{
	atf_tc_set_md_var(tc, "descr", "Tests if the rperm_open() works");
}
ATF_TC_BODY(rperm_get_open, tc)
{
	setup();

	int open_return = rperm_open(DEV_CMINOR, O_RDONLY, S_IFCHR, NULL);
        ATF_CHECK_EQ(0, open_return);
}


ATF_TP_ADD_TCS(tp)
{
	/* Exported */
	ATF_TP_ADD_TC(tp, rperm_get_open);

	return atf_no_error();
}
