#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication HPFlashFix(argc, argv);
    MainWindow window;
    window.show();

    return HPFlashFix.exec();
}
