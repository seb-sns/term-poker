# CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(MyCppProject)

set(CMAKE_CXX_STANDARD 17)

# Include headers from "include/"
include_directories(include)

# Add the source files

set(SOURCES
    main.cpp
    src/Card.cpp
    src/Deck.cpp
    src/HandEvaluator.cpp
    src/Player.cpp
    src/PokerGame.cpp
    src/User.cpp
    src/HandUtils.cpp
    src/Bot.cpp
    src/BotAlgorithm.cpp
    src/Pot.cpp
  )

add_executable(my_app ${SOURCES})

