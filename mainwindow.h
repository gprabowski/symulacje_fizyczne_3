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

        QObject::connect(ui->controlPointsCheckBox, QOverload<bool>::of(&QCheckBox::clicked), [&](const bool v)
            { w->simulation_settings.show_control_points = v; });

        QObject::connect(ui->frameCheckBox, QOverload<bool>::of(&QCheckBox::clicked), [&](const bool v)
            { w->simulation_settings.show_control_frame = v; });

        QObject::connect(ui->constraintCheckBox, QOverload<bool>::of(&QCheckBox::clicked), [&](const bool v)
            { w->simulation_settings.show_constraint = v; });

        QObject::connect(ui->jellyCheckBox, QOverload<bool>::of(&QCheckBox::clicked), [&](const bool v)
            { w->simulation_settings.show_jelly = v; });

        QObject::connect(ui->innerCheckBox, QOverload<bool>::of(&QCheckBox::clicked), [&](const bool v)
            { w->simulation_settings.show_inner = v; });

        QObject::connect(ui->randomCheckBox, QOverload<bool>::of(&QCheckBox::clicked), [&](const bool v)
            { w->simulation_settings.use_randomization = v; });

        QObject::connect(ui->massSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](const double v)
            { w->simulation_settings.mass = v; });

        QObject::connect(ui->kSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](const double v)
            { w->simulation_settings.k = v; });

        QObject::connect(ui->c1SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](const double v)
            { w->simulation_settings.c1 = v; });

        QObject::connect(ui->c2SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](const double v)
            { w->simulation_settings.c2 = v; });

        QObject::connect(ui->randomSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](const double v)
            { w->simulation_settings.random_max = v; });

        QObject::connect(ui->l0SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](const double v)
            { w->simulation_settings.l0 = v; });

        QObject::connect(ui->dtSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [&](const int v)
            { w->simulation_settings.dt_ms = v; });

        // reset and apply buttons
        QObject::connect(ui->applyButton, QOverload<bool>::of(&QPushButton::clicked), [&](const bool)
            { w->updateSetting(); });

        QObject::connect(ui->resetAndApplyButton, QOverload<bool>::of(&QPushButton::clicked), [&](const bool)
            { w->restartSimulation(); });
    }

    ~MainWindow()
    {
        delete ui;
    }

private:
    Ui::MainWindow* ui;
};
