#ifndef POKERGAME_H
#define POKERGAME_H

#include "Deck.h"
#include "Player.h"
#include "Pot.h"

#include <memory>
#include <vector>

class PokerGame {
public:
  enum class Round { preflop, flop, turn, river, showdown };

  void playGame();

  std::vector<Card> getTableCards() const { return tableCards; }
  Round getRound() const { return currentRound; }
  PotManager getPot() const { return potManager; }
  int getBigBlind() const { return bigBlind; }
  int getSmallBlind() const { return smallBlind; }
  int getCurrentBet() const { return currentBet; }
  int getActivePlayerCount() const;

  static PokerGame *getInstance();

private:
  static PokerGame *gameInstance;
  PokerGame();

  std::vector<std::unique_ptr<Player>> players;
  std::vector<Player *> activePlayers;
  Deck deck;
  std::vector<Card> tableCards;
  PotManager potManager;
  int currentBet;
  int bigBlind;
  int smallBlind;
  Round currentRound;

  void initializePlayers();
  void dealHoleCards();
  void playBettingRound();
  void dealTableCards(int count);
  void resetForNextHand();
  void showTableCards();
  void displayPots();
  void displayGameState();

  bool handlePlayerAction(Player *player, int playerIndex, int playersActed);
  int getPlayerBet(Player *player);
  void advanceRound();
  void collectBlinds();

  void determineWinner();
};

#endif
