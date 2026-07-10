#ifndef POKERGAME_H
#define POKERGAME_H

#include "BotConfig.h"
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
  const PotManager &getPot() const { return potManager; }
  int getBigBlind() const { return bigBlind; }
  int getSmallBlind() const { return smallBlind; }
  int getCurrentBet() const { return currentBet; }
  int getActivePlayerCount() const;

  static PokerGame *getInstance();

  // Sets the bot lineup for the game; call before the first getInstance().
  static void configureBots(const std::vector<BotSpec> &specs);

  // Destroys the current game so a fresh one can be created (for tests).
  static void resetInstance();

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
  int handNumber = 0;
  Round currentRound;

  void initializePlayers();
  void dealHoleCards();
  void playBettingRound();
  void dealTableCards(int count);
  void resetForNextHand();
  void publishState(const Player *acting = nullptr);

  bool handlePlayerAction(Player *player);
  int getPlayerBet(Player *player);
  void advanceRound();
  void collectBlinds();

  void determineWinner();
};

#endif
