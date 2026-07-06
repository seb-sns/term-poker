#ifndef BOTALGORITHM_H
#define BOTALGORITHM_H

#include "BotConfig.h"
#include "Deck.h"
#include "Player.h"
#include "PokerGame.h"

#include <random>

// Strategy shared by all bots: the personality profile decides when to fold,
// when to raise and how much, based on a hand strength estimate supplied by
// the subclass. Strength is normalized so that 1.0 means an average hand for
// the current table; profile.tightness is compared directly against it.
class BotAlgorithm {
public:
  explicit BotAlgorithm(BotProfile profile);
  virtual ~BotAlgorithm() = default;

  // Returns true if the bot wants to play the hand (call or raise).
  bool makeDecision(const PokerGame &game, const Player &bot);
  // Returns the chips to put in this action; at least the call amount.
  int calculateBet(const PokerGame &game, const Player &bot);

  const BotProfile &getProfile() const { return profile; }

protected:
  virtual double evaluateStrength(const PokerGame &game,
                                  const Player &bot) = 0;

private:
  double chance();

  BotProfile profile;
  double handStrength = 0.0;
  bool bluffing = false;
  std::mt19937 rng;
};

class MonteCarloHandStrength : public BotAlgorithm {
public:
  explicit MonteCarloHandStrength(BotProfile profile);

protected:
  double evaluateStrength(const PokerGame &game, const Player &bot) override;

private:
  Deck getRemainingCardDeck(const PokerGame &game, const Player &bot);

  int nSimulations;
};

class BasicHandStrength : public BotAlgorithm {
public:
  explicit BasicHandStrength(BotProfile profile);

protected:
  double evaluateStrength(const PokerGame &game, const Player &bot) override;
};

#endif
