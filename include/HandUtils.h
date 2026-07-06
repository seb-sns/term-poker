#ifndef HANDUTILS_H
#define HANDUTILS_H

#include "Card.h"
#include "Hand.h"
#include "HandEvaluation.h"
#include <string>
#include <vector>

std::vector<Card> getKickerCards(const std::vector<Card> &sevenCardHand,
                                 const std::vector<Card> &primaryHand);

bool isHigherKickerPrimary(const std::vector<Card> &a,
                           const std::vector<Card> &b);

bool isHigherKickerSecondary(const std::vector<Card> &a,
                             const std::vector<Card> &b);
bool isEqualOrBetter(const HandEvaluation &handA, const HandEvaluation &handB);

std::string handTypeToString(Hand::Type handType);

#endif
