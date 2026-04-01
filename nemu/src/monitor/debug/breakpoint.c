#include "monitor/breakpoint.h"
#include <stdio.h>

#define NR_BP 32

static BP bp_pool[NR_BP];
static BP *bp_head, *bp_free_;

void init_bp_pool() {
  int i;
  for (i = 0; i < NR_BP; i++) {
    bp_pool[i].NO = i;
    bp_pool[i].next = &bp_pool[i + 1];
  }
  bp_pool[NR_BP - 1].next = NULL;
  bp_head = NULL;
  bp_free_ = bp_pool;
}

bool new_bp(vaddr_t addr) {
  if (bp_free_ == NULL) {
    printf("No free breakpoint available\n");
    return false;
  }
  BP *bp = bp_free_;
  bp_free_ = bp_free_->next;
  bp->next = NULL;
  bp->addr = addr;
  bp->hitNum = 0;
  if (bp_head == NULL) bp_head = bp;
  else {
    BP *p = bp_head;
    while (p->next) p = p->next;
    p->next = bp;
  }
  printf("Success: set breakpoint %d at 0x%08x\n", bp->NO, bp->addr);
  return true;
}

bool free_bp(int no) {
  BP *p = bp_head, *prev = NULL;
  while (p) {
    if (p->NO == no) break;
    prev = p; p = p->next;
  }
  if (!p) return false;
  if (prev == NULL) bp_head = p->next; else prev->next = p->next;
  p->next = bp_free_;
  bp_free_ = p;
  return true;
}

bool check_bp(vaddr_t eip) {
  BP *p = bp_head;
  while (p) {
    if (p->addr == eip) {
      p->hitNum++;
      printf("Breakpoint %d hit at 0x%08x\n", p->NO, eip);
      return false;
    }
    p = p->next;
  }
  return true;
}

void print_bp() {
  BP *p = bp_head;
  if (!p) { printf("No breakpoint\n"); return; }
  printf("Num\tAddr\thitTimes\n");
  while (p) {
    printf("%d\t0x%08x\t%d\n", p->NO, p->addr, p->hitNum);
    p = p->next;
  }
}
