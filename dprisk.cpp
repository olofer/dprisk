/*
 * Dynamic programming computation of the (simplified) probability 
 * of winning battles in the board game "Risk" (rules: Hasbro 1963).
 *
 * COMPILE: g++ -Wall -O2 -o dprisk dprisk.cpp
 * USAGE: ./dprisk A D [N] [> tablefilename.txt]
 *
 * A = number of army units for attacker
 * D = number of army units for defender
 * N = (optional) number of simulation samples (to spot-check DP solution)
 *
 * The program computes the probability of attacker winning for the entire
 * [0..A] x [0..D] set of (A + 1) * (D + 1) combinations. The boundary conditions
 * for the array of numbers are: P(A > 0, D = 0) = 1, P(A = 0|1, D > 0) = 0.
 *
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <map>
//#include <chrono>
#include <random>

/*
  Let the state of the battle be 
    (a, d)

  The attacker has "a" army units and defender has "d" units.
  The transition probabilities for ending up in the below new states are needed.
    (a - 2, d)
    (a - 1, d - 1)
    (a - 1, d) 
    (a, d - 1) 
    (a, d - 2)

  These transitions depend on the number of dice that each player throws.
  The attacker can throw at most min(a - 1, 3) dice, where a >= 1.
  The defender can throw at most min(d, 2) dice, where d >= 1.
  If d is equal to 0, the attacker wins. 
  If a is equal to 1, the defender wins (attacker cannot throw any die).
  Compare highest die to each other (defender wins any tie). 
  Then (when applicable) compare next highest die to each other.
  Here assume every player always uses the maximum number of dice.

  These dice tuples require implementation {attacker, defender}.
    {1, 1}
    {2, 1}
    {3, 1}
    {1, 2}
    {2, 2}
    {3, 2}

  Each transition probability is required for each of the possible dice tuples.
  This table is precomputed and stored as a 6-vector of 5-vectors.
*/

int attacker_dice(int a) {
  int q = (a - 1 < 3 ? a - 1 : 3);
  return (q < 0 ? 0 : q);
}

int defender_dice(int d) {
  int q = (d < 2 ? d : 2);
  return (q < 0 ? 0 : q);
}

void sort_three(int a, 
                int b, 
                int c, 
                int* abc)
{
  if (a > b) {
    const int tmp = a;
    a = b;
    b = tmp;
  }
  if (b > c) {
    const int tmp = b;
    b = c;
    c = tmp;
  }
  if (a > b) {
    const int tmp = a;
    a = b;
    b = tmp;
  }
  abc[0] = a;
  abc[1] = b;
  abc[2] = c;
}

/* return 1 if attacker wins, otherwise 0 */
int simulate_battle(int a, int d, int uniform_dice_sides) {

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(1, uniform_dice_sides);

  std::vector<int> a_dice;
  std::vector<int> d_dice;

  while (a > 1 && d > 0) {

    a_dice.clear();
    const int na = attacker_dice(a);
    for (int i = 0; i < na; i++)
      a_dice.push_back(distrib(gen));

    d_dice.clear();
    const int nd = defender_dice(d);
    for (int i = 0; i < nd; i++)
      d_dice.push_back(distrib(gen));

    std::sort(a_dice.begin(), a_dice.end(), std::greater<int>());
    std::sort(d_dice.begin(), d_dice.end(), std::greater<int>());

    const int num_compares = (na > nd ? nd : na);

    for (int i = 0; i < num_compares; i++) {
      if (a_dice[i] > d_dice[i])
        d -= 1;
      else
        a -= 1;
    }
  }

  return (d == 0 ? 1 : 0);
}

