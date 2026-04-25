int init_monitor(int, char *[]);
void ui_mainloop(int);
int main(int argc, char *argv[]) {//NEMU的主函数，程序入口
  //初始化监视器，传入命令行参数，返回是否是批处理模式
  int is_batch_mode = init_monitor(argc, argv);
  //进入UI主循环，传入是否是批处理模式
  ui_mainloop(is_batch_mode);
  return 0;
}
