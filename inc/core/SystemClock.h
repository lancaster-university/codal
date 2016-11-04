#include <stdint.h>

#include "DeviceComponent.h"
#include "list.h"
#include "ErrorNo.h"

#include "Arduino.h"

#define SYSTEM_CLOCK_INIT 0x01

struct ClockEvent
{
    uint16_t value;
    uint64_t period;
    uint64_t countUs;
    struct list_head list;

    void addToList(list_head* head)
    {
        ClockEvent* tmp = NULL;
        struct list_head *iter, *q = NULL;

        list_for_each_safe(iter, q, head)
        {
            tmp = list_entry(iter, ClockEvent, list);

            if(tmp->countUs < this->countUs)
                continue;

            break;
        }

        list_add(&this->list, iter);
    }
    
    ClockEvent(uint64_t period, int value, list_head* head, bool repeating = false)
    {
        this->countUs = period;
        this->value = value;

        this->period = 0;

        if(repeating)
            this->period = period;

        addToList(head);
    };
};

struct ClockPrescalerConfig
{
    uint16_t prescaleValue;
    uint16_t register_config;
};

class SystemClock : protected DeviceComponent
{

public:
    SystemClock() {};

    virtual int init() { return DEVICE_OK; };

    virtual int setTime(uint64_t timestamp) { return DEVICE_OK; };
    virtual uint64_t getTime() { return 0; };

    virtual int eventAfter(uint64_t interval, uint16_t value) { return DEVICE_OK; };
    virtual int eventEvery(uint64_t interval, uint16_t value) { return DEVICE_OK; };

    virtual int start() { return DEVICE_OK; };
    virtual int stop() { return DEVICE_OK; };

    virtual ~SystemClock() { stop(); };
};
