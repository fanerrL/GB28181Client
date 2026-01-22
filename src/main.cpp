#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    GB28181::MainWindow window;
    window.show();

    return app.exec();
}
