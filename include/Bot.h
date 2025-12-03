#ifndef BOT_H
#define BOT_H

#include "BotAlgorithm.h"
#include "Player.h"
#include "PokerGame.h"

class Bot : public Player {
public:
  Bot(const std::string &name, std::unique_ptr<BotAlgorithm> algo,
      PokerGame &game);

  void takeTurn() override;
  bool makeDecision() override;
  int placeBet(int currentBet) override;

private:
  std::unique_ptr<BotAlgorithm> algorithm;
  PokerGame &game;

  int willingBet;
};

#endif
