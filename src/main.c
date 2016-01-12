#include <pebble.h>
#include "main.h"

int main(void) {
  init();
  app_event_loop();
  deinit();
}
