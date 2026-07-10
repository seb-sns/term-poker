#include "Pot.h"
#include "GameIO.h"
#include "Player.h"

#include <algorithm>
#include <string>

void PotManager::createMainPot(std::set<Player *> eligiblePlayers) {
  pots.emplace_back(Pot(0, eligiblePlayers, true));
}

void PotManager::buildShowdownPots(const std::vector<Player *> &handPlayers) {
  int totalContributed = 0;
  for (const Player *p : handPlayers) {
    totalContributed += p->getTotalBet();
  }

  // Every distinct amount a live player has put in caps one pot layer;
  // players who contributed at least that much are eligible for the layer.
  std::set<int> levels;
  for (const Player *p : handPlayers) {
    if (!p->hasFolded() && p->getTotalBet() > 0) {
      levels.insert(p->getTotalBet());
    }
  }

  pots.clear();
  int previousLevel = 0;
  for (int level : levels) {
    Pot pot(0, {}, pots.empty());
    for (Player *p : handPlayers) {
      pot.amount +=
          std::max(0, std::min(p->getTotalBet(), level) - previousLevel);
      if (!p->hasFolded() && p->getTotalBet() >= level) {
        pot.eligiblePlayers.insert(p);
      }
    }
    pots.push_back(std::move(pot));
    previousLevel = level;
  }

  // Defensive: a folded player should never have contributed more than the
  // highest live stake, but if it happens the excess stays in the last pot
  // so no chips leak.
  int distributed = 0;
  for (const Pot &pot : pots) {
    distributed += pot.amount;
  }
  if (!pots.empty()) {
    pots.back().amount += totalContributed - distributed;
  }
}

void PotManager::payOutPots() {
  for (Pot &pot : pots) {
    if (pot.eligiblePlayers.empty() || pot.amount <= 0) {
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
    pot.amount = 0;
  }
}

void PotManager::foldPlayer(Player *player) {
  for (Pot &pot : pots) {
    pot.eligiblePlayers.erase(player);
  }
}

void PotManager::clearPots() { pots.clear(); }

void PotManager::addToMainPot(int amount) { pots[0].amount += amount; }
