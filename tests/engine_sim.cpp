// Plays full games with randomized user actions and checks money invariants
// on every state update: chips and pots are never negative, and
// chips + pots always add up to the chips that entered the game.

#include "BotConfig.h"
#include "GameIO.h"
#include "PokerGame.h"

#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace {

class SimIO : public GameIO {
public:
  explicit SimIO(unsigned seed) : rng(seed) {}

  int failures = 0;
  long logCount = 0;

  void startGame() {
    expectedTotal = -1; // learned from the first snapshot
  }

  void log(const std::string &message, LogKind kind) override {
    ++logCount;
    if (kind == LogKind::Win && message.find(" wins -") != std::string::npos) {
      fail("negative payout: " + message);
    }
  }

  void updateState(const GameSnapshot &snapshot) override {
    int total = 0;
    for (const PlayerView &player : snapshot.players) {
      if (player.chips < 0) {
        fail(player.name + " has negative chips: " +
             std::to_string(player.chips));
      }
      total += player.chips;
    }
    for (const PotView &pot : snapshot.pots) {
      if (pot.amount < 0) {
        fail("negative pot: " + std::to_string(pot.amount));
      }
      total += pot.amount;
    }
    if (expectedTotal < 0) {
      expectedTotal = total;
    } else if (total != expectedTotal) {
      fail("chips not conserved: " + std::to_string(total) + " != " +
           std::to_string(expectedTotal));
    }
  }

  int promptBet(const BetRequest &request) override {
    int roll = static_cast<int>(rng() % 100);
    if (request.minBet >= request.chips) {
      // Calling costs the whole stack: all in or fold.
      return roll < 50 ? request.chips : 0;
    }
    if (roll < 20) {
      return 0; // fold or check
    }
    if (roll < 75) {
      return request.minBet; // call or check
    }
    if (roll < 95) {
      int extra = request.bigBlind * (1 + static_cast<int>(rng() % 5));
      return std::min(request.minBet + extra, request.chips);
    }
    return request.chips; // all in
  }

  void gameOver(const std::string &) override {}

private:
  void fail(const std::string &message) {
    ++failures;
    if (failures <= 10) {
      std::cerr << "FAIL: " << message << '\n';
    }
  }

  std::mt19937 rng;
  int expectedTotal = -1;
};

} // namespace

int main(int argc, char **argv) {
  int games = argc > 1 ? std::stoi(argv[1]) : 60;

  SimIO io(20260707);
  setGameIO(&io);

  // Mostly fast basic-evaluator bots; a couple of Monte Carlo lineups to
  // exercise that path as well.
  std::vector<std::string> lineups = {
      "caller,fish,caller,fish",
      "fish,fish,fish,fish,fish,caller",
      "caller,fish",
      "balanced,maniac",
  };

  for (int game = 0; game < games; ++game) {
    const std::string &lineup =
        lineups[game < 4 ? game : game % 3]; // Monte Carlo lineup only once
    std::vector<BotSpec> specs;
    std::string error;
    if (!parseBotLineup(lineup, specs, error)) {
      std::cerr << "FAIL: bad lineup \"" << lineup << "\": " << error << '\n';
      return 1;
    }
    PokerGame::resetInstance();
    PokerGame::configureBots(specs);
    io.startGame();
    long logStart = io.logCount;
    PokerGame::getInstance()->playGame();
    if (io.logCount - logStart > 2000000) {
      std::cerr << "FAIL: game " << game << " looks stuck\n";
      return 1;
    }
    if (io.failures > 0) {
      std::cerr << "engine_sim: invariant violations in game " << game
                << " (lineup " << lineup << ")\n";
      return 1;
    }
  }

  std::cout << "engine_sim: " << games
            << " games played, all invariants held\n";
  return 0;
}
