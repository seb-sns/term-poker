#ifndef PLAYER_H
#define PLAYER_H

#include "Card.h"
#include <string>
#include <vector>

class Player {
protected:
  std::string name;
  std::vector<Card> hand;
  int chipCount;
  int roundBet = 0;
  int totalBet = 0;
  bool folded;
  bool allIn;
  bool hasCards = false;

public:
  Player(const std::string &name);
  virtual ~Player() = default;

  // basic functionality
  void addCard(const Card &card);
  void addCards(const std::vector<Card> &cards);
  void clearHand();

  // actions
  virtual void takeTurn() = 0;
  virtual bool makeDecision() = 0;
  virtual int placeBet(int currentBet) = 0;

  // betting sates
  bool hasFolded() const { return folded; }
  void setFolded(bool fold) { folded = fold; }
  bool isAllIn() const { return allIn; }
  void setAllIn(bool isAllIn) { allIn = isAllIn; }

  // chips
  int getChipCount() const { return chipCount; }
  void setChipCount(int chips) { chipCount = chips; }
  void addChips(int chips) { chipCount += chips; }
  void resetRoundBet() { roundBet = 0; }
  void resetTotalBet() { totalBet = 0; }
  void adjustRoundBet(int chips) { roundBet += chips; }
  void adjustTotalBet(int chips) { totalBet += chips; }
  int getRoundBet() const { return roundBet; }
  int getTotalBet() const { return totalBet; }

  std::string getName() const { return name; }
  std::vector<Card> getCards() const { return hand; }
};

#endif
