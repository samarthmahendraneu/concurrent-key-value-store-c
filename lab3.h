#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "errno.h"
#include "string.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "pthread.h"
#include "fcntl.h"
#include "signal.h"
#include "stdarg.h"
#include "dirent.h"

#include "zlib.h"

#ifndef __LAB3_H__
#define __LAB3_H__

struct request {
    char op_status;             /* R/W/D, K/X */
    char name[31];              /* null-padded, max strlen = 30 */
    char len[8];                /* text, decimal, null-padded */
};


#define TABLE_MAX 200

#endif
