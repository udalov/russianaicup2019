russianaicup2019
================

My solution of Russian AI Cup 2019 -- CodeSide (http://2019.russianaicup.ru/)

Results:
* Round 1 ([v2](bc7bec6)) -- 69th place, 79.3% wins
* Round 2 ([v7](ef88794)) -- 55th place, 73.9% wins
* Finals ([v9](627d241)) -- 28rd place, 51.7% wins
* Sandbox overall -- 28th place

To configure and compile the project, install [CMake](https://cmake.org) and run:

    LOCAL=1 cmake -S src -B out
    make -Cout -j4

To run one game, download and unpack local runner from the web site, and run:

    ./c

for a default game versus the Quick start solution on complex level, with team size 2 and seed 42.

Optionally, set specific variables for custom games, pass arguments for seed and game duration, and `--vis` for debug info in the visualizer:

    P1=Local P2=Local TEAM_SIZE=1 LEVEL=levels/level.txt ./c 43 3600 --vis
