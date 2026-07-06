#include "GameIO.h"
#include "ConsoleIO.h"

static GameIO *currentIO = nullptr;

void setGameIO(GameIO *io) { currentIO = io; }

GameIO &gameIO() {
  static ConsoleIO fallback;
  return currentIO != nullptr ? *currentIO : fallback;
}
