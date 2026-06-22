#include "HandEvaluator.h"
#include "Card.h"
#include "HandEvaluation.h"
#include "HandUtils.h"

#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>

HandEvaluator::HandEvaluator(const std::vector<Card> &tableCards,
                             const std::vector<Card> &playerHoleCards) {
  sevenCardHand.reserve(7);

  sevenCardHand.insert(sevenCardHand.end(), tableCards.begin(),
                       tableCards.end());
  sevenCardHand.insert(sevenCardHand.end(), playerHoleCards.begin(),
                       playerHoleCards.end());
  std::sort(sevenCardHand.begin(), sevenCardHand.end());
};

HandEvaluation HandEvaluator::isPair() {

  std::vector<Card> matchingCards;
  matchingCards.reserve(2);
  std::vector<Card> unmatchedCards;
  unmatchedCards.reserve(5);
  for (int i = sevenCardHand.size() - 1; i >= 1; --i) {
    if (static_cast<int>(sevenCardHand.at(i).getRank()) ==
        static_cast<int>(sevenCardHand.at(i - 1).getRank())) {

      matchingCards.push_back(sevenCardHand.at(i));
      matchingCards.push_back(sevenCardHand.at(i - 1));

      unmatchedCards = getKickerCards(sevenCardHand, matchingCards);
      return HandEvaluation{Hand::Type::Pair, matchingCards, unmatchedCards};
    }
  }

  unmatchedCards = getKickerCards(sevenCardHand, matchingCards);
  return HandEvaluation{Hand::Type::HighCard, matchingCards, unmatchedCards};
}

HandEvaluation HandEvaluator::isTwoPair() {
  int count{0};

  std::vector<Card> matchingCards;
  matchingCards.reserve(4);

  std::vector<Card> unmatchedCards;
  unmatchedCards.reserve(5);

  for (int i = sevenCardHand.size() - 1; i >= 1; --i) {
    if (static_cast<int>(sevenCardHand.at(i).getRank()) ==
        static_cast<int>(sevenCardHand.at(i - 1).getRank())) {
      matchingCards.push_back(sevenCardHand.at(i));
      matchingCards.push_back(sevenCardHand.at(i - 1));

      count++;
      i--; // Skip overlapping pairs
      if (count == 2) {

        unmatchedCards = getKickerCards(sevenCardHand, matchingCards);
        return HandEvaluation{Hand::Type::TwoPair, matchingCards,
                              unmatchedCards};
      }
    }
  }
  unmatchedCards = getKickerCards(sevenCardHand, matchingCards);
  return HandEvaluation{Hand::Type::HighCard, matchingCards, unmatchedCards};
}

HandEvaluation HandEvaluator::isThreeOfAKind() {
  std::vector<Card> matchingCards;
  matchingCards.reserve(3);

  std::vector<Card> unmatchedCards;
  unmatchedCards.reserve(5);
  for (int i = sevenCardHand.size() - 1; i >= 2; --i) {
    if (static_cast<int>(sevenCardHand.at(i).getRank()) ==
            static_cast<int>(sevenCardHand.at(i - 1).getRank()) &&
        static_cast<int>(sevenCardHand.at(i).getRank()) ==
            static_cast<int>(sevenCardHand.at(i - 2).getRank())) {

      matchingCards.push_back(sevenCardHand.at(i));
      matchingCards.push_back(sevenCardHand.at(i - 1));
      matchingCards.push_back(sevenCardHand.at(i - 2));

      unmatchedCards = getKickerCards(sevenCardHand, matchingCards);
      return HandEvaluation{Hand::Type::ThreeOfAKind, matchingCards,
                            unmatchedCards};
    }
  }
  unmatchedCards = getKickerCards(sevenCardHand, matchingCards);
  return HandEvaluation{Hand::Type::HighCard, matchingCards, unmatchedCards};
}

HandEvaluation HandEvaluator::isStraight() {
  std::map<int, Card> rankToCard;

  for (const auto &card : sevenCardHand) {
    int rank = static_cast<int>(card.getRank());
    if (rankToCard.count(rank) == 0) {
      rankToCard[rank] = card;
    }
  }

  if (rankToCard.count(static_cast<int>(Card::Rank::Ace))) {
    rankToCard[1] = rankToCard[static_cast<int>(Card::Rank::Ace)];
  }

  int count{1};
  std::map<int, Card>::iterator prev = rankToCard.begin();
  std::map<int, Card>::iterator curr = std::next(prev);

  std::vector<Card> matchingCards;
  matchingCards.reserve(5);

  std::vector<Card> unmatchedCards;
  unmatchedCards.reserve(5);
  for (; curr != rankToCard.end(); ++curr, ++prev) {
    if ((curr->first - prev->first) == 1) {
      if (count == 1) {
        matchingCards.push_back(prev->second);
      }
      matchingCards.push_back(curr->second);
      ++count;
    } else {
      count = 1;
      matchingCards.clear();
    }

    if (count == 5) {
      unmatchedCards = getKickerCards(sevenCardHand, matchingCards);
      return HandEvaluation{Hand::Type::Straight, matchingCards,
                            unmatchedCards};
    }
  }
  matchingCards = std::vector(sevenCardHand.end() - 5, sevenCardHand.end());
  return HandEvaluation{Hand::Type::HighCard, matchingCards,
                        std::vector<Card>{}};
}

