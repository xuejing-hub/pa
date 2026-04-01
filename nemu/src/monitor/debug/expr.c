#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256,
  TK_NUMBER,
  TK_HEX,
  TK_REG,
  TK_EQ,
  TK_NEQ,
  TK_AND,
  TK_OR,
  TK_BOR,
  TK_BXOR,
  TK_BAND,
  TK_SHL,
  TK_SHR,
  TK_LT,
  TK_LE,
  TK_GT,
  TK_GE,
  TK_NEGATIVE,
  TK_DEREF,
  /* TODO: Add more token types */
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},  // spaces
  {"0x[1-9A-Fa-f][0-9A-Fa-f]*", TK_HEX},
  {"0|[1-9][0-9]*", TK_NUMBER},
  {"\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|eip|ax|cx|dx|bx|sp|bp|si|di|al|cl|dl|bl|ah|ch|dh|bh)", TK_REG},
  {"==", TK_EQ},
  {"!=", TK_NEQ},
  {"&&", TK_AND},
  {"\\|\\|", TK_OR},
  {"\\|", TK_BOR},
  {"\\^", TK_BXOR},
  {"&", TK_BAND},
  {"<<", TK_SHL},
  {">>", TK_SHR},
  {"<=", TK_LE},
  {">=", TK_GE},
  {"<", TK_LT},
  {">", TK_GT},
  {"!", '!'},
  {"\\+", '+'},      // plus
  {"\\-", '-'},
  {"\\*", '*'},
  {"\\/", '/'},
  {"\\(", '('},
  {"\\)", ')'},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;
static bool expr_error = false;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo - pmatch.rm_so;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        if (substr_len >= (int)sizeof(tokens[nr_token].str))
          assert(0);

        if (rules[i].token_type == TK_NOTYPE) {
          /* skip whitespace */
          break;
        }

        tokens[nr_token].type = rules[i].token_type;

        if (rules[i].token_type == TK_NUMBER) {
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
        }
        else if (rules[i].token_type == TK_HEX) {
          /* skip 0x */
          strncpy(tokens[nr_token].str, substr_start + 2, substr_len - 2);
          tokens[nr_token].str[substr_len - 2] = '\0';
        }
        else if (rules[i].token_type == TK_REG) {
          /* skip $ */
          strncpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);
          tokens[nr_token].str[substr_len - 1] = '\0';
        }
        else {
          tokens[nr_token].str[0] = '\0';
        }

                /* print success record for debugging/tokenization tracing */
                printf("Success record : nr token=%d,type=%d,str=%s\n", nr_token, tokens[nr_token].type, tokens[nr_token].str);

                nr_token++;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


