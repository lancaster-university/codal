#include "USBMSC.h"

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

static const USBEndpoint endpoints[USB_MSC_ENDPOINTS] = {
    {
        EP_TYPE_BULK_OUT,
        EP_DOUBLE_64
    },
    {
        EP_TYPE_BULK_IN,
        EP_DOUBLE_64
    }
};

USBMSC::USBMSC() : CodalUSBInterface()
{
}

struct MSCCBW
{
    uint32_t    signature;
    uint32_t    tag;
    uint32_t    dataTransferLength;
    uint8_t     flags;
    uint8_t     lun;
    uint8_t     length;
    uint8_t     cb[16];
};

struct MSCCSW
{
    uint32_t    signature;
    uint32_t    tag;
    uint32_t    dataResidue;
    uint8_t     status;
};

int USBMSC::classRequest(USBSetup& setup)
{
    switch(setup.bRequest)
    {
        case 0xFE:
            CodalUSB::usbInstance->send(0);
            CodalUSB::usbInstance->send(1);
            return DEVICE_OK;
        case 0xFF:
            //reset...
            return DEVICE_OK;
    }

    return DEVICE_NOT_SUPPORTED;
}

int USBMSC::endpointRequest(uint8_t endpoint, uint8_t endpointOffset)
{
    uint8_t type = endpoints[endpointOffset].type;

    MSCCBW cbw;

    memset(&cbw, 0, sizeof(cbw));

    if(type == EP_TYPE_BULK_OUT)
    {
        CodalUSB::usbInstance->read((uint8_t *)&cbw, sizeof(MSCCBW));

        if(cbw.signature == 0x43425355UL)
        {
            //DDRC |= 1 << 7;
            //PORTC |= 1 << 7;
        }

        endpoint++;
    }

    set_endpoint(endpoint);
    MSCCSW csw;

    csw.signature = 0x43425355UL;
    csw.tag = cbw.tag;

    csw.dataResidue = 0;
    csw.status = 0;

    return DEVICE_OK;
}

USBEndpoint* USBMSC::getEndpoints()
{
    return (USBEndpoint*)&endpoints[0];
}

uint8_t USBMSC::getEndpointCount()
{
    return USB_MSC_ENDPOINTS;
}

uint8_t* USBMSC::getDescriptor()
{
    return (uint8_t*)&msc_descriptor;
}

uint8_t USBMSC::getDescriptorSize()
{
    return sizeof(MSCDescriptor);
}
