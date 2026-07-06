#include "HandUtils.h"
#include "Card.h"
#include "Hand.h"
#include "HandEvaluation.h"
#include <map>
#include <string>
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

std::string handTypeToString(Hand::Type handType) {
  switch (handType) {
  case Hand::Type::None:
    return "None";
  case Hand::Type::HighCard:
    return "High Card";
  case Hand::Type::Pair:
    return "Pair";
  case Hand::Type::TwoPair:
    return "Two Pair";
  case Hand::Type::ThreeOfAKind:
    return "Three of a Kind";
  case Hand::Type::Straight:
    return "Straight";
  case Hand::Type::Flush:
    return "Flush";
  case Hand::Type::FullHouse:
    return "Full House";
  case Hand::Type::FourOfAKind:
    return "Four of a Kind";
  case Hand::Type::StraightFlush:
    return "Straight Flush";
  case Hand::Type::RoyalFlush:
    return "Royal Flush";
  }
  return "Unknown";
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
