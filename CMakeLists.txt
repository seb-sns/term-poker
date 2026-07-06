# CMakeLists.txt

cmake_minimum_required(VERSION 3.14)
project(Poker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG v5.0.0
)
FetchContent_MakeAvailable(ftxui)

# Include headers from "include/"
include_directories(include)

# Add the source files

set(SOURCES
    main.cpp
    src/Card.cpp
    src/ConsoleIO.cpp
    src/Deck.cpp
    src/GameIO.cpp
    src/HandEvaluator.cpp
    src/Player.cpp
    src/PokerGame.cpp
    src/Tui.cpp
    src/User.cpp
    src/HandUtils.cpp
    src/Bot.cpp
    src/BotAlgorithm.cpp
    src/BotConfig.cpp
    src/Pot.cpp
  )

add_executable(poker ${SOURCES})

find_package(Threads REQUIRED)
target_link_libraries(poker
  PRIVATE ftxui::component ftxui::dom ftxui::screen Threads::Threads)
