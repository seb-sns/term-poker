#include "User.h"
#include <iostream>
#include <limits>

User::User(const std::string &name) : Player(name) {}

void User::takeTurn() { displayHand(); }

bool User::makeDecision() {
  char choice;
  while (true) {
    std::cout << "Fold (f) or check/raise (b): ";
    std::cin >> choice;
    choice = std::tolower(choice);
    if (choice == 'f') {
      return true;
    }

    if (choice == 'b') {
      return false;
    }

    std::cout << "Invalid input\n";
  }
}

int User::placeBet(int currentBet) {
  int minBet{currentBet - roundBet};
  std::cout << name << ", current bet is " << currentBet << ", minimum bet is "
            << minBet << std::endl;
  std::cout << "Your chips: " << chipCount << std::endl;
  std::cout << "Your current bet this round: " << roundBet << std::endl;

  return getValidBet(currentBet, minBet);
}

int User::getValidBet(int currentBet, int minBet) {
  int bet;
  do {
    std::cout << "Enter your bet (0 to check/fold, " << minBet
              << " to call, or more to raise): ";
    std::cin >> bet;

    if (std::cin.fail()) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "Invalid input" << std::endl;
      bet = -1;
    } else if (bet == chipCount) {
      allIn = true;
    } else if (bet > chipCount) {
      std::cout << "Not enough chips" << std::endl;
      bet = -1;
    } else if (bet != 0 && bet < minBet) {
      std::cout << "Bet must be at least " << minBet << " or 0 to fold."
                << std::endl;
      bet = -1;
    }
  } while (bet < 0);

  return bet;
}

void User::displayHand() const {
  std::cout << "Your Hand: ";
  for (const auto &card : hand) {
    std::cout << card.toString() << " ";
  }
  std::cout << std::endl;
}
