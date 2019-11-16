/* Matricule - Nom Prenom
 * (Matricule2 - Nom2 Prenom2)
 *
 */
#include "Solver.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
using std::map;
using std::string;
using std::vector;
const char WALL = '#';
const int POS = 0;
const int SIZE = 1;
const int ORIENTATION = 2;
const int LINES_PER_RECORD = 4;
enum Orientation { Horizontal, Vertical };
struct Block {
  string id;
  int x;
  int y;
  int width;
  int height;
  Orientation orientation;
};

/**
 * Pretty print the list of blocks
 * @param blocks: vector of Blocks
 */
void pretty_print(vector<Block> &blocks) {
  for (unsigned i = 0; i < blocks.size(); i++) {
    if (blocks[i].id == "RE") {
      std::cout << "GOAL CAR" << std::endl;
    }
    std::cout << blocks[i].id << std::endl;
    std::cout << "at: " << blocks[i].x << ", " << blocks[i].y << std::endl;
    std::cout << "size: " << blocks[i].width << ", " << blocks[i].height
              << std::endl;
    std::cout << "orientation: "
              << (blocks[i].orientation == Horizontal ? "horizontal"
                                                      : "vertical")
              << std::endl;
  }
}

/**
 * Solves the given rush hour instance.
 * @param n: height and width of the instance
 * @param vehicles: vector of movable blocks (i.e., vehicles)
 * @param fixed: vector of immovable blocks
 * @param k: number of solutions to find
 */
