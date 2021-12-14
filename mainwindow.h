#pragma once

#include <QMainWindow>
#include <QWidget>

#include "./ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow)
    {
        ui->setupUi(this);

        auto& w = ui->openGLWidget;

        QObject::connect(ui->edgeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](const double v)
            { w->simulation_settings.edge_length = v; });

        QObject::connect(ui->densitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](const double v)
            { w->simulation_settings.density = v; });
        QObject::connect(ui->deviationSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](const double v)
            { w->simulation_settings.deviation = v; });
        QObject::connect(ui->angularVelocitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](const double v)
            { w->simulation_settings.angular_velocity = v; });
        QObject::connect(ui->dtSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [&](const int v)
            { w->simulation_settings.dt_ms = v; });
        QObject::connect(ui->gravityCheckBox, QOverload<bool>::of(&QCheckBox::clicked), [&](const bool v)
            { w->simulation_settings.gravity = v; });
        QObject::connect(ui->applyButton, QOverload<bool>::of(&QPushButton::clicked), [&](const bool v)
            { w->updateSetting(); });
        QObject::connect(ui->resetAndApplyButton, QOverload<bool>::of(&QPushButton::clicked), [&](const bool v)
            { w->restartSimulation(); });

        QObject::connect(ui->cubeCheckBox, QOverload<bool>::of(&QCheckBox::clicked), [&](const bool v)
            { w->display_cube = v; });
        QObject::connect(ui->diagonalCheckBox, QOverload<bool>::of(&QCheckBox::clicked), [&](const bool v)
            { w->display_diagonal = v; });
        QObject::connect(ui->pathCheckBox, QOverload<bool>::of(&QCheckBox::clicked), [&](const bool v)
            { w->display_path = v; });
        QObject::connect(ui->pointsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), w, &OpenGLWidget::resetPoints);
    }

    ~MainWindow()
    {
        delete ui;
    }

private:
    Ui::MainWindow* ui;
};
