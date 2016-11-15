#include "Pin.h"

PinMapping* Pin::getMap(uint8_t name)
{
    for(uint8_t i = 0; i < DEVICE_PIN_COUNT; i++)
        if(name == deviceMap[i].pin)
            return (PinMapping*)&deviceMap[i];

    return NULL;
}

/**
  * Disconnect any attached mBed IO from this pin.
  *
  * Used only when pin changes mode (i.e. Input/Output/Analog/Digital)
  */
void Pin::disconnect()
{
    if(status & (PIN_STATUS_DIGITAL_OUT | PIN_STATUS_DIGITAL_IN))
    {
        *map->configRegister &= ~_BV(map->bit);
        *map->portOut &= ~_BV(map->bit);
    }

    if(status & PIN_STATUS_ANALOG_IN)
        ADCSRA &= ~_BV(ADEN);

    status = 0;
}

/**
  * Performs a check to ensure that the current Pin is in control of a
  * DynamicPwm instance, and if it's not, allocates a new DynamicPwm instance.
  */
int Pin::obtainAnalogChannel()
{

}

/**
  * Interrupt handler for when an rise interrupt is triggered.
  */
void Pin::onRise()
{

}

/**
  * Interrupt handler for when an fall interrupt is triggered.
  */
void Pin::onFall()
{

}

/**
  * This member function manages the calculation of the timestamp of a pulse detected
  * on a pin whilst in IO_STATUS_EVENT_PULSE_ON_EDGE or IO_STATUS_EVENT_ON_EDGE modes.
  *
  * @param eventValue the event value to distribute onto the message bus.
  */
void Pin::pulseWidthEvent(int eventValue)
{

}

/**
  * This member function will construct an TimedInterruptIn instance, and configure
  * interrupts for rise and fall.
  *
  * @param eventType the specific mode used in interrupt context to determine how an
  *                  edge/rise is processed.
  *
  * @return DEVICE_OK on success
  */
int Pin::enableRiseFallEvents(int eventType)
{

}

/**
  * If this pin is in a mode where the pin is generating events, it will destruct
  * the current instance attached to this Pin instance.
  *
  * @return DEVICE_OK on success.
  */
int Pin::disableEvents()
{

}

/**
  * Constructor.
  * Create a Pin instance, generally used to represent a pin on the edge connector.
  *
  * @param id the unique EventModel id of this component.
  *
  * @param name the mbed uint8_t for this Pin instance.
  *
  * @param capability the capabilities this Pin instance should have.
  *                   (PIN_CAPABILITY_DIGITAL, PIN_CAPABILITY_ANALOG, PIN_CAPABILITY_AD, PIN_CAPABILITY_ALL)
  *
  * @code
  * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_ALL);
  * @endcode
  */
Pin::Pin(int id, uint8_t name, SystemClock* timer)
: timer(timer)
{
    this->id = id;
    this->map = getMap(name);
    this->status = 0;

    if(this->map == NULL)
        this->status |= PIN_STATUS_INVALID;
}

int Pin::getName()
{
    if(status & PIN_STATUS_INVALID)
        return DEVICE_NOT_SUPPORTED;

    return this->map->pin;
}

/**
  * Configures this IO pin as a digital output (if necessary) and sets the pin to 'value'.
  *
  * @param value 0 (LO) or 1 (HI)
  *
  * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if value is out of range, or DEVICE_NOT_SUPPORTED
  *         if the given pin does not have digital capability.
  *
  * @code
  * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
  * P0.setDigitalValue(1); // P0 is now HI
  * @endcode
  */
int Pin::setDigitalValue(int value)
{
    if(status & PIN_STATUS_INVALID || !(map->capability & PIN_CAPABILITY_DIGITAL))
        return DEVICE_NOT_SUPPORTED;

    uint8_t bit = _BV(map->bit);

    if(!(status & PIN_STATUS_DIGITAL_OUT))
    {
        disconnect();

        *map->configRegister |= bit;
        status |= PIN_STATUS_DIGITAL_OUT;
    }

    if(value)
        *map->portOut |= bit;
    else
        *map->portOut &= ~bit;

    return DEVICE_OK;
}

/**
  * Configures this IO pin as a digital input (if necessary) and tests its current value.
  *
  *
  * @return 1 if this input is high, 0 if input is LO, or DEVICE_NOT_SUPPORTED
  *         if the given pin does not have digital capability.
  *
  * @code
  * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
  * P0.getDigitalValue(); // P0 is either 0 or 1;
  * @endcode
  */
