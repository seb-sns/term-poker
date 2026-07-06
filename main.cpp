#include "BotConfig.h"
#include "ConsoleIO.h"
#include "PokerGame.h"
#include "Tui.h"

#include <cstring>
#include <iostream>

namespace {
void printUsage() {
  std::cerr << "Usage: poker [--classic] [--theme NAME] [--bots LIST]\n"
            << "  --classic      plain-console mode\n"
            << "  --theme NAME   one of: " << TuiIO::themeList() << "\n"
            << "  --bots LIST    comma-separated lineup of 1-6 bots, each\n"
            << "                 [Name=]style[(t=..,a=..,b=..)]\n"
            << "                 styles: " << botStyleList() << "\n"
            << "                 t: tightness 0-2, a: aggression 0-1,\n"
            << "                 b: bluff frequency 0-1\n"
            << "  example: poker --bots \"Vera=shark,maniac,Rocky=rock(t=1.8)\"\n";
}
} // namespace

int main(int argc, char **argv) {
  bool classic = false;
  TuiIO tui;

  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--classic") == 0) {
      classic = true;
    } else if (std::strcmp(argv[i], "--theme") == 0 && i + 1 < argc) {
      if (!tui.setTheme(argv[++i])) {
        std::cerr << "Unknown theme \"" << argv[i]
                  << "\". Available themes: " << TuiIO::themeList() << '\n';
        return 1;
      }
    } else if (std::strcmp(argv[i], "--bots") == 0 && i + 1 < argc) {
      std::vector<BotSpec> specs;
      std::string error;
      if (!parseBotLineup(argv[++i], specs, error)) {
        std::cerr << "Invalid --bots value: " << error << '\n';
        printUsage();
        return 1;
      }
      PokerGame::configureBots(specs);
    } else {
      printUsage();
      return 1;
    }
  }

  if (classic) {
    ConsoleIO io;
    setGameIO(&io);
    PokerGame::getInstance()->playGame();
    return 0;
  }

  setGameIO(&tui);
  tui.run();

  return 0;
}
