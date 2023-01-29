#include "MouseTrapper.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MouseTrapper w;
    w.show();
    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(QuitApplication()));
    return a.exec();
}
