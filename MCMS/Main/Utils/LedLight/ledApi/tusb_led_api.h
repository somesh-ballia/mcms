#ifdef __cplusplus
extern "C" {
#endif

#ifndef TUSB_LED_API_H
#define TUSB_LED_API_H

#include <usb.h>

#define TI_VENDOR_ID	0x0451
#define PRODUCT_ID		0x5f00
#define CONFIGURATION_VAL	0x1
#define INTERFACE_NUM		0x0
#define BUFF_LEN	64

#define COLOR_DOWN	(0x0)	// 0b'0000
#define COLOR_RED	(0x1)	// 0b'0001
#define COLOR_BLUE	(0x2)	// 0b'0010
#define COLOR_GREEN	(0x4)	// 0b'0100
#define HDD_MODE	(0xC)	// 0b'1100

typedef enum
{
	eSTATUS = 0,
	eMS = 1,
}eLED;

void *open_led(void);
void close_led(void);
int set_led(int port, char color);
int get_firmware_version(int *buf);

#endif

#ifdef __cplusplus
}
#endif