/* brute force precalculation by enumeration of the possible transition probabilities */
int calc_transitions(const std::vector<std::vector<int>>& order, 
                     std::vector<int>& counts, 
                     int na, 
                     int nd,
                     int uniform_dice_sides)
{
  const int S = uniform_dice_sides;
  if (S < 2) return 0;

  //std::unordered_map<std::vector<int>, int> M;
  std::map<std::vector<int>, int> M;
  for (size_t i = 0; i < order.size(); i++) {
    std::vector<int> tuple = {order[i][0], order[i][1]};
    M[tuple] = i;
  }

  //for (const auto& n : M) {
  //  std::cout << "(" << n.first[0] << "," << n.first[1] << ") = " << n.second << std::endl;
  //}

  counts.clear();
  for (size_t i = 0; i < order.size(); i++)
    counts.push_back(0);

  int O = 0;

  // Enumerate attacker with i, j, k
  // Enumerate defender with p, q

  if (na == 1 && nd == 1) {
    for (int i = 1; i <= S; i++) {
      for (int p = 1; p <= S; p++) {
        if (i > p)
          counts[M[{0, -1}]] += 1;  // attacker wins, defender looses 1 unit
        if (i <= p)
          counts[M[{-1, 0}]] += 1;  // defender wins
        O += 1;
      }
    }
    return O;
  } 

  if (na == 2 && nd == 1) {
    for (int i = 1; i <= S; i++) {
      for (int j = 1; j <= S; j++) {
        const int ijmax = (i > j ? i : j);
        for (int p = 1; p <= S; p++) {
          if (ijmax > p)
            counts[M[{0, -1}]] += 1;
          if (ijmax <= p)
            counts[M[{-1, 0}]] += 1;
          O += 1;
        }
      }
    }
    return O;
  }

  if (na == 3 && nd == 1) {
    for (int i = 1; i <= S; i++) {
      for (int j = 1; j <= S; j++) {
        const int ijmax = (i > j ? i : j);
        for (int k = 1; k <= S; k++) {
          const int ijkmax = (k > ijmax ? k : ijmax);
          for (int p = 1; p <= S; p++) {
            if (ijkmax > p)
              counts[M[{0, -1}]] += 1;
            if (ijkmax <= p)
              counts[M[{-1, 0}]] += 1;
            O += 1;
          }
        }
      }
    }
    return O;
  }

  if (na == 1 && nd == 2) {
    for (int i = 1; i <= S; i++) {
      for (int p = 1; p <= S; p++) {
        for (int q = 1; q <= S; q++) {
          const int pqmax = (p > q ? p : q);
          if (pqmax >= i)
            counts[M[{-1, 0}]] += 1;
          if (pqmax < i)
            counts[M[{0, -1}]] += 1;
          O += 1;
        }
      }
    }
    return O;
  }

  if (na == 2 && nd == 2) {
    for (int i = 1; i <= S; i++) {
      for (int j = 1; j <= S; j++) {
        const int ijmax = (i > j ? i : j);
        const int ijmin = (i < j ? i : j);
        for (int p = 1; p <= S; p++) {
          for (int q = 1; q <= S; q++) {
            const int pqmax = (p > q ? p : q);
            const int pqmin = (p < q ? p : q);
            if (ijmax > pqmax && ijmin > pqmin)
              counts[M[{0, -2}]] += 1;
            if (ijmax <= pqmax && ijmin > pqmin)
              counts[M[{-1, -1}]] += 1;
            if (ijmax > pqmax && ijmin <= pqmin)
              counts[M[{-1, -1}]] += 1;
            if (ijmax <= pqmax && ijmin <= pqmin)
              counts[M[{-2, 0}]] += 1;
            O += 1;
          }
        }
      }
    }
    return O;
  }

  if (na == 3 && nd == 2) {
    int ijk[3];
    for (int i = 1; i <= S; i++) {
      for (int j = 1; j <= S; j++) {
        const int ijmax = (i > j ? i : j);
        const int ijmin = (i < j ? i : j);
        for (int k = 1; k <= S; k++) {
          const int ijkmax = (k > ijmax ? k : ijmax);
          const int ijkmin = (k < ijmin ? k : ijmin);
          sort_three(i, j, k, ijk);
          if (ijk[0] != ijkmin || ijk[2] != ijkmax)
            break;
          const int ijkmid = ijk[1];
          for (int p = 1; p <= S; p++) {
            for (int q = 1; q <= S; q++) {
              const int pqmax = (p > q ? p : q);
              const int pqmin = (p < q ? p : q);
              if (ijkmax > pqmax && ijkmid > pqmin)
                counts[M[{0, -2}]] += 1;
              if (ijkmax <= pqmax && ijkmid > pqmin)
                counts[M[{-1, -1}]] += 1;
              if (ijkmax > pqmax && ijkmid <= pqmin)
                counts[M[{-1, -1}]] += 1;
              if (ijkmax <= pqmax && ijkmid <= pqmin)
                counts[M[{-2, 0}]] += 1;
              O += 1;
            }
          }
        }
      }
    }
    return O;
  } 

  return 0;
}

bool create_prob_table(std::vector<std::vector<int>>& dicetuples,
                       std::vector<std::vector<int>>& transitions,
                       std::vector<std::vector<double>>& probstable,
                       int uniform_dice_sides,
                       bool verbose = false)
{
  dicetuples.clear();
  transitions.clear();
  probstable.clear();

  const std::vector<std::vector<int>> T = {{1, 1}, {2, 1}, {3, 1}, {1, 2}, {2, 2}, {3, 2}};
  dicetuples = T;

  const std::vector<std::vector<int>> P = {{-2, 0}, {-1, -1}, {-1, 0}, {0, -1}, {0, -2}};
  transitions = P;

  if (verbose) {
    std::cout << "probs order ";
    for (size_t q = 0; q < transitions.size(); q++)
      std::cout << " (" << transitions[q][0] << "," << transitions[q][1] << ")";
    std::cout << std::endl;
  }

  for (size_t t = 0; t < dicetuples.size(); t++) {
    const int na = dicetuples[t][0];
    const int nd = dicetuples[t][1];
    std::vector<int> tcounts;
    int tdenom = calc_transitions(transitions, tcounts, na, nd, uniform_dice_sides);
    if (tdenom == 0) 
      return false;
    probstable.emplace_back();
    if (verbose)
      std::cout << "probs (na=" << na << ",nd=" << nd << ") =";
    int check = 0;
    for (size_t q = 0; q < transitions.size(); q++) {
      check += tcounts[q];
      probstable[t].push_back(static_cast<double>(tcounts[q]) / tdenom);
      if (verbose)
        std::cout << " " << probstable[t][probstable[t].size() - 1];
    }
    if (verbose)
      std::cout << std::endl;
    if (check != tdenom)
      return false;
  }
  return true;
}

