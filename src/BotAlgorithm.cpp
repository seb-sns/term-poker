#include "Player.h"

#include "PokerGame.h"

#include "BotAlgorithm.h"

#include "HandEvaluation.h"
#include "HandEvaluator.h"

#include "Card.h"
#include "Deck.h"

#include "HandUtils.h"

MonteCarloHandStrength::MonteCarloHandStrength(double pacificity)
    : nSimulations(10000), pacificity(pacificity) {}

Deck MonteCarloHandStrength::getRemainingCardDeck(const PokerGame &game,
                                                  const Player &bot) {
  const std::vector<Card> tableCards = game.getTableCards();
  const std::vector<Card> holeCards = bot.getCards();

  Deck deck{Deck()};
  for (const auto card : tableCards) {
    deck.removeCard(card);
  }

  for (const auto card : holeCards) {
    deck.removeCard(card);
  }
  return deck;
}

void MonteCarloHandStrength::evaluateStrength(const PokerGame &game,
                                              const Player &bot) {
  int count = 0;
  std::vector<Card> tableCards = game.getTableCards();
  tableCards.reserve(5);

  const std::vector<Card> holeCards = bot.getCards();
  std::vector<Card> opponentHoleCards;
  opponentHoleCards.reserve(2);

  Deck remainingCards = getRemainingCardDeck(game, bot);
  int remainingPlayers = game.getNPlayers() - 1;

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
  double score = count / static_cast<double>(nSimulations);
  double neutralValue = 1.0 / remainingPlayers;
  int adjustment = 6 - static_cast<int>(game.getRound());
  score = score * adjustment / 2;
  handStrengthAssessment = (score / neutralValue);
}

bool MonteCarloHandStrength::makeDecision(const PokerGame &game,
                                          const Player &bot) {

  MonteCarloHandStrength::evaluateStrength(game, bot);
  if (handStrengthAssessment >= pacificity) {
    return true;
  } else {
    return false;
  }
}

int MonteCarloHandStrength::calculateBet(const PokerGame &game,
                                         const Player &bot) {
  int currentChips = bot.getChipCount();
  int baseBetSize = (bot.getTotalBet() + game.getSmallBlind());
  int betSizeCalc = std::floor(baseBetSize * 1.5 * (2 - pacificity));
  int betSize = betSizeCalc - (betSizeCalc % 5);
  return std::min(currentChips, betSize);
}
