#include <QApplication>
#include "fzspdlog.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow* desktop = new MainWindow();

  desktop->show();

  int result = a.exec();

  delete desktop;

  desktop = nullptr;
  // 清理命令行参数
  if (argv) {
    for (int i = 0; i < argc; i++) {
      delete[] argv[i];
    }
    delete[] argv;
  }

  return result;
}
