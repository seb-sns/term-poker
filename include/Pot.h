#ifndef POT_H
#define POT_H

#include "Player.h"
#include <set>
#include <string>
#include <vector>

class Pot {
public:
  int amount;
  bool isMain;
  std::string winningHand; // set at showdown, for payout messages
  std::set<Player *> eligiblePlayers;

  Pot(int amount, std::set<Player *> eligiblePlayers, bool isMain)
      : amount(amount), isMain(isMain), eligiblePlayers(eligiblePlayers) {}
};

class PotManager {
public:
  std::vector<Pot> pots;

  void createMainPot(std::set<Player *> eligiblePlayers);
  void addToMainPot(int amount);
  void clearPots();
  void foldPlayer(Player *player);

  // Rebuilds the pots from each player's total contribution this hand,
  // splitting a side pot at every distinct all-in level.
  void buildShowdownPots(const std::vector<Player *> &handPlayers);
  void payOutPots();

  PotManager() {}
};

#endif
