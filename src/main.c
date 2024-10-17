#define PLAYDATE_SETUP
#include <playdate/api.h>

extern PlaydateAPI* playdate;

// Runs once on launch
int playdate_init(void) {
  playdate->display->setRefreshRate(30);
  LCDBitmap* b = playdate->graphics->newBitmap(400, 240, kColorBlack);
  return 0;
}

// Runs every frame
int playdate_update(void) {
  return 0;
}
