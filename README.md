# slm - Small Language Model

`slm` is a small command line tool that learns a character k-gram language model from a text file and uses it to generate new text. The output is not meant to be correct, but with a large enough k it often looks surprisingly like the training text.

## Build

Requires `make` and a C++17 compiler (g++). Works on Linux, macOS and Windows (MinGW/MSYS2).

    make          # builds the executable: slm
    make clean    # removes build/ and the executable

`make` only recompiles files that changed.

## Usage

    slm [-a pseudocount] k textfile length

| Argument | Meaning |
|----------|---------|
| `k` | k-gram size, a positive integer |
| `textfile` | training text, read as plain bytes (one byte = one character) |
| `length` | number of characters to output, including the starting k-gram. Must be at least `k` |
| `-a` | optional pseudocount, a non-negative number. Default is 0 |

Examples:

    slm 2 samples/moby_dick.txt 5
    slm -a 0.01 3 samples/moby_dick.txt 40

User errors (wrong number of arguments, bad numbers, unknown option, unreadable file, training text too short for the chosen k) are reported on stderr as `slm: <message>` and the program exits with status 1.

    $ slm 2 no_such_file.txt 100
    slm: cannot open file 'no_such_file.txt'

## How it works

Training counts every k-gram in the text, and for each k-gram the characters that follow it. The probability of character c after k-gram w is

$$
    P(c \mid w) = \frac{n(w,c) + a}{n(w) + a \cdot |A|}
$$

where `n(w, c)` is how many times c follows w, `n(w)` is how many times w is followed by anything, `a` is the pseudocount and `|A|` is the number of distinct characters in the training text. A k-gram at the very end of the text is not followed by anything and does not count towards `n(w)`.

Generation picks a starting k-gram according to the k-gram frequencies, then repeatedly samples the next character, slides the window one step and continues until the requested length is reached. If the current k-gram has no transitions (only possible with `a = 0`), the generator stops, so the output can be shorter than requested. With `a > 0` it never gets stuck, even in k-grams that never occurred in the training text, where the distribution is uniform.

## Repository layout

| Path | Purpose |
|------|---------|
| `src/NGramModel.{h,cpp}` | Class `NGramModel`: training, frequencies, probabilities, sampling of start k-gram and next character |
| `src/TextGenerator.{h,cpp}` | Class `TextGenerator`: the generation loop, owns the random number generator |
| `src/slm.cpp` | `main`, argument parsing and error handling |
| `Makefile` | Build rules, including `clean` |
| `samples/` | Training texts |

Errors are thrown as `std::invalid_argument` or `std::runtime_error` where they are detected and caught in `main`.

## Testing

I tested the program on `samples/moby_dick.txt` by checking:

- normal runs with k from 1 to 6 and with different pseudocounts,
- the stuck case, using a tiny training file,
- every error case from the assignment (missing file, wrong number of arguments, k or length not a positive integer, length smaller than k, bad or missing value after `-a`, unknown option, training text too short), checking both the message and that the exit status is 1,
- the Makefile: a second `make` does nothing, changing one source file recompiles only that file, and `make clean` removes all generated files.

The numbers in the assignment examples were also checked against the model: for "ababac" with k = 2 the frequency of "ab" is 0.4, and with a = 1 P(a | ab) = 0.6 and P(c | ac) = 0.333. For "Hubba bubba" with k = 3 the frequency of "bba" is 0.222.

## Findings

All examples are from `samples/moby_dick.txt`, and the output differs between runs.

**Effect of k.** With k = 1 the output is gibberish that only has the right letter mix:

    ourabld e thtuatss om w By, smprelifereaby

With k = 3 words are partly formed and short real words show up:

    ightego taken a mare is now hase. And with he I's holent oddenell dan whaless

With k = 6 most words are real, and grammar is still off but longer phrases and names from the book appear:

    of him to feed, Oh, _do_, _do!_ What's a singular object, any one known some violently

The reason is that a longer k-gram leaves few choices for the next character, so the model has less freedom and copies the book more. A very large k would mostly reproduce the training text.

**Effect of the pseudocount.** A small value such as `-a 0.01` makes the text a bit more creative. It lets characters appear after a k-gram that never had them, which gives odd word blends like "girlpoone" and "sobesoled", but the text still looks like English. A large value such as `-a 100` drowns the information in the training text and the output is close to random characters.

**Non-ASCII bytes.** The training text contains UTF-8 characters such as curly quotes, which take more than one byte. The program treats each byte as one character, as the assignment allows, so the alphabet contains single bytes of those characters. With a large pseudocount the bytes are drawn at random and form invalid UTF-8, which is why broken characters appear in the `-a 100` output.

**Stopping early.** With `a = 0` the generator stops when the current k-gram only occurred at the end of the training text. On a tiny training file the output was only a few characters although 50 were requested. With `-a 1` the same input gives the full length.