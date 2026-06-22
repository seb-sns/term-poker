#include "Card.h"
#include <ostream>
#include <string>

Card::Card() : suit(Suit::Clubs), rank(Rank::Two) {}
Card::Card(Suit suit, Rank rank) : suit(suit), rank(rank) {}

std::string Card::toString() const {
  std::string rankStr;
  switch (rank) {
  case Rank::Jack:
    rankStr = "J";
    break;
  case Rank::Queen:
    rankStr = "Q";
    break;
  case Rank::King:
    rankStr = "K";
    break;
  case Rank::Ace:
    rankStr = "A";
    break;
  default:
    rankStr = std::to_string(static_cast<int>(rank));
    break;
  }

  std::string suitStr;
  switch (suit) {
  case Suit::Clubs:
    suitStr = "♣";
    break;
  case Suit::Diamonds:
    suitStr = "♦";
    break;
  case Suit::Hearts:
    suitStr = "♥";
    break;
  case Suit::Spades:
    suitStr = "♠";
    break;
  }

  return rankStr + suitStr;
}

bool Card::operator==(const Card &other) const {
  return suit == other.suit && rank == other.rank;
}

bool Card::operator<(const Card &other) const {
  if (rank != other.rank) {
    return rank < other.rank;
  }
  return suit < other.suit;
}

bool Card::operator>(const Card &other) const {
  if (rank != other.rank) {
    return rank > other.rank;
  }
  return suit > other.suit;
}
