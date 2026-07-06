#ifndef GAMEIO_H
#define GAMEIO_H

#include "Card.h"

#include <string>
#include <vector>

struct PlayerView {
  std::string name;
  std::string style; // bot personality, or "you" for the user
  std::vector<Card> cards;
  int chips = 0;
  int roundBet = 0;
  bool folded = false;
  bool allIn = false;
  bool isUser = false;
  bool isActing = false;
};

struct PotView {
  int amount = 0;
  bool isMain = true;
};

struct GameSnapshot {
  std::string roundName;
  std::vector<Card> tableCards;
  std::vector<PotView> pots;
  std::vector<PlayerView> players;
  std::vector<Card> userHand;
  int currentBet = 0;
  bool showdown = false;
};

struct BetRequest {
  int currentBet = 0;
  int minBet = 0; // chips required to call
  int chips = 0;
  int roundBet = 0;
  int bigBlind = 0;
};

enum class LogKind {
  Action,  // a player acts: "Bot1 calls 20"
  Section, // a new phase: "Hand #3", "Flop"
  Info,    // neutral commentary
  Alert,   // dramatic events: all in, busting out
  Win,     // payouts and victories
};

struct LogEntry {
  LogKind kind;
  std::string text;
};

// Presentation layer for the game. The engine reports events through this
// interface and blocks on promptBet() until the user has chosen an action.
class GameIO {
public:
  virtual ~GameIO() = default;

  virtual void log(const std::string &message,
                   LogKind kind = LogKind::Action) = 0;
  virtual void updateState(const GameSnapshot &snapshot) = 0;

  // Returns the chips the user puts in: 0 to check/fold, request.chips to go
  // all in, otherwise a value in [request.minBet, request.chips].
  virtual int promptBet(const BetRequest &request) = 0;

  virtual void gameOver(const std::string &winnerName) = 0;
};

void setGameIO(GameIO *io);
GameIO &gameIO();

#endif
