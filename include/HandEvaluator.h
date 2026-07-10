#ifndef HANDEVALUATOR_H
#define HANDEVALUATOR_H

#include "Card.h"
#include "HandEvaluation.h"
#include <vector>

class HandEvaluator {
private:
  std::vector<Card> sevenCardHand;

public:
  HandEvaluator(const std::vector<Card> &tableCards,
                const std::vector<Card> &playerHoleCards);

  std::vector<Card> getSevenCardHand() { return sevenCardHand; }

  HandEvaluation isPair();
  HandEvaluation isTwoPair();
  HandEvaluation isThreeOfAKind();
  HandEvaluation isStraight();
  HandEvaluation isFlush();
  HandEvaluation isFullHouse();
  HandEvaluation isFourOfAKind();
  HandEvaluation isStraightFlush();
  HandEvaluation isRoyalFlush();

  HandEvaluation highCardFallback();

  HandEvaluation evaluateHand();
};

#endif
