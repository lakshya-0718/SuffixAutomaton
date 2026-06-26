# Suffix Automaton: Code Walkthrough

---

## Table of Contents
1. [Code Overview](#1-code-overview)
2. [Header Files and Macros](#2-header-files-and-macros)
3. [Global Constants and Variables](#3-global-constants-and-variables)
4. [The State Structure](#4-the-state-structure)
5. [sa_init(): Initializing the Automaton](#5-sa_init-initializing-the-automaton)
6. [sa_extend(): Building the Automaton](#6-sa_extend-building-the-automaton)
7. [contains(): Substring Search](#7-contains-substring-search)
8. [solve_lcs(): Longest Common Substring](#8-solve_lcs-longest-common-substring)
9. [main(): Program Entry Point](#9-main-program-entry-point)
10. [Program Flow Summary](#10-program-flow-summary)
11. [Example Execution Trace](#11-example-execution-trace)
12. [Complexity of Each Function](#12-complexity-of-each-function)

---

## 1. Code Overview

The program implements a **Suffix Automaton (SAM)**, a compact directed acyclic word graph that encodes every substring of a given string. The code is organized as follows:

| Section | Lines (approx.) | Purpose |
| :--- | :--- | :--- |
| Headers & Macros | 1–6 | Includes, shortcuts |
| Global Definitions | 8–14 | Constants, State struct, global arrays |
| `sa_init()` | 16–25 | Resets the automaton to an empty initial state |
| `sa_extend(char c)` | 27–68 | Adds one character; the core SAM algorithm |
| `contains(pattern)` | 70–82 | Checks if a pattern exists in the built SAM |
| `solve_lcs(T)` | 84–117 | Finds the Longest Common Substring with string T |
| `main()` | 119–148 | Handles user I/O and orchestrates the program |

---

## 2. Header Files and Macros

```cpp
#include <iostream>
#include <string>
#define endl "\n"
using namespace std;
```

**`#include <iostream>`**  
Includes the standard input/output library. This gives access to:
- `cin`: reads input from the keyboard.
- `cout`: prints output to the terminal.

**`#include <string>`**  
Includes the C++ standard string library, which provides the `string` class. This is used for storing and manipulating input strings (`s`, `t`, `pattern`).

**`#define endl "\n"`**  
This is a **preprocessor macro**. Every occurrence of `endl` in the code is textually replaced with `"\n"` (a newline character) before compilation.  
**Why not use `std::endl`?** `std::endl` flushes the output buffer (which is slow), whereas `"\n"` does not, making the program run much faster. This is standard competitive programming context that justifies the macro's existence.

**`using namespace std;`**  
Allows using standard library names like `cout`, `cin`, `string` directly, without writing `std::cout`, `std::cin` etc. everywhere. This is a convenience shortcut common in competitive programming.

---

## 3. Global Constants and Variables

```cpp
const int MAXN = 100005;
```

**`const int MAXN = 100005;`**  
Defines the maximum length of the input string. The `const` keyword means this value cannot be changed at runtime. It is set to 100005 (slightly more than 100,000) to handle edge cases where the string is exactly 100,000 characters long.
This value determines the size of the `st[]` array below.

```cpp
State st[MAXN * 2];
int sz, last;
```

**`State st[MAXN * 2];`**  
A global array of `State` objects, sized `2 * MAXN`. The Suffix Automaton for a string of length `n` has **at most 2n − 1 states**, so allocating `2 * MAXN` guarantees enough space for any input up to the limit.
Being global means this array is allocated in the **data segment** (not the stack), so it can hold large amounts of data without causing a stack overflow.

**`int sz;`**  
Tracks how many states currently exist in the automaton. It acts as a counter and also gives the index for the next new state to be created (since states are 0-indexed).

**`int last;`**  
Stores the index of the **last state** (the state that represents the entire string built so far). Initially it is 0 (the root). After each call to `sa_extend()`, it is updated to the newest state.

---

## 4. The State Structure

```cpp
struct State {
    int len;
    int link;
    int next[26];
};
```

Each **state** in the Suffix Automaton is represented by this structure. Every state corresponds to an equivalence class of substrings — a set of substrings that all end at the same set of positions in the original string.

**Fields:**

**`int len;`**  
The length of the **longest** substring in the equivalence class this state represents. Think of it as the “maximum reach” of this state. If `len = 5`, the state can represent substrings up to 5 characters long.
For the root (state 0), `len = 0` because it represents the empty string.

**`int link;`**  
The **suffix link**. Every state (except the root) has a suffix link pointing to another state. The suffix link of state `v` points to the state whose equivalence class contains the longest proper suffix of the shortest string in `v`’s class.
Intuitively: suffix links allow you to “back off” to a shorter matching context when the current state doesn’t have a transition for the next character.
The root’s suffix link is `-1` (sentinel indicating “no parent”).

**`int next[26];`**  
An array of 26 integers, one for each lowercase English letter (`'a'` to `'z'`). `next[i]` stores the index of the state you transition to when reading character `'a' + i`.
A value of `-1` means no transition exists for that character from this state.
This is essentially the **transition function** of the automaton, implemented as a fixed-size array for `O(1)` lookup per character.

---

## 5. sa_init(): Initializing the Automaton

```cpp
void sa_init() {
    st[0].len = 0;
    st[0].link = -1;
    for (int i = 0; i < 26; i++) {
        st[0].next[i] = -1;
    }
    sz = 1;
    last = 0;
}
```

This function initializes the Suffix Automaton to its empty starting state, just the root node, with no transitions.

**`st[0].len = 0;`**  
State 0 is the **root state**. It represents the empty string, which has length 0. All traversals begin from here.

**`st[0].link = -1;`**  
The root has no suffix link (it has no “parent” in the suffix link tree). `-1` is used as a sentinel value meaning “no link” / “beyond root”.

**`for (int i = 0; i < 26; i++) { st[0].next[i] = -1; }`**  
Initializes all 26 transitions of the root state to `-1`, meaning no character can be followed from the root initially. As characters are added to the string, transitions will be filled in.

**`sz = 1;`**  
The automaton currently has exactly 1 state (the root, at index 0). `sz` starts at 1 so that the next state created gets index 1.

**`last = 0;`**  
The “last” state (the one representing the entire string built so far) is currently the root, since no characters have been added yet.

---

## 6. sa_extend(): Building the Automaton

This is the heart of the entire program. Each call adds exactly one character to the Suffix Automaton.

```cpp
void sa_extend(char c) {
```
Takes a single character `c` to be appended to the string.

### Step 1: Create New State

```cpp
    int idx = c - 'a';
    int cur = sz++;
    st[cur].len = st[last].len + 1;
    for (int i = 0; i < 26; i++) {
        st[cur].next[i] = -1;
    }
```

**`int idx = c - 'a';`**  
Converts character `c` to a 0-based index. Since `'a'` has ASCII value 97:
- `'a' - 'a' = 0`
- `'b' - 'a' = 1`
- `'z' - 'a' = 25`

This index is used to access the `next[26]` array.

**`int cur = sz++;`**  
Creates a new state with index `sz`, then increments `sz`. The new state `cur` will represent all new suffixes created by appending character `c`.

**`st[cur].len = st[last].len + 1;`**  
The new state’s length is one more than the previous last state’s length. This is because the new state represents substrings that are exactly one character longer than what was previously the longest string.

**`for (int i = 0; i < 26; i++) { st[cur].next[i] = -1; }`**  
Initializes all transitions of the new state to `-1` (no transitions yet).

### Step 2: Walk Up Suffix Links and Add Transitions

```cpp
    int p = last;
    while (p != -1 && st[p].next[idx] == -1) {
        st[p].next[idx] = cur;
        p = st[p].link;
    }
```

**`int p = last;`**  
Start from the previous last state and walk backwards through the suffix link chain.

**`while (p != -1 && st[p].next[idx] == -1)`**  
Continue as long as:
- `p != -1`: we haven’t gone past the root (the root’s suffix link is `-1`, so going one step past the root exits the loop).
- `st[p].next[idx] == -1`: the current state `p` has no transition on character `c` (`idx`).

**`st[p].next[idx] = cur;`**  
For every ancestor state that doesn’t yet have a transition on `c`, add one pointing to the new state `cur`. This ensures that all suffixes ending in `c` are reachable in the automaton.

**`p = st[p].link;`**  
Move to the suffix link of `p` to continue walking up the suffix link chain.

### Step 3: Determine the Suffix Link for cur

After the loop, there are two cases:

#### Case A: p == -1 (walked past the root, no existing transition found anywhere)

```cpp
    if (p == -1) {
        st[cur].link = 0;
    }
```

**`st[cur].link = 0;`**  
If we exhausted all suffix links without finding any state with a transition on `c`, it means character `c` is entirely new to the automaton. The suffix link of `cur` is set directly to the root (state 0), since the shortest suffix of the current string that is a substring of anything in the automaton is the empty string.

#### Case B: p != -1 (found a state p that already has a transition on c)

```cpp
    else {
        int q = st[p].next[idx];
```

**`int q = st[p].next[idx];`**  
`q` is the state that `p` already transitions to on character `c`. We need to check if `q` is the right state to link `cur` to.

##### Sub-case B1: q is already minimal (no cloning needed)

```cpp
        if (st[p].len + 1 == st[q].len) {
            st[cur].link = q;
        }
```

**`if (st[p].len + 1 == st[q].len)`**  
This checks whether `q` is already a “minimal” state (one that begins exactly one step further than `p`). If `p.len + 1 == q.len`, then `q` correctly represents the suffix we need, and we can directly set `cur`’s suffix link to `q`.

**`st[cur].link = q;`**  
Set the suffix link of the new state to `q` directly. No structural changes needed.

##### Sub-case B2: q is not minimal (cloning required)

```cpp
        else {
            int clone = sz++;
            st[clone].len = st[p].len + 1;
            st[clone].link = st[q].link;
            for (int i = 0; i < 26; i++) {
                st[clone].next[i] = st[q].next[i];
            }
```

**`int clone = sz++;`**  
Creates a new **clone state**, i.e., a copy of `q` that will have a shorter `len`. Cloning is necessary to maintain the correct suffix link structure without breaking existing paths.

**`st[clone].len = st[p].len + 1;`**  
The clone gets the correct shorter length: one more than `p.len`. This is the “minimal” boundary that `q` should have had but didn’t.

**`st[clone].link = st[q].link;`**  
The clone inherits `q`’s original suffix link. It takes `q`’s place in the suffix link chain for the shorter context.

**`for (int i = 0; i < 26; i++) { st[clone].next[i] = st[q].next[i]; }`**  
The clone inherits all of `q`’s transitions. This ensures that any string that previously reached `q` from shorter contexts continues to work, now routed through `clone`.

```cpp
            while (p != -1 && st[p].next[idx] == q) {
                st[p].next[idx] = clone;
                p = st[p].link;
            }
```

**`while (p != -1 && st[p].next[idx] == q)`**  
Walk back up suffix links again from the current `p`. Any state that was transitioning to `q` on character `c` must now be redirected to `clone`, because `clone` takes over the shorter role.

**`st[p].next[idx] = clone;`**  
Redirect the transition from `q` to `clone`.

```cpp
            st[q].link = clone;
            st[cur].link = clone;
        }
    }
```

**`st[q].link = clone;`**  
Update `q`’s suffix link to point to `clone`. Since `clone` now represents the shorter versions of what `q` used to represent, `q` correctly delegates to it via the suffix link.

**`st[cur].link = clone;`**  
The new state `cur` also links to `clone`, since `clone` now correctly represents the relevant shorter suffixes.

```cpp
    last = cur;
}
```

**`last = cur;`**  
Update the global `last` pointer to the newly created state `cur`. On the next call to `sa_extend()`, this becomes the starting point for the suffix link walk.

---

## 7. contains(): Substring Search

```cpp
bool contains(const string& pattern) {
    int v = 0;
    int n = pattern.size();
    for (int i = 0; i < n; i++) {
        int idx = pattern[i] - 'a';
        if (idx < 0 || idx >= 26) return false;
        if (st[v].next[idx] == -1) return false;
        v = st[v].next[idx];
    }
    return true;
}
```

This function checks whether `pattern` is a substring of the string that was used to build the SAM.

**`int v = 0;`**  
Start at the root state. Every traversal of the SAM begins here.

**`int n = pattern.size();`**  
Get the length of the pattern string. This determines how many characters we process.

**`for (int i = 0; i < n; i++)`**  
Iterate over each character of the pattern, one by one.

**`int idx = pattern[i] - 'a';`**  
Convert the current character to its 0-based index (same technique as in `sa_extend()`).

**`if (idx < 0 || idx >= 26) return false;`**  
If the character is not a lowercase English letter, it cannot exist in the automaton (which only stores `'a'`–`'z'`). Return false immediately.

**`if (st[v].next[idx] == -1) return false;`**  
If there is no transition from the current state on this character, the pattern cannot be a substring of `s`. Return false immediately without checking further characters.

**`v = st[v].next[idx];`**  
Follow the transition and move to the next state. This is equivalent to reading one more character in the automaton.

**`return true;`**  
If the loop completes without returning `false`, every character of the pattern has a valid transition. The entire pattern exists as a substring in the SAM, so return `true`.

---

## 8. solve_lcs(): Longest Common Substring

```cpp
void solve_lcs(const string& T) {
    int v = 0;
    int l = 0;
    int best_len = 0;
    int best_pos = 0;
    int n = T.size();
```

**`int v = 0;`**  
Current state in the automaton. Start at root.

**`int l = 0;`**  
Current match length: how many consecutive characters from `T` we have successfully matched in the SAM so far.

**`int best_len = 0;`**  
The length of the longest common substring found so far. Starts at 0 (no match found yet).

**`int best_pos = 0;`**  
The ending index in `T` where the best match ended. Used at the end to reconstruct and print the actual substring.

**`int n = T.size();`**  
Length of string `T` (the second string).

```cpp
    for (int i = 0; i < n; i++) {
        int idx = T[i] - 'a';
        if (idx < 0 || idx >= 26) {
            v = 0;
            l = 0;
            continue;
        }
```

**`for (int i = 0; i < n; i++)`**  
Scan through string `T` character by character.

**`int idx = T[i] - 'a';`**  
Convert the current character to its 0-based index.

**`if (idx < 0 || idx >= 26) { v = 0; l = 0; continue; }`**  
If the character is invalid (not `'a'`–`'z'`), reset the state to root and the current match length to 0, then skip to the next character. An invalid character can never be part of a match in the SAM.

```cpp
        while (v != 0 && st[v].next[idx] == -1) {
            v = st[v].link;
            l = st[v].len;
        }
```

**`while (v != 0 && st[v].next[idx] == -1)`**  
If the current state has no transition for character `idx`, follow suffix links to find a state that does. Stop when:
- `v == 0`: we have reached the root (can’t go further back).
- `st[v].next[idx] != -1`: found a state with a valid transition for `idx`.

**`v = st[v].link;`**  
Move to the suffix link of `v`. This reduces the current matching context to the longest proper suffix that is still a valid state in the SAM.

**`l = st[v].len;`**  
After following a suffix link, update the current match length `l` to `st[v].len`. This is because the new state `v` represents substrings only up to `st[v].len` characters long, any longer match is no longer valid.

```cpp
        if (st[v].next[idx] != -1) {
            v = st[v].next[idx];
            l++;
        }
        else {
            l = 0;
        }
```

**`if (st[v].next[idx] != -1)`**  
After the suffix link walk, check if a valid transition now exists.

**`v = st[v].next[idx];`**  
Follow the transition to the next state — successfully matching one more character.

**`l++;`**  
Increase the match length by 1 because we matched one additional character.

**`else { l = 0; }`**  
If even after following all suffix links (reaching the root) there is still no transition for this character, then this character has no match at all. Reset match length to 0. (At this point, `v` is already the root due to the while loop condition.)

```cpp
        if (l > best_len) {
            best_len = l;
            best_pos = i;
        }
    }
```

**`if (l > best_len)`**  
After processing each character of `T`, check if the current match length beats the previous best.

**`best_len = l;`**  
Update the best match length.

**`best_pos = i;`**  
Record the current index `i` in `T` as the ending position of the best match found so far.

```cpp
    if (best_len == 0) {
        cout << "Longest Common Substring: NONE" << endl;
    }
    else {
        cout << "Longest Common Substring Length: " << best_len << endl;
        cout << "Longest Common Substring: " << endl;
        int start = best_pos - best_len + 1;
        for (int i = 0; i < best_len; i++) {
            cout << T[start + i];
        }
        cout << endl;
    }
}
```

**`if (best_len == 0)`**  
If no match was found at all, print “NONE”.

**`int start = best_pos - best_len + 1;`**  
Calculate the starting index of the LCS in `T`. Since `best_pos` is the last index of the match, subtracting `best_len - 1` gives the first index.
Example: if `best_pos = 4` and `best_len = 3`, then `start = 4 - 3 + 1 = 2`. Characters at indices 2, 3, 4 form the LCS.

**`for (int i = 0; i < best_len; i++) { cout << T[start + i]; }`**  
Print the LCS character by character directly from string `T`.

---

## 9. main(): Program Entry Point

```cpp
int main() {
    string s, t, pattern;
```

**`string s, t, pattern;`**  
Declares three string variables:
- `s`: the first string; the SAM is built on this.
- `t`: the second string; used as the query string for LCS.
- `pattern`: the string to search for inside `s`.

```cpp
    cout << "Enter String 1: ";
    cin >> s;
```

Prompts the user and reads the first string into `s`. `cin >>` reads one whitespace-delimited token (spaces terminate input).

```cpp
    sa_init();
    int n = s.size();
    for (int i = 0; i < n; i++) {
        sa_extend(s[i]);
    }
```

**`sa_init();`**  
Initializes the Suffix Automaton and resets the root state, sets `sz = 1`, `last = 0`.

**`int n = s.size();`**  
Gets the length of string `s`.

**`for (int i = 0; i < n; i++) { sa_extend(s[i]); }`**  
Builds the Suffix Automaton by feeding `s` one character at a time. After this loop, the SAM encodes every substring of `s`.

```cpp
    cout << "Enter String 2 for LCS: ";
    cin >> t;
    cout << endl;

    cout << "Application 1: Longest Common Substring" << endl;
    solve_lcs(t);
    cout << endl;
```

Reads string `t`, then calls `solve_lcs(t)` which traverses the SAM built on `s` using characters of `t` to find and print the Longest Common Substring.

```cpp
    cout << "Enter pattern to search in String 1: ";
    cin >> pattern;
    cout << endl;

    cout << "Application 2: Substring Search" << endl;
    if (contains(pattern)) {
        cout << "Pattern exists in String 1" << endl;
    }
    else {
        cout << "Pattern does not exist in String 1" << endl;
    }
    return 0;
}
```

Reads the pattern, calls `contains(pattern)`, and prints whether the pattern is a substring of `s`.

**`return 0;`**  
Returns 0 to the operating system, signaling that the program completed successfully. By convention, any non-zero return value indicates an error.

---

## 10. Program Flow Summary

```text
main()
  │
  ├── Read String s
  │
  ├── sa_init()           →  Reset SAM to empty (root only, sz=1, last=0)
  │
  ├── sa_extend(s[0])     ┐
  ├── sa_extend(s[1])     │  Build SAM on String s
  ├── ...                 │  (one character at a time)
  └── sa_extend(s[n-1])   ┘
  │
  ├── Read String t
  │
  ├── solve_lcs(t)        →  Greedy traversal of SAM with T, track best match
  │
  ├── Read pattern
  │
  └── contains(pattern)   →  Follow transitions in SAM, return true/false
```

---

## 11. Example Execution Trace

### Input:
```
String 1: aba
String 2: bab
Pattern: ab
```

### SAM Construction for "aba":

| Step | Character | New State | len | Key Action |
| :--- | :--- | :--- | :--- | :--- |
| 1 | 'a' | 1 | 1 | Root gets transition 'a' → 1. link [1] = 0 |
| 2 | 'b' | 2 | 2 | States 1, 0 get 'b' → 2. link [2] = 0 |
| 3 | 'a' | 3 | 3 | State 2 gets 'a' → 3. Root already has 'a' → 1 (which is minimal). link [3] = 1 |

After building, the SAM encodes all substrings of "aba": `a`, `b`, `ab`, `ba`, `aba` all reachable from the root.

### LCS Traversal with T = "bab":

| i | T[i] | idx | Current v | Transition? | Action | l | best_len |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 0 | 'b' | 1 | 0 (root) | root -> 2 | v = 2, l = 1 | 1 | 1 |
| 1 | 'a' | 0 | 2 | 2 -> 3 | v = 3, l = 2 | 2 | 2 |
| 2 | 'b' | 1 | 3 | 3 -> ? (no transition)<br>Follow link to state 1<br>1 -> 2 | v = 2, l = 2 | 2 | 2 |

**Result:** `LCS length = 2`. `LCS = "ba"` (ending at `best_pos = 1`). `start = 1 - 2 + 1 = 0`. Characters `T[0], T[1] = "ba"`.  
*Note: At `i = 2`, `l` becomes 2, which is not strictly greater than `best_len` (2), so `best_pos` remains 1.*

### Substring Search for "ab":

| i | Char | idx | State Transition | Result |
| :--- | :--- | :--- | :--- | :--- |
| 0 | 'a' | 0 | root (0) -> state 1 | v = 1 |
| 1 | 'b' | 1 | state 1 -> state 2 | v = 2 |

Loop ends without returning false. -> **return true**  
Output: `Pattern exists in String 1`

---

## 12. Complexity of Each Function

| Function | Time Complexity | Space Complexity | Notes |
| :--- | :--- | :--- | :--- |
| `sa_init()` | O(1) | O(1) | Constant work:<br>Initialize root, fill 26 slots |
| `sa_extend(c)` | O(1) amortized | O(1) per call | Inner while loops amortize to O(n) total across all calls |
| **Full SAM Build** | **O(n)** | **O(n)** | n calls to `sa_extend()`; at most 2n-1 states created |
| `contains(p)` | O(m) | O(1) extra | m = length of pattern; single pass, O(1) per character |
| `solve_lcs(T)` | O(k) | O(1) extra | k = length of T; suffix link walk amortizes to O(k) |
| **Overall Program**| **O(n + k + m)** | **O(n)** | n = \|S\|, k = \|T\|, m = \|pattern\| |

### Why is sa_extend() O(1) amortized?

Each call to `sa_extend()` may walk up several suffix links in the inner `while` loop. However, across all `n` calls (one per character of `s`), the total number of steps is bounded.

The key insight: the variable tracking match length can **increase by at most 1** per call (when we successfully extend), but each step in the suffix link walk **decreases** it. Since the total number of increases across all `n` calls is at most `n`, the total number of decreases (i.e., suffix link steps) is also bounded by `O(n)`. This gives an average cost of **O(1) per call**: the classic **amortized argument**.

Similarly in `solve_lcs()`, `l` increases by at most 1 per character of `T`, and each suffix link step decreases it, so the total suffix link steps are `O(k)`.
