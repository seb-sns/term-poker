#include "Tui.h"
#include "PokerGame.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <thread>

#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/color.hpp"

using namespace ftxui;

namespace {

// Game pacing levels, selectable at runtime with -/+.
constexpr int kSpeedDelaysMs[] = {900, 350, 120, 0};
constexpr const char *kSpeedNames[] = {"slow", "normal", "fast", "turbo"};
constexpr int kSpeedLevels = 4;

constexpr size_t kMaxLogLines = 200;

struct Theme {
  std::string name;
  Color felt;        // table background
  Color feltText;    // labels drawn on the felt
  Color accent;      // titles, section headers, focus marker
  Color gold;        // chip amounts, wins
  Color alert;       // all in, busting out
  Color cardSurface; // card face background, slightly raised from the UI
  Color cardEdge;    // card face outline
  Color suitRed;     // hearts and diamonds
  Color suitLight;   // spades and clubs, on the dark surface
  Color cardBack;    // face-down cards and empty slots
  Color buttonText; // label color on action buttons
  Color btnFold;
  Color btnCall;
  Color btnRaise;
  Color btnAllIn;
  std::vector<Color> seats; // per-player identity colors
};

const std::vector<Theme> &themes() {
  static const std::vector<Theme> all = {
      {
          "Emerald",
          Color::RGB(17, 87, 43),    // felt
          Color::RGB(210, 224, 210), // feltText
          Color::RGB(214, 178, 68),  // accent
          Color::RGB(240, 202, 96),  // gold
          Color::RGB(255, 138, 74),  // alert
          Color::RGB(36, 48, 40),    // cardSurface
          Color::RGB(118, 144, 126), // cardEdge
          Color::RGB(242, 108, 98),  // suitRed
          Color::RGB(232, 230, 220), // suitLight
          Color::RGB(96, 138, 108),  // cardBack
          Color::RGB(244, 241, 230), // buttonText
          Color::RGB(141, 48, 48),   // fold
          Color::RGB(41, 87, 148),   // call
          Color::RGB(42, 118, 69),   // raise
          Color::RGB(182, 121, 32),  // all in
          {Color::RGB(126, 200, 255), Color::RGB(255, 172, 133),
           Color::RGB(168, 226, 160), Color::RGB(221, 170, 226),
           Color::RGB(233, 219, 143), Color::RGB(178, 198, 228)},
      },
      {
          "Midnight",
          Color::RGB(25, 35, 68),    // felt
          Color::RGB(198, 208, 228), // feltText
          Color::RGB(122, 186, 255), // accent
          Color::RGB(196, 208, 228), // gold (cool silver)
          Color::RGB(255, 128, 128), // alert
          Color::RGB(37, 46, 74),    // cardSurface
          Color::RGB(112, 130, 172), // cardEdge
          Color::RGB(255, 122, 138), // suitRed
          Color::RGB(226, 232, 245), // suitLight
          Color::RGB(93, 111, 156),  // cardBack
          Color::RGB(233, 238, 246), // buttonText
          Color::RGB(122, 46, 78),   // fold
          Color::RGB(44, 76, 138),   // call
          Color::RGB(35, 110, 112),  // raise
          Color::RGB(146, 110, 46),  // all in
          {Color::RGB(140, 196, 255), Color::RGB(255, 168, 168),
           Color::RGB(150, 222, 194), Color::RGB(212, 172, 240),
           Color::RGB(240, 220, 150), Color::RGB(255, 190, 140)},
      },
      {
          "Crimson",
          Color::RGB(76, 22, 28),    // felt
          Color::RGB(226, 208, 200), // feltText
          Color::RGB(235, 172, 84),  // accent
          Color::RGB(233, 184, 95),  // gold
          Color::RGB(255, 110, 90),  // alert
          Color::RGB(62, 40, 42),    // cardSurface
          Color::RGB(158, 122, 114), // cardEdge
          Color::RGB(250, 116, 102), // suitRed
          Color::RGB(236, 226, 214), // suitLight
          Color::RGB(154, 100, 92),  // cardBack
          Color::RGB(243, 234, 224), // buttonText
          Color::RGB(70, 70, 82),    // fold
          Color::RGB(146, 74, 34),   // call
          Color::RGB(152, 44, 56),   // raise
          Color::RGB(196, 132, 34),  // all in
          {Color::RGB(255, 178, 130), Color::RGB(150, 206, 255),
           Color::RGB(196, 222, 150), Color::RGB(232, 168, 190),
           Color::RGB(238, 214, 140), Color::RGB(196, 190, 226)},
      },
      {
          "Carbon",
          Color::RGB(34, 36, 34),    // felt
          Color::RGB(198, 206, 198), // feltText
          Color::RGB(133, 218, 133), // accent
          Color::RGB(212, 222, 180), // gold
          Color::RGB(255, 150, 96),  // alert
          Color::RGB(46, 50, 46),    // cardSurface
          Color::RGB(126, 136, 126), // cardEdge
          Color::RGB(236, 108, 108), // suitRed
          Color::RGB(226, 228, 222), // suitLight
          Color::RGB(104, 112, 104), // cardBack
          Color::RGB(230, 234, 228), // buttonText
          Color::RGB(88, 60, 60),    // fold
          Color::RGB(62, 78, 94),    // call
          Color::RGB(58, 96, 62),    // raise
          Color::RGB(128, 104, 52),  // all in
          {Color::RGB(150, 216, 150), Color::RGB(226, 178, 150),
           Color::RGB(150, 196, 216), Color::RGB(206, 170, 206),
           Color::RGB(216, 210, 150), Color::RGB(170, 186, 202)},
      },
  };
  return all;
}

bool equalsIgnoreCase(const std::string &a, const std::string &b) {
  return a.size() == b.size() &&
         std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) {
           return std::tolower(static_cast<unsigned char>(x)) ==
                  std::tolower(static_cast<unsigned char>(y));
         });
}

