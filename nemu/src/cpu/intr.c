#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  
  // For now, just return to allow continuation
  // This is a temporary stub for basic debugging
  return;
}

void dev_raise_intr() {
}
