# dprisk
Basic dynamic programming calculation for the board game Risk

## Overview
Computes the probability of the attacker winning a single battle, for any combination of the number of attacker $(a)$ and defender $(d)$ army units $(a,d)$, for specified ranges $0\leq a\leq A$, $0\leq d\leq D$. Optionally also runs simulation for $(A,D)$ to verify the DP calculation. The DP code `dprisk.cpp` is standard `C++`.

## Demonstration script
The `python` script `dprisk-demo.py` shows how to run the program and visualizes the solution output.

![Solution map calculated by `dprisk.cpp`](/readme-figures/dprisk-demo-70x75.png)
