#include "PokerGame.h"
#include "GameIO.h"
#include "Hand.h"
#include "HandEvaluation.h"
#include "HandEvaluator.h"
#include "HandUtils.h"

#include "Bot.h"
#include "BotAlgorithm.h"
#include "User.h"

#include <algorithm>
#include <string>

PokerGame *PokerGame::gameInstance = nullptr;

namespace {
std::vector<BotSpec> &pendingBots() {
  static std::vector<BotSpec> specs = defaultBotLineup();
  return specs;
}
} // namespace

void PokerGame::configureBots(const std::vector<BotSpec> &specs) {
  pendingBots() = specs;
}

PokerGame *PokerGame::getInstance() {
  if (gameInstance == nullptr) {
    gameInstance = new PokerGame();
  }

  return gameInstance;
}

void PokerGame::resetInstance() {
  delete gameInstance;
  gameInstance = nullptr;
}

PokerGame::PokerGame()
    : currentBet(0), bigBlind(20), smallBlind(10),
      currentRound(Round::preflop) {
  initializePlayers();
}

void PokerGame::initializePlayers() {
  players.emplace_back(std::make_unique<User>("Player1"));
  for (const BotSpec &spec : pendingBots()) {
    players.emplace_back(
        std::make_unique<Bot>(spec.name, makeBotAlgorithm(spec), *this));
  }

  for (auto &player : players) {
    activePlayers.push_back(player.get());
  }
}

namespace {
std::string roundToString(PokerGame::Round round) {
  switch (round) {
  case PokerGame::Round::preflop:
    return "Pre-flop";
  case PokerGame::Round::flop:
    return "Flop";
  case PokerGame::Round::turn:
    return "Turn";
  case PokerGame::Round::river:
    return "River";
  case PokerGame::Round::showdown:
    return "Showdown";
  }
  return "Cards";
}
} // namespace

