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

/* TODO: Implement the functionality of watchpoint */

void print_wp() {
  WP *p = head;
  if (p == NULL) {
    printf("No watchpoint\n");
    return;
  }
  printf("Num\tExpression\tValue\n");
  while (p) {
    /* no expression/value stored in this simplified WP, just print NO */
    printf("%d\t(unspecified)\n", p->NO);
    p = p->next;
  }
}


