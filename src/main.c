#include <pebble.h>
#include <bash.h>

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}