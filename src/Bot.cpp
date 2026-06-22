#include "Bot.h"
#include "BotAlgorithm.h"

#include "PokerGame.h"

#include <iostream>

Bot::Bot(const std::string &name, std::unique_ptr<BotAlgorithm> algo,
         PokerGame &game)
    : Player(name), algorithm(std::move(algo)), game(game) {}

bool Bot::makeDecision() { return (algorithm->makeDecision(game, *this)); }

int Bot::placeBet(int currentBet) {
  willingBet = algorithm->calculateBet(game, *this);

  if (willingBet >= chipCount) {
    allIn = true;
    return chipCount;
  }

  int minBet = currentBet - roundBet;

  if (willingBet >= minBet && willingBet > 0) {
    return willingBet;
  }
  return 0;
}

void Bot::takeTurn() {}
