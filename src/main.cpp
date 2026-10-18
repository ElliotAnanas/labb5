#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include "NGramModel.h"
#include "TextGenerator.h"

namespace {

const char *USAGE = "usage: slm [-a pseudocount] k textfile length";

std::size_t parsePositiveInt(const std::string &s, const std::string &what) {
    if (s.empty() || !std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c); }))
        throw std::invalid_argument(what + " must be a positive integer, got '" + s + "'");
    std::size_t v;
    try {
        v = std::stoull(s);
    } catch (const std::out_of_range &) {
        throw std::invalid_argument(what + " is too large: '" + s + "'");
    }
    if (v == 0) throw std::invalid_argument(what + " must be a positive integer, got '" + s + "'");
    return v;
}

double parsePseudocount(const std::string &s) {
    std::size_t pos = 0;
    double v;
    try {
        v = std::stod(s, &pos);
    } catch (const std::exception &) {
        throw std::invalid_argument("-a needs a non-negative number, got '" + s + "'");
    }
    if (pos != s.size() || !std::isfinite(v) || v < 0.0)
        throw std::invalid_argument("-a needs a non-negative number, got '" + s + "'");
    return v;
}

}  // namespace

int main(int argc, char *argv[]) {
    try {
        double alpha = 0.0;
        int i = 1;
        while (i < argc && argv[i][0] == '-' && argv[i][1] != '\0') {
            std::string opt = argv[i];
            if (opt == "-a") {
                if (i + 1 >= argc) throw std::invalid_argument("option -a needs a value");
                alpha = parsePseudocount(argv[i + 1]);
                i += 2;
            } else {
                throw std::invalid_argument("unknown option '" + opt + "'");
            }
        }
        if (argc - i != 3) throw std::invalid_argument(std::string("wrong number of arguments\n") + USAGE);

        std::size_t k = parsePositiveInt(argv[i], "k");
        std::string path = argv[i + 1];
        std::size_t length = parsePositiveInt(argv[i + 2], "output length");
        if (length < k)
            throw std::invalid_argument("output length (" + std::to_string(length) +
                                        ") must be at least k (" + std::to_string(k) + ")");

        NGramModel model(k, alpha);
        model.trainFromFile(path);
        TextGenerator gen(model);
        std::cout << gen.generate(length) << '\n';
    } catch (const std::exception &e) {
        std::cerr << "slm: " << e.what() << '\n';
        return 1;
    }
    return 0;
}