#ifndef DEVICE_SIZE_H
#define DEVICE_SIZE_H

#include "stdint.h"

#define PROCESSOR_WORD_TYPE uint32_t

static inline void drain_pin_charge(int pinID)
{
    int grp = pinID >> 5;
    int mask = 1 << (pinID & 31);
    PORT->Group[grp].OUTCLR.reg = mask;
    PORT->Group[grp].DIRSET.reg = mask;
    PORT->Group[grp].DIRCLR.reg = mask;
}

#endif
