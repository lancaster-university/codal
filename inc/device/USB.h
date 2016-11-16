#ifndef DEVICE_USB_H
#define DEVICE_USB_H

#include <avr/io.h>

#define DEVICE_USB_ENDPOINTS    6

#define EP_SINGLE_64            0x32
#define EP_DOUBLE_64            0x36
#define EP_SINGLE_16            0x12

#define EP_TYPE_CONTROL                 (0x00)
#define EP_TYPE_BULK_IN                 ((1<<EPTYPE1) | (1<<EPDIR))
#define EP_TYPE_BULK_OUT                (1<<EPTYPE1)
#define EP_TYPE_INTERRUPT_IN            ((1<<EPTYPE1) | (1<<EPTYPE0) | (1<<EPDIR))
#define EP_TYPE_INTERRUPT_OUT           ((1<<EPTYPE1) | (1<<EPTYPE0))
#define EP_TYPE_ISOCHRONOUS_IN          ((1<<EPTYPE0) | (1<<EPDIR))
#define EP_TYPE_ISOCHRONOUS_OUT         (1<<EPTYPE0)

static inline void configure_usb()
{
    #if defined(UHWCON)
    	UHWCON |= (1<<UVREGE);			// power internal reg
    #endif
    	USBCON = (1<<USBE) | (1<<FRZCLK);	// clock frozen, usb enabled

    // ATmega32U4
    #if defined(PINDIV)
    #if F_CPU == 16000000UL
    	PLLCSR |= (1<<PINDIV);                   // Need 16 MHz xtal
    #elif F_CPU == 8000000UL
    	PLLCSR &= ~(1<<PINDIV);                  // Need  8 MHz xtal
    #else
    #error "Clock rate of F_CPU not supported"
    #endif

    #elif defined(__AVR_AT90USB82__) || defined(__AVR_AT90USB162__) || defined(__AVR_ATmega32U2__) || defined(__AVR_ATmega16U2__) || defined(__AVR_ATmega8U2__)
    	// for the u2 Series the datasheet is confusing. On page 40 its called PINDIV and on page 290 its called PLLP0
    #if F_CPU == 16000000UL
    	// Need 16 MHz xtal
    	PLLCSR |= (1 << PLLP0);
    #elif F_CPU == 8000000UL
    	// Need 8 MHz xtal
    	PLLCSR &= ~(1 << PLLP0);
    #endif

    // AT90USB646, AT90USB647, AT90USB1286, AT90USB1287
    #elif defined(PLLP2)
    #if F_CPU == 16000000UL
    #if defined(__AVR_AT90USB1286__) || defined(__AVR_AT90USB1287__)
    	// For Atmel AT90USB128x only. Do not use with Atmel AT90USB64x.
    	PLLCSR = (PLLCSR & ~(1<<PLLP1)) | ((1<<PLLP2) | (1<<PLLP0)); // Need 16 MHz xtal
    #elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB647__)
    	// For AT90USB64x only. Do not use with AT90USB128x.
    	PLLCSR = (PLLCSR & ~(1<<PLLP0)) | ((1<<PLLP2) | (1<<PLLP1)); // Need 16 MHz xtal
    #else
    #error "USB Chip not supported, please defined method of USB PLL initialization"
    #endif
    #elif F_CPU == 8000000UL
    	// for Atmel AT90USB128x and AT90USB64x
    	PLLCSR = (PLLCSR & ~(1<<PLLP2)) | ((1<<PLLP1) | (1<<PLLP0)); // Need 8 MHz xtal
    #else
    #error "Clock rate of F_CPU not supported"
    #endif
    #else
    #error "USB Chip not supported, please defined method of USB PLL initialization"
    #endif

    	PLLCSR |= (1<<PLLE);
    	while (!(PLLCSR & (1<<PLOCK)))		// wait for lock pll
    	{
    	}

    	// Some tests on specific versions of macosx (10.7.3), reported some
    	// strange behaviors when the board is reset using the serial
    	// port touch at 1200 bps. This delay fixes this behavior.
    	//delay(1);
    #if defined(OTGPADE)
    	USBCON = (USBCON & ~(1<<FRZCLK)) | (1<<OTGPADE);	// start USB clock, enable VBUS Pad
    #else
    	USBCON &= ~(1 << FRZCLK);	// start USB clock
    #endif

    #if defined(RSTCPU)
    #if defined(LSM)
    	UDCON &= ~((1<<RSTCPU) | (1<<LSM) | (1<<RMWKUP) | (1<<DETACH));	// enable attach resistor, set full speed mode
    #else // u2 Series
    	UDCON &= ~((1 << RSTCPU) | (1 << RMWKUP) | (1 << DETACH));	// enable attach resistor, set full speed mode
    #endif
    #else
    	// AT90USB64x and AT90USB128x don't have RSTCPU
    	UDCON &= ~((1<<LSM) | (1<<RMWKUP) | (1<<DETACH));	// enable attach resistor, set full speed mode
    #endif
}

static inline uint8_t read8()
{
    return UEDATX;
}

static inline void send8(uint8_t data)
{
    UEDATX = data;
}

static inline uint8_t recieved_setup()
{
    return UEINTX & (1 << RXSTPI);
}

static inline void wait_tx_int(void)
{
    while (!(UEINTX & (1<<TXINI)));
}

static inline void clear_tx_int(void)
{
    UEINTX = ~(1<<TXINI);
}

static inline void wait_rx_int(void)
{
    while (!(UEINTX & (1 << RXOUTI)));
}

static inline uint8_t wait_rx_tx_int()
{
    while (!(UEINTX & ((1<<TXINI)|(1<<RXOUTI))));
    return (UEINTX & (1<<RXOUTI)) == 0;
}

static inline void clear_rx_int(void)
{
    UEINTX = ~(1 << RXOUTI);
}

static inline void clear_setup()
{
    UEINTX = ~((1<<RXSTPI) | (1<<RXOUTI) | (1<<TXINI));
}

static inline void stall()
{
    UECONX = (1<<STALLRQ) | (1<<EPEN);
}

static inline void set_endpoint(uint8_t ep)
{
    UENUM = ep;
}

static inline uint8_t get_frame_number()
{
	return UDFNUML;
}

int init_endpoint(uint8_t index, uint8_t type, uint8_t size);

int reset_endpoint(uint8_t index);

#endif
