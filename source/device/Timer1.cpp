#include "Timer1.h"
#include "DeviceEvent.h"
#include "DeviceCompat.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static uint64_t current_time_us = 0;

static ClockPrescalerConfig* clock_configuration = NULL;
static uint32_t overflow_period = 0;
static uint16_t timer_id = 0;

static uint32_t ins_frequency = 0;

const static ClockPrescalerConfig Timer1Prescalers[TIMER_ONE_PRESCALER_OPTIONS] = {
    {1, _BV(CS10)},
    {8, _BV(CS11)},
    {64, _BV(CS10) | _BV(CS11)},
    {256, _BV(CS12)},
    {1024, _BV(CS10) | _BV(CS12)},
};

LIST_HEAD(event_list);

#define TIMER_1_MAX     65536  // 16 bit

void set_interrupt(uint32_t time_us)
{
    if(TIMSK1 & _BV(OCIE1A))
        return;

    if(time_us > overflow_period)
        return;

    uint16_t counter = 0;

    // calculate the timer value based on a number
    uint32_t tick_time = ins_frequency * clock_configuration->prescaleValue;

    uint16_t new_value =((uint32_t)time_us * 1000) / tick_time;

    counter = TCNT1;

    if(new_value < counter)
        return;

    OCR1A = new_value;

    TIMSK1 |= _BV(OCIE1A);

    return;
}

void consume_events(uint8_t compareEvent)
{
    if(list_empty(&event_list))
        return;

    ClockEvent* tmp = NULL;
    struct list_head *iter, *q = NULL;

    uint32_t period_us = overflow_period;

    uint8_t interrupt_set = 0;
    uint8_t first = 1;

    list_for_each_safe(iter, q, &event_list)
    {
        tmp = list_entry(iter, ClockEvent, list);

        //Serial.print("T: ");
        //Serial.println((uint32_t)tmp->countUs);

        // if we have received a compare event, we know we will be ripping off the top!
        if(first && compareEvent)
        {
            first = 0;

            // use a rough approximation of the time...
            period_us = (uint32_t)tmp->countUs;

            current_time_us += period_us;

            // fire our event and process the next event
            DeviceEvent(timer_id, tmp->value);

            // remove from the event list
            list_del(iter);

            // if this event is repeating, reset our timestamp
            if(tmp->period > 0)
            {
                // update our count, and readd to our event list
                tmp->countUs = tmp->period;
                tmp->addToList(&event_list);
            }
            else
            {
                delete tmp;
                continue;
            }
        }
        else
            tmp->countUs -= period_us;

        if(!interrupt_set && tmp->countUs < overflow_period)
        {
            // calculate the timer value based on a number
            uint32_t tickTime = ins_frequency * clock_configuration->prescaleValue;

            OCR1A = (uint16_t)(((uint32_t)tmp->countUs * 1000) / tickTime);
            TIMSK1 |= _BV(OCIE1A);
            interrupt_set = 1;
        }
    }
}

ISR(TIMER1_OVF_vect)
{
    // update our timestamp and consume any Clock events.
    current_time_us += overflow_period;
    consume_events(0);
}

ISR(TIMER1_COMPA_vect)
{
    // unset our compare interrupt and consume any Clock events.
    TIMSK1 &= ~_BV(OCIE1A);
    consume_events(1);
}

void Timer1::read()
{
    uint16_t counter = 0;

    __disable_irq();
    counter = TCNT1;
    TCNT1 = 0;
    __enable_irq();

    if(clock_configuration->prescaleValue == 8)
        counter = (counter >> 1);
    else
    {
        // scale our instruction time based on our current prescaler
        uint32_t tickTime = ins_frequency * clock_configuration->prescaleValue;
        counter = (tickTime * counter) / 1000;
    }

    current_time_us += counter;
}

int Timer1::setClockSelect(uint64_t precisionUs)
{
    // calculate a scaled minimum instruction time in nano seconds based on the CPU clock
    uint32_t ns = 1000000 / ((uint32_t)F_CPU / 1000);
    uint32_t timePerTick = 0;

    precisionUs *= 1000;

    clock_configuration = (ClockPrescalerConfig*)&Timer1Prescalers[TIMER_ONE_PRESCALER_OPTIONS - 1];

    // calculate the nearest clock configuration to the given period.
    for(uint8_t i = TIMER_ONE_PRESCALER_OPTIONS - 1; i >= 0; i--)
    {
        // timer per tick = (prescale) * ( 1 / frequency)
        timePerTick = clock_configuration->prescaleValue * ns;

        if(timePerTick < precisionUs)
            break;

        clock_configuration = (ClockPrescalerConfig*)&Timer1Prescalers[i];
    }

    uint32_t timerTickNs = ns * clock_configuration->prescaleValue;

    overflow_period = ((TIMER_1_MAX - 1) * timerTickNs) / 1000;

    return DEVICE_OK;
}

Timer1::Timer1(uint16_t id)
{
    timer_id = id;
    ins_frequency = 1000000 / ((uint32_t)F_CPU / 1000);
}

// prescaler set to 1, counter increments every 0.0625
int Timer1::init()
{
    if(status & SYSTEM_CLOCK_INIT)
        return DEVICE_OK;

    // clear control register A
    TCCR1A = 0;

    // set mode 0: normal timer
    TCCR1B = 0;

    start();

    status |= SYSTEM_CLOCK_INIT;

    return DEVICE_OK;
}

int Timer1::setTime(uint64_t timestamp)
{
    current_time_us = timestamp;
    return DEVICE_OK;
}

uint64_t Timer1::getTime()
{
    read();
    return current_time_us;
}

int Timer1::eventAfterUs(uint64_t interval, uint16_t value)
{
    if(new ClockEvent(interval, value, &event_list))
        return DEVICE_OK;

    return DEVICE_NO_RESOURCES;
}

int Timer1::eventAfter(uint64_t interval, uint16_t value)
{
    return eventAfterUs(interval * 1000, value);
}

int Timer1::eventEveryUs(uint64_t period, uint16_t value)
{
    ClockEvent* clk = new ClockEvent(period, value, &event_list, true);

    if(!clk)
        return DEVICE_NO_RESOURCES;

    if(event_list.next == &clk->list && period < overflow_period)
    {
        __disable_irq();
        set_interrupt(period);
        __enable_irq();
    }

    return DEVICE_OK;
}

int Timer1::eventEvery(uint64_t period, uint16_t value)
{
    return eventEveryUs(period * 1000, value);
}

int Timer1::start(uint64_t precisionUs)
{
    TIMSK1 = _BV(TOIE1); // interrupt on overflow

    setClockSelect(precisionUs);

    uint8_t sreg = SREG;
    __disable_irq();
    TCNT1 = 0;
    SREG = sreg;

    TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
    TCCR1B = (uint8_t)clock_configuration->register_config;

    // this is apparently a well known bug, where we could receive a phantom interrupt
    // as if the timer has ticked for the correct duration.
    // we essentially spin until the timer has ticked
    uint16_t counter = 0;

    // we previously set the TCNT1 reg to 0, so if it hasn't ticked, it will still
    // be zero
    do
    {
        sreg = SREG;
        __disable_irq();

        counter = TCNT1;
        SREG = sreg;
    } while (counter == 0);

    __enable_irq();

    return DEVICE_OK;
}

int Timer1::stop()
{
    TIMSK1 &= ~_BV(TOIE1); // disable interrupt on overflow
    TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
    return DEVICE_OK;
}

Timer1::~Timer1()
{
    stop();
}
