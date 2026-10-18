#include "TextGenerator.h"

#include <stdexcept>

TextGenerator::TextGenerator(const NGramModel &model)
    : model_(model), rng_(std::random_device{}()) {}

TextGenerator::TextGenerator(const NGramModel &model, unsigned seed)
    : model_(model), rng_(seed) {}

std::string TextGenerator::generate(std::size_t length) {
    if (length < model_.order())
        throw std::invalid_argument("output length must be at least k (" +
                                    std::to_string(model_.order()) + ")");
    std::string gram = model_.sampleStart(rng_);
    std::string out = gram;
    while (out.size() < length) {
        char c;
        if (!model_.sampleNext(gram, rng_, c)) break;  // stuck, give up
        out.push_back(c);
        gram.erase(0, 1);
        gram.push_back(c);
    }
    return out;
}