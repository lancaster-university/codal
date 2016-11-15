#include "CodalUSB.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

static volatile uint8_t usb_configuration = 0;
static volatile uint8_t usb_status = 0; // meaning of bits see usb_20.pdf, Figure 9-4. Information Returned by a GetStatus() Request to a Device
static volatile uint8_t usb_suspended = 0; // copy of UDINT to check SUSPI and WAKEUPI bits

static int usb_start;
static int usb_end;

#define EP_SINGLE_64 0x32
#define EP_DOUBLE_64 0x36
#define EP_SINGLE_16 0x12

static const DeviceDescriptor device_descriptor =
{
    18,
    1,
    0x0002,
    0,
    0,
    0,
    64,
    0x0B6A,
    0x5346,
    0x0001,
    1,
    2,
    3,
    1
};

static const ConfigDescriptor static_config = {
    9,
    2,
    0,
    0,
    1,
    0,
    USB_CONFIG_BUS_POWERED,
    250
};

static const MSCDescriptor msc_descriptor =
{
    {
        9,                  /// descriptor size in bytes
        4,                  /// descriptor type - interface
        0,                  /// interface number
        0,                  /// alternate setting number
        2,                  /// number of endpoints
        8,                  /// class code - mass storage
        6,                  /// subclass code - SCSI transparent command set
        80,                 /// protocol code - bulk only transport
        0,                  /// interface string index
    },
    {
        7,                  /// descriptor size in bytes
        5,                  /// descriptor type - endpoint
        0x01,               /// endpoint direction and number - out, 1
        2,                  /// transfer type - bulk
        64,                 /// maximum packet size
        0,                  /// maximum NAK rate
    },
    {
        7,                  /// descriptor size in bytes
        5,                  /// descriptor type - endpoint
        0x82,               /// endpoint direction and number - in, 2
        2,                  /// transfer type - bulk
        64,                 /// maximum packet size
        0,                  /// not used
    }
};

static const USBEndpoint endpoints[USB_EP_COUNT] =
{
   {
       EP_TYPE_CONTROL,
       EP_SINGLE_64
   },
   {
       EP_TYPE_BULK_OUT,
       EP_DOUBLE_64
   },
   {
       EP_TYPE_BULK_IN,
       EP_DOUBLE_64
   }
};

static const StringDescriptor descriptors[STRING_DESCRIPTOR_COUNT] =
{
    /*! index 0 - language */
    {
        4,              // length
        3,              // descriptor type - string
        {0x09,0x04,     0,0}      // languageID - United States
    },
    /*! index 1 - product ID */
    {
        18,             // length
        3,              // descriptor type - string
        {'A',0,'t',0,'m',0,'e',0,'g',0,'a',0,'3',0,'2',0,    0,0}
    },
    /*! index 2 - manufacturer ID */
    {
        12,             // length
        3,              // descriptor type - string
        {'J',0,'A',0,'M',0,'E',0,'S',0,     0,0}
    },
    /*! index 3 - serial number */
    {   26,             // length
        3,              // descriptor type - string
        {'1',0,'2',0,'3',0,'4',0,'5',0,'6',0,'7',0,'8',0,'9',0,'A',0,'B',0,'C',0,    0,0}
    }
};

static void read(volatile uint8_t* data, uint8_t count)
{
    while (count--)
        *data++ = UEDATX;
}

static uint8_t read8()
{
    return UEDATX;
}

static void send(volatile uint8_t* data, uint8_t count)
{
    while (count--)
        UEDATX = *data++;
}
static void send8(uint8_t data)
{
    UEDATX = data;
}

static inline void set_endpoint(uint8_t ep)
{
    UENUM = ep;
}

static inline uint8_t rec_setup()
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

static void init_base_endpoint()
{
    UENUM = 0;
    UECONX |= _BV(EPEN);
    UECFG0X = EP_TYPE_CONTROL;//endpoints[0].type;
    UECFG1X = EP_SINGLE_64;//endpoints[0].size;
}

static void endpoints_init()
{
    for (uint8_t i = 1; i < USB_EP_COUNT; i++)
    {
        UENUM = i;
        UECONX = _BV(EPEN);
        UECFG0X = endpoints[i].type;
        UECFG1X = endpoints[i].size;
    }

    UERST = 0x7E;    // And reset them
    UERST = 0;
}

static bool class_request(USBSetup *setup)
{
    /*
        here we should pass to the USB interface
        based on setup.wIndex which indicates the interface index.
    */

    /* determine request type */
    /*switch (setup->bRequest)
    {
        case 0xFE:
            //clear_tx_int();
            break;

        default:
            stall();
            break;
    }*/
    return true;
    //return false;
}

void set_markers(int end)
{
    set_endpoint(0);
    usb_start = 0;
    usb_end = end;
}

static bool __usb_send(uint8_t d)
{
    if (usb_start < usb_end)
    {
        if (!wait_rx_tx_int())
            return false;

        send8(d);

        if (!((usb_start + 1) & 0x3F))
            clear_tx_int();    // Fifo is full, release this packet
    }

    usb_start++;
    return true;
}