Element cardFace(const Card &card, const Theme &t) {
  bool red = card.getSuit() == Card::Suit::Hearts ||
             card.getSuit() == Card::Suit::Diamonds;
  return text(card.toString()) | bold |
         color(red ? t.suitRed : t.suitLight) | center |
         size(WIDTH, EQUAL, 4) | size(HEIGHT, EQUAL, 1) |
         borderStyled(ROUNDED, t.cardEdge) | bgcolor(t.cardSurface);
}

Element cardBack(const Theme &t) {
  return text("░░") | center | size(WIDTH, EQUAL, 4) | size(HEIGHT, EQUAL, 1) |
         borderRounded | color(t.cardBack);
}

Element cardSlot(const Theme &t) {
  return text("  ") | center | size(WIDTH, EQUAL, 4) | size(HEIGHT, EQUAL, 1) |
         borderRounded | color(t.cardBack) | dim;
}

// "Bot1 raises to 240" -> seat-colored name, plain verb, gold amount.
Element actionLine(const std::string &line, const Theme &t,
                   const std::map<std::string, int> &seatColors) {
  Elements parts;
  size_t rest = 0;

  size_t space = line.find(' ');
  if (space != std::string::npos) {
    auto it = seatColors.find(line.substr(0, space));
    if (it != seatColors.end()) {
      parts.push_back(text(line.substr(0, space)) |
                      color(t.seats[it->second % t.seats.size()]));
      rest = space;
    }
  }

  size_t numStart = line.size();
  while (numStart > rest &&
         std::isdigit(static_cast<unsigned char>(line[numStart - 1]))) {
    --numStart;
  }
  parts.push_back(text(line.substr(rest, numStart - rest)));
  if (numStart < line.size()) {
    parts.push_back(text(line.substr(numStart)) | color(t.gold));
  }
  return hbox(std::move(parts));
}

} // namespace

std::string TuiIO::themeList() {
  std::string names;
  for (const Theme &t : themes()) {
    if (!names.empty()) {
      names += ", ";
    }
    names += t.name;
  }
  return names;
}

bool TuiIO::setTheme(const std::string &name) {
  const auto &all = themes();
  for (size_t i = 0; i < all.size(); ++i) {
    if (equalsIgnoreCase(all[i].name, name)) {
      themeIndex = static_cast<int>(i);
      return true;
    }
  }
  return false;
}

void TuiIO::postRedraw() {
  if (screen != nullptr) {
    screen->PostEvent(Event::Custom);
  }
}

