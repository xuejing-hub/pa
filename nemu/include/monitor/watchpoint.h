#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  /* expression stored as string */
  char expr[64];
  /* last evaluated value */
  int oldValue;
  /* number of times this wp was hit */
  int hitNum;
} WP;

/* initialize watchpoint pool */
void init_wp_pool();
/* create a new watchpoint for expression e, return true on success */
bool new_wp(char *e);
/* delete watchpoint by number, return true on success */
bool free_wp(int no);
/* check all watchpoints; return true if no watchpoint triggered, false if triggered */
bool watch_wp();
/* Print all active watchpoints */
void print_wp();

#endif
