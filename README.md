# BlaBla PRNG
C++ implementation of JP Aumasson's [BlaBla](https://github.com/veorq/blabla/blob/master/BlaBla.swift) PRNG (64-bit version of ChaCha based on Blake2b). Should run faster than ChaCha for 64-bit systems.

This code is a modification of [Orson Peters' C++ ChaCha PRNG implementation](https://gist.github.com/orlp/32f5d1b631ab092608b1).

# Example Usage

```cpp
#include <iostream>
#include <random>
#include "blabla.h"

int main() {
    // Instantiate generator and seed with random entropy
    std::random_device rd;
    BlaBlaPRNG::BlaBla<> gen(rd());

    // Define a uniform integer distribution in the range [1, 100]
    std::uniform_int_distribution<> dist(1, 100);

    // Generate and print 10 random numbers in the range [1, 100]
    for (int i = 0; i < 10; ++i) {
        std::cout << dist(gen) << " ";
    }

    return 0;
}
```
