#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QApplication::setApplicationName("MSI Control");
    QApplication::setOrganizationName("msi-ctl");

    MainWindow window;
    window.show();

    return app.exec();
}