//    Clipped by usb_start/usb_end
int usb_send(const uint8_t* d, int len)
{
    uint8_t* end = (uint8_t*)d + len;

    while (d != end)
        if (!__usb_send(*d++))
            return -1;

    return len;
}

//    Construct a dynamic configuration descriptor
//    This really needs dynamic endpoint allocation etc
//    TODO
static bool send_config(int maxlen)
{
    ConfigDescriptor config;
    memcpy(&config, &static_config, sizeof(ConfigDescriptor));

    // Calculate our length including the config descriptor...
    // HAS TO BE CORRECT.
    config.clen = sizeof(MSCDescriptor) + sizeof(ConfigDescriptor);
    config.numInterfaces = 1;

    set_markers(maxlen);
    usb_send((uint8_t *)&config, sizeof(ConfigDescriptor));
    usb_send((uint8_t *)&msc_descriptor, sizeof(MSCDescriptor));

    return true;
}

static bool send_desc(USBSetup& setup)
{
    int ret;
    uint8_t type = setup.wValueH;

    if (type == USB_CONFIGURATION_DESCRIPTOR_TYPE)
        return send_config(setup.wLength);

    set_markers(setup.wLength);

    const uint8_t* desc_addr = 0;

    if (type == USB_DEVICE_DESCRIPTOR_TYPE)
        return usb_send((uint8_t *)&device_descriptor, sizeof(DeviceDescriptor));

    else if (type == USB_STRING_DESCRIPTOR_TYPE)
    {
        // check if we exceed our bounds.
        if(setup.wValueL > STRING_DESCRIPTOR_COUNT - 1)
            return false;

        // send the string descriptor the host asked for.
        int size =  sizeof(StringDescriptor) - (sizeof(descriptors[setup.wValueL].data) - descriptors[setup.wValueL].len);
        return usb_send((uint8_t *)&descriptors[setup.wValueL], sizeof(StringDescriptor));
    }

    return true;
}


ISR(USB_COM_vect)
{
    set_endpoint(0);

    if (!rec_setup())
        return;

    USBSetup setup;
    read((uint8_t*)&setup,8);
    clear_setup();


    // wait for previous transfer to complete
    if (setup.bmRequestType & DIRECTION)
        while (!(UEINTX & (1<<TXINI)));
    else
    // wait for reception to complete
        UEINTX = ~(1<<TXINI);

    bool ok = true;

    // Standard Requests
    uint8_t r = setup.bRequest;
    uint16_t wValue = (setup.wValueH << 8) | setup.wValueL;

    uint8_t request_type = setup.bmRequestType;

    if ((request_type & TYPE) == TYPE_STANDARD)
    {
        switch(setup.bRequest)
        {
            case GET_STATUS:
                if (request_type == (REQUEST_DEVICETOHOST | REQUEST_STANDARD | REQUEST_DEVICE))
                {
                    send8(usb_status);
                    send8(0);
                }
                else
                {
                    send8(0);
                    send8(0);
                }
                break;

            case CLEAR_FEATURE:
                if((request_type == (REQUEST_HOSTTODEVICE | REQUEST_STANDARD | REQUEST_DEVICE)) && (wValue == DEVICE_REMOTE_WAKEUP))
                    usb_status &= ~FEATURE_REMOTE_WAKEUP_ENABLED;

                break;
            case SET_FEATURE:
                if((request_type == (REQUEST_HOSTTODEVICE | REQUEST_STANDARD | REQUEST_DEVICE)) && (wValue == DEVICE_REMOTE_WAKEUP))
                    usb_status |= FEATURE_REMOTE_WAKEUP_ENABLED;

                break;
            case SET_ADDRESS:
                wait_tx_int();
                UDADDR = setup.wValueL | (1<<ADDEN);
                break;
            case GET_DESCRIPTOR:
                ok = send_desc(setup);
                break;
            case SET_DESCRIPTOR:
                stall();
                break;
            case GET_CONFIGURATION:
                send8(1);
                break;

            case SET_CONFIGURATION:
                if (REQUEST_DEVICE == (request_type & REQUEST_RECIPIENT))
                {
                    endpoints_init();
                    usb_configuration = setup.wValueL;
                }
                else
                    ok = false;
                break;
        }
    }
    else
    {
        set_markers(setup.wLength);        //    Max length of transfer
        ok = class_request(&setup);
    }

    if (ok)
        clear_tx_int();
    else
        stall();
}

ISR(USB_GEN_vect)
{
    if(UDINT & _BV(EORSTI))
    {
        init_base_endpoint();

        usb_configuration = 0;

        UEIENX |= _BV(RXSTPE);
    }

    UDINT &= ~_BV(EORSTI);
}

void CodalUSB::configure()
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

CodalUSB::CodalUSB()
{
    configure();

    UDINT &= ~((1<<WAKEUPI) | (1<<SUSPI)); // clear already pending WAKEUP / SUSPEND requests
    UDIEN = (1<<EORSTE) | (1<<SOFE) | (1<<SUSPE);    // Enable interrupts for EOR (End of Reset), SOF (start of frame) and SUSPEND
}