bool check_parentheses(int p, int q){
    if(p >= q){
        printf("error:p>=q in check_parentheses\n");
        return false;
    }

    if(tokens[p].type != '(' || tokens[q].type != ')')
        return false;

    int count = 0;
    for(int curr = p + 1; curr < q; curr++){
        if(tokens[curr].type == '(')
            count += 1;

        if(tokens[curr].type == ')'){
            if(count != 0)
                count -= 1;
            else
                return false;
        }
    }

    if(count == 0)
        return true;
    else
        return false;
}
int priority(int i) {
    /* Lower number -> lower precedence (DominantOp picks smallest). */
    if (tokens[i].type == TK_OR) return 0;               /* || */
    if (tokens[i].type == TK_AND) return 1;              /* && */
    if (tokens[i].type == TK_BOR) return 2;              /* |  */
    if (tokens[i].type == TK_BXOR) return 3;             /* ^  */
    if (tokens[i].type == TK_BAND) return 4;             /* &  */
    if (tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ) return 5; /* == != */
    if (tokens[i].type == TK_LT || tokens[i].type == TK_LE || tokens[i].type == TK_GT || tokens[i].type == TK_GE) return 6; /* < <= > >= */
    if (tokens[i].type == TK_SHL || tokens[i].type == TK_SHR) return 7; /* << >> */
    if (tokens[i].type == '+' || tokens[i].type == '-') return 8;
    if (tokens[i].type == '*' || tokens[i].type == '/') return 9;
    if (tokens[i].type == TK_NEGATIVE || tokens[i].type == TK_DEREF || tokens[i].type == '!') return 10; /* unary */
    return 10000;
}
int DominantOp(int p, int q) {
    int i = 0, j, cnt;
    int op = 10000, opp, pos = -1;

    for (i = p; i <= q; i++){
        if (tokens[i].type == TK_NUMBER || tokens[i].type == TK_REG || tokens[i].type == TK_HEX)
            continue;
        else if (tokens[i].type == '(') {
            cnt = 0;
            for (j = i + 1; j <= q; j++) {
                if (tokens[j].type == ')') {
                    cnt++;
                    i += cnt;
                    break;
                }
                else
                    cnt++;
            }
        }
        else {
            opp = priority(i);
            if (opp <= op) {
                pos = i;
                op = opp;
            }
        }
    }
    return pos;
}
int eval(int p, int q)
{
    if (expr_error) return 0;

    if(p > q){
        printf("error:p>q in eval,p=%d,q=%d\n", p, q);
        expr_error = true;
        return 0;
    }

    if(p == q){
        int num;
        switch(tokens[p].type){
            case TK_NUMBER:
                sscanf(tokens[p].str, "%d", &num);
                return num;

            case TK_HEX:
                sscanf(tokens[p].str, "%x", &num);
                return num;

            case TK_REG:
                for(int i = 0; i < 8; i++){
                    if(strcmp(tokens[p].str, regsl[i]) == 0)
                        return reg_l(i);
                    if(strcmp(tokens[p].str, regsw[i]) == 0)
                        return reg_w(i);
                    if(strcmp(tokens[p].str, regsb[i]) == 0)
                        return reg_b(i);
                }
                if(strcmp(tokens[p].str, "eip") == 0)
                    return cpu.eip;
                else{
                    printf("error in TK_REG in eval()\n");
                    assert(0);
                }
        }
    }

    if(p < q){
        if(check_parentheses(p, q) == true)
            return eval(p + 1, q - 1);
        else{
            int op = DominantOp(p, q);
            if (op < 0) {
                printf("error: no dominant operator in eval(p=%d,q=%d)\n", p, q);
                expr_error = true;
                return 0;
            }
            vaddr_t addr;
            int result;

            switch(tokens[op].type){
                case TK_NEGATIVE:
                    return -eval(p + 1, q);

                case TK_DEREF:
                    addr = eval(p + 1, q);
                    result = vaddr_read(addr, 4);
                    printf("addr=%u(0x%x)----->value=%d(0x%08x)\n", addr, addr, result, result);
                    return result;

                case '!':
                    result = eval(p + 1, q);
                    if(result != 0)
                        return 0;
                    else
                        return 1;
            }

            int val1 = eval(p, op - 1);
            int val2 = eval(op + 1, q);

            switch(tokens[op].type){
                case '+': return val1 + val2;
                case '-': return val1 - val2;
                case '*': return val1 * val2;
                case '/':
                    if (val2 == 0) {
                        printf("error: division by zero in eval()\n");
                        expr_error = true;
                        return 0;
                    }
                    return val1 / val2;
                case TK_SHL: return val1 << val2;
                case TK_SHR: return val1 >> val2;
                case TK_LT: return val1 < val2;
                case TK_LE: return val1 <= val2;
                case TK_GT: return val1 > val2;
                case TK_GE: return val1 >= val2;
                case TK_BAND: return val1 & val2;
                case TK_BXOR: return val1 ^ val2;
                case TK_BOR: return val1 | val2;
                case TK_EQ: return val1 == val2;
                case TK_NEQ: return val1 != val2;
                case TK_AND: return val1 && val2;
                case TK_OR: return val1 || val2;
                default: assert(0);
            }
        }
    }

    return 0;
}
uint32_t expr(char *e, bool *success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }

    expr_error = false;

    /* Determine unary minus/dereference: if the previous token is a number,
     * hex, register or a closing parenthesis, then '-' and '*' are binary;
     * otherwise they are unary (negative / dereference).
     */
    if (tokens[0].type == '-')
        tokens[0].type = TK_NEGATIVE;

    if (tokens[0].type == '*')
        tokens[0].type = TK_DEREF;

    for (int i = 1; i < nr_token; i++) {
        if (tokens[i].type == '-') {
            int prev = tokens[i - 1].type;
            if (!(prev == TK_NUMBER || prev == TK_HEX || prev == TK_REG || prev == ')'))
                tokens[i].type = TK_NEGATIVE;
        }

        if (tokens[i].type == '*') {
            int prev = tokens[i - 1].type;
            if (!(prev == TK_NUMBER || prev == TK_HEX || prev == TK_REG || prev == ')'))
                tokens[i].type = TK_DEREF;
        }
    }

    int ret = eval(0, nr_token - 1);
    if (expr_error) {
        *success = false;
        return 0;
    }

    *success = true;
    return (uint32_t)ret;
}