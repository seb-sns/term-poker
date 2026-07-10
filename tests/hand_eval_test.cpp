// Cross-checks HandEvaluator/isEqualOrBetter against a brute-force
// reference evaluator, plus directed cases for previously broken spots.

#include "Card.h"
#include "Hand.h"
#include "HandEvaluation.h"
#include "HandEvaluator.h"
#include "HandUtils.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const std::string &label) {
  if (!condition) {
    ++failures;
    std::cerr << "FAIL: " << label << '\n';
  }
}

// "AS" = ace of spades, "TD" = ten of diamonds, "9H", "2C", ...
Card card(const std::string &text) {
  int rank;
  switch (text[0]) {
  case 'T': rank = 10; break;
  case 'J': rank = 11; break;
  case 'Q': rank = 12; break;
  case 'K': rank = 13; break;
  case 'A': rank = 14; break;
  default: rank = text[0] - '0'; break;
  }
  Card::Suit suit;
  switch (text[1]) {
  case 'C': suit = Card::Suit::Clubs; break;
  case 'D': suit = Card::Suit::Diamonds; break;
  case 'H': suit = Card::Suit::Hearts; break;
  default: suit = Card::Suit::Spades; break;
  }
  return Card(suit, static_cast<Card::Rank>(rank));
}

std::vector<Card> cards(const std::vector<std::string> &texts) {
  std::vector<Card> out;
  for (const auto &t : texts) {
    out.push_back(card(t));
  }
  return out;
}

// Reference score for exactly five cards: [category, tiebreakers...],
// higher lexicographic order = better hand. Categories: 0 high card,
// 1 pair, 2 two pair, 3 trips, 4 straight, 5 flush, 6 full house,
// 7 quads, 8 straight flush.
std::vector<int> scoreFive(const std::vector<Card> &five) {
  std::vector<int> ranks;
  for (const Card &c : five) {
    ranks.push_back(static_cast<int>(c.getRank()));
  }
  std::sort(ranks.rbegin(), ranks.rend());

  bool flush = true;
  for (const Card &c : five) {
    flush = flush && c.getSuit() == five[0].getSuit();
  }

  int straightHigh = 0;
  {
    std::vector<int> distinct = ranks;
    distinct.erase(std::unique(distinct.begin(), distinct.end()),
                   distinct.end());
    if (distinct.size() == 5) {
      if (distinct[0] - distinct[4] == 4) {
        straightHigh = distinct[0];
      } else if (distinct[0] == 14 && distinct[1] == 5 &&
                 distinct[1] - distinct[4] == 3) {
        straightHigh = 5; // wheel
      }
    }
  }

  // Group ranks by count, biggest group first, then by rank.
  std::vector<std::pair<int, int>> groups; // {count, rank}
  for (size_t i = 0; i < ranks.size();) {
    size_t j = i;
    while (j < ranks.size() && ranks[j] == ranks[i]) {
      ++j;
    }
    groups.push_back({static_cast<int>(j - i), ranks[i]});
    i = j;
  }
  std::sort(groups.begin(), groups.end(),
            [](const auto &a, const auto &b) { return a > b; });

  std::vector<int> key;
  if (straightHigh && flush) {
    key = {8, straightHigh};
  } else if (groups[0].first == 4) {
    key = {7, groups[0].second, groups[1].second};
  } else if (groups[0].first == 3 && groups[1].first == 2) {
    key = {6, groups[0].second, groups[1].second};
  } else if (flush) {
    key = {5};
  } else if (straightHigh) {
    key = {4, straightHigh};
  } else if (groups[0].first == 3) {
    key = {3};
  } else if (groups[0].first == 2 && groups[1].first == 2) {
    key = {2};
  } else if (groups[0].first == 2) {
    key = {1};
  } else {
    key = {0};
  }
  // Append remaining ranks in group order as generic tiebreakers.
  if (key.size() == 1) {
    for (const auto &g : groups) {
      for (int k = 0; k < g.first; ++k) {
        key.push_back(g.second);
      }
    }
  }
  return key;
}

