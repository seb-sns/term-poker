#include "ConsoleIO.h"

#include <iomanip>
#include <iostream>
#include <limits>

void ConsoleIO::log(const std::string &message, LogKind kind) {
  if (kind == LogKind::Section) {
    std::cout << "\n--- " << message << " ---" << std::endl;
  } else {
    std::cout << message << std::endl;
  }
}

void ConsoleIO::updateState(const GameSnapshot &snapshot) {
  latest = snapshot;
}

void ConsoleIO::printGameState() const {
  std::cout << "\n╔════════════════════════════════════════════╗\n";
  std::cout << "║                POKER GAME STATE            ║\n";
  std::cout << "╠════════════════════════════════════════════╣\n";
  std::cout << "║                                            ║\n";

  std::cout << "║                " << latest.roundName << ": ";
  for (const auto &card : latest.tableCards) {
    std::cout << card.toString() << " ";
  }
  std::cout << std::endl;

  std::cout << "║                ";
  for (const PotView &pot : latest.pots) {
    std::cout << (pot.isMain ? "Main pot: " : "Side pot: ") << pot.amount
              << "  ";
  }
  std::cout << std::endl;

  std::cout << "║                                            ║\n";
  std::cout << "╠════════════════════════════════════════════╣\n";
  std::cout << "║                PLAYER STATUS               ║\n";
  std::cout << "╠════════════════════════════════════════════╣\n";

  for (const PlayerView &player : latest.players) {
    std::cout << "║ ";

    if (player.folded) {
      std::cout << std::left << std::setw(15) << player.name << "│ FOLDED"
                << std::setw(19) << ""
                << " ║\n";
    } else {
      std::cout << std::left << std::setw(15) << player.name
                << "│ Bet: " << std::setw(4) << player.roundBet
                << " │ Chips: " << std::setw(6) << player.chips << " ║\n";
    }
  }

  std::cout << "╚════════════════════════════════════════════╝\n\n";
}

int ConsoleIO::promptBet(const BetRequest &request) {
  printGameState();

  std::cout << "Your Hand: ";
  for (const auto &card : latest.userHand) {
    std::cout << card.toString() << " ";
  }
  std::cout << std::endl;

  std::cout << "Current bet is " << request.currentBet << ", minimum bet is "
            << request.minBet << std::endl;
  std::cout << "Your chips: " << request.chips << std::endl;
  std::cout << "Your current bet this round: " << request.roundBet
            << std::endl;

  int bet;
  do {
    std::cout << "Enter your bet (0 to check/fold, " << request.minBet
              << " to call, or more to raise): ";
    std::cin >> bet;

    if (std::cin.fail()) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "Invalid input" << std::endl;
      bet = -1;
    } else if (bet == request.chips) {
      // All in is always allowed, even below the minimum bet.
    } else if (bet > request.chips) {
      std::cout << "Not enough chips" << std::endl;
      bet = -1;
    } else if (bet != 0 && bet < request.minBet) {
      std::cout << "Bet must be at least " << request.minBet << " or 0 to fold."
                << std::endl;
      bet = -1;
    }
  } while (bet < 0);

  return bet;
}

void ConsoleIO::gameOver(const std::string & /*winnerName*/) {
  std::cout << "=== Thank you for playing! ===" << std::endl;
}
