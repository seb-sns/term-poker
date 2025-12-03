#ifndef DECK_H
#define DECK_H

#include "Card.h"
#include <random>
#include <vector>

class Deck {
private:
  std::vector<Card> cards;
  std::random_device rd;
  std::mt19937 gen;

public:
  Deck();

  std::vector<Card> getCards() { return cards; }

  void shuffle();
  Card dealCard();
  void removeCard(Card card);
  bool isEmpty() const;
  int size() const;
  void reset();

  Deck(Deck &other) noexcept;
  Deck(Deck &&other) noexcept;
  Deck &operator=(Deck &&other) noexcept;
};

#endif
