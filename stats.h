#include "lab3.h"

#ifndef __LAB3_STATS_H__
#define __LAB3_STATS_H__

void inc_write();
void inc_read();
void inc_delete();
void inc_fail();

int get_writes();
int get_reads();
int get_deletes();
int get_fails();

#endif

