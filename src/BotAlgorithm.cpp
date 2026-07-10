#include "BotAlgorithm.h"

#include "Card.h"
#include "Deck.h"
#include "HandEvaluation.h"
#include "HandEvaluator.h"
#include "HandUtils.h"
#include "Player.h"
#include "PokerGame.h"
#include "Pot.h"

#include <algorithm>
#include <cmath>

namespace {

// Early streets inflate strength a little so bots see speculative hands.
double roundFactor(PokerGame::Round round) {
  switch (round) {
  case PokerGame::Round::preflop:
    return 1.15;
  case PokerGame::Round::flop:
    return 1.08;
  case PokerGame::Round::turn:
    return 1.03;
  default:
    return 1.0;
  }
}

int totalPot(const PokerGame &game) {
  int total = 0;
  for (const Pot &pot : game.getPot().pots) {
    total += pot.amount;
  }
  return total;
}

} // namespace

BotAlgorithm::BotAlgorithm(BotProfile profile)
    : profile(std::move(profile)), rng(std::random_device{}()) {}

double BotAlgorithm::chance() {
  return std::uniform_real_distribution<double>(0.0, 1.0)(rng);
}

bool BotAlgorithm::makeDecision(const PokerGame &game, const Player &bot) {
  handStrength = evaluateStrength(game, bot);
  bluffing = false;
  cheapCall = false;

  if (handStrength >= profile.tightness) {
    return true;
  }
  // Weak hand: occasionally play it anyway to stay unpredictable.
  if (chance() < profile.bluffFrequency) {
    bluffing = true;
    return true;
  }
  // Completing the small blind or matching a min-bet is almost always
  // worth the pot odds; just call, never raise.
  int minBet = std::max(0, game.getCurrentBet() - bot.getRoundBet());
  if (minBet <= game.getBigBlind()) {
    cheapCall = true;
    return true;
  }
  return false;
}

int BotAlgorithm::calculateBet(const PokerGame &game, const Player &bot) {
  int minBet = std::max(0, game.getCurrentBet() - bot.getRoundBet());
  if (cheapCall) {
    return minBet;
  }
  int pot = std::max(totalPot(game), 2 * game.getBigBlind());

  // How far above the fold threshold the hand is; a bluff pretends to have
  // a moderately strong hand.
  double margin = bluffing
                      ? 0.45
                      : std::min(handStrength - profile.tightness, 1.5);

  double raiseChance = profile.aggression * (0.18 + 0.35 * margin);
  if (chance() >= raiseChance) {
    return minBet; // call, or check when there is nothing to match
  }

  double potFraction = 0.30 + 0.45 * profile.aggression * margin;
  if (bluffing) {
    potFraction = std::min(potFraction, 0.55);
  }
  int extra = std::max(game.getBigBlind(),
                       static_cast<int>(pot * potFraction));
  extra = std::min(extra, pot); // never overbet more than the pot

  int bet = minBet + extra;
  bet -= bet % 5;
  return std::max(bet, minBet + game.getBigBlind());
}

MonteCarloHandStrength::MonteCarloHandStrength(BotProfile profile)
    : BotAlgorithm(std::move(profile)), nSimulations(5000) {}

Deck MonteCarloHandStrength::getRemainingCardDeck(const PokerGame &game,
                                                  const Player &bot) {
  const std::vector<Card> tableCards = game.getTableCards();
  const std::vector<Card> holeCards = bot.getCards();

  Deck deck{Deck()};
  for (const auto &card : tableCards) {
    deck.removeCard(card);
  }

  for (const auto &card : holeCards) {
    deck.removeCard(card);
  }
  return deck;
}

double MonteCarloHandStrength::evaluateStrength(const PokerGame &game,
                                                const Player &bot) {
  int count = 0;
  std::vector<Card> tableCards = game.getTableCards();
  tableCards.reserve(5);

  const std::vector<Card> holeCards = bot.getCards();
  std::vector<Card> opponentHoleCards;
  opponentHoleCards.reserve(2);

  Deck remainingCards = getRemainingCardDeck(game, bot);
  int remainingPlayers = game.getActivePlayerCount() - 1;

  if (remainingPlayers <= 0) {
    return 1.0;
  }

  for (int i = 0; i < nSimulations; ++i) {
    Deck simRemainingCards = remainingCards;
    simRemainingCards.shuffle();
    std::vector<Card> simTableCards = tableCards;
    for (int j = 0; j < 5 - simTableCards.size(); ++j) {
      simTableCards.push_back(simRemainingCards.dealCard());
    }

    HandEvaluator handEvaluator{HandEvaluator(simTableCards, holeCards)};
    HandEvaluation handEvaluation{handEvaluator.evaluateHand()};
    bool won{true};
    for (int k = 0; k < remainingPlayers; ++k) {
      opponentHoleCards.clear();
      opponentHoleCards.push_back(simRemainingCards.dealCard());
      opponentHoleCards.push_back(simRemainingCards.dealCard());

      HandEvaluator opponentHandEvaluator{
          HandEvaluator(simTableCards, opponentHoleCards)};
      HandEvaluation opponentHandEvaluation{
          opponentHandEvaluator.evaluateHand()};
      if (!isEqualOrBetter(handEvaluation, opponentHandEvaluation)) {
        won = false;
        break;
      }
    }
    if (won) {
      ++count;
    }
  }
  double winProbability = count / static_cast<double>(nSimulations);
  double neutralValue = 1.0 / (remainingPlayers + 1);
  return (winProbability / neutralValue) * roundFactor(game.getRound());
}

BasicHandStrength::BasicHandStrength(BotProfile profile)
    : BotAlgorithm(std::move(profile)) {}

double BasicHandStrength::evaluateStrength(const PokerGame &game,
                                           const Player &bot) {
  std::vector<Card> tableCards = game.getTableCards();

  const std::vector<Card> holeCards = bot.getCards();
  HandEvaluator handEvaluator{HandEvaluator(tableCards, holeCards)};
  HandEvaluation handEvaluation{handEvaluator.evaluateHand()};
  int score = static_cast<int>(handEvaluation.type);
  // HighCard(1) .. RoyalFlush(10) mapped onto the same ~1.0-neutral scale
  // as the Monte Carlo estimate: a pair rates just above average.
  return score * roundFactor(game.getRound()) / 2.5;
}
