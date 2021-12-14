#include "simulationthread.h"
#include <QDebug>
#include <QtMath>
#include <cmath>
#include <type_traits>

template <typename T>
struct function_traits
    : public function_traits<decltype(&T::operator())>
{
};
// For generic types, directly use the result of the signature of its 'operator()'

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType (ClassType::*)(Args...) const>
// we specialize for pointers to member function
{
    enum
    {
        arity = sizeof...(Args)
    };
    // arity is the number of arguments.

    typedef ReturnType result_type;

    template <size_t i>
    struct arg
    {
        typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
        // the i-th argument is equivalent to the i-th tuple element of a tuple
        // composed of those arguments.
    };
};

void SimulationThread::precalculate()
{
    // Precalculate I with respect to the fixed vertex
    const float edge_squared = settings.edge_length * settings.edge_length;
    const float edge_third = edge_squared * settings.edge_length;
    initial_center = QVector3D(0.f, settings.edge_length * sqrtf(3.f) * 0.5, 0.f);
    mass = (edge_third)*settings.density;
    const float edge_fifth = std::pow(settings.edge_length, 5);
    const float density_div_6 = settings.density / 6.0f;

    const float data[] = {
        1.f / 12.f * edge_squared * (9 + 2 * edge_third * settings.density),
        0.f,
        0.f,
        0.f,
        edge_fifth * density_div_6,
        0.f,
        0.f,
        0.f,
        1.f / 12.f * edge_squared * (9 + 2 * edge_third * settings.density)
    };

    tensor = QMatrix3x3(data);

    const float data_inv[] = {
        12.0f / (9 * edge_squared + 2 * edge_fifth * settings.density),
        0.f,
        0.f,
        0.f,
        6.f / (edge_fifth * settings.density),
        0.f,
        0.f,
        0.f,
        12.0f / (9 * edge_squared + 2 * edge_fifth * settings.density)
    };

    tensor_inverted = QMatrix3x3(data_inv);
}

SimulationThread::SimulationThread(const SimulationSettings& s)
{
    settings = s;
    s_timer.setTimerType(Qt::PreciseTimer);
    connect(&s_timer, &QTimer::timeout, this, &SimulationThread::update);
    s_timer.moveToThread(this);
}

template <typename Derivative, typename ResultType = typename function_traits<Derivative>::result_type>
inline ResultType rk4_helper(float h, Derivative derivative)
{
    typedef typename function_traits<Derivative>::result_type func_type;
    // regular der at this place
    // k1 = f(y(t0), t0);
    // k2 = f(y(t0) + k1 * h / 2, t0 + h / 2)
    // k3 = f(y(t0) + k2 * h / 2, t0 + h / 2)
    // k4 = f(y(t0) + k3 * h, t0 + h)

    func_type k1 = derivative();
    func_type k2 = derivative(func_type(k1 * h / 2.f));
    func_type k3 = derivative(func_type(k2 * h / 2.f));
    func_type k4 = derivative(func_type(k3 * h));

    return (1 / 6.f * k1 + 1 / 3.f * k2 + 1 / 3.f * k3 + 1 / 6.f * k4) * h;
}

inline QVector3D mult(const QMatrix3x3& m, const QVector3D v)
{
    return QVector3D {
        v[0] * m(0, 0) + v[1] * m(0, 1) + v[2] * m(0, 2),
        v[0] * m(1, 0) + v[1] * m(1, 1) + v[2] * m(1, 2),
        v[0] * m(2, 0) + v[1] * m(2, 1) + v[2] * m(2, 2)
    };
}

RK4_ret SimulationThread::rk4(const QVector3D& body_torque)
{
    RK4_ret ret {};
    // first part == change of angular velocity of gravitational rotation
    const auto pre1 = mult(tensor_inverted, body_torque);
    ret.ang_vel_t = rk4_helper(settings.dt_ms / 1000.f, [=](QVector3D W = QVector3D(0.f, 0.f, 0.f)) -> QVector3D
        { return pre1 + mult(tensor_inverted, QVector3D::crossProduct(mult(tensor, current_av + W), current_av + W)); });

    // second part == change of total quaternion of rotation
    ret.quat_t = rk4_helper(settings.dt_ms / 1000.f, [=](QQuaternion Q = QQuaternion(0.f, 0.f, 0.f, 0.f)) -> QQuaternion
        { return (current_rotation + Q).normalized() * QQuaternion(0.f, (current_av + ret.ang_vel_t) * 0.5f); });

    return ret;
}

void SimulationThread::update() noexcept
{
    QVector3D world_torque = settings.gravity
        ? QVector3D::crossProduct({ current_rotation.rotatedVector(initial_center) }, (mass * 9.81f * QVector3D { 0.f, -1.f, 0.0f }))
        : QVector3D();
    QVector3D body_torque = current_rotation.conjugated().rotatedVector(world_torque);

    const auto res = rk4(body_torque);

    current_av += res.ang_vel_t;
    current_rotation += res.quat_t;
    current_rotation.normalize();

    float ang;
    QVector3D ax;
    current_rotation.getAxisAndAngle(&ax, &ang);
    emit positionChanged(current_rotation);
}

void SimulationThread::updateSettings(const SimulationSettings& s)
{
    settings = s;
    precalculate();
}

void SimulationThread::restart(const SimulationSettings& s)
{
    exit();
    wait();
    settings = s;
    precalculate();
    current_rotation = QQuaternion::fromAxisAndAngle(1, 0, 0, settings.deviation);
    current_av = settings.angular_velocity * QVector3D(0.f, 1.f, 0.f);
    start();
}

void SimulationThread::run()
{
    s_timer.start(settings.dt_ms);
    exec();
}
