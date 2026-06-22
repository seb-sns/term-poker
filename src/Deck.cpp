#include "Deck.h"
#include <algorithm>
#include <stdexcept>

Deck::Deck() : gen(rd()) { reset(); }

void Deck::reset() {
  cards.clear();
  cards.reserve(52);

  for (int suit = static_cast<int>(Card::Suit::Clubs);
       suit <= static_cast<int>(Card::Suit::Spades); suit++) {
    for (int rank = static_cast<int>(Card::Rank::Two);
         rank <= static_cast<int>(Card::Rank::Ace); rank++) {
      cards.emplace_back(static_cast<Card::Suit>(suit),
                         static_cast<Card::Rank>(rank));
    }
  }
}

void Deck::shuffle() { std::shuffle(cards.begin(), cards.end(), gen); }

Card Deck::dealCard() {
  if (cards.empty()) {
    throw std::runtime_error("Cannot deal cards from an empty deck");
  }
  Card card = cards.back();
  cards.pop_back();
  return card;
}

void Deck::removeCard(Card card) {
  auto iter = std::find(cards.begin(), cards.end(), card);
  if (iter != cards.end()) {
    cards.erase(iter);
  } else {
    throw std::runtime_error("Cannot remove card that does not exist in deck");
  }
}

bool Deck::isEmpty() const { return cards.empty(); }

int Deck::size() const { return static_cast<int>(cards.size()); }

Deck::Deck(Deck &other)
    : cards(other.cards), gen(std::random_device{}()) {}

Deck::Deck(const Deck &other)
    : cards(other.cards), gen(std::random_device{}()) {}

Deck::Deck(Deck &&other) noexcept
    : cards(std::move(other.cards)), gen(std::random_device{}()) {}

Deck &Deck::operator=(Deck &&other) noexcept {
  if (this != &other) {
    cards = std::move(other.cards);
    gen = std::mt19937(std::random_device{}());
  }
  return *this;
}

Deck &Deck::operator=(const Deck &other) {
  if (this != &other) {
    cards = other.cards;
    gen = std::mt19937(std::random_device{}());
  }
  return *this;
}
