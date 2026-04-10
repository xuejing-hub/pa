int init_monitor(int, char *[]);
void ui_mainloop(int);

int main(int argc, char *argv[]) {
  /* 使用命令行参数初始化监控程序，决定是否进入批处理模式 */
  int is_batch_mode = init_monitor(argc, argv);

  /* 进入命令处理主循环；批处理模式下会在执行完命令后退出 */
  ui_mainloop(is_batch_mode);

  /* 正常退出 */
  return 0;
}
