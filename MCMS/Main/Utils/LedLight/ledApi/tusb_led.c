#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <usb.h>

#define TI_VENDOR_ID	0x0451
#define PRODUCT_ID		0x5f00
#define CONFIGURATION_VAL	0x1
#define INTERFACE_NUM		0x0
#define BUFF_LEN	64

usb_dev_handle *tusb = NULL;

void close_tusb(void) {
  if(tusb!=NULL)
    usb_close(tusb);
}

int main(int argc, char** argv)
{
	struct usb_bus *bus;
	struct usb_device *dev;
	struct usb_device *tusb_dev = NULL;
	int ret = 0;
	char data[BUFF_LEN] = {0x0,0x0,0x0,0x0};
	unsigned char val = 0;

	usb_init();
	if(usb_find_busses())
		perror("Bus scan");
	if(usb_find_devices())
		perror("Device scan");

	/* Enumeration TUSB2316/3210 on bus */
	for (bus = usb_busses; bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if(dev->descriptor.idVendor==TI_VENDOR_ID &&       /* TI */
					dev->descriptor.idProduct==PRODUCT_ID) {      /* TUSB2316/3210 */
						if(tusb_dev!=NULL) {
							/* More than one, but there's no way of knowing which one to
							     program, so give up */
							fprintf(stderr,"Multiple TUSB2316/3210s found, previously dev[%p] = [%x:%x], now dev[%p] = [%x:%x].\n", tusb_dev, tusb_dev->descriptor.idVendor, tusb_dev->descriptor.idProduct, dev, dev->descriptor.idVendor, dev->descriptor.idProduct);
							return 1;
						}
						tusb_dev=dev;
				  }
		}
	}
	if(tusb_dev==NULL) {
		fprintf(stderr,"TUSB2316/3210 not found.\n");
		return 1;
	}
	printf("Found TUSB2316/3210!. dev[%p] = [%x:%x]\n", tusb_dev, tusb_dev->descriptor.idVendor, tusb_dev->descriptor.idProduct); 

	/* Open USB dev and set configuation & interface */
	if((tusb=usb_open(tusb_dev))==NULL) {
		fprintf(stderr,"Error opening USB device.\n");
		return 1;
	}
	atexit(close_tusb);

	if(0 > usb_set_configuration(tusb, CONFIGURATION_VAL)) {
		perror("Can't set configuration");
		fprintf(stderr,"(make sure you have write access to the device)\n");
		return 1;
	}
	if(usb_claim_interface(tusb, INTERFACE_NUM))
		perror("Can't claim interface");

	if(argc !=3){
		printf("Invalid id\n");
		return 1;
	}

	// get firmware version
	memset(data, 0, BUFF_LEN);
	ret = usb_interrupt_write(tusb, 1, data, BUFF_LEN, 500);
	printf("Write %d bytes [%x, %x, %x, %x] success!\n", ret, data[0], data[1], data[2], data[3]);
	ret = usb_interrupt_read(tusb, 1, data, BUFF_LEN, 500);
	printf("respond %d bytes [%x, %x, %x, %x] success!\n", 4, data[0], data[1], data[2], data[3]);

	val = strtol(argv[1], NULL, 0)%3;
	data[0] = val==0  ? 0x16: val==1 ? 0x1e : 0x1f;
	data[1] = 0;	// All GPIO output
	data[2] = strtol(argv[2], NULL, 0);

	/* Send data */
	ret = usb_interrupt_write(tusb, 1/* ep always 1 */, data, BUFF_LEN, 500);
	printf("Write %d bytes [%x, %x, %x] success!\n", ret, data[0], data[1], data[2]);
	ret = usb_interrupt_read(tusb, 1/* ep always 1 */, data, BUFF_LEN, 500);
	printf("respond %d bytes [%x, %x, %x] success!\n", 3, data[0], data[1], data[2]);

	return 0;
}
