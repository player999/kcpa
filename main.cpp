#include "kcpa.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    KCPA w;
    w.show();

    return a.exec();
}
