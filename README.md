# Suffix Automaton: Applications in C++

A compact and powerful string data structure with `O(n)` construction time, demonstrating **Longest Common Substring** and **Substring Search** in linear time.

## Objective
This project implements a Suffix Automaton (SAM) in C++ and demonstrates two of its classical applications:
1. **Longest Common Substring (LCS)** — Find the longest substring common to two strings.
2. **Substring Search (Pattern Matching)** — Determine if a given pattern exists within a string.

The goal is to showcase the efficiency and elegance of the Suffix Automaton as a general-purpose string processing tool, demonstrating linear-time algorithms for problems that are significantly slower under naive approaches.

## What is a Suffix Automaton?
A Suffix Automaton is a Directed Acyclic Graph (DAG), a compact deterministic automaton that represents all substrings of a string. It is the minimal DFA recognizing all suffixes of that string.

**Key properties:** 
* Built incrementally, character by character, in `O(n)` time and `O(n)` space.
* Has at most `2n − 1` states and `3n − 4` transitions for a string of length `n`.
* Every path starting from the root corresponds to a substring of the original string.
* Supports `O(p)` pattern matching for a pattern of length `p`.

> **📖 Read more:**
> * For a detailed theoretical explanation, see [`PROJECT_CONTEXT.md`](./PROJECT_CONTEXT.md).
> * For a line-by-line code walkthrough, see [`CODE_EXPLANATION.md`](./CODE_EXPLANATION.md).

## Applications Implemented

### 1. Longest Common Substring
* Builds a Suffix Automaton on String 1, then traverses it greedily using characters from String 2.
* Maintains a running match length `l`.
* When no transition exists for the current character, follows suffix links to shorten the match.
* Tracks the best match length and ending position seen during traversal.
* **Result:** The longest substring that appears in both strings.

### 2. Substring Search (Pattern Matching)
* Traverses the Suffix Automaton of String 1 using each character of the pattern.
* If every character has a valid transition, the pattern exists in the string.
* If any transition is missing, the pattern is absent.
* **Result:** Whether the pattern is a substring of String 1.

## Time & Space Complexity

| Operation | Time Complexity | Space Complexity |
| :--- | :--- | :--- |
| **SAM Construction** | `O(n)` | `O(n)` |
| **Longest Common Substring** | `O(m + n)` | `O(1)` extra |
| **Substring Search** | `O(p)` | `O(1)` extra |

*(Where `n` = length of String 1, `m` = length of String 2, `p` = length of pattern.)*  
The `next[26]` array per state results in an effective `O(26*n) = O(n)` space usage.

## How to Compile
Make sure you have `g++` (C++11 or later) installed.
```bash
g++ -o code code.cpp
```
Or with optimizations:
```bash
g++ -O2 -std=c++17 -o code code.cpp
```

## How to Run
The program is interactive and prompts for input at each step. Run the compiled executable:
```bash
./code
```
* **Enter String 1:** → The string to build the SAM on
* **Enter String 2 for LCS:** → String to find the LCS with
* **Enter pattern to search in String 1:** → Pattern to check inside String 1

## Sample Input / Output

### Example 1
```text
Enter String 1: abcde
Enter String 2 for LCS: cdefg

Application 1: Longest Common Substring
Longest Common Substring Length: 3
Longest Common Substring: cde

Enter pattern to search in String 1: bcd

Application 2: Substring Search
Pattern exists in String 1
```

### Example 2
```text
Enter String 1: ababab
Enter String 2 for LCS: bababa

Application 1: Longest Common Substring
Longest Common Substring Length: 5
Longest Common Substring: babab

Enter pattern to search in String 1: xyz

Application 2: Substring Search
Pattern does not exist in String 1
```

### Example 3 (No Common Substring)
```text
Enter String 1: abc
Enter String 2 for LCS: xyz

Application 1: Longest Common Substring
Longest Common Substring: NONE

Enter pattern to search in String 1: ab

Application 2: Substring Search
Pattern exists in String 1
```

## Constraints & Assumptions
* Input strings must contain only lowercase English letters (`a`–`z`).
* Maximum supported string length: 100,000 characters (defined by `MAXN = 100005`).
* If multiple Longest Common Substrings of equal maximum length exist, one of them is reported.

## Folder Structure

```text
├── code.cpp               # Code (SAM Construction & Applications)
├── README.md              # Project Overview
├── PROJECT_CONTEXT.md     # Theory: Motivation, Concepts, Complexity
└── CODE_EXPLANATION.md    # Implementation Walkthrough, Function-by-Function
```

## References
* [cp-algorithms: Suffix Automaton](https://cp-algorithms.com/string/suffix-automaton.html)
* Blumer et al. (1985): *The Smallest Automaton Recognizing the Subwords of a Text*
* Crochemore & Hancart (1997): *Automata for Matching Patterns*

---
*Developed as part of a course project demonstrating the implementation and applications of the Suffix Automaton in C++.*
