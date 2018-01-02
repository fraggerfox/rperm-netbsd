# $NetBSD$
#

WARNS?=6
.include <bsd.own.mk>

CPPFLAGS+=	-I${NETBSDSRCDIR}/sys -I${.CURDIR}/ -D_TEST -g

TESTS_C+=	t_rperm
SRCS.t_rperm+=	t_rperm.c
CPPFLAGS.t_rperm.c= -D_RPERM_TESTING -DDIAGNOSTIC -D_KERNTYPES

.include <bsd.dep.mk>
.include <bsd.test.mk>
