#ifndef USER_H
#define USER_H

#include "Player.h"

class User : public Player {
public:
  User(const std::string &name);

  void takeTurn() override;
  bool makeDecision() override;
  int placeBet(int currentBet) override;
};

#endif