void solve(int n, vector<Block> &vehicles, vector<Block> fixed, int k) {
  pretty_print(vehicles);
  pretty_print(fixed);

  // A COMPLETER
  Solver s;
  int time_bound = 20;
  int vehicles_number = vehicles.size();
  int action = 4;
  // horizontal 0 is left, 1 is right
  // vertical   0 is bot, 1 is top 
  int Vehicles[n][n][time_bound][vehicles_number];
  int Moves[n][n][time_bound][action];

  vec<Lit> lits;
  vec<Lit> lits2;

  // ajout des propositions
  for (int i = 0 ; i < n ; i++) {
    for (int j = 0 ; j < n ; j++) {
      for (int t = 0 ; t < time_bound ; t++) {
        for (int v = 0 ; v < vehicles_number ; v++) {
          Vehicles[i][j][t][v] = s.newVar();
        }
        for (int a = 0 ; a < action ; a++) {
            Moves[i][j][t][a] = s.newVar();
          }
      }
    }
  }

  // initial configuration
  int startBoard[n][n];
  for (int i  = 0 ; i < n ; i++) {
    for (int j = 0 ; j < n ; j++) {
      startBoard[i][j]=-1;
      }
    }

  for (int v = 0 ; v < vehicles_number ; v++) {
    startBoard[n-1-vehicles[v].y][vehicles[v].x] = v;
  }

  for (int i = 0 ; i < n ; i++) {
    for (int j = 0 ; j < n ; j++) {
      for (int v = 0 ; v < vehicles_number ; v++) {
        if (startBoard[i][j] == v) {
          s.addUnit(Lit(Vehicles[i][j][0][v]));
        }
        else {
          s.addUnit(~Lit(Vehicles[i][j][0][v]));
        }
      }
    }
  }

  // at most one move per round
  for (int t = 0 ; t < time_bound ; t++) {
    for (int i = 0 ; i < n ; i++) {
      for (int j = 0 ; j < n ; j++) {
        for (int a = 0 ; a < action ; a++) {
          for (int i_ = 0 ; i_ < n ; i_++) {
            for (int j_ = 0 ; j_ < n ; j_++) {
              for (int a_ = 0 ; a_ < action ; a_++) {
                if (i != i_ || j != j_ || a != a_) {
                  s.addBinary(~Lit(Moves[i][j][t][a]),~Lit(Moves[i_][j_][t][a_]));
                }
              }
            }
          }
        }
      }
    }
  }

  // at most one vehicle per case
  for (int t = 0 ; t < time_bound ; t++) {
    for (int i = 0 ; i < n ; i++) {
      for (int j = 0 ; j < n ; j++) {
        for (int v = 0 ; v < vehicles_number ; v++) {
          for (int v_ = 0 ; v_ < vehicles_number ; v_++) {
            if (v != v_) {
              s.addBinary(~Lit(Vehicles[i][j][t][v]),~Lit(Vehicles[i][j][t][v_]));
            }
          }
        }
      }
    }
  }

  // At most one vehicle type on the board
  // Vijtv -> (-Vi_j_v) && ...
  for (int t = 0 ; t < time_bound ; t++) {
    for (int i = 0 ; i < n ; i++) {
      for (int j = 0 ; j < n ; j++) {
        for (int v = 0 ; v < vehicles_number ; v++) {
          for (int i_ = 0 ; i_ < n ; i_++) {
            for (int j_ = 0 ; j_ < n ; j_++) {
              if (i_ != i && j_ != j) {
                s.addBinary(~Lit(Vehicles[i][j][t][v]),~Lit(Vehicles[i_][j_][t][v]));
          	  }
          	}
          }
        }
      }
    }
  }
  // Vérifier no collision

  for (int t = 0 ; t < time_bound ; t++) {
    for (int i = 0 ; i < n ; i++) {
      for (int j = 0 ; j < n ; j++) {
        for (int v = 0 ; v < vehicles_number ; v++) {

          if (vehicles[v].orientation == Horizontal) {
            if (j + vehicles[v].width <= n) {
              for (int v_ = 0 ; v < vehicles_number ; v++) {
                if (v != v_) { // pas deux voitures sur la même case
                  s.addBinary(~Lit(Vehicles[i][j][t][v]),~Lit(Vehicles[i][j][t][v_]));
                }

                if (vehicles[v_].orientation == Horizontal) {
                  for (int d = 1 ; d < vehicles[v_].width && j-d >= 0 ; d++) {
                    s.addBinary(~Lit(Vehicles[i][j][t][v]),~Lit(Vehicles[i][j-d][t][v_]));
                  }
                }

                else {
                  for (int d = 0 ; d < vehicles[v_].height && i-d >= 0 ; d++) {
                    s.addBinary(~Lit(Vehicles[i][j][t][v]),~Lit(Vehicles[i-d][j][t][v_]));
                    s.addBinary(~Lit(Vehicles[i][j][t][v]),~Lit(Vehicles[i-d][j+1][t][v_]));
                    s.addBinary(~Lit(Vehicles[i][j][t][v]),~Lit(Vehicles[i-d][j+2][t][v_]));
                  }
                }
              }
            }
          }

          else { //vehicle vertical
          	if (i + vehicles[v].height <= n) {
              for (int v_ = 0 ; v < vehicles_number ; v++) {
                if (v != v_) { // pas deux voitures sur la même case
                  s.addBinary(~Lit(Vehicles[i][j][t][v]),~Lit(Vehicles[i][j][t][v_]));
                }

                if (vehicles[v_].orientation == Vertical) {
                  for (int d = 1 ; d < vehicles[v_].height && i-d >= 0 ; d++) {
                    s.addBinary(~Lit(Vehicles[i][j][t][v]),~Lit(Vehicles[i-d][j][t][v_]));
                  }
                }

                else {
                  for (int d = 0 ; d < vehicles[v_].width && j-d >= 0 ; d++) {
                    s.addBinary(~Lit(Vehicles[i][j][t][v]),~Lit(Vehicles[i][j-d][t][v_]));
                    s.addBinary(~Lit(Vehicles[i][j][t][v]),~Lit(Vehicles[i-1][j-d][t][v_]));
                    s.addBinary(~Lit(Vehicles[i][j][t][v]),~Lit(Vehicles[i-2][j-d][t][v_]));
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  //                 G,D                H B
  // Vérifier action 0,1 = horizontal | 2,3 = vertical
  for (int t = 0 ; t < time_bound-1 ; t++) {
    for (int i = 0 ; i < n ; i++) {
      for (int j = 0 ; j < n ; j++) {
        lits.clear();
        lits.push(~Lit(Moves[i][j][t][0]));
        lits2.clear();
        lits2.push(~Lit(Moves[i][j][t][2]));
        for (int v = 0 ; v < vehicles_number ; v++) {
          if (vehicles[v].orientation == Horizontal) {
            lits.push(Lit(Vehicles[i][j][t][v]));
          }
          else {
            lits2.push(Lit(Vehicles[i][j][t][v]));
          }
        s.addClause(lits);
        s.addClause(lits2);
        }
      }
    }
  }
  for (int t = 0 ; t < time_bound-1 ; t++) {
    for (int i = 0 ; i < n ; i++) {
      for (int j = 0 ; j < n ; j++) {
        lits.clear();
        lits.push(~Lit(Moves[i][j][t][1]));
        lits2.clear();
        lits2.push(~Lit(Moves[i][j][t][3]));
        for (int v = 0 ; v < vehicles_number ; v++) {
          if (vehicles[v].orientation == Horizontal) {
            lits.push(Lit(Vehicles[i][j][t][v]));
          }
          else {
            lits2.push(Lit(Vehicles[i][j][t][v]));
          }
        s.addClause(lits);
        s.addClause(lits2);
        }
      }
    }
  }

  // Moves
  for (int t = 0 ; t < time_bound-1 ; t++) { // Move left
    for (int i = 0 ; i < n ; i++) {
      for (int j = 1 ; j < n ; j++) {
        for (int v = 0 ; v < vehicles_number ; v++) {
          // Mijt0 -> -a && -b && -c && -d
          s.addBinary(~Lit(Moves[i][j][t][0]),~Lit(Vehicles[i][j][t+1][v]));
          // Mijt0 -> a == b
          lits.clear();
          lits.push(~Lit(Moves[i][j][t][0]));
          lits.push(~Lit(Vehicles[i][j][t][v]));
          lits.push(Lit(Vehicles[i][j-1][t+1][v]));
          s.addClause(lits);

          lits.clear();
          lits.push(~Lit(Moves[i][j][t][0]));
          lits.push(Lit(Vehicles[i][j][t][v]));
          lits.push(~Lit(Vehicles[i][j-1][t+1][v]));
          s.addClause(lits);
        }
      }
    }
  }
  for (int t = 0 ; t < time_bound-1 ; t++) { // car same place
    for (int i = 0 ; i < n ; i++) {
      for (int j = 1 ; j < n ; j++) {
        for (int v = 0 ; v < vehicles_number ; v++) {
          for (int i_ = 0 ; i_ < n ; i_++) {
            for (int j_ = 0 ; j_ < n ; j_++) { // M i,j,t,0 ->  (Vehicle xnor Vehicles)^()^()
              if (!((j_ == j && i_ == i) || (j_ == j-1 && i_ == i))) {
                lits.clear();
                lits2.clear();
                lits.push(~Lit(Moves[i][j][t][0]));
                lits2.push(~Lit(Moves[i][j][t][0]));
                lits.push(~Lit(Vehicles[i_][j_][t][v]));
                lits2.push(Lit(Vehicles[i_][j_][t][v]));
                lits.push(Lit(Vehicles[i_][j_][t+1][v]));
                lits2.push(~Lit(Vehicles[i_][j_][t+1][v]));
                s.addClause(lits);
                s.addClause(lits2);
              }
            }
          }
        }
      }
    }
  }
  for (int t = 0 ; t < time_bound-1 ; t++) { // Disallow bad left move
    for (int i = 0 ; i < n ; i++) {
      s.addUnit(~Lit(Moves[i][0][t][0]));
    }
  }
  for (int t = 0 ; t < time_bound-1 ; t++) { // Move right
    for (int i = 0 ; i < n ; i++) {
      for (int j = 0 ; j < n-1 ; j++) {
        for (int v = 0 ; v < vehicles_number ; v++) {
          s.addBinary(~Lit(Moves[i][j][t][1]),~Lit(Vehicles[i][j][t+1][v]));
          lits.clear();
          lits.push(~Lit(Moves[i][j][t][1]));
          lits.push(~Lit(Vehicles[i][j][t][v]));
          lits.push(Lit(Vehicles[i][j+1][t+1][v]));
          s.addClause(lits);

          lits.clear();
          lits.push(~Lit(Moves[i][j][t][1]));
          lits.push(Lit(Vehicles[i][j][t][v]));
          lits.push(~Lit(Vehicles[i][j+1][t+1][v]));
          s.addClause(lits);
        }
      }
    }
  }
  for (int t = 0 ; t < time_bound-1 ; t++) { // car same place
    for (int i = 0 ; i < n ; i++) {
      for (int j = 0 ; j < n-1 ; j++) {
        for (int v = 0 ; v < vehicles_number ; v++) {
          for (int i_ = 0 ; i_ < n ; i_++) {
            for (int j_ = 0 ; j_ < n ; j_++) {
              if (!((j_ == j && i_ == i) || (j_ == j+1 && i_ == i))) {
                lits.clear();
                lits2.clear();
                lits.push(~Lit(Moves[i][j][t][1]));
                lits2.push(~Lit(Moves[i][j][t][1]));
                lits.push(~Lit(Vehicles[i_][j_][t][v]));
                lits2.push(Lit(Vehicles[i_][j_][t][v]));
                lits.push(Lit(Vehicles[i_][j_][t+1][v]));
                lits2.push(~Lit(Vehicles[i_][j_][t+1][v]));
                s.addClause(lits);
                s.addClause(lits2);
              }
            }
          }
        }
      }
    }
  }

  // 2
  for (int t = 0 ; t < time_bound-1 ; t++) { // Move Up
    for (int i = 1 ; i < n ; i++) {
      for (int j = 0 ; j < n ; j++) {
        for (int v = 0 ; v < vehicles_number ; v++) {
          s.addBinary(~Lit(Moves[i][j][t][2]),~Lit(Vehicles[i][j][t+1][v]));
          lits.clear();
          lits.push(~Lit(Moves[i][j][t][2]));
          lits.push(~Lit(Vehicles[i][j][t][v]));
          lits.push(Lit(Vehicles[i-1][j][t+1][v]));
          s.addClause(lits);
          lits.clear();
          lits.push(~Lit(Moves[i][j][t][2]));
          lits.push(Lit(Vehicles[i][j][t][v]));
          lits.push(~Lit(Vehicles[i-1][j][t+1][v]));
          s.addClause(lits);
        }
      }
    }
  }
  for (int t = 0 ; t < time_bound-1 ; t++) {
    for (int i = 1 ; i < n ; i++) {
      for (int j = 0 ; j < n ; j++) {
        for (int v = 0 ; v < vehicles_number ; v++) {
          for (int i_ = 0 ; i_ < n ; i_++) {
            for (int j_ = 0 ; j_ < n ; j_++) {
              if (!((j_ == j && i_ == i) || (j_ == j && i_ == i-1))) {
                lits.clear();
                lits2.clear();
                lits.push(~Lit(Moves[i][j][t][2]));
                lits2.push(~Lit(Moves[i][j][t][2]));
                lits.push(~Lit(Vehicles[i_][j_][t][v]));
                lits2.push(Lit(Vehicles[i_][j_][t][v]));
                lits.push(Lit(Vehicles[i_][j_][t+1][v]));
                lits2.push(~Lit(Vehicles[i_][j_][t+1][v]));
                s.addClause(lits);
                s.addClause(lits2);
              }
            }
          }
        }
      }
    }
  }
    for (int t = 0 ; t < time_bound-1 ; t++) { // Disallow bad up move
      for (int j = 0 ; j < n ; j++) {
      	s.addUnit(~Lit(Moves[0][j][t][2]));
      }
    }
  // 3
  for (int t = 0 ; t < time_bound-1 ; t++) { //Move down
    for (int i = 0 ; i < n-1 ; i++) {
      for (int j = 0 ; j < n ; j++) {
        for (int v = 0 ; v < vehicles_number ; v++) {
          s.addBinary(~Lit(Moves[i][j][t][3]),~Lit(Vehicles[i][j][t+1][v]));
          lits.clear();
          lits.push(~Lit(Moves[i][j][t][3]));
          lits.push(~Lit(Vehicles[i][j][t][v]));
          lits.push(Lit(Vehicles[i+1][j][t+1][v]));
          s.addClause(lits);
          lits.clear();
          lits.push(~Lit(Moves[i][j][t][3]));
          lits.push(Lit(Vehicles[i][j][t][v]));
          lits.push(~Lit(Vehicles[i+1][j][t+1][v]));
          s.addClause(lits);
        }
      }
    }
  }
  for (int t = 0 ; t < time_bound-1 ; t++) { // car same place
    for (int i = 0 ; i < n-1 ; i++) {
      for (int j = 0 ; j < n ; j++) {
        for (int v = 0 ; v < vehicles_number ; v++) {
          for (int i_ = 0 ; i_ < n ; i_++) {
            for (int j_ = 0 ; j_ < n ; j_++) {
              if (!((j_ == j && i_ == i) || (j_ == j && i_ == i+1))) {
                lits.clear();
                lits2.clear();
                lits.push(~Lit(Moves[i][j][t][3]));
                lits2.push(~Lit(Moves[i][j][t][3]));
                lits.push(~Lit(Vehicles[i_][j_][t][v]));
                lits2.push(Lit(Vehicles[i_][j_][t][v]));
                lits.push(Lit(Vehicles[i_][j_][t+1][v]));
                lits2.push(~Lit(Vehicles[i_][j_][t+1][v]));
                s.addClause(lits);
                s.addClause(lits2);
              }
            }
          }
        }
      }
    }
  }
/*
  // if some cell has changed its status, then there was a move somewhere affecting it
  // Changement -> Move
  // -Changement v Move
  for (int t = 0 ; t < time_bound-1 ; t++) {
    for (int i = 0 ; i < n ; i++) {
      for (int j = 0 ; j < n ; j++) {
        for (int i_ ; i_ < n ; i_++) {
          for (int j_ ; j_ < n ; j_++) {
            for (int v = 0 ; v < vehicles_number ; v++) {
              lits.clear();
              lits.push(~Lit(Vehicles[i_][j_][t][v]));
              lits.push(~Lit(Vehicles[i_][j_][t+1][v]));
              for (int a = 0 ; a < action ; a++) {
                lits.push(Lit(Moves[i][j][t][a]));
              }
              s.addClause(lits);

              lits.clear();
              lits.push(Lit(Vehicles[i_][j_][t][v]));
              lits.push(Lit(Vehicles[i_][j_][t+1][v]));
              for (int a = 0 ; a < action ; a++) {
                lits.push(Lit(Moves[i][j][t][a]));
              }
              s.addClause(lits);
            }
          }
        }
      }
    }
  }
*//*
  // 1 move i,j == voiture
  // M -> V
  for (int t = 0 ; t < time_bound-1 ; t++) {
    for (int i = 0 ; i < n ; i++) {
      for (int j = 0 ; j < n ; j++) {
        lits.clear();
        lits.push(~Lit(Moves[i][j][t][0]));
        for (int v = 0 ; v < vehicles_number ; v++) {
          lits.push(Lit(Vehicles[i][j][t][v]));
        }
        s.addClause(lits);

        lits.clear();
        lits.push(~Lit(Moves[i][j][t][1]));
        for (int v = 0 ; v < vehicles_number ; v++) {
          lits.push(Lit(Vehicles[i][j][t][v]));
        }
        s.addClause(lits);

        lits.clear();
        lits.push(~Lit(Moves[i][j][t][2]));
        for (int v = 0 ; v < vehicles_number ; v++) {
          lits.push(Lit(Vehicles[i][j][t][v]));
        }
        s.addClause(lits);

        lits.clear();
        lits.push(~Lit(Moves[i][j][t][3]));
        for (int v = 0 ; v < vehicles_number ; v++) {
          lits.push(Lit(Vehicles[i][j][t][v]));
        }
        s.addClause(lits);
      }
    }
  }
*/
  // No move = rien qui change
  // -MT -> VijTv == VijT+1v
  // MT || VijTv == VijT+1v
  for (int t = 0 ; t < time_bound-1 ;t++) { // FONCTIONNEL
  	lits.clear();
    for (int i = 0 ; i < n ; i++) {
      for (int j = 0 ; j < n ; j++) {
        for (int a = 0 ; a < action ; a++) {
          lits.push(Lit(Moves[i][j][t][a]));
        }
      }
    }
    for (int i = 0 ; i < n ; i++) {
      for (int j = 0 ; j < n ; j++) {
        for (int v = 0 ; v < vehicles_number ; v++) {
          lits.push(~Lit(Vehicles[i][j][t][v]));
          lits.push(Lit(Vehicles[i][j][t+1][v]));
          s.addClause(lits);
          lits.pop();
          lits.pop();
          lits.push(Lit(Vehicles[i][j][t][v]));
          lits.push(~Lit(Vehicles[i][j][t+1][v]));
          s.addClause(lits);
          lits.pop();
          lits.pop();
        }
      }
    }
  }

  // Finish
  lits.clear();
  for (int t = 0 ; t < time_bound ; t++) {
    lits.push(Lit(Vehicles[n-1-vehicles[0].y][n-2][t][0]));
  }
  s.addClause(lits);

  // Move when not finished
  // not solved -> move
  // no move -> resolu
  /*for (int t = 0 ; t < time_bound ; t++) {
    lits.clear();
    lits.push(Lit(Vehicles[n-1-vehicles[0].y][n-2][t][0]));
    for (int i = 0 ;  i < 0 ; i++) {
      for (int j = 0 ; j < 0 ; j++) {
        for (int a = 0 ; a < action ; a++) {
          lits.push(Lit(Moves[i][j][t][a]));
        }
      }
    }
    s.addClause(lits);
  }*/
  // Solution
  s.solve();
  if (!s.okay()) {
    std::cout << "pas de solution" << std::endl;
  }
  else {
    for (int t = 0 ; t < time_bound ; t++) {
      for (int i = 0 ; i < n ; i++) {
        for (int j = 0 ; j < n ; j++) {
          bool found = false;
          for (int v = 0 ; v < vehicles_number ; v++) {
            if (s.model[Vehicles[i][j][t][v]] == l_True) {found = true; std::cout << ' ' << vehicles[v].id << ' ';}
          }
        if (!found) {std::cout << " XX ";}
        }
        std::cout << std::endl;
      }
      std::cout << "Move = ";
      bool found = false;
      for (int i = 0 ; i < n ; i++) {
        for (int j = 0 ; j < n ; j++) {
          for (int a = 0 ; a < action ; a++) {
            if (s.model[Moves[i][j][t][a]] == l_True) {
              found = true; std::cout << i << ',' << j << " va ";
              switch(a) {
                case 0: std::cout << "à gauche" << std::endl;break;
                case 1: std::cout << "à droite" << std::endl;break;
                case 2: std::cout << "en haut" << std::endl;break;
                default: std::cout << "en bas" << std::endl;break;
              }
            }
          }
        }
      }
    if (!found) {std::cout << " No Move " << std::endl;}  
    std::cout << std::endl;
    }
  }
}

/**
 * Prints program help message
 * @param program: program name
 */
void print_help(char *program) {
  std::cerr << "Usage:" << std::endl;
  std::cerr << program << " [-k NOMBRE]" << std::endl;
}

int main(int argc, char **argv) {
  int k = 1;

  // Parse command line arguments
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-k") == 0) {
      try {
        k = std::stoi(argv[i + 1]);
      } catch (const std::logic_error &e) {
        print_help(argv[0]);
        return EXIT_FAILURE;
      }
    } else if ((strcmp(argv[i], "--help") == 0) ||
               (strcmp(argv[i], "-h") == 0)) {
      print_help(argv[0]);
      return EXIT_SUCCESS;
    }
  }

  // read instance on standard input
  string line;
  int n;
  std::cin >> n;
  std::getline(std::cin, line); // skip endline after dimensions

  vector<Block> vehicles; // vector of movable vehicles
  vector<Block> fixed;    // vector of immovable walls
  while (std::getline(std::cin, line)) {
    string id = line;
    int x, y, width, height;
    Orientation orientation;
    for (int i = 0; i < LINES_PER_RECORD - 1; i++) {
      std::getline(std::cin, line);
      std::istringstream ss(line);

      switch (i) {
      case POS:
        ss >> x >> y;
        break;
      case SIZE:
        ss >> width >> height;
        break;
      case ORIENTATION:
        orientation = line[0] == 'h' ? Horizontal : Vertical;
        break;
      }
    }

    Block new_block = {id, x, y, width, height, orientation};
    if (id[0] == WALL) {
      fixed.push_back(new_block);
    } else {
      vehicles.push_back(new_block);
    }
  }

  solve(n, vehicles, fixed, k);

  return EXIT_SUCCESS;
}