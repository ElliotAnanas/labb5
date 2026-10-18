#ifndef TEXT_GENERATOR_H
#define TEXT_GENERATOR_H

#include <cstddef>
#include <random>
#include <string>

#include "NGramModel.h"

// Generates text from a trained NGramModel.
class TextGenerator {
public:
    explicit TextGenerator(const NGramModel &model);
    TextGenerator(const NGramModel &model, unsigned seed);  // reproducible output

    // length counts the starting k-gram; must be >= model order (else std::invalid_argument).
    // Output can be shorter than length if the generator gets stuck.
    std::string generate(std::size_t length);

private:
    const NGramModel &model_;
    std::mt19937 rng_;
};

#endif