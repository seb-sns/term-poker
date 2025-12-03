#include "Player.h"
#include <algorithm>

Player::Player(const std::string &name)
  : name(name), chipCount(1000), folded(false) {}

void Player::addCard(const Card &card) {
  hand.push_back(card);
}

void Player::addCards(const std::vector<Card> &cards) {
  hand.insert(hand.end(), cards.begin(), cards.end());
}

void Player::clearHand() {
  hand.clear();
  folded=false;
}