HandEvaluation HandEvaluator::isFlush() {
  std::unordered_map<Card::Suit, std::vector<Card>> suitMap;
  std::vector<Card> matchingCards;
  matchingCards.reserve(5);

  for (const auto &card : sevenCardHand) {
    suitMap[card.getSuit()].push_back(card);
  }

  for (const auto &[suit, cards] : suitMap) {
    if (cards.size() >= 5) {
      matchingCards = std::vector(cards.end() - 5, cards.end());
      return HandEvaluation{Hand::Type::Flush, matchingCards,
                            std::vector<Card>{}};
    }
  }
  matchingCards = std::vector(sevenCardHand.end() - 5, sevenCardHand.end());
  return HandEvaluation{Hand::Type::HighCard, matchingCards,
                        std::vector<Card>{}};
}

HandEvaluation HandEvaluator::isFullHouse() {
  HandEvaluation twoPair{isTwoPair()};
  HandEvaluation threeOfAKind{isThreeOfAKind()};

  std::vector<Card> matchingCards;
  matchingCards.reserve(5);

  if (twoPair.type == Hand::Type::TwoPair &&
      threeOfAKind.type == Hand::Type::ThreeOfAKind) {

    matchingCards.insert(matchingCards.end(), threeOfAKind.primaryCards.begin(),
                         threeOfAKind.primaryCards.end());

    std::vector<Card> remainingPairs =
        getKickerCards(twoPair.primaryCards, threeOfAKind.primaryCards);

    matchingCards.insert(matchingCards.end(), remainingPairs.begin(),
                         remainingPairs.begin() + 2);

    return HandEvaluation{Hand::Type::FullHouse, matchingCards,
                          std::vector<Card>{}};

  } else {
    std::vector<Card> matchingCards =
        std::vector(sevenCardHand.end() - 5, sevenCardHand.end());

    return HandEvaluation{Hand::Type::HighCard, matchingCards,
                          std::vector<Card>{}};
  }
}

HandEvaluation HandEvaluator::isFourOfAKind() {
  std::vector<Card> matchingCards;
  matchingCards.reserve(4);

  std::vector<Card> unmatchedCards;
  unmatchedCards.reserve(1);

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

      unmatchedCards = getKickerCards(sevenCardHand, matchingCards);
      return HandEvaluation{Hand::Type::FourOfAKind, matchingCards,
                            unmatchedCards};
    }
  }
  matchingCards = std::vector(sevenCardHand.end() - 5, sevenCardHand.end());
  return HandEvaluation{Hand::Type::HighCard, matchingCards,
                        std::vector<Card>{}};
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

    int count{1};
    std::vector<Card> matchingCards;
    matchingCards.reserve(5);
    auto prev = rankToCard.begin();
    auto curr = std::next(prev);
    for (; curr != rankToCard.end(); ++curr, ++prev) {
      if ((curr->first - prev->first) == 1) {
        if (count == 1) {
          matchingCards.push_back(prev->second);
        }
        matchingCards.push_back(curr->second);
        ++count;
      } else {
        count = 1;
        matchingCards.clear();
      }
      if (count == 5) {
        return HandEvaluation{Hand::Type::StraightFlush, matchingCards,
                              std::vector<Card>{}};
      }
    }
  }

  std::vector<Card> matchingCards{
      std::vector(sevenCardHand.end() - 5, sevenCardHand.end())};
  return HandEvaluation{Hand::Type::HighCard, matchingCards,
                        std::vector<Card>{}};
}

HandEvaluation HandEvaluator::isRoyalFlush() {
  HandEvaluation straightFlush{isStraightFlush()};
  if (straightFlush.type != Hand::Type::StraightFlush) {
    std::vector<Card> matchingCards{
        std::vector(sevenCardHand.end() - 5, sevenCardHand.end())};
    return HandEvaluation{Hand::Type::HighCard, matchingCards,
                          std::vector<Card>{}};
  }
  bool foundAce{false};
  bool foundKing{false};

  for (const auto &card : straightFlush.primaryCards) {
    if (card.getRank() == Card::Rank::Ace)
      foundAce = true;
    if (card.getRank() == Card::Rank::King)
      foundKing = true;

    if (foundAce && foundKing) {
      return HandEvaluation{Hand::Type::RoyalFlush, straightFlush.primaryCards,
                            straightFlush.secondaryCards};
    }
  }
  return HandEvaluation{Hand::Type::StraightFlush, straightFlush.primaryCards,
                        straightFlush.secondaryCards};
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
