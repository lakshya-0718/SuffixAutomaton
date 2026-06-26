// Demonstrates two applications of the Suffix Automaton:
// 1. Longest Common Substring
// 2. Substring Search (Pattern Matching)
#include <iostream>
#include <string>
#define endl "\n"
using namespace std;

const int MAXN = 100005; // max input string size

// Assumption: Input strings contain only lowercase English letters ('a' to 'z').
// The transition array next[26] maps each lowercase letter to an index 0-25.
struct State {
    int len; // length of longest string in state
    int link; // suffix link
    int next[26]; // transitions 
};

State st[MAXN * 2]; // 2*n-1 max states, stores all states
int sz, last; // current states, state representing the string built

// initializing suffix automaton
void sa_init() {
    st[0].len = 0; // length of root state
    st[0].link = -1; // root has no suffix link
    for (int i = 0; i < 26; i++) {
        st[0].next[i] = -1; // initially, no transitions
    } 
    sz = 1; // only root state exists
    last = 0; // currently, the last state is root
}

// adds character to suffix automaton
void sa_extend(char c) {
    int idx = c - 'a'; // converts character to index
    int cur = sz++; // new state curr, increase state count
    st[cur].len = st[last].len + 1; // curr represents string one char longer than last
    for (int i = 0; i < 26; i++) {
        st[cur].next[i] = -1; // initialize all transitions as absent
    }
    int p = last; // walking backward from previous last state
    while (p != -1 && st[p].next[idx] == -1) { // current state has no transition for this char
        st[p].next[idx] = cur; // add a transition to current state (string extended)
        p = st[p].link; // move to suffix link of p
    }
    if (p == -1) { // beyond root
        st[cur].link = 0; // suffix link of current state to root
    } 
    else {
        int q = st[p].next[idx]; // new state by transition
        if (st[p].len + 1 == st[q].len) { // if current transition correct & minimal
            st[cur].link = q; // set suffix link directly to q
        } 
        else { // clone
            int clone = sz++; // create clone state, increase count of states
            st[clone].len = st[p].len + 1; // clone gets proper length boundary
            st[clone].link = st[q].link; // clone gets same suffix link as q
            for (int i = 0; i < 26; i++) {
                st[clone].next[i] = st[q].next[i]; // copy all transitions from q to clone
            }
            while (p != -1 && st[p].next[idx] == q) { // backward through suffix links
                st[p].next[idx] = clone; // transition to q changed to clone
                p = st[p].link; 
            }
            st[q].link = clone; // old state q point suffix link to clone
            st[cur].link = clone; // new state cur point suffix link to clone
        }
    }
    last = cur; // updates the last state to newly added state
}

// Traverses the suffix automaton from the root, checks pattern
bool contains(const string& pattern) {
    int v = 0; // root state
    int n = pattern.size(); // length of pattern string
    for (int i = 0; i < n; i++) { // pattern processed character wise
        int idx = pattern[i] - 'a'; // index of each character 
        if (idx < 0 || idx >= 26) return false; // if not lowercase english letter
        if (st[v].next[idx] == -1) return false; // if no transition for character in that state
        v = st[v].next[idx]; // if transition exists, move to it
    }
    return true;
}

// Finds one Longest Common Substring (LCS) 
// If multiple LCS of the same maximum length exist, this implementation returns one of them.
void solve_lcs(const string& T) {
    int v = 0; // current state in automaton
    int l = 0; // length of current matched substring
    int best_len = 0; // length of longest matched substring so far
    int best_pos = 0; // indexing of position matched in T
    int n = T.size(); // length of string T
    for (int i = 0; i < n; i++) { // scanning string T
        int idx = T[i] - 'a'; // character to index
        if (idx < 0 || idx >= 26) { // if invalid match - reset
            v = 0;
            l = 0;
            continue;
        }
        while (v != 0 && st[v].next[idx] == -1) { // no transition, follow suffix link until transition or root
            v = st[v].link; 
            l = st[v].len;
        }
        if (st[v].next[idx] != -1) { // if valid transition, move to that state, increase length
            v = st[v].next[idx];
            l++;
        } 
        else {
            l = 0; // no transition found even after following suffix links, reset length to 0
        }
        if (l > best_len) { // when a longer match is found, update the answer
            best_len = l;
            best_pos = i;
        }
    }
    if (best_len == 0) { // no common substring found
        cout << "Longest Common Substring: NONE" << endl;
    }
    else {
        cout << "Longest Common Substring Length: " << best_len << endl; // length of lcs
        cout << "Longest Common Substring: " << endl;
        int start = best_pos - best_len + 1;
        for (int i = 0; i < best_len; i++) { // printing the substring character by character
            cout << T[start + i];
        }
        cout << endl;
    }
}

int main() {
    string s, t, pattern; // s: first string, t: second string, pattern: substring to check

    cout << "Enter String 1: ";
    cin >> s;

    sa_init(); // initializing suffix automaton
    int n = s.size(); // length of string s

    for (int i = 0; i < n; i++) { // building suffix automaton
        sa_extend(s[i]);
    }

    cout << "Enter String 2 for LCS: ";
    cin >> t;
    cout << endl;

    cout << "Application 1: Longest Common Substring" << endl;
    solve_lcs(t);
    cout << endl;

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