#include "Pot.h"
#include "GameIO.h"
#include "Player.h"

#include <string>

void Pot::setRoundBeginAmount(int amount) { roundBeginAmount = amount; }

void Pot::setEligibilePlayers(std::set<Player *> players) {
  eligiblePlayers = players;
}

void PotManager::createMainPot(std::set<Player *> eligiblePlayers) {
  pots.emplace_back(Pot(0, eligiblePlayers, true));
}

void PotManager::createSidePot(Player *player, Pot &mainPot, int playersActed) {
  int roundBet = player->getRoundBet();
  int sidePotAmount = mainPot.roundBeginAmount + (playersActed + 1) * roundBet;
  mainPot.amount -= sidePotAmount;
  pots.emplace_back(Pot(sidePotAmount, mainPot.eligiblePlayers, false));
  pots[pots.size() - 1].minBet = roundBet;
  mainPot.eligiblePlayers.erase(player);
}

void PotManager::addToSidePot(int playerBet, int index) {
  pots[index].amount += playerBet;
}

int PotManager::determineNewSidePot(const Player *player) {
  int i = 1;
  for (const Pot &pot : pots) {
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
  for (const Pot &pot : pots) {
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
  for (const Pot &pot : pots) {
    if (pot.eligiblePlayers.empty()) {
      continue;
    }
    int nWinners = static_cast<int>(pot.eligiblePlayers.size());
    int payout = pot.amount / nWinners;
    int remainder = pot.amount % nWinners;
    // Odd chip goes to the first eligible player by set ordering.
    bool firstWinner = true;
    for (Player *p : pot.eligiblePlayers) {
      int amount = payout + (firstWinner ? remainder : 0);
      firstWinner = false;
      p->addChips(amount);
      std::string message = p->getName() + " wins " + std::to_string(amount);
      if (!pot.winningHand.empty()) {
        message += " with " + pot.winningHand;
      }
      if (!pot.isMain) {
        message += " (side pot)";
      }
      gameIO().log(message, LogKind::Win);
    }
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