int Pin::getDigitalValue(PinMode pull)
{
    if(status & PIN_STATUS_INVALID || !(map->capability & PIN_CAPABILITY_DIGITAL))
        return DEVICE_NOT_SUPPORTED;

    uint8_t bit = _BV(map->bit);

    if(!(status & PIN_STATUS_DIGITAL_IN))
    {
        disconnect();

        *map->configRegister &= bit;

        if(pull == PullNone)
            *map->portOut &= bit;
        else
            *map->portOut |= bit;

        status |= PIN_STATUS_DIGITAL_IN;
    }

    return *map->portIn & bit;
}

/**
  * Configures this IO pin as an analog/pwm output, and change the output value to the given level.
  *
  * @param value the level to set on the output pin, in the range 0 - 1024
  *
  * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if value is out of range, or DEVICE_NOT_SUPPORTED
  *         if the given pin does not have analog capability.
  */
int Pin::setAnalogValue(int value)
{

}

/**
  * Configures this IO pin as an analog/pwm output (if necessary) and configures the period to be 20ms,
  * with a duty cycle between 500 us and 2500 us.
  *
  * A value of 180 sets the duty cycle to be 2500us, and a value of 0 sets the duty cycle to be 500us by default.
  *
  * This range can be modified to fine tune, and also tolerate different servos.
  *
  * @param value the level to set on the output pin, in the range 0 - 180.
  *
  * @param range which gives the span of possible values the i.e. the lower and upper bounds (center +/- range/2). Defaults to DEVICE_PIN_DEFAULT_SERVO_RANGE.
  *
  * @param center the center point from which to calculate the lower and upper bounds. Defaults to DEVICE_PIN_DEFAULT_SERVO_CENTER
  *
  * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if value is out of range, or DEVICE_NOT_SUPPORTED
  *         if the given pin does not have analog capability.
  */
int Pin::setServoValue(int value, int range, int center)
{
    if(status & PIN_STATUS_INVALID || !(map->capability & PIN_CAPABILITY_ANALOG) || timer == NULL)
        return DEVICE_NOT_SUPPORTED;


}

/**
  * Configures this IO pin as an analogue input (if necessary), and samples the Pin for its analog value.
  *
  * @return the current analogue level on the pin, in the range 0 - 1024, or
  *         DEVICE_NOT_SUPPORTED if the given pin does not have analog capability.
  *
  * @code
  * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
  * P0.getAnalogValue(); // P0 is a value in the range of 0 - 1024
  * @endcode
  */
int Pin::getAnalogValue(AnalogRef ref)
{
    if(status & PIN_STATUS_INVALID || !(map->capability & PIN_CAPABILITY_ANALOG))
        return DEVICE_NOT_SUPPORTED;

    uint8_t low, high;

    if(!(status & PIN_STATUS_ANALOG_IN))
    {
        disconnect();

        ADCSRA |= _BV(ADEN);

        status |= PIN_STATUS_DIGITAL_IN;
    }

    // the adc has 8 channels, on the uno, 6 are available for GPIO.
    // reuse the bit field on our device map to determine the channel.
    ADMUX = (ref << 6) | (map->bit & 0x0F);

    ADCSRA |= _BV(ADSC);

    while(ADCSRA & _BV(ADSC));

    // must read the low 8 first otherwise the result is cleared.
    low = ADCL;
    high = ADCH;

    return (high << 8) | low;
}

/**
  * Determines if this IO pin is currently configured as an input.
  *
  * @return 1 if pin is an analog or digital input, 0 otherwise.
  */
int Pin::isInput()
{
    return status & (PIN_STATUS_DIGITAL_IN | PIN_STATUS_ANALOG_IN);
}

/**
  * Determines if this IO pin is currently configured as an output.
  *
  * @return 1 if pin is an analog or digital output, 0 otherwise.
  */
int Pin::isOutput()
{
    return status & (PIN_STATUS_DIGITAL_OUT | PIN_STATUS_ANALOG_OUT);
}

/**
  * Determines if this IO pin is currently configured for digital use.
  *
  * @return 1 if pin is digital, 0 otherwise.
  */
int Pin::isDigital()
{
    return status & (PIN_STATUS_DIGITAL_IN | PIN_STATUS_DIGITAL_OUT);
}

/**
  * Determines if this IO pin is currently configured for analog use.
  *
  * @return 1 if pin is analog, 0 otherwise.
  */
int Pin::isAnalog()
{
    return status & (PIN_STATUS_ANALOG_IN | PIN_STATUS_ANALOG_OUT);
}

