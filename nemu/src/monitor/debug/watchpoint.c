#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* allocate a new watchpoint with expression e */
bool new_wp(char *e) {
  if (free_ == NULL) {
    printf("No free watchpoint available\n");
    return false;
  }

  WP *wp = free_;
  free_ = free_->next;

  wp->next = NULL;
  strncpy(wp->expr, e, sizeof(wp->expr) - 1);
  wp->expr[sizeof(wp->expr) - 1] = '\0';
  wp->hitNum = 0;
  bool success = false;
  int val = expr(wp->expr, &success);
  if (!success) {
    printf("error in new_wp: bad expression '%s'\n", wp->expr);
    /* return wp to free list */
    wp->next = free_;
    free_ = wp;
    return false;
  }
  wp->oldValue = val;

  /* insert wp at end of active list */
  if (head == NULL) {
    head = wp;
  } else {
    WP *p = head;
    while (p->next) p = p->next;
    p->next = wp;
  }

  printf("Success: set watchpoint %d, expr=%s, oldvalue=%d\n", wp->NO, wp->expr, wp->oldValue);
  return true;
}

/* free a watchpoint by number */
bool free_wp(int no) {
  if (head == NULL) return false;
  WP *p = head, *prev = NULL;
  while (p) {
    if (p->NO == no) break;
    prev = p;
    p = p->next;
  }
  if (p == NULL) return false;

  if (prev == NULL) head = p->next;
  else prev->next = p->next;

  /* return to free list */
  p->next = free_;
  free_ = p;
  return true;
}

/* check whether any watchpoint triggers; return true if none triggered, false if triggered */
bool watch_wp() {
  WP *p = head;
  if (!p) return true;
  while (p) {
    bool success = false;
    int val = expr(p->expr, &success);
    if (!success) {
      printf("error in watch_wp: bad expression '%s'\n", p->expr);
      p = p->next;
      continue;
    }
    if (val != p->oldValue) {
      p->hitNum++;
      printf("Hardware watchpoint %d: %s\n", p->NO, p->expr);
      printf("old value: %d\nnew value: %d\n", p->oldValue, val);
      p->oldValue = val;
      return false; /* stop execution */
    }
    p = p->next;
  }
  return true;
}

void print_wp() {
  WP *p = head;
  if (p == NULL) {
    printf("No watchpoint\n");
    return;
  }
  printf("Num\tExpr\toldValue\thitTimes\n");
  while (p) {
    printf("%d\t%s\t%d\t%d\n", p->NO, p->expr, p->oldValue, p->hitNum);
    p = p->next;
  }
}


