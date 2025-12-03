#ifndef HAND_H
#define HAND_H

struct Hand {

  enum class Type {
    None = 0,
    HighCard,
    Pair,
    TwoPair,
    ThreeOfAKind,
    Straight,
    Flush,
    FullHouse,
    FourOfAKind,
    StraightFlush,
    RoyalFlush
  };

  Type hand;
  Hand(Type hand) : hand(hand) {}
};

#endif
