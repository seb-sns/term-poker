#include "PokerGame.h"
#include "Hand.h"
#include "HandEvaluation.h"
#include "HandEvaluator.h"
#include "HandUtils.h"

#include "Bot.h"
#include "BotAlgorithm.h"
#include "User.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>

PokerGame *PokerGame::gameInstance = nullptr;

PokerGame *PokerGame::getInstance() {
  if (gameInstance == nullptr) {
    gameInstance = new PokerGame();
  }

  return gameInstance;
}

PokerGame::PokerGame()
    : currentBet(0), bigBlind(20), smallBlind(10),
      currentRound(Round::preflop) {
  initiliazePlayers();
}

void PokerGame::initiliazePlayers() {
  players.emplace_back(std::make_unique<User>("Player1"));
  players.emplace_back(std::make_unique<Bot>(
      "Bot1", std::make_unique<MonteCarloHandStrength>(0.4), *this));
  players.emplace_back(std::make_unique<Bot>(
      "Bot2", std::make_unique<MonteCarloHandStrength>(0.6), *this));
  players.emplace_back(std::make_unique<Bot>(
      "Bot3", std::make_unique<BasicHandStrength>(0.4), *this));
  players.emplace_back(std::make_unique<Bot>(
      "Bot4", std::make_unique<BasicHandStrength>(0.6), *this));

  for (auto &player : players) {
    activePlayers.push_back(player.get());
  }
}

void PokerGame::playGame() {
  std::cout << "=== Texas Hold'em Poker ===" << std::endl;
  while (players.size() >= 2) {
    std::cout << "\n--- New hand ---" << std::endl;
    std::set<Player *> activePlayersSet(activePlayers.begin(),
                                        activePlayers.end());
    potManager.createMainPot(activePlayersSet);
    resetForNextHand();
    collectBlinds();

    // preflop
    dealHoleCards();
    playBettingRound();

    if (getActivePlayerCount() > 1) {
      // Flop
      advanceRound();
      dealTableCards(3);
      playBettingRound();
    }
    if (getActivePlayerCount() > 1) {
      // Turn
      advanceRound();
      dealTableCards(1);
      playBettingRound();
    }
    if (getActivePlayerCount() > 1) {
      // River
      advanceRound();
      dealTableCards(1);
      playBettingRound();
    }

    advanceRound();
    determineWinner();
  }

  std::cout << "Winner is: " << players[0]->getName() << '\n' << '\n';
  std::cout << "=== Thank you for playing! ===" << std::endl;
}

void PokerGame::resetForNextHand() {
  deck.reset();
  deck.shuffle();
  tableCards.clear();
  currentBet = 0;
  currentRound = Round::preflop;

  activePlayers.clear();

  for (const auto &player : players) {
    if (player->getChipCount() == 0) {
      std::cout << player->getName() << " has gone bust!\n";
    }
  }

  players.erase(std::remove_if(players.begin(), players.end(),
                               [](const std::unique_ptr<Player> &player) {
                                 return (player->getChipCount() == 0);
                               }),
                players.end());

  std::rotate(players.begin(), players.end() - 1, players.end());

  for (auto &player : players) {
    player->clearHand();
    player->resetTotalBet();
    player->setFolded(false);
    player->setAllIn(false);
    player->resetRoundBet();
    activePlayers.push_back(player.get());
  }

  potManager.clearPots();
  std::set<Player *> activePlayersSet(activePlayers.begin(),
                                      activePlayers.end());
  potManager.createMainPot(activePlayersSet);
}

void PokerGame::collectBlinds() {
  std::cout << "\n--- Collecting Blinds ---" << std::endl;
  std::cout << activePlayers[0]->getName()
            << " posts small blind: " << smallBlind << std::endl;
  std::cout << activePlayers[1]->getName() << " posts big blind: " << bigBlind
            << std::endl;

  activePlayers[0]->addChips(-smallBlind);
  activePlayers[0]->adjustRoundBet(smallBlind);
  activePlayers[1]->addChips(-bigBlind);
  activePlayers[1]->adjustRoundBet(bigBlind);
  potManager.addToMainPot(smallBlind + bigBlind);
  currentBet = bigBlind;
}

void PokerGame::dealHoleCards() {
  std::cout << "\n--- Dealing hole cards ---" << std::endl;
  for (int i = 0; i < 2; i++) {
    for (auto *player : activePlayers) {
      player->addCard(deck.dealCard());
    }
  }
}

void PokerGame::dealTableCards(int count) {
  for (int i = 0; i < count; i++) {
    tableCards.push_back(deck.dealCard());
  }
}

void PokerGame::showTableCards() {
  std::string roundName;
  switch (currentRound) {
  case Round::preflop:
    roundName = "Pre-flop";
    break;
  case Round::flop:
    roundName = "Flop";
    break;
  case Round::turn:
    roundName = "Turn";
    break;
  case Round::river:
    roundName = "River";
    break;
  case Round::showdown:
    roundName = "Showdown";
    break;
  default:
    roundName = "Cards";
    break;
  }

  std::cout << "║                ";
  std::cout << roundName << ": ";
  for (const auto &card : tableCards) {
    std::cout << card.toString() << " ";
  }
  std::cout << std::endl;
}

