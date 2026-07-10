#include "HandEvaluator.h"
#include "Card.h"
#include "HandEvaluation.h"
#include "HandUtils.h"

#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>

namespace {

// Rank -> one representative card, with an ace also mapped to 1 so that
// wheel straights (A-2-3-4-5) are found.
std::map<int, Card> buildRankMap(const std::vector<Card> &cards) {
  std::map<int, Card> rankToCard;
  for (const auto &card : cards) {
    int rank = static_cast<int>(card.getRank());
    if (rankToCard.count(rank) == 0) {
      rankToCard[rank] = card;
    }
  }
  if (rankToCard.count(static_cast<int>(Card::Rank::Ace))) {
    rankToCard[1] = rankToCard[static_cast<int>(Card::Rank::Ace)];
  }
  return rankToCard;
}

// The five cards of the highest straight in the map, ordered highest first,
// or an empty vector if there is no straight.
std::vector<Card> bestStraightRun(const std::map<int, Card> &rankToCard) {
  std::vector<Card> run;
  std::vector<Card> best;

  int prevKey = -2; // the ace-low entry sits at key 1, so track map keys
  for (const auto &[key, card] : rankToCard) {
    if (key - prevKey != 1) {
      run.clear();
    }
    run.push_back(card);
    prevKey = key;
    if (run.size() >= 5) {
      // Ranks ascend through the map, so a later window is always higher.
      best = std::vector<Card>(run.end() - 5, run.end());
    }
  }
  std::reverse(best.begin(), best.end());
  return best;
}

} // namespace

HandEvaluator::HandEvaluator(const std::vector<Card> &tableCards,
                             const std::vector<Card> &playerHoleCards) {
  sevenCardHand.reserve(7);

  sevenCardHand.insert(sevenCardHand.end(), tableCards.begin(),
                       tableCards.end());
  sevenCardHand.insert(sevenCardHand.end(), playerHoleCards.begin(),
                       playerHoleCards.end());
  std::sort(sevenCardHand.begin(), sevenCardHand.end());
};

// "Not this hand" result: a plain high-card evaluation built from the top
// available cards. Safe for hands with fewer than five cards.
HandEvaluation HandEvaluator::highCardFallback() {
  return HandEvaluation{Hand::Type::HighCard, std::vector<Card>{},
                        getKickerCards(sevenCardHand, std::vector<Card>{})};
}

HandEvaluation HandEvaluator::isPair() {
  std::vector<Card> matchingCards;
  matchingCards.reserve(2);

  for (int i = sevenCardHand.size() - 1; i >= 1; --i) {
    if (static_cast<int>(sevenCardHand.at(i).getRank()) ==
        static_cast<int>(sevenCardHand.at(i - 1).getRank())) {

      matchingCards.push_back(sevenCardHand.at(i));
      matchingCards.push_back(sevenCardHand.at(i - 1));

      return HandEvaluation{Hand::Type::Pair, matchingCards,
                            getKickerCards(sevenCardHand, matchingCards)};
    }
  }
  return highCardFallback();
}

HandEvaluation HandEvaluator::isTwoPair() {
  int count{0};

  std::vector<Card> matchingCards;
  matchingCards.reserve(4);

  for (int i = sevenCardHand.size() - 1; i >= 1; --i) {
    if (static_cast<int>(sevenCardHand.at(i).getRank()) ==
        static_cast<int>(sevenCardHand.at(i - 1).getRank())) {
      matchingCards.push_back(sevenCardHand.at(i));
      matchingCards.push_back(sevenCardHand.at(i - 1));

      count++;
      i--; // Skip overlapping pairs
      if (count == 2) {
        return HandEvaluation{Hand::Type::TwoPair, matchingCards,
                              getKickerCards(sevenCardHand, matchingCards)};
      }
    }
  }
  return highCardFallback();
}

HandEvaluation HandEvaluator::isThreeOfAKind() {
  std::vector<Card> matchingCards;
  matchingCards.reserve(3);

  for (int i = sevenCardHand.size() - 1; i >= 2; --i) {
    if (static_cast<int>(sevenCardHand.at(i).getRank()) ==
            static_cast<int>(sevenCardHand.at(i - 1).getRank()) &&
        static_cast<int>(sevenCardHand.at(i).getRank()) ==
            static_cast<int>(sevenCardHand.at(i - 2).getRank())) {

      matchingCards.push_back(sevenCardHand.at(i));
      matchingCards.push_back(sevenCardHand.at(i - 1));
      matchingCards.push_back(sevenCardHand.at(i - 2));

      return HandEvaluation{Hand::Type::ThreeOfAKind, matchingCards,
                            getKickerCards(sevenCardHand, matchingCards)};
    }
  }
  return highCardFallback();
}

HandEvaluation HandEvaluator::isStraight() {
  std::vector<Card> matchingCards = bestStraightRun(buildRankMap(sevenCardHand));
  if (matchingCards.empty()) {
    return highCardFallback();
  }
  return HandEvaluation{Hand::Type::Straight, matchingCards,
                        std::vector<Card>{}};
}