void PokerGame::playGame() {
  constexpr int handsPerBlindLevel = 12;

  gameIO().log("Texas Hold'em", LogKind::Section);
  while (players.size() >= 2) {
    ++handNumber;
    if (handNumber > 1 && handNumber % handsPerBlindLevel == 1) {
      // Tournament-style escalation keeps late heads-up play from
      // stalling forever on tiny blinds.
      smallBlind *= 2;
      bigBlind *= 2;
      gameIO().log("Blinds up: " + std::to_string(smallBlind) + "/" +
                       std::to_string(bigBlind),
                   LogKind::Alert);
    }
    gameIO().log("Hand #" + std::to_string(handNumber), LogKind::Section);
    resetForNextHand();
    if (players.size() < 2) {
      break;
    }
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

  gameIO().log(players[0]->getName() + " wins the game", LogKind::Win);
  gameIO().gameOver(players[0]->getName());
}

void PokerGame::publishState(const Player *acting) {
  GameSnapshot snapshot;
  snapshot.roundName = roundToString(currentRound);
  snapshot.tableCards = tableCards;
  snapshot.currentBet = currentBet;
  snapshot.showdown = currentRound == Round::showdown;

  for (const Pot &pot : potManager.pots) {
    snapshot.pots.push_back({pot.amount, pot.isMain});
  }

  for (Player *player : activePlayers) {
    PlayerView view;
    view.name = player->getName();
    if (const Bot *bot = dynamic_cast<const Bot *>(player)) {
      view.style = bot->getStyle();
    } else {
      view.style = "you";
    }
    view.cards = player->getCards();
    view.chips = player->getChipCount();
    view.roundBet = player->getRoundBet();
    view.folded = player->hasFolded();
    view.allIn = player->isAllIn();
    view.isUser = dynamic_cast<User *>(player) != nullptr;
    view.isActing = player == acting;
    if (view.isUser) {
      snapshot.userHand = view.cards;
    }
    snapshot.players.push_back(std::move(view));
  }

  gameIO().updateState(snapshot);
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
      gameIO().log(player->getName() + " is out of chips", LogKind::Alert);
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
  int smallBlindPosted = std::min(smallBlind, activePlayers[0]->getChipCount());
  int bigBlindPosted = std::min(bigBlind, activePlayers[1]->getChipCount());

  gameIO().log(activePlayers[0]->getName() + " posts small blind " +
               std::to_string(smallBlindPosted));
  gameIO().log(activePlayers[1]->getName() + " posts big blind " +
               std::to_string(bigBlindPosted));

  activePlayers[0]->addChips(-smallBlindPosted);
  activePlayers[0]->adjustRoundBet(smallBlindPosted);
  activePlayers[0]->adjustTotalBet(smallBlindPosted);
  if (activePlayers[0]->getChipCount() == 0) {
    activePlayers[0]->setAllIn(true);
  }
  activePlayers[1]->addChips(-bigBlindPosted);
  activePlayers[1]->adjustRoundBet(bigBlindPosted);
  activePlayers[1]->adjustTotalBet(bigBlindPosted);
  if (activePlayers[1]->getChipCount() == 0) {
    activePlayers[1]->setAllIn(true);
  }
  potManager.addToMainPot(smallBlindPosted + bigBlindPosted);
  currentBet = bigBlindPosted;
}

void PokerGame::dealHoleCards() {
  for (int i = 0; i < 2; i++) {
    for (auto *player : activePlayers) {
      player->addCard(deck.dealCard());
    }
  }
  publishState();
}

void PokerGame::dealTableCards(int count) {
  for (int i = 0; i < count; i++) {
    tableCards.push_back(deck.dealCard());
  }
  publishState();
}

void PokerGame::playBettingRound() {
  gameIO().log(roundToString(currentRound), LogKind::Section);

  int startingPlayer =
      (currentRound == Round::preflop && activePlayers.size() > 2) ? 2 : 0;
  int currentPlayer = (startingPlayer);
  int playersActed = 0;
  std::vector<bool> hasActed(activePlayers.size(), false);

  if (currentRound != Round::preflop) {
    currentBet = 0;
  }
  publishState();

  while (activePlayers.size() > 1 &&
         playersActed < static_cast<int>(activePlayers.size())) {
    Player *player = activePlayers[currentPlayer];

    if (!player->hasFolded() && player->isAllIn()) {
      hasActed[currentPlayer] = true;
      playersActed++;
    }

    else if (!player->hasFolded() && !hasActed[currentPlayer]) {
      if (handlePlayerAction(player)) {
        hasActed[currentPlayer] = true;
        playersActed++;
      } else {
        std::fill(hasActed.begin(), hasActed.end(), false);
        hasActed[currentPlayer] = true;
        playersActed = 1;
      }
      publishState();
    } else if (!player->hasFolded() && hasActed[currentPlayer]) {
      playersActed++;
    }

    currentPlayer = (currentPlayer + 1) % activePlayers.size();

    if (getActivePlayerCount() <= 1)
      break;
  }
}

bool PokerGame::handlePlayerAction(Player *player) {
  if (dynamic_cast<User *>(player)) {
    publishState(player);
  }

  int minBet{currentBet - player->getRoundBet()};
  int playerBet{std::min(getPlayerBet(player), player->getChipCount())};
  if (playerBet == 0 && minBet > 0) {
    player->setFolded(true);
    potManager.foldPlayer(player);
    gameIO().log(player->getName() + " folds");
    return true;
  }
  if (playerBet == 0) {
    gameIO().log(player->getName() + " checks");
    return true;
  }

  // Chips move straight into the pot; side pots are settled at showdown
  // from each player's total contribution.
  player->adjustRoundBet(playerBet);
  player->adjustTotalBet(playerBet);
  player->addChips(-playerBet);
  potManager.addToMainPot(playerBet);
  if (player->getChipCount() == 0) {
    player->setAllIn(true);
  }

  int bet{player->getRoundBet()};
  if (player->isAllIn()) {
    gameIO().log(player->getName() + " is all in", LogKind::Alert);
  }
  if (bet > currentBet) {
    currentBet = bet;
    gameIO().log(player->getName() + " raises to " + std::to_string(bet));
    return false;
  }
  gameIO().log(player->getName() + " calls " + std::to_string(bet));
  return true;
}

int PokerGame::getPlayerBet(Player *player) {
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
}

int PokerGame::getActivePlayerCount() const {
  return std::count_if(activePlayers.begin(), activePlayers.end(),
                       [](Player *p) { return !p->hasFolded(); });
}

void PokerGame::determineWinner() {
  gameIO().log("Showdown", LogKind::Section);
  std::vector<Player *> handPlayers;
  for (const auto &player : players) {
    handPlayers.push_back(player.get());
  }
  potManager.buildShowdownPots(handPlayers);
  publishState();

  for (Pot &pot : potManager.pots) {
    if (pot.eligiblePlayers.size() <= 1) {
      continue;
    }
    HandEvaluation currentBestHand{Hand::Type::None, std::vector<Card>{},
                                   std::vector<Card>{}};
    std::set<Player *> winningPlayers;
    for (Player *player : pot.eligiblePlayers) {
      if (player->hasFolded()) {
        continue;
      }
      HandEvaluator playerEvaluator{
          HandEvaluator(tableCards, player->getCards())};
      HandEvaluation playerHand{playerEvaluator.evaluateHand()};

      bool playerBeatsBest = isEqualOrBetter(playerHand, currentBestHand);
      bool bestBeatsPlayer = isEqualOrBetter(currentBestHand, playerHand);

      if (playerBeatsBest && !bestBeatsPlayer) {
        currentBestHand = playerHand;
        winningPlayers.clear();
        winningPlayers.insert(player);
      } else if (playerBeatsBest && bestBeatsPlayer) {
        winningPlayers.insert(player);
      }
    }
    pot.eligiblePlayers = winningPlayers;
    pot.winningHand = handTypeToString(currentBestHand.type);
  }

  potManager.payOutPots();
  publishState();
}
