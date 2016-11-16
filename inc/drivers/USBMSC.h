#include "CodalUSB.h"

#define USB_MSC_ENDPOINTS       2

typedef struct
{
    InterfaceDescriptor msc;
    EndpointDescriptor  in;
    EndpointDescriptor  out;
} MSCDescriptor;

class USBMSC : public CodalUSBInterface
{
    public:
    USBMSC();

    int setup(USBSetup& setup);

    int handle();

    USBEndpoint* getEndpoints();

    uint8_t getEndpointCount();

    uint8_t* getDescriptor();

    uint8_t getDescriptorSize();
};
