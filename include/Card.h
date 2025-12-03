#ifndef CARD_H
#define CARD_H

#include <ostream>
#include <string>

class Card {
public:
  enum class Suit { Clubs, Diamonds, Hearts, Spades };

  enum class Rank {
    Two = 2,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine,
    Ten,
    Jack,
    Queen,
    King,
    Ace
  };

  Card();
  Card(Suit suit, Rank rank);

  Suit getSuit() const { return suit; };
  Rank getRank() const { return rank; };

  std::string toString() const;

  bool operator==(const Card &other) const;
  bool operator<(const Card &other) const;
  bool operator>(const Card &other) const;

private:
  Suit suit;
  Rank rank;
};

std::ostream &operator<<(std::ostream &os, const Card &card);
#endif