// Best five-card score among all C(7,5) combinations.
std::vector<int> scoreSeven(const std::vector<Card> &seven) {
  std::vector<int> best;
  for (size_t skipA = 0; skipA < seven.size(); ++skipA) {
    for (size_t skipB = skipA + 1; skipB < seven.size(); ++skipB) {
      std::vector<Card> five;
      for (size_t i = 0; i < seven.size(); ++i) {
        if (i != skipA && i != skipB) {
          five.push_back(seven[i]);
        }
      }
      std::vector<int> key = scoreFive(five);
      if (key > best) {
        best = key;
      }
    }
  }
  return best;
}

Hand::Type expectedType(const std::vector<int> &refKey) {
  if (refKey[0] == 8 && refKey[1] == 14) {
    return Hand::Type::RoyalFlush;
  }
  return static_cast<Hand::Type>(refKey[0] + 1); // ref 0 = HighCard = 1
}

HandEvaluation evaluate(const std::vector<Card> &seven) {
  HandEvaluator evaluator(std::vector<Card>(seven.begin(), seven.begin() + 5),
                          std::vector<Card>(seven.end() - 2, seven.end()));
  return evaluator.evaluateHand();
}

// -1 if a < b, 0 if equal, 1 if a > b, via the engine's comparison.
int engineCompare(const HandEvaluation &a, const HandEvaluation &b) {
  bool aGeB = isEqualOrBetter(a, b);
  bool bGeA = isEqualOrBetter(b, a);
  if (aGeB && bGeA) {
    return 0;
  }
  return aGeB ? 1 : -1;
}

int refCompare(const std::vector<int> &a, const std::vector<int> &b) {
  if (a == b) {
    return 0;
  }
  return a > b ? 1 : -1;
}

void directedTests() {
  // Two pair: the high pair decides first. Board gives both a queen kicker.
  {
    HandEvaluation acesUp = evaluate(cards({"AH", "AD", "2C", "2D", "QS",
                                            "7H", "4C"}));
    HandEvaluation kingsUp = evaluate(cards({"KH", "KD", "QC", "QD", "8S",
                                             "7D", "4D"}));
    check(acesUp.type == Hand::Type::TwoPair, "aces up is two pair");
    check(engineCompare(acesUp, kingsUp) == 1, "AA22 beats KKQQ");
  }
  // Full house: trips decide before the pair.
  {
    HandEvaluation kkk22 = evaluate(cards({"KH", "KD", "KC", "2D", "2S",
                                           "7H", "4C"}));
    HandEvaluation qqqaa = evaluate(cards({"QH", "QD", "QC", "AD", "AS",
                                           "7D", "4D"}));
    check(engineCompare(kkk22, qqqaa) == 1, "KKK22 beats QQQAA");
  }
  // Kickers must be the highest remaining cards.
  {
    HandEvaluation aceKicker = evaluate(cards({"8H", "8D", "AC", "9D", "5S",
                                               "4H", "2C"}));
    HandEvaluation queenKicker = evaluate(cards({"8S", "8C", "QC", "9H", "5D",
                                                 "4D", "2D"}));
    check(engineCompare(aceKicker, queenKicker) == 1,
          "pair kicker: ace beats queen");
  }
  // A six-card run must give the highest straight, not the lowest.
  {
    HandEvaluation nineHigh = evaluate(cards({"4H", "5D", "6C", "7D", "8S",
                                              "9H", "2C"}));
    HandEvaluation eightHigh = evaluate(cards({"4S", "5C", "6D", "7H", "8D",
                                               "2H", "2D"}));
    check(nineHigh.type == Hand::Type::Straight, "run of six is a straight");
    check(engineCompare(nineHigh, eightHigh) == 1,
          "9-high straight beats 8-high");
  }
  // Six suited cards through the ace: royal flush, not 9..K straight flush.
  {
    HandEvaluation royal = evaluate(cards({"9H", "TH", "JH", "QH", "KH",
                                           "AH", "2C"}));
    check(royal.type == Hand::Type::RoyalFlush,
          "six-card suited run to the ace is royal");
  }
  // Wheel is the lowest straight.
  {
    HandEvaluation wheel = evaluate(cards({"AH", "2D", "3C", "4D", "5S",
                                           "9H", "JC"}));
    HandEvaluation sixHigh = evaluate(cards({"2H", "3D", "4C", "5D", "6S",
                                             "9D", "JD"}));
    check(wheel.type == Hand::Type::Straight, "wheel is a straight");
    check(engineCompare(sixHigh, wheel) == 1, "6-high straight beats wheel");
  }
  // Identical ranks in different suits split the pot.
  {
    HandEvaluation a = evaluate(cards({"AH", "KD", "8C", "6D", "4S",
                                       "3H", "2C"}));
    HandEvaluation b = evaluate(cards({"AD", "KH", "8D", "6C", "4H",
                                       "3S", "2D"}));
    check(engineCompare(a, b) == 0, "identical ranks tie");
  }
  // Evaluating hole cards alone (preflop bot path) must not blow up.
  {
    HandEvaluator preflopPair(std::vector<Card>{}, cards({"AH", "AD"}));
    check(preflopPair.evaluateHand().type == Hand::Type::Pair,
          "two-card evaluation: pair");
    HandEvaluator preflopHigh(std::vector<Card>{}, cards({"AH", "KD"}));
    check(preflopHigh.evaluateHand().type == Hand::Type::HighCard,
          "two-card evaluation: high card");
  }
}

