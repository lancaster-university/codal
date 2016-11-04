#include "SystemClock.h"

#define TIMER_ONE_DEFAULT_PRECISION_US    1
#define TIMER_ONE_PRESCALER_OPTIONS       5

#define TIMER_ONE_PRESCALER_OPTIONS       5

class Timer1 : SystemClock
{
    void read();
    int setClockSelect(uint64_t periodUs);

public:
    Timer1(uint16_t id = DEVICE_ID_TIMER_1);

    int init();

    int setTime(uint64_t timestamp);
    uint64_t getTime();

    int eventAfterUs(uint64_t interval, uint16_t value);
    int eventAfter(uint64_t interval, uint16_t value);

    int eventEveryUs(uint64_t period, uint16_t value);
    int eventEvery(uint64_t period, uint16_t value);

    int start(uint64_t precisionUs = TIMER_ONE_DEFAULT_PRECISION_US);
    int stop();

    ~Timer1();
};