void PokerGame::playBettingRound() {
  std::cout << "\n--- "
            << (currentRound == Round::preflop ? "Pre-flop"
                : currentRound == Round::flop  ? "Flop"
                : currentRound == Round::turn  ? "Turn"
                                               : "River")
            << " ---" << std::endl;

  int startingPlayer =
      (currentRound == Round::preflop && activePlayers.size() > 2) ? 2 : 0;
  int currentPlayer = (startingPlayer);
  int playersActed = 0;
  std::vector<bool> hasActed(activePlayers.size(), false);

  if (currentRound != Round::preflop) {
    currentBet = 0;
  }

  while (activePlayers.size() > 1 && playersActed < activePlayers.size()) {
    Player *player = activePlayers[currentPlayer];

    if (!player->hasFolded() && player->isAllIn()) {
      hasActed[currentPlayer] = true;
      playersActed++;
    }

    else if (!player->hasFolded() && !hasActed[currentPlayer]) {
      if (handlePlayerAction(player, currentPlayer, playersActed)) {
        hasActed[currentPlayer] = true;
        playersActed++;
      } else {
        std::fill(hasActed.begin(), hasActed.end(), false);
        hasActed[currentPlayer] = true;
        playersActed = 1;
      }
    } else if (!player->hasFolded() && hasActed[currentPlayer]) {
      playersActed++;
    }

    currentPlayer = (currentPlayer + 1) % activePlayers.size();

    if (getActivePlayerCount() <= 1)
      break;
  }
}

bool PokerGame::handlePlayerAction(Player *player, int playerIndex,
                                   int playersActed) {
  std::cout << "\n" << player->getName() << "'s turn: ";

  if (dynamic_cast<User *>(player)) {
    displayGameState();
    player->takeTurn();
    std::cout << std::endl;
  }

  int minBet{currentBet - player->getRoundBet()};
  int playerBet{getPlayerBet(player, currentBet)};
  if (playerBet == 0 && minBet > 0) {
    player->setFolded(true);
    potManager.foldPlayer(player);
    std::cout << player->getName() << " folds." << '\n';
    return true;
  } else if (playerBet == 0) {
    std::cout << player->getName() << " checks." << '\n';
    return true;
  } else if (playerBet > 0 && (player->isAllIn())) {
    std::cout << player->getName() << " goes all in." << '\n';
    player->adjustRoundBet(playerBet);
    player->adjustTotalBet(playerBet);
    player->addChips(-playerBet);
    int bet{player->getRoundBet()};

    if (minBet > bet) {
      if (potManager.pots.size() == 1) {
        potManager.addToMainPot(playerBet);
        potManager.createSidePot(player, potManager.pots[0], playersActed);
        std::cerr << "\n";
        return true;
      }
      int index = potManager.determineNewSidePot(player);
      if (!(index == 0)) {
        potManager.pots[index].amount += playerBet;
        potManager.addToSidePot(playerBet, index);
        return true;
      }
      index = potManager.findNewPotSplitLocation(player);
      potManager.pots[index].amount += playerBet;
      potManager.createSidePot(player, potManager.pots[index], playersActed);
      return true;
    } else if (bet > currentBet) {
      potManager.addToMainPot(playerBet);
      currentBet = bet;
      std::cout << player->getName() << " raises to " << bet << "."
                << std::endl;
      return false;
    } else {
      potManager.addToMainPot(playerBet);
      std::cout << player->getName() << " calls " << bet << "." << std::endl;
      return true;
    }

  } else {
    player->adjustRoundBet(playerBet);
    player->adjustTotalBet(playerBet);
    player->addChips(-playerBet);
    potManager.addToMainPot(playerBet);

    int bet{player->getRoundBet()};

    if (bet > currentBet) {
      currentBet = bet;
      std::cout << player->getName() << " raises to " << bet << "."
                << std::endl;
      return false;
    } else {
      std::cout << player->getName() << " calls " << bet << "." << std::endl;
      return true;
    }
  }

  return true;
}

int PokerGame::getPlayerBet(Player *player, int minBet) {
  if (player->hasFolded())
    return 0;

  if (dynamic_cast<Bot *>(player)) {
    if (player->makeDecision()) {
      int bet{player->placeBet(currentBet)};
      return bet;
    }
    return 0;
  }

  int bet{player->placeBet(currentBet)};
  return bet;
}

void PokerGame::addToMainPot(int amount) {
  potManager.pots[0].amount += amount;
}

void PokerGame::adjustSidePots(int amount) {
  int pot;
  int potMinimum;
  int delta;
  for (auto &currentPot : sidePots) {
    pot = currentPot.first;
    potMinimum = currentPot.second;
    delta = potMinimum - amount;
    if (delta == 0) {
      currentPot.first += amount;
      return;
    } else if (delta > 0) {
      sidePots.emplace_front(delta, amount);
      return;
    }
  }
  sidePots.back().first = delta;
  sidePots.emplace_back(amount, amount);
  return;
}