void randomizedCrossCheck() {
  std::mt19937 rng(20260707);
  std::vector<Card> deck;
  for (int suit = 0; suit < 4; ++suit) {
    for (int rank = 2; rank <= 14; ++rank) {
      deck.emplace_back(static_cast<Card::Suit>(suit),
                        static_cast<Card::Rank>(rank));
    }
  }

  int categoryMismatches = 0;
  int orderMismatches = 0;
  for (int iteration = 0; iteration < 20000; ++iteration) {
    std::shuffle(deck.begin(), deck.end(), rng);
    // Two players sharing a board, like a real showdown.
    std::vector<Card> board(deck.begin(), deck.begin() + 5);
    std::vector<Card> holeA(deck.begin() + 5, deck.begin() + 7);
    std::vector<Card> holeB(deck.begin() + 7, deck.begin() + 9);

    std::vector<Card> sevenA = board, sevenB = board;
    sevenA.insert(sevenA.end(), holeA.begin(), holeA.end());
    sevenB.insert(sevenB.end(), holeB.begin(), holeB.end());

    HandEvaluation evalA = HandEvaluator(board, holeA).evaluateHand();
    HandEvaluation evalB = HandEvaluator(board, holeB).evaluateHand();
    std::vector<int> refA = scoreSeven(sevenA);
    std::vector<int> refB = scoreSeven(sevenB);

    if (evalA.type != expectedType(refA)) {
      if (++categoryMismatches <= 5) {
        std::cerr << "category mismatch: engine "
                  << handTypeToString(evalA.type) << " vs reference "
                  << handTypeToString(expectedType(refA)) << " for";
        for (const Card &c : sevenA) {
          std::cerr << ' ' << c.toString();
        }
        std::cerr << '\n';
      }
    }
    if (engineCompare(evalA, evalB) != refCompare(refA, refB)) {
      if (++orderMismatches <= 5) {
        std::cerr << "order mismatch (engine " << engineCompare(evalA, evalB)
                  << ", ref " << refCompare(refA, refB) << "):";
        for (const Card &c : sevenA) {
          std::cerr << ' ' << c.toString();
        }
        std::cerr << " |";
        for (const Card &c : sevenB) {
          std::cerr << ' ' << c.toString();
        }
        std::cerr << '\n';
      }
    }
  }
  check(categoryMismatches == 0, "hand categories match reference (" +
                                     std::to_string(categoryMismatches) +
                                     " mismatches)");
  check(orderMismatches == 0, "hand ordering matches reference (" +
                                  std::to_string(orderMismatches) +
                                  " mismatches)");
}

} // namespace

int main() {
  directedTests();
  randomizedCrossCheck();
  if (failures == 0) {
    std::cout << "hand_eval_test: all checks passed\n";
    return 0;
  }
  std::cout << "hand_eval_test: " << failures << " check(s) failed\n";
  return 1;
}
