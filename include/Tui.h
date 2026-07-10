#ifndef TUI_H
#define TUI_H

#include "GameIO.h"

#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace ftxui {
class ScreenInteractive;
}

// FTXUI frontend. run() owns the UI loop on the calling thread and plays the
// game on a worker thread; the engine blocks in promptBet() until the user
// picks an action in the UI.
class TuiIO : public GameIO {
public:
  void run();

  void log(const std::string &message, LogKind kind) override;
  void updateState(const GameSnapshot &snapshot) override;
  int promptBet(const BetRequest &request) override;
  void gameOver(const std::string &winnerName) override;

  bool setTheme(const std::string &name);
  static std::string themeList();

private:
  enum class Action { Fold, Call, Raise, AllIn };
  void submitAction(Action action);
  void togglePause();
  void adjustSpeed(int delta);
  void postRedraw();

  std::mutex mtx;
  std::condition_variable cv;
  ftxui::ScreenInteractive *screen = nullptr;

  GameSnapshot snapshot;
  std::vector<LogEntry> logLines;
  std::map<std::string, int> seatColorIndex; // stable per-player log colors
  int nextSeatColor = 0;

  BetRequest request;
  bool promptActive = false;
  bool betSubmitted = false;
  int betResult = 0;

  bool finished = false;
  std::string winnerName;
  std::function<void()> resetFocus;

  int themeIndex = 0;

  // Game pacing: the engine thread waits in log() on cvPace while paused.
  std::condition_variable cvPace;
  bool paused = false;
  int speedIndex = 1;

  // UI widget state
  int raiseAmount = 0;
  int raiseMin = 0;
  int raiseMax = 0;
  bool canCall = false;
  bool canRaise = false;
  std::string foldLabel = "Fold";
  std::string callLabel = "Call";
  std::string raiseLabel = "Raise";
  std::string allInLabel = "All in";
};

#endif
