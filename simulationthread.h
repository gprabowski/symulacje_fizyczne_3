#ifndef SPRINGTHREAD_H
#define SPRINGTHREAD_H

#include <QQuaternion>
#include <QThread>
#include <QTimer>
#include <array>

using point_positions_t = std::array<std::array<std::array<QVector3D, 4>, 4>, 4>;
using frame_position_t = std::array<std::array<std::array<QVector3D, 2>, 2>, 2>;

struct SimulationSettings
{
    // interface musi dawać możliwość dodania:
    //  1. wyświetlanie punktów kontrolnych
    bool show_control_points;
    //  2. wyświetlanie ramki sterującej i połączeń z kostką
    bool show_control_frame;
    //  3. prostopadłościanu ograniczającego
    bool show_constraint;
    //  4. cieniowanej kostki beziera
    bool show_jelly;
    //  5. cieniowanej zdeformowanej bryły
    bool show_inner;
    //  6. zaburzenie
    bool use_randomization;
    //  7. zmiana masy punktów kontrolnych
    float mass;
    //  8. wartość tłumienia k
    float k;
    //  9. współczynnik sprężystości c1 (połączeń między masami)
    float c1;
    //  10. c2 (kostka a ramka sterująca)
    float c2;
    //  11. początkowe zaburzenie, określające maksymalną wartość losowanych prędkości lub odchyleń
    float random_max;

    // 12. dt in ms
    int dt_ms;
};

class SimulationThread : public QThread
{
    Q_OBJECT

public:
    SimulationThread(const SimulationSettings& s);

    virtual ~SimulationThread()
    {
        exit();
        wait();
    }

    void update() noexcept;
    float get_position() noexcept;
    float get_init_position() noexcept;
    unsigned long long update_count = 0;

    void updateSettings(const SimulationSettings& s);

public slots:
    void restart(const SimulationSettings& s);
    void frame_changed(frame_position_t f);
signals:
    void positionChanged(point_positions_t pos);

private:
    void precalculate();

    QTimer s_timer;
    SimulationSettings settings;
    point_positions_t current_positions;
    frame_position_t frame;

protected:
    void run() override;
};

#endif // SPRINGTHREAD_H
