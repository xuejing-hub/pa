#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "monitor/breakpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}



static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}



static int cmd_q(char *args) {
  return -1;
}


//单步执行指令，参数为要执行的指令数，默认为1
static int cmd_si(char *args) {
  if (args == NULL) {
    cpu_exec(1);
    return 0;
  }

  char *n_str = strtok(args, " ");
  if (n_str == NULL) {
    cpu_exec(1);
    return 0;
  }

  int n = strtol(n_str, NULL, 10);
  if (n <= 0) n = 1;
  cpu_exec(n);
  return 0;
}



//打印寄存器信息
static int cmd_info(char *args) {
  if (args == NULL || *args == '\0') {
    printf("args error in cmd_info\n");
    return 0;
  }
  while (*args == ' ') args++;   // 跳过前导空格
  switch (*args) {
    case 'r': {
      int i;
      for (i = 0; i < 8; i++) {
        printf("%s  0x%x\n", regsl[i], reg_l(i));
      }
      printf("eip  0x%x\n", cpu.eip);
      for (i = 0; i < 8; i++) {
        printf("%s  0x%x\n", regsw[i], reg_w(i));
      }
      for (i = 0; i < 8; i++) {
        printf("%s  0x%x\n", regsb[i], reg_b(i));
      }
      return 0;
    }
    case 'w':
      print_wp();
      return 0;
    default:
      printf("args error in cmd_info\n");
      return 0;
  }
}


//扫描内存，参数为要扫描的单元数和表达式，表达式求值后得到起始地址
static int cmd_x(char *args) {
  if (args == NULL) {
    printf("Usage: x N EXPR\n");
    return 0;
  }
  while (*args == ' ') args++;
  if (*args == '\0') {
    printf("Usage: x N EXPR\n");
    return 0;
  }
  char *endptr = NULL;
  long n_long = strtol(args, &endptr, 10);
  if (endptr == args) {
    printf("Usage: x N EXPR\n");
    return 0;
  }
  if (n_long <= 0) {
    printf("Invalid count: %ld\n", n_long);
    return 0;
  }
  int n = (int)n_long;
  while (*endptr == ' ') endptr++;
  if (*endptr == '\0') {
    printf("Usage: x N EXPR\n");
    return 0;
  }
  bool success = false;
  vaddr_t addr = expr(endptr, &success);
  if (!success) {
    printf("Bad expression: %s\n", endptr);
    return 0;
  }
  const uint32_t PMEM_SIZE = 128 * 1024 * 1024;
  printf("Memory:\n");
  for (int i = 0; i < n; i++) {
    if (addr > PMEM_SIZE - 4) {
      printf("Address out of bound: 0x%08x\n", addr);
      return 0;
    }
    uint32_t val = vaddr_read(addr, 4);
    printf("0x%08x: 0x%08x\n", addr, val);
    addr += 4;
  }
  return 0;
}


//求值表达式，参数为表达式字符串，返回值为表达式的计算结果
static int cmd_p(char *args) {
  if (args == NULL) {
    printf("Usage: p EXPR\n");
    return 0;
  }
  while (*args == ' ' || *args == '\t') args++;
  if (*args == '\0') {
    printf("Usage: p EXPR\n");
    return 0;
  }
  bool success = false;
  uint32_t res = expr(args, &success);
  if (!success) {
    printf("error in expr(): %s\n", args);
  } else {
    printf("the value of expr is: %u (0x%x)\n", res, res);
  }
  return 0;
}


//设置监视点，参数为表达式字符串，表达式求值后得到监视点的初始值
static int cmd_w(char *args) {
  if (args == NULL) {
    printf("Usage: w EXPR\n");
    return 0;
  }

  while (*args == ' ' || *args == '\t') args++;
  if (*args == '\0') {
    printf("Usage: w EXPR\n");
    return 0;
  }

  if (new_wp(args)) {
    return 0;
  }

  printf("Failed to set watchpoint for '%s'\n", args);
  return 0;
}


//删除监视点，参数为监视点编号
static int cmd_d(char *args) {
  if (args == NULL) {
    printf("Usage: d N\n");
    return 0;
  }

  while (*args == ' ' || *args == '\t') args++;
  if (*args == '\0') {
    printf("Usage: d N\n");
    return 0;
  }

  char *endptr = NULL;
  long no = strtol(args, &endptr, 10);

  if (endptr == args) {
    printf("Invalid watchpoint number '%s'\n", args);
    return 0;
  }

  while (*endptr == ' ' || *endptr == '\t') endptr++;
  if (*endptr != '\0') {
    printf("Invalid watchpoint number '%s'\n", args);
    return 0;
  }

  if (no < 0) {
    printf("Invalid watchpoint number '%s'\n", args);
    return 0;
  }

  if (free_wp((int)no)) {
    printf("Success delete watchpoint %ld\n", no);
  } else {
    printf("error: no watchpoint %ld\n", no);
  }

  return 0;
}



static int cmd_b(char *args) {
  if (!args) { printf("Usage: b ADDR\n"); return 0; }
  vaddr_t addr = strtoul(args, NULL, 0);
  if (new_bp(addr)) return 0;
  printf("Failed to set breakpoint at %s\n", args);
  return 0;
}
static int cmd_db(char *args) {
  if (!args) { printf("Usage: db N\n"); return 0; }
  int no = strtol(args, NULL, 10);
  if (free_bp(no)) printf("Success delete breakpoint %d\n", no);
  else printf("error: no breakpoint %d\n", no);
  return 0;
}

static int cmd_help(char *args);

static int cmd_shell(char *args) {
  if (args == NULL) {
    printf("Usage: shell COMMAND\n");
    return 0;
  }

  FILE *fp = popen(args, "r");
  if (fp == NULL) {
    printf("Failed to run command: %s\n", args);
    return 0;
  }

  char buf[256];
  while (fgets(buf, sizeof(buf), fp) != NULL) {
    fputs(buf, stdout);
  }

  pclose(fp);
  return 0;
}

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
    { "help", "Display informations about all supported commands", cmd_help },
    { "c", "Continue the execution of the program", cmd_c },
    { "q", "Exit NEMU", cmd_q },
    {"si", "args:[N];execute [N] instructions step by step" , cmd_si},
    {"info","args:r/w;print information about registers or watchpoint" ,cmd_info},
    {"x" ,"x [N] [EXPR];scan the memory" ,cmd_x},
    {"p" ,"expr" ,cmd_p},
    {"w" ,"set the watchpoint",cmd_w},
    {"d" ,"delete the watchpoint",cmd_d},
    {"b" ,"set breakpoint at ADDR", cmd_b},
    {"db" ,"delete breakpoint by NO", cmd_db},
    {"shell", "Run a shell command", cmd_shell},
    {"!", "Run a shell command (alias)", cmd_shell},
};


#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}


void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