void PokerGame::advanceRound() {
  switch (currentRound) {
  case Round::preflop:
    currentRound = Round::flop;
    break;
  case Round::flop:
    currentRound = Round::turn;
    break;
  case Round::turn:
    currentRound = Round::river;
    break;
  case Round::river:
    currentRound = Round::showdown;
    break;
  default:
    break;
  }
  for (auto &player : players) {
    player->resetRoundBet();
  }
  for (Pot &pot : potManager.pots) {
    pot.setRoundBeginAmount(pot.amount);
  }
}

int PokerGame::getActivePlayerCount() const {
  return std::count_if(activePlayers.begin(), activePlayers.end(),
                       [](Player *p) { return !p->hasFolded(); });
}

void PokerGame::determineWinner() {
  std::cout << "\n--- Showdown ---" << std::endl;

  for (Pot &pot : potManager.pots) {
    if (pot.eligiblePlayers.size() == 1) {
      continue;
    }
    HandEvaluation currentBestHand{Hand::Type::None, std::vector<Card>{},
                                   std::vector<Card>{}};
    std::set<Player *> winningPlayers;
    for (Player *player : pot.eligiblePlayers) {
      bool isBetter = false;
      bool isEqual = false;
      HandEvaluator playerEvaluator{
          HandEvaluator(tableCards, player->getCards())};
      HandEvaluation playerHand{playerEvaluator.evaluateHand()};
      if (playerHand.type > currentBestHand.type) {
        isBetter = true;
      }

      else if (playerHand.type == currentBestHand.type) {
        if (isHigherKickerPrimary(playerHand.primaryCards,
                                  currentBestHand.primaryCards)) {
          std::cerr << player->getName() << " has a better primary kicker"
                    << '\n';
          isBetter = true;
        } else if (playerHand.primaryCards.size() != 5 &&
                   isHigherKickerSecondary(playerHand.secondaryCards,
                                           currentBestHand.secondaryCards)) {
          std::cerr << player->getName() << " has a better secondary kicker"
                    << '\n';
          isBetter = true;

        } else if (!isHigherKickerPrimary(currentBestHand.primaryCards,
                                          playerHand.primaryCards)) {
          // Since we know that player hand is not better than the current best
          // hand
          std::cerr << player->getName() << "has a equal primary kicker"
                    << '\n';
          isEqual = true;
        }

        else if (playerHand.primaryCards.size() != 5 &&
                 !isHigherKickerSecondary(currentBestHand.primaryCards,
                                          playerHand.primaryCards)) {
          // Since we know that player hand is not better than the current best
          // hand
          std::cerr << player->getName() << "has a equal secondary kicker"
                    << '\n';
          isEqual = true;
        }
      }

      if (isBetter) {
        currentBestHand = playerHand;
        winningPlayers.clear();
        winningPlayers.insert(player);
      } else if (isEqual) {
        winningPlayers.insert(player);
      }
    }
    pot.eligiblePlayers = winningPlayers;
    std::cout << "Best hand was ";
    printHandType(currentBestHand.type);
    std::cout << std::endl;
  }

  potManager.payOutPots();
  std::cout << std::endl;

  std::cout << "Next round..." << std::endl;
}

void PokerGame::displayPots() {
  std::cout << "║                ";
  for (Pot pot : potManager.pots) {
    if (pot.isMain) {
      std::cout << "Main pot: " << pot.amount << "  ";
    } else {
      std::cout << "Side pot: " << pot.amount << "  ";
    }
  }
  std::cout << std::endl;
}

void PokerGame::displayGameState() {
  void clearScreen();

  std::cout << "\n╔════════════════════════════════════════════╗\n";
  std::cout << "║                POKER GAME STATE            ║\n";
  std::cout << "╠════════════════════════════════════════════╣\n";
  std::cout << "║                                            ║\n";
  showTableCards();
  displayPots();
  std::cout << "║                                            ║\n";
  std::cout << "╠════════════════════════════════════════════╣\n";
  std::cout << "║                PLAYER STATUS               ║\n";
  std::cout << "╠════════════════════════════════════════════╣\n";

  for (size_t i = 0; i < activePlayers.size(); ++i) {
    auto *player = activePlayers[i];
    std::cout << "║ ";

    if (player->hasFolded()) {
      std::cout << std::left << std::setw(15) << player->getName() << "│ FOLDED"
                << std::setw(19) << ""
                << " ║\n";
    } else {
      std::cout << std::left << std::setw(15) << player->getName()
                << "│ Bet: " << std::setw(4) << player->getRoundBet()
                << " │ Chips: " << std::setw(6) << player->getChipCount()
                << " ║\n";
    }
  }

  std::cout << "╚════════════════════════════════════════════╝\n\n";
}
