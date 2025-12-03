#include "Player.h"
#include <set>
#include <vector>

class Pot {
public:
  int amount;
  int roundBeginAmount;
  int minBet;
  bool isMain;
  std::set<Player *> eligiblePlayers;

  void setRoundBeginAmount(int amount);
  void setEligibilePlayers(std::set<Player *> players);
  Pot(int amount, std::set<Player *> eligiblePlayers, bool isMain)
      : amount(amount), eligiblePlayers(eligiblePlayers), isMain(isMain) {}
};

class PotManager {
public:
  std::vector<Pot> pots;

  void createMainPot(std::set<Player *> eligiblePlayers);
  void createSidePot(Player *player, Pot &pot, const int playersActed);
  void addToMainPot(int amount);
  void addToSidePot(int playerBet, int index);
  void clearPots();
  void foldPlayer(Player *player);
  void adjustWinners(std::set<Player *> losingPlayers);
  void payOutPots();

  int determineNewSidePot(const Player *player);
  int findNewPotSplitLocation(const Player *player);

  PotManager(){};
};
