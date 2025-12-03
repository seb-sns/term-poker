#include "PokerGame.h"

int main() {
  PokerGame *game = PokerGame::getInstance();
  game->playGame();

  return 0;
}
