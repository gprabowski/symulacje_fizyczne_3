#include "mainwindow.h"

#include <QApplication>
#include <QColorSpace>
#include <QSurfaceFormat>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    QSurfaceFormat f;
    f.setRenderableType(QSurfaceFormat::OpenGL);
    f.setVersion(4, 2);
    f.setProfile(QSurfaceFormat::CoreProfile);
    f.setColorSpace(QColorSpace::SRgb);
    f.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    f.setDepthBufferSize(24);
    f.setStencilBufferSize(8);
    QSurfaceFormat::setDefaultFormat(f);
    MainWindow w;
    w.show();
    return a.exec();
}
