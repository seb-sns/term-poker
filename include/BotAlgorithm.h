#ifndef BOTALGORITHM_H
#define BOTALGORITHM_H

#include "Deck.h"
#include "Player.h"
#include "PokerGame.h"

class BotAlgorithm {
public:
  virtual ~BotAlgorithm() = default;

  virtual bool makeDecision(const PokerGame &game, const Player &bot) = 0;
  virtual int calculateBet(const PokerGame &game, const Player &bot) = 0;
};

class MonteCarloHandStrength : public BotAlgorithm {
public:
  MonteCarloHandStrength(double pacificity);

  bool makeDecision(const PokerGame &game, const Player &bot) override;
  int calculateBet(const PokerGame &game, const Player &bot) override;
  void evaluateStrength(const PokerGame &game, const Player &bot);

  // utils
  Deck getRemainingCardDeck(const PokerGame &game, const Player &bot);

private:
  int nSimulations;
  double pacificity; // Recommend range between 0 and 2 with 0 playing every
                     // hand and and 2 only playing at 100% confidence

  double handStrengthAssessment;
};

#endif
