#include "mainwindow.h"

#include <QApplication>  //用 Qt 框架,啟用.exe 的「總管家」模組。

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show(); //show也是內建函數
    return a.exec(); //整個程式進入無限迴圈，"持續監聽" 有沒有被按按鍵之類的
}
