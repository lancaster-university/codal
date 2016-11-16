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

USBMSC::USBMSC()
: CodalUSBInterface()
{

}

int USBMSC::setup(USBSetup& setup)
{

}

int USBMSC::handle()
{

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
