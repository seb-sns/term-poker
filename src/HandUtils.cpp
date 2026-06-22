#include "HandUtils.h"
#include "Card.h"
#include "Hand.h"
#include "HandEvaluation.h"
#include <iostream>
#include <map>
#include <vector>

std::vector<Card> getKickerCards(const std::vector<Card> &sevenCardHand,
                                 const std::vector<Card> &primaryHand) {
  std::vector<Card> remainingCards;
  std::map<Card, int> primaryCardCounts;
  std::size_t remainingCardSlots =
      (primaryHand.size() <= 5) ? (5 - primaryHand.size()) : 0;

  for (const auto &item : primaryHand) {
    primaryCardCounts[item]++;
  }

  for (const auto &item : sevenCardHand) {
    if (primaryCardCounts[item] > 0) {
      primaryCardCounts[item]--;
    } else {
      remainingCards.push_back(item);
    }
  }

  std::size_t slots = std::min(remainingCardSlots, remainingCards.size());
  return std::vector<Card>(remainingCards.begin(),
                           remainingCards.begin() + slots);
}

bool isHigherKickerPrimary(const std::vector<Card> &a,
                           const std::vector<Card> &b) {
  for (int i = a.size() - 1; i >= 0; --i) {
    int aRank = static_cast<int>(a[i].getRank());
    int bRank = static_cast<int>(b[i].getRank());
    if (aRank > bRank) {
      return true;
    }
    if (aRank < bRank) {
      return false;
    }
  }
  return false;
}

bool isHigherKickerSecondary(const std::vector<Card> &a,
                             const std::vector<Card> &b) {

  for (int i = a.size() - 1; i >= 0; --i) {
    int aRank = static_cast<int>(a[i].getRank());
    int bRank = static_cast<int>(b[i].getRank());
    if (aRank > bRank) {
      return true;
    }
    if (aRank < bRank) {
      return false;
    }
  }
  return false;
}

void printHandType(Hand::Type &handType) {
  switch (handType) {
  case Hand::Type::None:
    std::cout << "None";
    break;
  case Hand::Type::HighCard:
    std::cout << "High Card";
    break;
  case Hand::Type::Pair:
    std::cout << "Pair";
    break;
  case Hand::Type::TwoPair:
    std::cout << "Two Pair";
    break;
  case Hand::Type::ThreeOfAKind:
    std::cout << "Three of a Kind";
    break;
  case Hand::Type::Straight:
    std::cout << "Straight";
    break;
  case Hand::Type::Flush:
    std::cout << "Flush";
    break;
  case Hand::Type::FullHouse:
    std::cout << "Full House";
    break;
  case Hand::Type::FourOfAKind:
    std::cout << "Four of a Kind";
    break;
  case Hand::Type::StraightFlush:
    std::cout << "Straight Flush";
    break;
  case Hand::Type::RoyalFlush:
    std::cout << "Royal Flush";
    break;
  }
}

bool isEqualOrBetter(const HandEvaluation &handA, const HandEvaluation &handB) {
  if (handA.type > handB.type) {
    return true;
  }
  if (handA.type != handB.type) {
    return false;
  }

  if (isHigherKickerPrimary(handA.primaryCards, handB.primaryCards)) {
    return true;
  }
  if (isHigherKickerPrimary(handB.primaryCards, handA.primaryCards)) {
    return false;
  }
  // Primary cards are equal; compare secondary kickers.
  return isHigherKickerSecondary(handA.secondaryCards, handB.secondaryCards) ||
         (handA.secondaryCards == handB.secondaryCards);
}
