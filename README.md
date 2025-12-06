<h1 align="center">
  Terminal Poker
</h1>

<p align="center">
  <img src="https://img.shields.io/badge/C++-17-blue.svg" alt="C++17">
</p>

<p align="center">
  A simple terminal-based no limits Texas hold 'em poker game
</p>

<p align="center">
  <img src="screenshot.png">
</p>

<h2>
  Build instructions
</h2>
<h3>
  Requirements
</h3>

- CMake 3.10+
- C++17 compatible compiler
  
<h3>
  Build
</h3>

    mkdir build && cd build
    cmake ..
    cmake --build
    
<h3>
  Run
</h3>
  
    ./poker
    
<h2>
  How to play
</h2>
<p>
  When it is you turn to play you will be prompted to either fold call or raise.<br>
  You will fold if you enter 0 (while the minimum bet is higher than 0).<br>
  You will call if you enter the current minimum bet.<br>
  You will raise if you enter above the minimum bet.<br>

  The game has full support for correct hand evaluations when considering high cards/kickers, pot-splitting and going all-in.
</p>

<h3>
  Bot behaviour
</h3>
<p>
  Bots use a monte carlo simulation to assess the strength of their hand and will play/place bets depending on their own level of pacificity.
</p>
