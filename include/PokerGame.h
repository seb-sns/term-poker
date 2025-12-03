#ifndef POKERGAME_H
#define POKERGAME_H

#include "Deck.h"
#include "Player.h"
#include "Pot.h"

#include <deque>
#include <map>
#include <memory>
#include <utility>
#include <vector>

class PokerGame {
public:
  enum class Round { preflop, flop, turn, river, showdown };

  void playGame();

  std::vector<Card> getTableCards() const { return tableCards; }
  Round getRound() const { return currentRound; }
  PotManager getPot() const { return potManager; }
  std::deque<std::pair<int, int>> getSidePot() const { return sidePots; }
  int getBigBlind() const { return bigBlind; }
  int getSmallBlind() const { return smallBlind; }
  int getCurrentBet() const { return currentBet; }
  int getNPlayers() const { return activePlayers.size(); }

  static PokerGame *getInstance();

private:
  static PokerGame *gameInstance;
  PokerGame();

  std::vector<std::unique_ptr<Player>> players;
  std::vector<Player *> activePlayers;
  Deck deck;
  std::vector<Card> tableCards;
  PotManager potManager;
  std::deque<std::pair<int, int>> sidePots;
  int currentBet;
  int bigBlind;
  int smallBlind;
  Round currentRound;

  void initiliazePlayers();
  void dealHoleCards();
  void playBettingRound();
  void dealTableCards(int count);
  void resetForNextHand();
  void showTableCards();
  void displayPots();
  void displayGameState();

  bool handlePlayerAction(Player *player, int playerIndex, int playersActed);
  int getPlayerBet(Player *player, int minBet);
  void addToMainPot(int amount);
  void adjustSidePots(int amount);

  void advanceRound();
  bool isBettingComplete() const;
  void collectBlinds();
  int getActivePlayerCount() const;
  std::vector<Player *> getPlayersWhoCanBet() const;

  void determineWinner();
};

#endif
