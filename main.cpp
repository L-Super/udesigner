#include <QApplication>
#include "udesigner.h"
#include <QFile>
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QFile qss(":resources/MacOS.qss");
    if( qss.open(QFile::ReadOnly))
    {
        qApp->setStyleSheet(qss.readAll());
        qss.close();
    }
    UDesigner u;
    u.show();
    //test
    return QApplication::exec();
}
