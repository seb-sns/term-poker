#include "Pot.h"
#include "Player.h"

#include <iostream>

void Pot::setRoundBeginAmount(int amount) { roundBeginAmount = amount; }

void Pot::setEligibilePlayers(std::set<Player *> players) {
  eligiblePlayers = players;
}

void PotManager::createMainPot(std::set<Player *> eligiblePlayers) {
  pots.emplace_back(Pot(0, eligiblePlayers, true));
}

void PotManager::createSidePot(Player *player, Pot &mainPot, int playersActed) {
  std::cerr << "Creating side pot..." << '\n';
  int roundBet = player->getRoundBet();
  int nEligiblePlayers = mainPot.eligiblePlayers.size();
  int sidePotAmount = mainPot.roundBeginAmount + (playersActed + 1) * roundBet;
  mainPot.amount -= sidePotAmount;
  pots.emplace_back(Pot(sidePotAmount, mainPot.eligiblePlayers, false));
  pots[pots.size() - 1].minBet = roundBet;
  mainPot.eligiblePlayers.erase(player);
  return;
}

void PotManager::addToSidePot(int playerBet, int index) {
  pots[index].amount += playerBet;
}

int PotManager::determineNewSidePot(const Player *player) {
  int i = 1;
  for (Pot pot : pots) {
    if (pot.isMain) {
      continue;
    }
    if (pot.minBet == player->getRoundBet()) {
      return i;
    }
    ++i;
  }
  return 0;
}

int PotManager::findNewPotSplitLocation(const Player *player) {
  int i = 1;
  for (Pot pot : pots) {
    if (pot.isMain) {
      continue;
    }
    if (pot.minBet < player->getRoundBet()) {
      return i - 1;
    }
    ++i;
  }
  return i - 1;
}

void PotManager::payOutPots() {
  for (Pot pot : pots) {
    int payout = pot.amount / (pot.eligiblePlayers.size());
    std::cout << "Payout of " << payout << " goes to ";
    for (Player *p : pot.eligiblePlayers) {
      std::cout << p->getName() << " ";
      p->addChips(payout);
    }
    std::cout << std::endl;
  }
}

void PotManager::adjustWinners(std::set<Player *> losingPlayers) {
  for (Player *p : losingPlayers) {
    for (Pot &pot : pots) {
      pot.eligiblePlayers.erase(p);
    }
  }
}

void PotManager::foldPlayer(Player *player) {
  for (Pot &pot : pots) {
    pot.eligiblePlayers.erase(player);
  }
}

void PotManager::clearPots() { pots.clear(); }

void PotManager::addToMainPot(int amount) { pots[0].amount += amount; }
