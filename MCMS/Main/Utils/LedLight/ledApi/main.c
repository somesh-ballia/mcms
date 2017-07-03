#include <stdio.h>
#include <unistd.h>
#include <usb.h>

#include "tusb_led_api.h"

int main(int argc, char** argv)
{
	open_led();

	set_led(eSTATUS, COLOR_RED);
	sleep(3);
	set_led(eSTATUS, COLOR_BLUE);
	sleep(3);
	set_led(eSTATUS, COLOR_GREEN);
	sleep(3);
	set_led(eSTATUS, COLOR_DOWN);
	sleep(3);

	set_led(eMS, COLOR_RED);
	sleep(3);
	set_led(eMS, COLOR_BLUE);
	sleep(3);
	set_led(eMS, COLOR_GREEN);
	sleep(3);
	set_led(eMS, COLOR_DOWN);
	sleep(3);

	close_led();
	return 0;
}
