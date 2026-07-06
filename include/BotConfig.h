#ifndef BOTCONFIG_H
#define BOTCONFIG_H

#include <memory>
#include <string>
#include <vector>

class BotAlgorithm;

// Personality knobs driving a bot's decisions.
struct BotProfile {
  std::string style;           // preset name, shown in the UI
  double tightness = 1.0;      // [0,2] fold threshold; higher folds more
  double aggression = 0.5;     // [0,1] how often and how big it raises
  double bluffFrequency = 0.1; // [0,1] chance to play a weak hand anyway
};

enum class BotEvaluator {
  MonteCarlo, // simulates opponent hands to estimate win probability
  Basic,      // only looks at its current made hand
};

struct BotSpec {
  std::string name;
  BotEvaluator evaluator;
  BotProfile profile;
};

// The lineup used when --bots is not given.
std::vector<BotSpec> defaultBotLineup();

// Parses a lineup like "Vera=shark,maniac,Rocky=rock(t=1.8,a=0.3)".
// Each entry is [Name=]style[(k=v,...)] with k in t/a/b (or tightness/
// aggression/bluff). Returns false and sets error on invalid input.
bool parseBotLineup(const std::string &text, std::vector<BotSpec> &out,
                    std::string &error);

std::string botStyleList();

std::unique_ptr<BotAlgorithm> makeBotAlgorithm(const BotSpec &spec);

#endif
