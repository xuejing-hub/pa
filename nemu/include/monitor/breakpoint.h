#ifndef __BREAKPOINT_H__
#define __BREAKPOINT_H__

#include "common.h"

typedef struct breakpoint {
  int NO;
  struct breakpoint *next;
  vaddr_t addr;
  int hitNum;
} BP;

void init_bp_pool();
bool new_bp(vaddr_t addr);
bool free_bp(int no);
/* check whether current eip hits a breakpoint; return false if hit */
bool check_bp(vaddr_t eip);
void print_bp();

#endif
