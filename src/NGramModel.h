#ifndef NGRAM_MODEL_H
#define NGRAM_MODEL_H

#include <cstddef>
#include <map>
#include <random>
#include <string>
#include <vector>

// Character k-gram model with optional pseudocount smoothing.
class NGramModel {
public:
    // k >= 1, pseudocount >= 0, otherwise std::invalid_argument.
    NGramModel(std::size_t k, double pseudocount);

    // Throws std::runtime_error if the file can't be opened or the text is too short.
    void trainFromFile(const std::string &path);
    void train(const std::string &text);

    std::size_t order() const { return k_; }
    std::size_t alphabetSize() const { return alphabet_.size(); }

    // f(w): relative frequency of a k-gram in the training text (0 if unseen).
    double frequency(const std::string &gram) const;
    // P(c | gram) including pseudocount (0 if undefined, i.e. no data and alpha = 0).
    double probability(const std::string &gram, char c) const;

    // Pick a starting k-gram according to the k-gram frequencies.
    std::string sampleStart(std::mt19937 &rng) const;
    // Sample the next char after gram. Returns false if gram has no transitions.
    bool sampleNext(const std::string &gram, std::mt19937 &rng, char &out) const;

private:
    std::size_t k_;
    double alpha_;
    std::size_t totalGrams_ = 0;
    std::vector<char> alphabet_;
    std::map<std::string, std::size_t> gramCount_;                    // all k-grams
    std::map<std::string, std::map<char, std::size_t>> transitions_;  // gram -> next char counts
    std::map<std::string, std::size_t> followedCount_;                // n(w)
};

#endif