void TuiIO::log(const std::string &message, LogKind kind) {
  {
    std::lock_guard<std::mutex> lock(mtx);
    logLines.push_back({kind, message});
    if (logLines.size() > kMaxLogLines) {
      logLines.erase(logLines.begin());
    }
  }
  postRedraw();
  // Pace the game so bot actions appear one at a time instead of all at
  // once; pausing holds the engine thread here between actions.
  std::unique_lock<std::mutex> lock(mtx);
  int delay = kSpeedDelaysMs[speedIndex];
  if (delay > 0) {
    cvPace.wait_for(lock, std::chrono::milliseconds(delay),
                    [this] { return paused; });
  }
  cvPace.wait(lock, [this] { return !paused; });
}

void TuiIO::togglePause() {
  {
    std::lock_guard<std::mutex> lock(mtx);
    paused = !paused;
  }
  cvPace.notify_all();
  postRedraw();
}

void TuiIO::adjustSpeed(int delta) {
  {
    std::lock_guard<std::mutex> lock(mtx);
    speedIndex = std::clamp(speedIndex + delta, 0, kSpeedLevels - 1);
  }
  cvPace.notify_all();
  postRedraw();
}

void TuiIO::updateState(const GameSnapshot &newSnapshot) {
  {
    std::lock_guard<std::mutex> lock(mtx);
    snapshot = newSnapshot;
    for (const PlayerView &player : snapshot.players) {
      if (seatColorIndex.find(player.name) == seatColorIndex.end()) {
        seatColorIndex[player.name] = nextSeatColor++;
      }
    }
  }
  postRedraw();
}

int TuiIO::promptBet(const BetRequest &newRequest) {
  std::unique_lock<std::mutex> lock(mtx);
  request = newRequest;

  // The smallest raise is a call plus one big blind; the slider never needs
  // to reach a plain call because the Call button covers that.
  raiseMin = std::min(request.minBet + request.bigBlind, request.chips);
  raiseMax = request.chips;
  raiseAmount = raiseMin;
  canCall = request.minBet > 0 && request.minBet < request.chips;
  canRaise = request.chips > request.minBet;
  foldLabel = request.minBet > 0 ? "Fold" : "Check";
  callLabel = "Call " + std::to_string(request.minBet);
  allInLabel = "All in (" + std::to_string(request.chips) + ")";

  betSubmitted = false;
  promptActive = true;

  lock.unlock();
  if (screen != nullptr && resetFocus) {
    screen->Post(resetFocus);
  }
  postRedraw();
  lock.lock();

  cv.wait(lock, [this] { return betSubmitted; });
  promptActive = false;

  lock.unlock();
  postRedraw();
  return betResult;
}

void TuiIO::submitAction(Action action) {
  {
    std::lock_guard<std::mutex> lock(mtx);
    if (!promptActive || betSubmitted) {
      return;
    }
    switch (action) {
    case Action::Fold:
      betResult = 0;
      break;
    case Action::Call:
      betResult = std::min(request.minBet, request.chips);
      break;
    case Action::Raise:
      betResult = std::min(std::max(raiseAmount, raiseMin), raiseMax);
      break;
    case Action::AllIn:
      betResult = request.chips;
      break;
    }
    betSubmitted = true;
  }
  cv.notify_all();
}

void TuiIO::gameOver(const std::string &winner) {
  {
    std::lock_guard<std::mutex> lock(mtx);
    winnerName = winner;
    finished = true;
  }
  postRedraw();
}

