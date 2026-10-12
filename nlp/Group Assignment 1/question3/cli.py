#!/usr/bin/env python3
"""
Question 3 - Part 5: Continuous Terminal CLI Application
Interactive spelling corrector running a continuous while-loop.
Prompts user for input, outputs corrected sentence, visually highlights
changed words with ANSI colors and asterisks, prints latency in ms,
and exits when user enters 'exit'.
"""

import sys
import os
import re
import time
import pickle
from collections import Counter, defaultdict

# Terminal Colors
GREEN = "\033[1;92m"
YELLOW = "\033[1;93m"
CYAN = "\033[1;96m"
BOLD = "\033[1m"
RESET = "\033[0m"

DATA_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data", "brown_corpus_model.pkl")


class SelfContainedCorrector:
    def __init__(self, data_file=DATA_PATH):
        with open(data_file, "rb") as f:
            d = pickle.load(f)
        self.vocab = set(d["vocab"])
        self.unigram = Counter(d["unigram_counts"])
        self.bigram = Counter(d["bigram_counts"])
        self.unigram_h = Counter(d["unigram_history"])

        # Precompute Symmetric Delete map
        self.deletes_map = defaultdict(set)
        for w in self.vocab:
            w_l = w.lower()
            for i in range(len(w_l)):
                self.deletes_map[w_l[:i] + w_l[i+1:]].add(w_l)

    def get_candidates(self, word: str):
        w = word.lower()
        cands = set()
        for i in range(len(w)):
            d = w[:i] + w[i+1:]
            if d in self.vocab and d != w:
                cands.add(d)
            if d in self.deletes_map:
                for target in self.deletes_map[d]:
                    if target != w and abs(len(target) - len(w)) <= 1:
                        cands.add(target)
        if w in self.deletes_map:
            for target in self.deletes_map[w]:
                if len(target) == len(w) + 1:
                    cands.add(target)
        return cands

    def correct_non_word(self, word: str):
        cands = self.get_candidates(word)
        return max(cands, key=lambda c: self.unigram.get(c, 0)) if cands else word

    def correct_real_word(self, prev_w: str, word: str, threshold: float = 15.0):
        w = word.lower()
        p_orig = (self.bigram.get((prev_w, w), 0) + 0.05) / (self.unigram_h.get(prev_w, 0) + 0.05 * (len(self.vocab) + 1))
        cands = self.get_candidates(w)
        if not cands:
            return word, False
        scored = []
        for c in cands:
            p_c = (self.bigram.get((prev_w, c), 0) + 0.05) / (self.unigram_h.get(prev_w, 0) + 0.05 * (len(self.vocab) + 1))
            scored.append((c, p_c))
        best_c, best_p = max(scored, key=lambda x: x[1])
        if best_p >= threshold * p_orig:
            return best_c, True
        return word, False

    def correct_sentence(self, sentence: str):
        words = re.findall(r"[A-Za-z]+|[^\w\s]", sentence)
        corrections = []
        out = []
        prev_w = "<s>"

        for token in words:
            if not token.isalpha():
                out.append(token)
                continue
            t_low = token.lower()
            if t_low not in self.vocab:
                fixed = self.correct_non_word(t_low)
                fixed_fmt = fixed.capitalize() if token[0].isupper() else fixed
                corrections.append({"original": token, "corrected": fixed_fmt, "type": "non-word"})
                out.append(fixed_fmt)
                prev_w = fixed
            elif len(t_low) > 2:
                fixed, changed = self.correct_real_word(prev_w, t_low)
                if changed:
                    fixed_fmt = fixed.capitalize() if token[0].isupper() else fixed
                    corrections.append({"original": token, "corrected": fixed_fmt, "type": "real-word"})
                    out.append(fixed_fmt)
                    prev_w = fixed
                else:
                    out.append(token)
                    prev_w = t_low
            else:
                out.append(token)
                prev_w = t_low

        reconstructed = ""
        for i, t in enumerate(out):
            if i > 0 and t.isalpha() and out[i-1].isalpha():
                reconstructed += " " + t
            elif i > 0 and t.isalpha() and out[i-1] in (".", ",", "!", "?"):
                reconstructed += " " + t
            elif i > 0 and t in (".", ",", "!", "?"):
                reconstructed += t
            else:
                reconstructed += (" " if i > 0 else "") + t

        return {"corrected": reconstructed, "corrections": corrections}


def main():
    print(f"{CYAN}{'=' * 65}{RESET}")
    print(f"{BOLD}  INTERACTIVE SPELLING CORRECTOR CLI (Method B: Symmetric Delete){RESET}")
    print(f"{CYAN}{'=' * 65}{RESET}")
    print("  Initializing Brown Corpus vocabulary and language model...")

    t0 = time.time()
    corrector = SelfContainedCorrector()
    print(f"  Ready in {time.time() - t0:.2f}s! ({len(corrector.vocab):,} vocabulary words)")
    print("  Type any English sentence to correct spelling (type 'exit' to quit).\n")

    while True:
        try:
            user_input = input(f"{BOLD}Enter sentence > {RESET}").strip()
        except (EOFError, KeyboardInterrupt):
            print(f"\n{CYAN}Exiting. Goodbye!{RESET}")
            break

        if not user_input:
            continue
        if user_input.lower() == "exit":
            print(f"{CYAN}Exiting CLI. Goodbye!{RESET}")
            break

        t_start = time.perf_counter()
        result = corrector.correct_sentence(user_input)
        latency_ms = (time.perf_counter() - t_start) * 1000.0

        print(f"\n  Input   : {user_input}")
        print(f"  Output  : {result['corrected']}")

        if result["corrections"]:
            print(f"  {BOLD}Changes:{RESET}")
            for c in result["corrections"]:
                c_color = GREEN if c["type"] == "non-word" else YELLOW
                print(f"    • '{c['original']}' → '{c_color}{BOLD}{c['corrected']}{RESET}' [{c['type']}]")
        else:
            print("  No spelling errors detected.")

        print(f"  {CYAN}Latency : {latency_ms:.2f} ms{RESET}\n")


if __name__ == "__main__":
    main()
