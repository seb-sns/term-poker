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
  u_long remainingCardSlots{sevenCardHand.size() - primaryHand.size()};

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

  return std::vector<Card>(remainingCards.begin(),
                           remainingCards.begin() + remainingCardSlots);
}

bool isHigherKickerPrimary(const std::vector<Card> &a,
                           const std::vector<Card> &b) {
  for (int i = a.size() - 1; i >= 0; --i) {
    if (static_cast<int>(a[i].getRank()) < static_cast<int>(b[i].getRank())) {
      return false;
    } else if (static_cast<int>(a[i].getRank()) >
               static_cast<int>(b[i].getRank())) {
      return true;
    }
  }
  return false;
}

bool isHigherKickerSecondary(const std::vector<Card> &a,
                             const std::vector<Card> &b) {

  for (int i = a.size() - 1; i >= 0; --i) {
    if ((static_cast<int>(a[i].getRank()) > static_cast<int>(b[i].getRank()))) {
      return true;
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
    std::cout << "Fush";
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

  else if (handA.type == handB.type) {
    if (handA.primaryCards == handB.primaryCards &&
        handA.primaryCards.size() == 5) {
      return true;
    } else if (handA.primaryCards == handB.primaryCards &&
               handA.secondaryCards == handB.secondaryCards) {
      return true;
    } else if (isHigherKickerPrimary(handA.primaryCards, handB.primaryCards)) {
      return true;
    }

    else if (isHigherKickerSecondary(handA.secondaryCards,
                                     handB.secondaryCards)) {
      return true;
    }
  }
  return false;
}
