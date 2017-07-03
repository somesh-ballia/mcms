#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <usb.h>

#include "tusb_led_api.h"

static usb_dev_handle *tusb = NULL;
static struct usb_device *tusb_dev = NULL;

static int init_dev(void)
{
	struct usb_bus *bus;
	struct usb_device *dev;

	usb_init();
	if(usb_find_busses() < 0)
		perror("Bus scan");
	if(usb_find_devices() < 0)
		perror("Device scan");

	/* Enumeration TUSB2316/3210 on bus */
	for (bus = usb_busses; bus; bus = bus->next) 
	{
		for (dev = bus->devices; dev; dev = dev->next) 
		{
			if(dev->descriptor.idVendor==TI_VENDOR_ID &&       /* TI */
					dev->descriptor.idProduct==PRODUCT_ID) 
			{      
				/* TUSB2316/3210 */
				if(tusb_dev!=NULL && tusb_dev!=dev) 
				{
					/* More than one, but there's no way of knowing which one to
					     program, so give up */
					fprintf(stderr,"Multiple TUSB2316/3210s found, previously dev[%p] = [%x:%x], now dev[%p] = [%x:%x].\n", tusb_dev, tusb_dev->descriptor.idVendor, tusb_dev->descriptor.idProduct, dev, dev->descriptor.idVendor, dev->descriptor.idProduct);
					errno = -ERANGE;
					return -ERANGE;
				}
				tusb_dev=dev;		// Found devices
			}
		}
	}
	if(tusb_dev == NULL) 
	{
		fprintf(stderr,"TUSB2316/3210 not found.\n");
		errno = -ENODEV;
		return -ENODEV;
	}

	printf("Found TUSB2316/3210!. dev[%p] = [%x:%x]\n", tusb_dev, tusb_dev->descriptor.idVendor, tusb_dev->descriptor.idProduct); 

	return 0;
}

void *open_led(void)
{
	if(NULL == tusb_dev || NULL == tusb)	// If uninitial
	{
		if(0 == init_dev())		// initialize tusb_dev
		{
			tusb = usb_open(tusb_dev);	// initialize tusb
			if(tusb && (0 <= usb_set_configuration(tusb, CONFIGURATION_VAL)) )
			{
				if(0 == usb_claim_interface(tusb, INTERFACE_NUM))
				{
					return tusb;		// Init Successful
				}
				else
				{
					perror("Can't claim interface");
				}
			}
			else
			{
				perror("Can't set configuration");
			}

			close_led();
			return NULL;
		}
	}

	return tusb;
}

void close_led(void) {
  if(tusb!=NULL)
    usb_close(tusb);
  tusb = NULL;
}

/*** Don't call this func directly. Shoule be called form API of this lib ***/
static int send_cmd(char* data)
{
	/* Send data */
	int ret = 0;

	ret = usb_interrupt_write(tusb, 1/* ep always 1 */, data, BUFF_LEN, 500);
#ifdef DEBUG
	printf("Write %d bytes [%x, %x, %x] success!\n", ret, data[0], data[1], data[2]);
#endif
	ret = usb_interrupt_read(tusb, 1/* ep always 1 */, data, BUFF_LEN, 500);
#ifdef DEBUG
	printf("respond %d bytes [%x, %x, %x] success!\n", 3, data[0], data[1], data[2]);
#endif

	// Return successful
	return ret;
}

int set_led(int port, char color)
{
	char data[BUFF_LEN] = {0x0,0x0,0x0,0x0};

	if(NULL == tusb)	// If tusb uninitial
	{
		tusb = (usb_dev_handle *)open_led();	// initialize tusb
		if(NULL == tusb)	// Check again
			return errno;
	}

	// Set_configuration & Claim_interface Successful
	memset(data, 0, BUFF_LEN);
	port  &= 0x3;	// Just only support port[0~2]
	color &= 0xF;	// Just only support color 0x0~0xf;

	data[0] = port==0  ? 0x16: port==1 ? 0x1e : 0x1f;
	data[1] = 0;	// All GPIO output
	data[2] = color;

	if(0 > send_cmd(data))
		return errno;
	else
		return 0;
}

int get_firmware_version(int *buf)
{
	char data[BUFF_LEN] = {0x0,0x0,0x0,0x0};

	if(NULL == buf){
		errno = -EINVAL;
		perror(__FUNCTION__);
		return errno;
	}

	if(NULL == tusb)	// If tusb uninitial
	{
		tusb = (usb_dev_handle*)open_led();	// initialize tusb
		if(NULL == tusb)	// Check again
			return errno;
	}

	// get firmware version
	memset(data, 0, BUFF_LEN);		// all 0 means get version

	if(0 < send_cmd(data)){
		// Return version
		*buf = *(int*)data;
		return 0;
	}

	return errno;
}

#ifdef __cplusplus
}
#endif
