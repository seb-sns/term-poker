#include "User.h"
#include "GameIO.h"
#include "PokerGame.h"

User::User(const std::string &name) : Player(name) {}

void User::takeTurn() {}

bool User::makeDecision() { return false; }

int User::placeBet(int currentBet) {
  BetRequest request;
  request.currentBet = currentBet;
  request.minBet = currentBet - roundBet;
  request.chips = chipCount;
  request.roundBet = roundBet;
  request.bigBlind = PokerGame::getInstance()->getBigBlind();

  int bet = gameIO().promptBet(request);
  if (bet == chipCount) {
    allIn = true;
  }
  return bet;
}