/**
  * Configures this IO pin as a "makey makey" style touch sensor (if necessary)
  * and tests its current debounced state.
  *
  * Users can also subscribe to DeviceButton events generated from this pin.
  *
  * @return 1 if pin is touched, 0 if not, or DEVICE_NOT_SUPPORTED if this pin does not support touch capability.
  *
  * @code
  * DeviceMessageBus bus;
  *
  * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_ALL);
  * if(P0.isTouched())
  * {
  *     //do something!
  * }
  *
  * // subscribe to events generated by this pin!
  * bus.listen(DEVICE_ID_IO_P0, DEVICE_BUTTON_EVT_CLICK, someFunction);
  * @endcode
  */
int Pin::isTouched()
{

}

/**
  * Configures this IO pin as an analog/pwm output if it isn't already, configures the period to be 20ms,
  * and sets the pulse width, based on the value it is given.
  *
  * @param pulseWidth the desired pulse width in microseconds.
  *
  * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if value is out of range, or DEVICE_NOT_SUPPORTED
  *         if the given pin does not have analog capability.
  */
int Pin::setServoPulseUs(int pulseWidth)
{

}

/**
  * Configures the PWM period of the analog output to the given value.
  *
  * @param period The new period for the analog output in milliseconds.
  *
  * @return DEVICE_OK on success, or DEVICE_NOT_SUPPORTED if the
  *         given pin is not configured as an analog output.
  */
int Pin::setAnalogPeriod(int period)
{

}

/**
  * Configures the PWM period of the analog output to the given value.
  *
  * @param period The new period for the analog output in microseconds.
  *
  * @return DEVICE_OK on success, or DEVICE_NOT_SUPPORTED if the
  *         given pin is not configured as an analog output.
  */
int Pin::setAnalogPeriodUs(int period)
{

}

/**
  * Obtains the PWM period of the analog output in microseconds.
  *
  * @return the period on success, or DEVICE_NOT_SUPPORTED if the
  *         given pin is not configured as an analog output.
  */
int Pin::getAnalogPeriodUs()
{

}

/**
  * Obtains the PWM period of the analog output in milliseconds.
  *
  * @return the period on success, or DEVICE_NOT_SUPPORTED if the
  *         given pin is not configured as an analog output.
  */
int Pin::getAnalogPeriod()
{

}

/**
  * Configures the pull of this pin.
  *
  * @param pull one of the mbed pull configurations: PullUp, PullDown, PullNone
  *
  * @return DEVICE_NOT_SUPPORTED if the current pin configuration is anything other
  *         than a digital input, otherwise DEVICE_OK.
  */
int Pin::setPull(PinMode pull)
{

}

/**
  * Configures the events generated by this Pin instance.
  *
  * DEVICE_PIN_EVENT_ON_EDGE - Configures this pin to a digital input, and generates events whenever a rise/fall is detected on this pin. (DEVICE_PIN_EVT_RISE, DEVICE_PIN_EVT_FALL)
  * DEVICE_PIN_EVENT_ON_PULSE - Configures this pin to a digital input, and generates events where the timestamp is the duration that this pin was either HI or LO. (DEVICE_PIN_EVT_PULSE_HI, DEVICE_PIN_EVT_PULSE_LO)
  * DEVICE_PIN_EVENT_ON_TOUCH - Configures this pin as a makey makey style touch sensor, in the form of a DeviceButton. Normal button events will be generated using the ID of this pin.
  * DEVICE_PIN_EVENT_NONE - Disables events for this pin.
  *
  * @param eventType One of: DEVICE_PIN_EVENT_ON_EDGE, DEVICE_PIN_EVENT_ON_PULSE, DEVICE_PIN_EVENT_ON_TOUCH, DEVICE_PIN_EVENT_NONE
  *
  * @code
  * DeviceMessageBus bus;
  *
  * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
  * P0.eventOn(DEVICE_PIN_EVENT_ON_PULSE);
  *
  * void onPulse(DeviceEvent evt)
  * {
  *     int duration = evt.timestamp;
  * }
  *
  * bus.listen(DEVICE_ID_IO_P0, DEVICE_PIN_EVT_PULSE_HI, onPulse, MESSAGE_BUS_LISTENER_IMMEDIATE)
  * @endcode
  *
  * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER if the given eventype does not match
  *
  * @note In the DEVICE_PIN_EVENT_ON_PULSE mode, the smallest pulse that was reliably detected was 85us, around 5khz. If more precision is required,
  *       please use the InterruptIn class supplied by ARM mbed.
  */
int Pin::eventOn(int eventType)
{

}