void TuiIO::run() {
  auto screenInteractive = ScreenInteractive::Fullscreen();
  screen = &screenInteractive;

  std::thread gameThread([] { PokerGame::getInstance()->playGame(); });

  auto themedButton = [this](Color Theme::*bg) {
    ButtonOption option;
    option.transform = [this, bg](const EntryState &s) {
      const Theme &t = themes()[themeIndex];
      std::string label =
          s.focused ? "▸ " + s.label + " ◂" : "  " + s.label + "  ";
      Element e = text(label) | center | borderEmpty | bgcolor(t.*bg) |
                  color(t.buttonText);
      if (s.focused) {
        e = e | bold;
      }
      return e;
    };
    return option;
  };

  auto foldButton = Button(
      &foldLabel, [this] { submitAction(Action::Fold); },
      themedButton(&Theme::btnFold));
  auto callButton = Button(
      &callLabel, [this] { submitAction(Action::Call); },
      themedButton(&Theme::btnCall));
  auto raiseSlider = Slider("", &raiseAmount, &raiseMin, &raiseMax, 1);
  auto raiseButton = Button(
      &raiseLabel, [this] { submitAction(Action::Raise); },
      themedButton(&Theme::btnRaise));
  auto allInButton = Button(
      &allInLabel, [this] { submitAction(Action::AllIn); },
      themedButton(&Theme::btnAllIn));

  auto maybeCall = Maybe(callButton, &canCall);
  auto actionRow = Container::Horizontal({
      foldButton,
      maybeCall,
      Maybe(Container::Horizontal({raiseSlider, raiseButton}), &canRaise),
      allInButton,
  });
  auto actionBar = Maybe(actionRow, &promptActive);

  // Each new prompt starts with the safest action focused, so Enter
  // calls/checks rather than repeating whatever was focused last time.
  resetFocus = [this, actionRow, maybeCall, foldButton] {
    actionRow->SetActiveChild(canCall ? maybeCall : foldButton);
  };

  auto quitButton = Button("Quit", screenInteractive.ExitLoopClosure(),
                           themedButton(&Theme::btnFold));
  auto quitBar = Maybe(quitButton, &finished);

  auto root = Container::Vertical({actionBar, quitBar});

  auto ui = Renderer(root, [&] {
    std::lock_guard<std::mutex> lock(mtx);
    const Theme &t = themes()[themeIndex];

    // --- Header ---
    Element pauseBadge =
        paused ? text(" ⏸ PAUSED ") | bold | color(t.alert)
               : text(std::string(" ") + kSpeedNames[speedIndex] + " ") | dim;
    Element header =
        hbox({
            text(" ♠ ") | color(t.accent),
            text("♥ ") | color(t.suitRed),
            text("TEXAS HOLD'EM") | bold | color(t.accent),
            text(" ♦ ") | color(t.suitRed),
            text("♣ ") | color(t.accent),
            filler(),
            text(snapshot.roundName.empty() ? "Starting..."
                                            : snapshot.roundName) |
                bold | color(t.accent),
            text("  · ") | dim,
            pauseBadge,
            text(" ·  space pause  ·  -/+ speed  ·  t theme  ·  q quit ") |
                dim,
        }) |
        border;

    // --- Community cards + pots, on the felt ---
    Elements community;
    for (const Card &card : snapshot.tableCards) {
      community.push_back(cardFace(card, t));
    }
    for (size_t i = snapshot.tableCards.size(); i < 5; ++i) {
      community.push_back(cardSlot(t));
    }

    Elements potParts;
    for (const PotView &pot : snapshot.pots) {
      if (!potParts.empty()) {
        potParts.push_back(text("   ") | color(t.feltText));
      }
      potParts.push_back(text(pot.isMain ? "Pot " : "Side pot ") |
                         color(t.feltText));
      potParts.push_back(text(std::to_string(pot.amount)) | bold |
                         color(t.gold));
    }
    if (potParts.empty()) {
      potParts.push_back(text("Pot ") | color(t.feltText));
      potParts.push_back(text("0") | bold | color(t.gold));
    }
    if (snapshot.currentBet > 0) {
      potParts.push_back(text("   bet to match ") | color(t.feltText));
      potParts.push_back(text(std::to_string(snapshot.currentBet)) | bold |
                         color(t.gold));
    }

    Element table = vbox({
                        filler(),
                        hbox(community) | hcenter,
                        hbox(potParts) | hcenter,
                        filler(),
                    }) |
                    bgcolor(t.felt) | flex;

    // --- Players ---
    Elements playerRows;
    for (const PlayerView &player : snapshot.players) {
      Color seat = t.seats[seatColorIndex[player.name] % t.seats.size()];

      Element marker =
          text(player.isActing ? " ▶ " : "   ") | color(t.accent);
      Element name = text(player.name) | color(seat) |
                     size(WIDTH, EQUAL, 12);
      if (player.isUser) {
        name = name | bold;
      }
      Element style = text(player.style) | dim | size(WIDTH, EQUAL, 10);

      Elements holeCards;
      bool reveal = player.isUser || (snapshot.showdown && !player.folded);
      for (const Card &card : player.cards) {
        holeCards.push_back(reveal ? cardFace(card, t) : cardBack(t));
      }
      while (holeCards.size() < 2) {
        holeCards.push_back(cardSlot(t));
      }

      Element status;
      if (player.folded) {
        status = text("folded") | dim;
      } else if (player.allIn) {
        status = text("ALL IN") | bold | color(t.alert);
      } else {
        status = hbox({text("bet ") | dim, text(std::to_string(
                                               player.roundBet)) |
                                               color(t.gold)});
      }

      Element row = hbox({
          vbox({
              filler(),
              hbox({marker, name, style,
                    hbox({text("⛁ ") | dim,
                          text(std::to_string(player.chips)) |
                              color(t.gold)}) |
                        size(WIDTH, EQUAL, 10),
                    status}),
              filler(),
          }),
          filler(),
          hbox(holeCards),
      });
      if (player.folded) {
        row = row | dim;
      }
      playerRows.push_back(row);
    }
    Element playersPane =
        window(text(" Players ") | bold | color(t.accent), vbox(playerRows)) |
        flex;

    // --- Action log ---
    Elements logElements;
    for (const LogEntry &entry : logLines) {
      switch (entry.kind) {
      case LogKind::Section:
        if (!logElements.empty()) {
          logElements.push_back(text(""));
        }
        logElements.push_back(text("── " + entry.text + " ──") | bold |
                              color(t.accent));
        break;
      case LogKind::Action:
        logElements.push_back(
            hbox({text(" "), actionLine(entry.text, t, seatColorIndex)}));
        break;
      case LogKind::Info:
        logElements.push_back(text(" " + entry.text) | dim);
        break;
      case LogKind::Alert:
        logElements.push_back(text(" " + entry.text) | bold |
                              color(t.alert));
        break;
      case LogKind::Win:
        logElements.push_back(text(" " + entry.text) | bold | color(t.gold));
        break;
      }
    }
    if (!logElements.empty()) {
      logElements.back() = logElements.back() | focus;
    }
    Element logPane = window(text(" Action log ") | bold | color(t.accent),
                             vbox(logElements) | yframe) |
                      size(WIDTH, EQUAL, 42);

    // --- Bottom: action bar / status ---
    raiseLabel = (request.currentBet > 0 ? "Raise to " : "Bet ") +
                 std::to_string(request.roundBet + raiseAmount);

    Element bottom;
    if (finished) {
      bottom = window(text(" Game over ") | bold | color(t.accent),
                      hbox({
                          text(" 🏆 " + winnerName + " wins the game ") |
                              bold | color(t.gold) | vcenter,
                          filler(),
                          quitButton->Render(),
                      }));
    } else if (promptActive) {
      Elements actionElements = {foldButton->Render()};
      if (canCall) {
        actionElements.push_back(callButton->Render());
      }
      if (canRaise) {
        actionElements.push_back(
            hbox({
                raiseSlider->Render() | color(t.accent) | vcenter |
                    size(WIDTH, EQUAL, 24),
                raiseButton->Render(),
            }));
      }
      actionElements.push_back(allInButton->Render());
      actionElements.push_back(filler());
      actionElements.push_back(text("Tab move · Enter act ") | dim | vcenter);
      bottom = window(text(" Your turn ") | bold | color(t.accent),
                      hbox(std::move(actionElements)));
    } else {
      bottom = window(text(" Waiting ") | dim,
                      text(snapshot.userHand.empty()
                               ? " You are out of the hand — spectating. "
                               : " Waiting for other players... ") |
                          dim);
    }

    return vbox({
        header,
        table | size(HEIGHT, GREATER_THAN, 7),
        hbox({playersPane, logPane}) | flex,
        bottom,
    });
  });

  ui = CatchEvent(ui, [&](Event event) {
    if (event == Event::Character('q') || event == Event::Character('Q')) {
      screenInteractive.Exit();
      return true;
    }
    if (event == Event::Character('t') || event == Event::Character('T')) {
      themeIndex = (themeIndex + 1) % static_cast<int>(themes().size());
      return true;
    }
    if (event == Event::Character(' ') || event == Event::Character('p') ||
        event == Event::Character('P')) {
      togglePause();
      return true;
    }
    if (event == Event::Character('+') || event == Event::Character('=')) {
      adjustSpeed(+1);
      return true;
    }
    if (event == Event::Character('-') || event == Event::Character('_')) {
      adjustSpeed(-1);
      return true;
    }
    return false;
  });

  screenInteractive.Loop(ui);

  bool gameStillRunning;
  {
    std::lock_guard<std::mutex> lock(mtx);
    gameStillRunning = !finished;
  }
  if (gameStillRunning) {
    // The engine thread is blocked mid-hand; tearing it down cleanly isn't
    // possible, so end the process directly.
    gameThread.detach();
    std::_Exit(0);
  }
  gameThread.join();
}
