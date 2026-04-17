#include "cpu/exec.h"

make_EHelper(test) {
  rtlreg_t result, zero = 0;

  rtl_and(&result, &id_dest->val, &id_src->val);
  rtl_update_ZFSF(&result, id_dest->width);
  rtl_set_CF(&zero);
  rtl_set_OF(&zero);

  print_asm_template2(test);
}

make_EHelper(and) {
  rtlreg_t result, zero = 0;

  rtl_and(&result, &id_dest->val, &id_src->val);
  operand_write(id_dest, &result);

  rtl_update_ZFSF(&result, id_dest->width);
  rtl_set_CF(&zero);
  rtl_set_OF(&zero);

  print_asm_template2(and);
}

make_EHelper(xor) {
  rtlreg_t result, zero = 0;

  rtl_xor(&result, &id_dest->val, &id_src->val);
  operand_write(id_dest, &result);

  rtl_set_CF(&zero);
  rtl_set_OF(&zero);
  rtl_update_ZFSF(&result, id_dest->width);

  print_asm_template2(xor);
}

make_EHelper(or) {
  rtlreg_t result, zero = 0;

  rtl_or(&result, &id_dest->val, &id_src->val);
  operand_write(id_dest, &result);

  rtl_update_ZFSF(&result, id_dest->width);
  rtl_set_CF(&zero);
  rtl_set_OF(&zero);

  print_asm_template2(or);
}

make_EHelper(sar) {
  rtlreg_t result;
  rtlreg_t cnt = id_src->val & 0x1f;

  if (cnt == 0) {
    print_asm_template2(sar);
    return;
  }

  rtl_sext(&result, &id_dest->val, id_dest->width);
  rtl_sar(&result, &result, &id_src->val);
  operand_write(id_dest, &result);
  rtl_update_ZFSF(&result, id_dest->width);
  
  print_asm_template2(sar);
}

make_EHelper(shl) {
  rtlreg_t result;
  rtlreg_t cnt = id_src->val & 0x1f;

  if (cnt == 0) {
    print_asm_template2(shl);
    return;
  }

  rtl_shl(&result, &id_dest->val, &id_src->val);
  operand_write(id_dest, &result);
  rtl_update_ZFSF(&result, id_dest->width);

  print_asm_template2(shl);
}

make_EHelper(shr) {
  rtlreg_t result;
  rtlreg_t cnt = id_src->val & 0x1f;

  if (cnt == 0) {
    print_asm_template2(shr);
    return;
  }

  rtl_shr(&result, &id_dest->val, &id_src->val);
  operand_write(id_dest, &result);
  rtl_update_ZFSF(&result, id_dest->width);

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  rtlreg_t result;

  rtl_mv(&result, &id_dest->val);
  rtl_not(&result);

  operand_write(id_dest, &result);

  print_asm_template1(not);
}

make_EHelper(rol) {
  rtlreg_t result;
  rtlreg_t cnt = id_src->val & 0x1f;

  if (cnt == 0) {
    print_asm_template2(rol);
    return;
  }

  rtl_shl(&result, &id_dest->val, &id_src->val);
  rtl_shr(&t0, &id_dest->val, &(rtlreg_t){32 - cnt});
  rtl_or(&result, &result, &t0);
  operand_write(id_dest, &result);

  print_asm_template2(rol);
}
