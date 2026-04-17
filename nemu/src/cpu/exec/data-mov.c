#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&id_src->val);

  print_asm_template1(push);
}

make_EHelper(pop) {
  rtlreg_t tmp;
  rtl_pop(&tmp);
  operand_write(id_dest, &tmp);

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  TODO();

  print_asm("pusha");
}

make_EHelper(popa) {
  TODO();

  print_asm("popa");
}

make_EHelper(leave) {
  rtlreg_t tmp;

  rtl_mv(&cpu.esp, &cpu.ebp);
  rtl_pop(&tmp);
  rtl_mv(&cpu.ebp, &tmp);

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    rtlreg_t ax_val, sign;

    rtl_lr_w(&ax_val, R_AX);
    rtl_sext(&ax_val, &ax_val, 2);
    rtl_sari(&sign, &ax_val, 31);
    rtl_sr_w(R_DX, &sign);
  } else {
    rtlreg_t sign;

    rtl_sari(&sign, &cpu.eax, 31);
    rtl_mv(&cpu.edx, &sign);
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    rtl_lr(&t2, R_AX, 2);
    rtl_sext(&t2, &t2, 2);
    rtl_sari(&t2, &t2, 31);
    rtl_sr(R_DX, 2, &t2);
  }
  else {
    rtl_sari(&t2, &cpu.eax, 31);
    rtl_sr(R_EDX, 4, &t2);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
