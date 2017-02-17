/*
The MIT License (MIT)

Copyright (c) 2016 British Broadcasting Corporation.
This software is provided by Lancaster University by arrangement with the BBC.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "AnalogButton.h"

/**
 * Constructor.
 * Create an AnalogButton instance used to generate button events for an AnalogSensor.
 *
 * @param pin the pin on which the AnalogSensor senses.
 * @param id the unique EventModel id of this component.
 * @param sensor the AnalogSensor to generate button events for.
 * @param activeAboveThreshold whether the button should be considered active when readings are above the given threshold or below.
 * @param threshold the threshold at which the button is considered active.
 *
 */
AnalogButton::AnalogButton(DevicePin &pin, uint16_t id, AnalogSensor &sensor, bool activeAboveThreshold, int threshold) : DeviceButton(pin, id, DEVICE_BUTTON_ALL_EVENTS, ACTIVE_LOW, PullNone), analogSensor(sensor) {
    this->activeAboveThreshold = activeAboveThreshold;
    this->threshold = threshold;
}

/**
 * Determines if this button is instantenously active (i.e. pressed).
 * Internal method, use before debouncing.
 */
int AnalogButton::buttonActive()
{
    if (activeAboveThreshold)
    {
        return analogSensor.getValue() >= threshold;
    }
    else
    {
        return analogSensor.getValue() <= threshold;
    }
}

/**
 * If set to true, the button will be considered active when the AnalogSensor's
 * readings are above the threshold. If false, the button will be considered active
 * when the AnalogSensor's readings are below the threshold
 *
 * @param above Determines if the button should be active above or below the set threshold
 *
 * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
 */
int AnalogButton::setActiveAboveThreshold(bool above) {
    this->activeAboveThreshold = above;

    return DEVICE_OK;
}

/**
 * Determines whether or not the button is active above or below the set threshold
 *
 * @return true if the button is active above the threshold and false if it is active below
 */
bool AnalogButton::isActiveAboveThreshold() {
    return activeAboveThreshold;
}

/**
 * Set the threshold used to determine if the button is active or not.
 *
 * @param value The threshold at which the button is active
 *
 * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
 */
int AnalogButton::setThreshold(uint16_t value) {
    this->threshold = value;

    return DEVICE_OK;
}

/**
 * Gets the current threshold at which the button is considered active.
 *
 * @return the AnalogSensor reading above or below which the button is considered active
 */
int AnalogButton::getThreshold() {
    return threshold;
}

/**
 * Destructor for AnalogButton
 */
AnalogButton::~AnalogButton()
{
}