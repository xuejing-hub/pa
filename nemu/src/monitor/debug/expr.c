#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256, TK_EQ

  /* number token */
  , TK_NUM

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* rules - order matters (more specific before general) */
  {" +", TK_NOTYPE},    // spaces
  {"0[xX][0-9a-fA-F]+", TK_NUM}, // hex number
  {"[0-9]+", TK_NUM},             // decimal number
  {"\\+", '+'},         // plus
  {"==", TK_EQ},          // equal
  {"\\-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"\\(", '('},
  {"\\)", ')'}
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

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;


        /* Now a new token is recognized with rules[i]. Record it. */
        switch (rules[i].token_type) {
          case TK_NOTYPE:
            /* skip spaces */
            break;
          case TK_NUM:
            tokens[nr_token].type = TK_NUM;
            if (substr_len >= (int)sizeof(tokens[nr_token].str))
              substr_len = sizeof(tokens[nr_token].str) - 1;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          default:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str[0] = '\0';
            nr_token++;
            break;
        }

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

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  if (nr_token == 0) {
    *success = false;
    return 0;
  }

  /* Single number */
  if (nr_token == 1 && tokens[0].type == TK_NUM) {
    *success = true;
    return (uint32_t)strtoul(tokens[0].str, NULL, 0);
  }

  /* Simple binary op: NUM op NUM */
  if (nr_token == 3 && tokens[0].type == TK_NUM && tokens[2].type == TK_NUM) {
    uint32_t a = (uint32_t)strtoul(tokens[0].str, NULL, 0);
    uint32_t b = (uint32_t)strtoul(tokens[2].str, NULL, 0);
    uint32_t res = 0;
    switch (tokens[1].type) {
      case '+': res = a + b; break;
      case '-': res = a - b; break;
      case '*': res = a * b; break;
      case '/': res = b ? (a / b) : 0; break;
      default: *success = false; return 0;
    }
    *success = true;
    return res;
  }

  *success = false;
  return 0;
}
