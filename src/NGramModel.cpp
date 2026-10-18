#include "NGramModel.h"

#include <fstream>
#include <iterator>
#include <set>
#include <stdexcept>

NGramModel::NGramModel(std::size_t k, double pseudocount) : k_(k), alpha_(pseudocount) {
    if (k_ == 0) throw std::invalid_argument("k must be a positive integer");
    if (!(alpha_ >= 0.0)) throw std::invalid_argument("pseudocount must be non-negative");
}

void NGramModel::trainFromFile(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open file '" + path + "'");
    std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    train(text);
}

void NGramModel::train(const std::string &text) {
    if (text.size() < k_ + 1)
        throw std::runtime_error("training text is too short: need at least " +
                                 std::to_string(k_ + 1) + " characters for k = " +
                                 std::to_string(k_));
    gramCount_.clear();
    transitions_.clear();
    followedCount_.clear();

    std::set<char> letters(text.begin(), text.end());
    alphabet_.assign(letters.begin(), letters.end());

    totalGrams_ = text.size() - k_ + 1;
    for (std::size_t i = 0; i < totalGrams_; ++i) {
        std::string gram = text.substr(i, k_);
        ++gramCount_[gram];
        if (i + k_ < text.size()) {  // the last k-gram is followed by nothing
            ++transitions_[gram][text[i + k_]];
            ++followedCount_[gram];
        }
    }
}

double NGramModel::frequency(const std::string &gram) const {
    auto it = gramCount_.find(gram);
    if (it == gramCount_.end()) return 0.0;
    return static_cast<double>(it->second) / static_cast<double>(totalGrams_);
}

double NGramModel::probability(const std::string &gram, char c) const {
    double n = 0.0, count = 0.0;
    auto f = followedCount_.find(gram);
    if (f != followedCount_.end()) {
        n = static_cast<double>(f->second);
        const auto &m = transitions_.at(gram);
        auto cit = m.find(c);
        if (cit != m.end()) count = static_cast<double>(cit->second);
    }
    double denom = n + alpha_ * static_cast<double>(alphabet_.size());
    if (denom <= 0.0) return 0.0;
    return (count + alpha_) / denom;
}

std::string NGramModel::sampleStart(std::mt19937 &rng) const {
    if (gramCount_.empty()) throw std::logic_error("model is not trained");
    std::vector<const std::string *> grams;
    std::vector<double> weights;
    for (const auto &p : gramCount_) {
        grams.push_back(&p.first);
        weights.push_back(static_cast<double>(p.second));
    }
    std::discrete_distribution<std::size_t> d(weights.begin(), weights.end());
    return *grams[d(rng)];
}

bool NGramModel::sampleNext(const std::string &gram, std::mt19937 &rng, char &out) const {
    std::vector<double> weights;
    weights.reserve(alphabet_.size());
    double sum = 0.0;
    auto t = transitions_.find(gram);
    for (char c : alphabet_) {
        double count = 0.0;
        if (t != transitions_.end()) {
            auto it = t->second.find(c);
            if (it != t->second.end()) count = static_cast<double>(it->second);
        }
        double w = count + alpha_;
        weights.push_back(w);
        sum += w;
    }
    if (sum <= 0.0) return false;  // stuck: no transitions and alpha = 0
    std::discrete_distribution<std::size_t> d(weights.begin(), weights.end());
    out = alphabet_[d(rng)];
    return true;
}