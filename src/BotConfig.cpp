#include "BotConfig.h"
#include "BotAlgorithm.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <stdexcept>

namespace {

struct StyleDef {
  BotEvaluator evaluator;
  double tightness;
  double aggression;
  double bluffFrequency;
};

const std::map<std::string, StyleDef> &styles() {
  static const std::map<std::string, StyleDef> all = {
      {"balanced", {BotEvaluator::MonteCarlo, 1.00, 0.40, 0.07}},
      {"shark", {BotEvaluator::MonteCarlo, 1.15, 0.60, 0.12}},
      {"rock", {BotEvaluator::MonteCarlo, 1.50, 0.20, 0.03}},
      {"maniac", {BotEvaluator::MonteCarlo, 0.55, 0.75, 0.22}},
      {"caller", {BotEvaluator::Basic, 0.50, 0.08, 0.02}},
      {"fish", {BotEvaluator::Basic, 0.70, 0.28, 0.09}},
  };
  return all;
}

std::string trim(const std::string &s) {
  size_t begin = s.find_first_not_of(" \t");
  size_t end = s.find_last_not_of(" \t");
  return begin == std::string::npos ? "" : s.substr(begin, end - begin + 1);
}

std::string toLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return s;
}

// Splits on commas that are not inside parentheses.
std::vector<std::string> splitTopLevel(const std::string &text) {
  std::vector<std::string> parts;
  std::string current;
  int depth = 0;
  for (char c : text) {
    if (c == '(') {
      ++depth;
    } else if (c == ')') {
      --depth;
    }
    if (c == ',' && depth == 0) {
      parts.push_back(current);
      current.clear();
    } else {
      current += c;
    }
  }
  parts.push_back(current);
  return parts;
}

bool applyOverride(BotProfile &profile, const std::string &key, double value,
                   std::string &error) {
  if (key == "t" || key == "tightness") {
    if (value < 0.0 || value > 2.0) {
      error = "tightness must be between 0 and 2";
      return false;
    }
    profile.tightness = value;
  } else if (key == "a" || key == "aggression") {
    if (value < 0.0 || value > 1.0) {
      error = "aggression must be between 0 and 1";
      return false;
    }
    profile.aggression = value;
  } else if (key == "b" || key == "bluff") {
    if (value < 0.0 || value > 1.0) {
      error = "bluff must be between 0 and 1";
      return false;
    }
    profile.bluffFrequency = value;
  } else {
    error = "unknown option \"" + key + "\" (use t/tightness, a/aggression, "
                                        "b/bluff)";
    return false;
  }
  return true;
}

bool parseSpec(const std::string &raw, BotSpec &spec, std::string &error) {
  std::string item = trim(raw);
  if (item.empty()) {
    error = "empty bot entry";
    return false;
  }

  size_t eq = item.find('=');
  size_t paren = item.find('(');
  if (eq != std::string::npos && (paren == std::string::npos || eq < paren)) {
    spec.name = trim(item.substr(0, eq));
    item = trim(item.substr(eq + 1));
    if (spec.name.empty()) {
      error = "empty bot name in \"" + raw + "\"";
      return false;
    }
  }

  std::string overrides;
  paren = item.find('(');
  if (paren != std::string::npos) {
    if (item.back() != ')') {
      error = "missing \")\" in \"" + raw + "\"";
      return false;
    }
    overrides = item.substr(paren + 1, item.size() - paren - 2);
    item = trim(item.substr(0, paren));
  }

  auto it = styles().find(toLower(item));
  if (it == styles().end()) {
    error = "unknown bot style \"" + item + "\"";
    return false;
  }

  spec.evaluator = it->second.evaluator;
  spec.profile.style = it->first;
  spec.profile.tightness = it->second.tightness;
  spec.profile.aggression = it->second.aggression;
  spec.profile.bluffFrequency = it->second.bluffFrequency;

  if (!overrides.empty()) {
    for (const std::string &pairText : splitTopLevel(overrides)) {
      std::string kv = trim(pairText);
      size_t kvEq = kv.find('=');
      if (kvEq == std::string::npos) {
        error = "expected key=value in \"" + kv + "\"";
        return false;
      }
      std::string key = toLower(trim(kv.substr(0, kvEq)));
      double value;
      try {
        value = std::stod(trim(kv.substr(kvEq + 1)));
      } catch (const std::exception &) {
        error = "invalid number in \"" + kv + "\"";
        return false;
      }
      if (!applyOverride(spec.profile, key, value, error)) {
        return false;
      }
    }
  }
  return true;
}

} // namespace

std::vector<BotSpec> defaultBotLineup() {
  std::vector<BotSpec> specs;
  std::string error;
  parseBotLineup("balanced,shark,maniac,caller", specs, error);
  return specs;
}

bool parseBotLineup(const std::string &text, std::vector<BotSpec> &out,
                    std::string &error) {
  out.clear();
  for (const std::string &part : splitTopLevel(text)) {
    BotSpec spec;
    if (!parseSpec(part, spec, error)) {
      return false;
    }
    out.push_back(std::move(spec));
  }
  if (out.empty() || out.size() > 6) {
    error = "choose between 1 and 6 bots";
    return false;
  }
  for (size_t i = 0; i < out.size(); ++i) {
    if (out[i].name.empty()) {
      out[i].name = "Bot" + std::to_string(i + 1);
    }
  }
  return true;
}

std::string botStyleList() {
  std::string names;
  for (const auto &entry : styles()) {
    if (!names.empty()) {
      names += ", ";
    }
    names += entry.first;
  }
  return names;
}

std::unique_ptr<BotAlgorithm> makeBotAlgorithm(const BotSpec &spec) {
  switch (spec.evaluator) {
  case BotEvaluator::Basic:
    return std::make_unique<BasicHandStrength>(spec.profile);
  case BotEvaluator::MonteCarlo:
  default:
    return std::make_unique<MonteCarloHandStrength>(spec.profile);
  }
}
