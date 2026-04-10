#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head = NULL, *free_ = NULL;



static void trim_expr(char *s) {
  if (s == NULL) return;

  char *start = s;
  while (*start == ' ' || *start == '\t') start++;

  if (start != s) {
    memmove(s, start, strlen(start) + 1);
  }

  size_t len = strlen(s);
  while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t')) {
    s[len - 1] = '\0';
    len--;
  }
}




static bool eval_wp_expr(const char *expr_str, uint32_t *out_val) {
  if (expr_str == NULL || out_val == NULL) return false;

  bool success = false;
  uint32_t val = expr((char *)expr_str, &success);
  if (!success) return false;

  *out_val = val;
  return true;
}




void init_wp_pool() {
  for (int i = 0; i < NR_WP; i++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1) ? NULL : &wp_pool[i + 1];
    wp_pool[i].expr[0] = '\0';
    wp_pool[i].oldValue = 0;
    wp_pool[i].hitNum = 0;
  }

  head = NULL;
  free_ = wp_pool;
}



bool new_wp(char *e) {
  if (e == NULL) {
    printf("error: watchpoint expression is NULL\n");
    return false;
  }

  if (free_ == NULL) {
    printf("No free watchpoint available\n");
    return false;
  }

  WP *wp = free_;
  free_ = free_->next;
  wp->next = NULL;

  strncpy(wp->expr, e, sizeof(wp->expr) - 1);
  wp->expr[sizeof(wp->expr) - 1] = '\0';
  trim_expr(wp->expr);

  if (wp->expr[0] == '\0') {
    printf("error: empty watchpoint expression\n");
    wp->next = free_;
    free_ = wp;
    return false;
  }

  wp->hitNum = 0;

  uint32_t val = 0;
  if (!eval_wp_expr(wp->expr, &val)) {
    printf("error in new_wp: bad expression '%s'\n", wp->expr);
    wp->expr[0] = '\0';
    wp->next = free_;
    free_ = wp;
    return false;
  }
  wp->oldValue = val;

  /* insert at tail of active list */
  if (head == NULL) {
    head = wp;
  } else {
    WP *p = head;
    while (p->next != NULL) p = p->next;
    p->next = wp;
  }

  printf("Watchpoint %d set: %s\n", wp->NO, wp->expr);
  printf("Initial value = %u (0x%08x)\n", wp->oldValue, wp->oldValue);
  return true;
}



bool free_wp(int no) {
  if (head == NULL) {
    printf("No watchpoint to delete\n");
    return false;
  }

  WP *p = head;
  WP *prev = NULL;

  while (p != NULL && p->NO != no) {
    prev = p;
    p = p->next;
  }

  if (p == NULL) {
    printf("Watchpoint %d not found\n", no);
    return false;
  }

  if (prev == NULL) {
    head = p->next;
  } else {
    prev->next = p->next;
  }

  printf("Delete watchpoint %d: %s\n", p->NO, p->expr);

  p->expr[0] = '\0';
  p->oldValue = 0;
  p->hitNum = 0;

  p->next = free_;
  free_ = p;
  return true;
}




bool watch_wp() {
  if (head == NULL) return true;

  bool all_unchanged = true;
  WP *p = head;

  while (p != NULL) {
    uint32_t val = 0;
    if (!eval_wp_expr(p->expr, &val)) {
      printf("error in watch_wp: bad expression '%s'\n", p->expr);
      p = p->next;
      continue;
    }

    if (val != p->oldValue) {
      p->hitNum++;

      printf("Hardware watchpoint %d: %s\n", p->NO, p->expr);
      printf("Old value = %u (0x%08x)\n", p->oldValue, p->oldValue);
      printf("New value = %u (0x%08x)\n", val, val);

      p->oldValue = val;
      all_unchanged = false;

      /* stop immediately after the first triggered watchpoint */
      return false;
    }

    p = p->next;
  }

  return all_unchanged;
}



void print_wp() {
  if (head == NULL) {
    printf("No watchpoint\n");
    return;
  }

  printf("Num\tHit\tValue(hex)\tValue(dec)\tExpr\n");

  WP *p = head;
  while (p != NULL) {
    printf("%d\t%d\t0x%08x\t%u\t\t%s\n",
           p->NO, p->hitNum, p->oldValue, p->oldValue, p->expr);
    p = p->next;
  }
}

