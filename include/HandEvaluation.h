#ifndef HANDEVALUATION_H
#define HANDEVALUATION_H

#include "Card.h"
#include "Hand.h"

#include <vector>

struct HandEvaluation {
  Hand::Type type;
  std::vector<Card> primaryCards;
  std::vector<Card> secondaryCards;
};

#endif
