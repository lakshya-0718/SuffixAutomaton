# Suffix Automaton: Theory, Motivation and Context

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Motivation](#2-motivation)
3. [What is a Suffix Automaton?](#3-what-is-a-suffix-automaton)
4. [Components of a Suffix Automaton](#4-components-of-a-suffix-automaton)
   - 4.1 [States](#41-states)
   - 4.2 [Transitions](#42-transitions)
   - 4.3 [Suffix Links](#43-suffix-links)
   - 4.4 [Clone States](#44-clone-states)
5. [Construction Process](#5-construction-process)
6. [Applications](#6-applications)
   - 6.1 [Longest Common Substring](#61-longest-common-substring)
   - 6.2 [Substring Search](#62-substring-search)
7. [Advantages](#7-advantages)
8. [Limitations](#8-limitations)
9. [Complexity Analysis](#9-complexity-analysis)
10. [Future Scope](#10-future-scope)
11. [References](#11-references)

---

## 1. Introduction

String processing is one of the most fundamental areas in computer science. Problems like pattern matching, finding repeated substrings, or computing the longest common substring appear constantly in fields ranging from bioinformatics (DNA sequence alignment) to search engines (autocomplete, plagiarism detection) to compilers (lexical analysis).

A **Suffix Automaton (SAM)** — also called a **Directed Acyclic Word Graph (DAWG)** — is one of the most powerful and compact string data structures ever invented. First described by Blumer et al. in 1985, it represents all substrings of a string in a single, minimal deterministic finite automaton.

This project demonstrates the Suffix Automaton through two of its classical applications:
- **Longest Common Substring (LCS)** between two strings
- **Substring Search (Pattern Matching)** within a string

Both applications run in **linear time**, making the SAM among the most efficient approaches for these problems.

---

## 2. Motivation

### Why not use simpler approaches?

Consider a string `s` of length `n`. Two naive approaches to substring problems are:

| Approach | LCS Time | Pattern Match Time | Space |
| :--- | :--- | :--- | :--- |
| Brute Force (all substrings) | O(n² × m) | O(n × m) | O(1) |
| Suffix Array | O(n log n) build, O(m log n) query | O(m log n) | O(n) |
| Suffix Trie | O(n) build | O(m) | O(n²) — too large |
| **Suffix Automaton** | **O(n) build, O(m) query** | **O(m)** | **O(n)** |

A **Suffix Trie** stores every suffix of a string explicitly. For a string of length `n`, this means up to `O(n²)` nodes — completely impractical for large inputs.

The Suffix Automaton solves this by merging equivalent states: substrings that always appear at the same set of positions in the text are represented by a single state. The result is a structure with at most **2n − 1 states** and **3n − 4 transitions**, while still encoding every substring.

### The core motivation:

> **Can we represent all O(n²) substrings of a string using only O(n) space and answer substring queries in O(m) time?**

The Suffix Automaton answers: **yes**.

---

## 3. What is a Suffix Automaton?

A **Suffix Automaton** for a string `s` is a **minimal deterministic finite automaton (DFA)** that accepts exactly all suffixes of `s`.

More precisely, it is a directed graph where:
- Each **node (state)** represents an equivalence class of substrings.
- Each **edge (transition)** is labeled with a character.
- Reading characters from the **root (initial state)** along any valid path spells out a substring of `s`.
- Every substring of `s` is readable by exactly one path from the root.

### Key Properties:

**1. Compactness:**  
Every substring of `s` corresponds to a unique path from the root. The automaton has at most `2n − 1` states and `3n − 4` transitions — linear in the length of the input string.

**2. Completeness:**  
Every path from the root to any reachable state spells out a valid substring of `s`. No spurious substrings are accepted.

**3. Uniqueness:**  
The SAM is the **smallest** automaton (fewest states) that accepts all suffixes of `s`. It is provably minimal.

**4. Incrementally Constructible:**  
Characters can be appended one at a time in `O(1)` amortized time. This makes the SAM suitable for online string processing.

---

## 4. Components of a Suffix Automaton

### 4.1 States

Each state in the SAM represents a set of substrings of `s` called an **endpos-equivalence class**.

**Endpos set:** For a substring `w` of `s`, its endpos set `endpos(w)` is the set of all ending positions where `w` occurs in `s`.

For example, if `s = "abab"`:
- `endpos("ab") = {2, 4}` — "ab" ends at positions 2 and 4
- `endpos("b") = {2, 4}` — same positions
- `endpos("a") = {1, 3}` — different positions

Two substrings with the **same endpos set** are placed in the **same state**. This is the key compression mechanism — substrings that always co-occur are merged.

Each state stores:
- **`len`**: the length of the longest substring in its equivalence class.
- **`link`**: the suffix link (see below).
- **`next[26]`**: transitions to other states.

The **root state** (state 0) represents the empty string and has endpos equal to all positions.

---

### 4.2 Transitions

A **transition** is a directed edge from state `u` to state `v` labeled with character `c`. It means: if you are at state `u` and read character `c`, you move to state `v`.

Transitions encode the extension of a substring by one character. Reading a path of transitions from the root spells out a substring of `s`.

In the implementation, transitions are stored as an array `next[26]` in each state, where index `i` corresponds to the letter `'a' + i`. A value of `-1` means no transition exists for that letter.

---

### 4.3 Suffix Links

The **suffix link** of a state `v` (written `link(v)`) is the most important structural element of the SAM.

**Definition:** `link(v)` points to the state `u` such that:
- `u`'s equivalence class contains the longest proper suffix of the shortest string in `v`'s class.

More intuitively: if state `v` represents substrings of lengths `[min_len, max_len]`, then `link(v)` represents the next shorter group. Suffix links form a **tree** rooted at state 0.

**Why are suffix links important?**

1. **Construction:** During SAM construction, suffix links guide the algorithm when determining how to connect a new state into the existing structure.

2. **Search:** During LCS computation, when the current state has no transition for the next character, we follow suffix links backward to find a shorter matching context. This "fall-back" mechanism mirrors how the KMP failure function works for pattern matching.

3. **Structural Insight:** The suffix link tree partitions all substrings of `s` into a hierarchy of equivalence classes, revealing relationships between substrings at different lengths.

**Example:** For `s = "abbc"`:
- State for `"abbc"` has suffix link to state for `"bbc"` (longest proper suffix present in SAM)
- State for `"bbc"` has suffix link to state for `"bc"`
- And so on, eventually reaching the root.

---

### 4.4 Clone States

Clone states are a technical necessity introduced during construction to maintain the **correct suffix link invariant**.

**When is cloning needed?**

When adding a new character, the algorithm walks up suffix links to find the first state `p` that already has a transition on the new character, leading to state `q`. If `st[p].len + 1 != st[q].len`, then `q` is "too long" — it incorrectly bundles together substrings that should be in separate equivalence classes given the new character.

**What does cloning do?**

A clone of `q` is created with:
- The same transitions as `q` (so all existing paths still work).
- The same suffix link as `q` (to preserve the suffix link tree).
- A shorter `len = st[p].len + 1` (the correct boundary).

Then:
- All states that transitioned to `q` on the current character (reachable via suffix links from `p`) are redirected to the clone.
- `q`'s suffix link is updated to point to the clone.
- The new state's suffix link also points to the clone.

This ensures the endpos partition remains valid and the automaton stays minimal after each character insertion.

---

## 5. Construction Process

The SAM is built **online** — one character at a time — using the `sa_extend()` function. Here is a high-level walkthrough:

### Initialization (`sa_init`)
- Create the root state (state 0) with `len = 0`, `link = -1`, all transitions set to `-1`.
- Set `sz = 1` (one state exists) and `last = 0` (last state is root).

### Extending by one character (`sa_extend(c)`)

**Step 1 — Create new state:**  
A new state `cur` is created with `len = last.len + 1`. This state will represent all new substrings formed by appending character `c` to every suffix of the current string (including the whole string itself).

**Step 2 — Walk up suffix links:**  
Starting from `last`, walk up the suffix link chain. For every state `p` that has no transition on `c`, add a transition from `p` to `cur`. This allows every suffix of the current string extended by `c` to be recognized.

Stop when either:
- A state `p` already has a transition on `c` (found: existing path), or
- We fall off the root (`p = -1`): character `c` is entirely new.

**Step 3 — Set suffix link for `cur`:**

- **Case A (fell off root):** `cur.link = root`. No existing occurrence of `c` — link directly to root.

- **Case B (found state `p` with transition to `q`):**
  - If `p.len + 1 == q.len`: `q` is already the correct state. Set `cur.link = q`.
  - If `p.len + 1 != q.len`: Clone `q`. Redirect transitions and fix suffix links as described above.

**Step 4 — Update `last`:**  
Set `last = cur`. The new state is now the "current end" of the automaton.

### Visual Example: Building SAM for `"ab"`

```text
After 'a':
  State 0 (root) --[a]--> State 1 (len=1, link=0)
  last = 1

After 'b':
  State 0 --[a]--> State 1
  State 0 --[b]--> State 2 (len=2, link=0)
  State 1 --[b]--> State 2
  last = 2
```

State 2 represents substrings `"b"` and `"ab"` (both end only at position 2).

---

## 6. Applications

### 6.1 Longest Common Substring

**Problem:** Given two strings `s` and `t`, find the longest substring that appears in both.

**Naive approach:** `O(n × m)` — generate all substrings of `s`, check each against `t`.

**SAM approach:**
1. Build the SAM on string `s` in `O(n)` time.
2. Traverse the SAM using characters of string `t`.

During traversal, maintain:
- `v` — the current state in the SAM.
- `l` — the current match length (how many recent characters of `t` are also in `s`).

For each character of `t`:
- If the current state has a transition on this character: follow it, increment `l`.
- If not: follow suffix links backward until a state with a valid transition is found (shortening `l` accordingly), then follow the transition.
- If even the root has no transition: reset `l = 0`, stay at root.

Track the maximum `l` seen — this is the length of the LCS.

**Time complexity:** `O(n + m)` total.

**Why it works:**  
Every path from the root in the SAM spells a substring of `s`. By greedily matching characters of `t` and backtracking via suffix links when stuck, we always maintain the longest current suffix of `t` that is also a substring of `s`. The maximum value of this running match length is the LCS length.

---

### 6.2 Substring Search

**Problem:** Given string `s` and pattern `p`, determine if `p` is a substring of `s`.

**Naive approach:** `O(n × m)` — check every starting position.

**SAM approach:**
1. Build the SAM on string `s` in `O(n)` time.
2. Start at the root and follow transitions for each character of `p`.
3. If all transitions are followed successfully: `p` is a substring of `s`.
4. If any transition is missing: `p` is not a substring of `s`.

**Time complexity:** `O(n)` build + `O(m)` query.

**Why it works:**  
Every substring of `s` is accepted by the SAM — i.e., there exists a valid path from the root that spells it out. If we can follow a transition for every character of `p` without getting stuck, then `p` is indeed a substring of `s`. If any transition is missing, no path can spell `p`, meaning it does not occur in `s`.

---

## 7. Advantages

| Advantage | Details |
| :--- | :--- |
| **Linear construction time** | `O(n)` to build, regardless of the string's structure |
| **Linear space** | At most `2n-1` states, `3n-4` transitions — far smaller than a Suffix Trie |
| **`O(m)` query time** | Pattern matching in time proportional only to pattern length |
| **Online construction** | Can extend the automaton as new characters arrive |
| **Versatile** | Supports LCS, pattern matching, counting distinct substrings, finding lexicographically k-th substring, and more |
| **No preprocessing of pattern** | Unlike KMP or Rabin-Karp, no need to preprocess the pattern |
| **Multiple queries** | Once built on `s`, the SAM can answer unlimited pattern queries in `O(m)` each |

---

## 8. Limitations

| Limitation | Details |
| :--- | :--- |
| **Lowercase letters only (in this implementation)** | The `next[26]` array assumes only `'a'`–`'z'`. General alphabets require a hash map instead of a fixed array. |
| **Single string** | This implementation builds the SAM on one string. Generalised SAM for multiple strings requires additional handling. |
| **Memory usage** | Even though `O(n)`, the constant factor is large: each state stores 28 integers (len + link + 26 transitions), totalling ~112 bytes per state. For MAXN = 100005, total memory ≈ 22 MB. |
| **Complex implementation** | The cloning logic and suffix link management make the SAM significantly harder to implement and debug compared to simpler structures like a hash set of substrings. |
| **No positional information** | The basic SAM does not store which positions substrings occur at. Tracking occurrences requires additional data (endpos sets) computed via a topological sort. |
| **Not suitable for 2D/tree patterns** | The SAM is designed for linear strings only. |

---

## 9. Complexity Analysis

### Construction

| Metric | Bound | Explanation |
| :--- | :--- | :--- |
| Number of states | ≤ 2n − 1 | Proven by the endpos equivalence class theory |
| Number of transitions | ≤ 3n − 4 | Follows from the state bound and DAG structure |
| Construction time | O(n) | Each call to `sa_extend()` is O(1) amortized |
| Construction space | O(n) | O(26) per state × O(n) states |

### Amortized Analysis of `sa_extend()`

Each call may invoke one or two `while` loops walking up suffix links. Define a potential function Φ = (current match length `l`).

- Each call increases `l` by at most 1 (one new character added).
- Each suffix link step in the `while` loop decreases `l` by at least 1.
- Since `l` can only increase `n` times total (once per character), the total number of suffix link steps across all `n` calls is `O(n)`.
- Therefore, each call costs `O(1)` amortized.

### Query Operations

| Operation | Time | Space |
| :--- | :--- | :--- |
| Pattern matching (`contains`) | O(m) | O(1) extra |
| Longest Common Substring (`solve_lcs`) | O(k) | O(1) extra |

Where `m` is the pattern length and `k` is the length of the second string.

### Comparison with Other Structures

| Structure | Build Time | Build Space | Query Time |
| :--- | :--- | :--- | :--- |
| Brute Force | O(1) | O(1) | O(n × m) |
| KMP | O(m) pattern preprocessing | O(m) | O(n + m) |
| Suffix Array | O(n log n) or O(n) | O(n) | O(m log n) |
| Suffix Tree | O(n) | O(n) | O(m) |
| **Suffix Automaton** | **O(n)** | **O(n)** | **O(m)** |

The SAM matches the Suffix Tree in asymptotic complexity but is typically easier to implement correctly and has better cache performance due to its more regular structure.

---

## 10. Future Scope

The Suffix Automaton implemented here is a foundation. Several extensions are possible:

### 1. Generalised Suffix Automaton
Build a single SAM for **multiple strings** simultaneously. This enables efficient LCS computation across many strings, used in bioinformatics for multi-sequence alignment.

### 2. Counting Distinct Substrings
The number of distinct substrings of `s` equals the sum over all states (except root) of `(st[v].len - st[st[v].link].len)`. This is computable in `O(n)` from the SAM.

### 3. Lexicographically K-th Substring
By counting paths through the SAM, it is possible to find the k-th lexicographically smallest distinct substring in `O(n + k log σ)` time, where σ is the alphabet size.

### 4. Longest Repeated Substring
The longest repeated substring of `s` corresponds to the deepest internal node in the suffix link tree (i.e., the state with the largest `len` that has more than one endpos).

### 5. Number of Occurrences of a Pattern
By computing the size of the endpos set for each state (via a topological sort and propagation through suffix links), the SAM can report how many times a pattern occurs in `s` in `O(n)` preprocessing + `O(m)` query.

### 6. Palindrome and Periodicity Detection
Combined with other structures (e.g., Eertree / palindromic tree), SAM-based techniques can solve complex string problems involving palindromes, periods, and borders.

### 7. Extended Alphabet Support
The current implementation uses `next[26]` (fixed array) for efficiency with lowercase Latin letters. For Unicode or arbitrary alphabets, replacing this with a hash map (`unordered_map<char, int>`) generalises the structure at a small constant-time cost per transition.

---

## 11. References

### Foundational Papers
- Blumer, A., Blumer, J., Haussler, D., Ehrenfeucht, A., Chen, M., & Seiferas, J. (1985). *The Smallest Automaton Recognizing the Subwords of a Text*. Theoretical Computer Science, 40, 31–55.
- Crochemore, M., & Hancart, C. (1997). *Automata for Matching Patterns*. In Handbook of Formal Languages. Springer.
- Mohri, M., Moreno, P., & Weinstein, E. (2009). *Efficient Large-Scale Music Similarity Search*. (Application of string automata.)

### Online Resources
- [cp-algorithms.com — Suffix Automaton](https://cp-algorithms.com/string/suffix-automaton.html) — The primary algorithmic reference for this implementation.
- [Codeforces Blog — SAM Tutorial by e-maxx](https://codeforces.com/blog/entry/54400) — Community explanation with worked examples.
- [Stanford ICPC Notebook](https://github.com/jaehyunp/stanfordacm) — Competitive programming reference implementations.

### Textbooks
- Crochemore, M., & Lecroq, T. (2007). *Stringology*. In Algorithms and Theory of Computation Handbook.
- Gusfield, D. (1997). *Algorithms on Strings, Trees, and Sequences*. Cambridge University Press.

---

*For a line-by-line explanation of the code, see [`CODE_EXPLANATION.md`](./CODE_EXPLANATION.md).*  
*For a project overview and usage instructions, see [`README.md`](./README.md).*
