#ifndef CONSOLEIO_H
#define CONSOLEIO_H

#include "GameIO.h"

class ConsoleIO : public GameIO {
public:
  void log(const std::string &message, LogKind kind) override;
  void updateState(const GameSnapshot &snapshot) override;
  int promptBet(const BetRequest &request) override;
  void gameOver(const std::string &winnerName) override;

private:
  void printGameState() const;

  GameSnapshot latest;
};

#endif