HandEvaluation HandEvaluator::isFlush() {
  std::unordered_map<Card::Suit, std::vector<Card>> suitMap;

  for (const auto &card : sevenCardHand) {
    suitMap[card.getSuit()].push_back(card);
  }

  for (const auto &[suit, cards] : suitMap) {
    if (cards.size() >= 5) {
      // Suited cards keep the ascending sort; take the top five, highest
      // first.
      std::vector<Card> matchingCards(cards.rbegin(), cards.rbegin() + 5);
      return HandEvaluation{Hand::Type::Flush, matchingCards,
                            std::vector<Card>{}};
    }
  }
  return highCardFallback();
}

HandEvaluation HandEvaluator::isFullHouse() {
  HandEvaluation twoPair{isTwoPair()};
  HandEvaluation threeOfAKind{isThreeOfAKind()};

  if (twoPair.type != Hand::Type::TwoPair ||
      threeOfAKind.type != Hand::Type::ThreeOfAKind) {
    return highCardFallback();
  }

  std::vector<Card> matchingCards;
  matchingCards.reserve(5);
  matchingCards.insert(matchingCards.end(), threeOfAKind.primaryCards.begin(),
                       threeOfAKind.primaryCards.end());

  // The best pair is the highest pair that is not the trips; the two-pair
  // cards are ordered high pair first.
  Card::Rank tripsRank = threeOfAKind.primaryCards.front().getRank();
  for (const Card &card : twoPair.primaryCards) {
    if (card.getRank() != tripsRank &&
        matchingCards.size() < 5) {
      matchingCards.push_back(card);
    }
  }
  if (matchingCards.size() < 5) {
    return highCardFallback();
  }

  return HandEvaluation{Hand::Type::FullHouse, matchingCards,
                        std::vector<Card>{}};
}

HandEvaluation HandEvaluator::isFourOfAKind() {
  std::vector<Card> matchingCards;
  matchingCards.reserve(4);

  for (int i = sevenCardHand.size() - 1; i >= 3; --i) {
    if (static_cast<int>(sevenCardHand.at(i).getRank()) ==
            static_cast<int>(sevenCardHand.at(i - 1).getRank()) &&
        static_cast<int>(sevenCardHand.at(i).getRank()) ==
            static_cast<int>(sevenCardHand.at(i - 2).getRank()) &&
        static_cast<int>(sevenCardHand.at(i).getRank()) ==
            static_cast<int>(sevenCardHand.at(i - 3).getRank())) {

      matchingCards.push_back(sevenCardHand.at(i));
      matchingCards.push_back(sevenCardHand.at(i - 1));
      matchingCards.push_back(sevenCardHand.at(i - 2));
      matchingCards.push_back(sevenCardHand.at(i - 3));

      return HandEvaluation{Hand::Type::FourOfAKind, matchingCards,
                            getKickerCards(sevenCardHand, matchingCards)};
    }
  }
  return highCardFallback();
}

HandEvaluation HandEvaluator::isStraightFlush() {
  std::unordered_map<Card::Suit, std::vector<Card>> suitMap;
  for (const auto &card : sevenCardHand) {
    suitMap[card.getSuit()].push_back(card);
  }

  for (const auto &[suit, cards] : suitMap) {
    if (cards.size() < 5) {
      continue;
    }
    std::vector<Card> matchingCards = bestStraightRun(buildRankMap(cards));
    if (!matchingCards.empty()) {
      return HandEvaluation{Hand::Type::StraightFlush, matchingCards,
                            std::vector<Card>{}};
    }
  }
  return highCardFallback();
}

HandEvaluation HandEvaluator::isRoyalFlush() {
  HandEvaluation straightFlush{isStraightFlush()};
  if (straightFlush.type != Hand::Type::StraightFlush) {
    return highCardFallback();
  }
  // An ace-high straight flush is royal; cards are ordered highest first
  // (a wheel leads with the five, so it never matches).
  if (straightFlush.primaryCards.front().getRank() == Card::Rank::Ace) {
    return HandEvaluation{Hand::Type::RoyalFlush, straightFlush.primaryCards,
                          straightFlush.secondaryCards};
  }
  return straightFlush;
}

HandEvaluation HandEvaluator::evaluateHand() {
  HandEvaluation hand;

  hand = isRoyalFlush();
  if (hand.type != Hand::Type::HighCard)
    return hand;

  hand = isFourOfAKind();
  if (hand.type != Hand::Type::HighCard)
    return hand;

  hand = isFullHouse();
  if (hand.type != Hand::Type::HighCard)
    return hand;

  hand = isFlush();
  if (hand.type != Hand::Type::HighCard)
    return hand;

  hand = isStraight();
  if (hand.type != Hand::Type::HighCard)
    return hand;

  hand = isThreeOfAKind();
  if (hand.type != Hand::Type::HighCard)
    return hand;

  hand = isTwoPair();
  if (hand.type != Hand::Type::HighCard)
    return hand;

  hand = isPair();
  return hand;
}
