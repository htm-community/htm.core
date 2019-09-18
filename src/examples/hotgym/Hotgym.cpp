#include "HelloSPTP.hpp"

#include <string> // stoi

//this runs as executable
int main(int argc, char* argv[]) {
  htm::UInt EPOCHS = 5000; // number of iterations (calls to SP/TP compute() )

  if(argc == 2) {
    EPOCHS = std::stoi(argv[1]);
  }

  auto bench = examples::BenchmarkHotgym();
  bench.run(EPOCHS);
  return 0;
}
