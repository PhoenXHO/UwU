#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <chrono>

using namespace std;

class Timer
{
    public:
        Timer()
        {
            start_point = chrono::high_resolution_clock::now();
        }

        ~Timer()
        {
            Stop();
        }

        void Stop()
        {
            auto end_point = chrono::high_resolution_clock::now();
            auto _start = chrono::time_point_cast<chrono::microseconds>(start_point).time_since_epoch().count();
            auto _end = chrono::time_point_cast<chrono::microseconds>(end_point).time_since_epoch().count();

            auto duration = _end - _start;
            double ms = duration * 0.001;

            cout << endl << duration << "us (" << ms << "ms)\n";
        }

    private:
        chrono::time_point<chrono::high_resolution_clock> start_point;
};

#endif // TIMER_H_INCLUDED