/* 0 <= a <= A, 0 <= d <= D */
int linear_index(int a, int A, int d, int D) {
  return (1 + A) * d + a;
}

int update_elements(int A, 
                    int D, 
                    std::vector<double>& P,
                    double unused_value,
                    const std::vector<std::vector<int>>& dicetuples,
                    const std::vector<std::vector<int>>& transitions,
                    const std::vector<std::vector<double>>& probstable)
{
  std::map<std::vector<int>, int> dice_map;
  for (size_t i = 0; i < dicetuples.size(); i++) {
    dice_map[{dicetuples[i][0], dicetuples[i][1]}] = i;
  }

  int num_updated = 0;
  double* data = P.data();

  for (int a = 1; a <= A; a++) {
  //for (int a = A; a >= 1; a--) {
    for (int d = 1; d <= D; d++) {
      if (data[linear_index(a, A, d, D)] != unused_value)
        continue;

      const int na = attacker_dice(a);
      const int nd = defender_dice(d);
      const int q = dice_map[{na, nd}];

      // Try to compute the value of this element by "backward induction"

      double this_val = 0.0;

      for (size_t i = 0; i < transitions.size(); i++) {
        const int delta_a = transitions[i][0];
        const int delta_d = transitions[i][1];
        const double prob_qi = probstable[q][i];

        if (prob_qi == 0)
          continue;

        const int idx = linear_index(a + delta_a, A, d + delta_d, D);

        if (data[idx] == unused_value) {
          this_val = -1.0;
          break;
        }

        this_val += prob_qi * data[idx];
      }

      if (this_val >= 0.0) {
        data[linear_index(a, A, d, D)] = this_val;
        num_updated += 1;
      }

    }
  }
  return num_updated;
}

int main(int argc, char** argv) {

  const int num_text_digits = 16;
  const int uniform_dice_sides = 6;

  if (argc != 3 && argc != 4) {
    std::cout << "usage: " << argv[0] << " attackers defenders [samples]" << std::endl;
    return 1;
  }

  long int tmp = std::strtol(argv[1], nullptr, 0);
  int A = static_cast<int>(tmp);

  tmp = std::strtol(argv[2], nullptr, 0);
  int D = static_cast<int>(tmp);

  int N = 0;
  if (argc > 3) {
    tmp = std::strtol(argv[3], nullptr, 0);
    N = static_cast<int>(tmp);    
  }

  if (A < 2 || D < 1) {
    std::cout << "requiring: A >= 2 and D >= 1" << std::endl;
    return 1;
  }

  if (N >= 1) {
    int num_atk_wins = 0;
    for (int i = 0; i < N; i++) {
      const int atk_win = simulate_battle(A, D, uniform_dice_sides);
      num_atk_wins += atk_win;
    }
    std::cout << std::setprecision(num_text_digits) << static_cast<double>(num_atk_wins) / N << std::endl;
    return 0;
  }

  const double unused_value = -1.0;
  const int sz = (1 + A) * (1 + D);

  std::vector<double> P(sz, 0.0);

  for (int i = 0; i <= A; i++) {
    for (int j = 0; j <= D; j++) {
      int k = linear_index(i, A, j, D);
      P.data()[k] = unused_value;
    }
  }

  // Set boundary conditions

  for (int j = 0; j <= D; j++) {
    P.data()[linear_index(0, A, j, D)] = 0.0;
    P.data()[linear_index(1, A, j, D)] = 0.0;
  }

  for (int i = 2; i <= A; i++) {
    P.data()[linear_index(i, A, 0, D)] = 1.0;
  }

  std::vector<std::vector<int>> dicetuples;
  std::vector<std::vector<int>> transitions;
  std::vector<std::vector<double>> probstable;

  bool ok = create_prob_table(dicetuples, transitions, probstable, uniform_dice_sides, false);

  if (!ok) {
    std::cout << "prob table computation failed" << std::endl;
    return 1;
  }

  int elems_total = 0;
  int passes = 0;

  for (;;) {
    const int elems = update_elements(A, 
                                      D, 
                                      P,
                                      unused_value,
                                      dicetuples,
                                      transitions,
                                      probstable);
    passes += 1;
    if (elems == 0)
      break;

    elems_total += elems;
  }

  if (elems_total != (A - 1) * D) {
    std::cout << "DP calculation failed (passes = " << passes << ")" << std::endl;
    return 1;
  }

  // finally write results to standard output 
  // (supposed to be redirected into a file)
  // rows: 0..A, cols: 0..D

  for (int a = 0; a <= A; a++) {
    for (int d = 0; d <= D; d++) {
      std::cout << std::setprecision(num_text_digits) << P.data()[linear_index(a, A, d, D)] << " ";
    }
    std::cout << std::endl;
  }

  return 0;
}
