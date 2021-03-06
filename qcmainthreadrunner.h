#pragma once

#include <QObject>
#include <QCoreApplication>
#include <QEventLoop>
#include <QThreadPool>

class QCMainThreadRunner
{

public:

    /// Run a function on main thread. If it is already in main thread, it will be executed in next tick.
    template <typename F>
    static void run(F func) {
        QObject tmp;
        QObject::connect(&tmp, &QObject::destroyed, QCoreApplication::instance(), std::move(func), Qt::QueuedConnection);
    }

    /// Run a function with custom data on main thread. If it is already in main thread, it will be executed in next tick.
    template <typename F,typename P1>
    static void run(F func,P1 p1) {
        auto wrapper = [=]() -> void{
            func(p1);
        };

        QObject tmp;
        QObject::connect(&tmp, &QObject::destroyed, QCoreApplication::instance(), std::move(wrapper), Qt::QueuedConnection);
    }

    template <typename F,typename P1, typename P2>
    static void run(F func,P1 p1, P2 p2) {
        auto wrapper = [=]() -> void{
            func(p1, p2);
        };

        QObject tmp;
        QObject::connect(&tmp, &QObject::destroyed, QCoreApplication::instance(), std::move(wrapper), Qt::QueuedConnection);
    }

    template <typename F>
    static void blockingRun(F func) {
        QObject* emitter1 = new QObject();

        QObject::connect(emitter1, &QObject::destroyed, QCoreApplication::instance(),
                         func, Qt::BlockingQueuedConnection);
        delete emitter1;
    }

    template <typename F, typename ... Args>
    static auto blockingRunRet(F func) -> decltype(func()) {
        decltype(func()) t;

        auto wrapper = [&]() -> void {
            t = func();
        };

        QObject* emitter1 = new QObject();

        QObject::connect(emitter1, &QObject::destroyed, QCoreApplication::instance(),
                         wrapper, Qt::BlockingQueuedConnection);
        delete emitter1;

        return t;
    }

    /// Wait until all threads to exit and removes all threads from this thread pool
    /// Unlike QThreadPool::waitForDone(). It will process events from event loop while waiting.

    static void waitForFinished(QThreadPool& pool);

    QCMainThreadRunner();

};

template <typename F>
const QCMainThreadRunner& operator<<(const QCMainThreadRunner& runner,F f) {
    QCMainThreadRunner::blockingRun(f);
    return runner;
}

#define MAIN_THREAD \
    QCMainThreadRunner() << [&]()

