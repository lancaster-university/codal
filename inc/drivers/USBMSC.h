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

    int classRequest(USBSetup& setup);

    int endpointRequest(uint8_t endpoint, uint8_t endpointOffset);

    USBEndpoint* getEndpoints();

    uint8_t getEndpointCount();

    uint8_t* getDescriptor();

    uint8_t getDescriptorSize();
};
