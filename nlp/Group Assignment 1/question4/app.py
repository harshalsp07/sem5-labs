"""
Question 4: Integrated Background Editor
Standalone Streamlit Web Application: Live-Typing Editor, Background Streaming Simulation,
Real-Time Alert Feed, and End-of-Passage Constituency & N-Gram Structural Analysis.
"""

import os
import sys
import time
import re
import math
import random
import pickle
from collections import Counter, defaultdict
from typing import List, Tuple, Dict, Optional, Any, Set

import pandas as pd
import streamlit as st
import nltk
from nltk import Nonterminal, induce_pcfg
from nltk.corpus import treebank, brown, gutenberg

# Data paths
DATA_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")
Q1_CACHE = os.path.join(DATA_DIR, "q1_english_models.pkl")
Q3_CACHE = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "question3", "data", "brown_corpus_model.pkl")
PCFG_CACHE = os.path.join(DATA_DIR, "treebank_pcfg.pkl")
LM_CACHE = os.path.join(DATA_DIR, "shared_ngram_lm.pkl")


# --- Part 1: PCFG Constituency Parser ---
class PCFGConstituencyParser:
    def __init__(self):
        self.lexical_rules = defaultdict(dict)
        self.binary_rules = defaultdict(list)
        self.unary_rules = defaultdict(list)

    def train(self, cache_file: str = PCFG_CACHE):
        if os.path.exists(cache_file):
            try:
                with open(cache_file, "rb") as f:
                    data = pickle.load(f)
                    self.lexical_rules = defaultdict(dict, data["lexical"])
                    self.binary_rules = defaultdict(list, {tuple(k): v for k, v in data["binary"].items()})
                    self.unary_rules = defaultdict(list, data["unary"])
                    return
            except Exception:
                pass

        try:
            trees = treebank.parsed_sents()
        except LookupError:
            nltk.download("treebank", quiet=True)
            trees = treebank.parsed_sents()

        prods = []
        for tree in trees:
            t = tree.copy(deep=True)
            t.chomsky_normal_form(horzMarkov=1)
            prods.extend(t.productions())

        pcfg = induce_pcfg(Nonterminal("S"), prods)
        for prod in pcfg.productions():
            lhs = prod.lhs().symbol()
            log_p = math.log(prod.prob()) if prod.prob() > 0 else -100.0
            rhs = prod.rhs()
            if len(rhs) == 1 and isinstance(rhs[0], str):
                self.lexical_rules[rhs[0].lower()][lhs] = max(self.lexical_rules[rhs[0].lower()].get(lhs, -100.0), log_p)
            elif len(rhs) == 2:
                self.binary_rules[(rhs[0].symbol(), rhs[1].symbol())].append((lhs, log_p))
            elif len(rhs) == 1:
                self.unary_rules[rhs[0].symbol()].append((lhs, log_p))

        try:
            with open(cache_file, "wb") as f:
                pickle.dump({
                    "lexical": dict(self.lexical_rules),
                    "binary": dict(self.binary_rules),
                    "unary": dict(self.unary_rules)
                }, f)
        except Exception:
            pass

    def parse_log_probability(self, words: List[str]) -> Tuple[Optional[float], bool]:
        N = len(words)
        if N == 0:
            return None, False
        chart = [[{} for _ in range(N + 1)] for _ in range(N + 1)]

        for i, w in enumerate(words):
            w_l = w.lower()
            if w_l in self.lexical_rules:
                for lhs, lp in self.lexical_rules[w_l].items():
                    chart[i][i+1][lhs] = max(chart[i][i+1].get(lhs, -1000.0), lp)
            if not chart[i][i+1]:
                chart[i][i+1]["NN"] = -9.0
            for b, p_b in list(chart[i][i+1].items()):
                for a, p_r in self.unary_rules.get(b, []):
                    tot = p_b + p_r
                    if tot > chart[i][i+1].get(a, -1000.0):
                        chart[i][i+1][a] = tot

        for span in range(2, N + 1):
            for i in range(N - span + 1):
                j = i + span
                for k in range(i + 1, j):
                    left, right = chart[i][k], chart[k][j]
                    if not left or not right:
                        continue
                    for b, p_b in left.items():
                        for c, p_c in right.items():
                            for a, p_r in self.binary_rules.get((b, c), []):
                                tot = p_b + p_c + p_r
                                if tot > chart[i][j].get(a, -1000.0):
                                    chart[i][j][a] = tot
                for b, p_b in list(chart[i][j].items()):
                    for a, p_r in self.unary_rules.get(b, []):
                        tot = p_b + p_r
                        if tot > chart[i][j].get(a, -1000.0):
                            chart[i][j][a] = tot

        top = chart[0][N]
        root_cats = ["S", "SINV", "SQ", "FRAG", "NP"]
        best_p = max([top[c] for c in root_cats if c in top], default=None)
        if best_p is not None:
            return round(best_p, 2), True
        elif top:
            best_cat = max(top.keys(), key=lambda k: top[k])
            return round(top[best_cat], 2), True
        return None, False


# --- Part 2: Shared Smoothed N-gram LM ---
class SharedNgramLM:
    def __init__(self, k: float = 0.05):
        self.k = k
        self.vocab = set()
        self.unigram_counts = Counter()
        self.bigram_counts = Counter()
        self.bigram_history = Counter()
        self.trigram_counts = Counter()
        self.trigram_history = Counter()

    def train(self, cache_file: str = LM_CACHE):
        if os.path.exists(cache_file):
            try:
                with open(cache_file, "rb") as f:
                    d = pickle.load(f)
                    self.vocab = set(d["vocab"])
                    self.unigram_counts = Counter(d["unigram_counts"])
                    self.bigram_counts = Counter(d["bigram_counts"])
                    self.bigram_history = Counter(d["bigram_history"])
                    self.trigram_counts = Counter(d["trigram_counts"])
                    self.trigram_history = Counter(d["trigram_history"])
                    return
            except Exception:
                pass

        try:
            sents = brown.sents()
        except LookupError:
            nltk.download("brown", quiet=True)
            sents = brown.sents()

        for s in sents:
            words = [w.lower() for w in s if re.match(r"^[a-z]+$", w.lower())]
            if not words:
                continue
            pad = ["<s>", "<s>"] + words + ["</s>"]
            for i in range(2, len(pad)):
                w1, w2, w3 = pad[i-2], pad[i-1], pad[i]
                self.trigram_counts[(w1, w2, w3)] += 1
                self.trigram_history[(w1, w2)] += 1
                self.bigram_counts[(w2, w3)] += 1
                self.bigram_history[w2] += 1
                self.unigram_counts[w3] += 1
                if w3 not in ("<s>", "</s>"):
                    self.vocab.add(w3)

    def score_sentence(self, words: List[str]) -> Tuple[float, float, float, float]:
        clean = [w.lower() for w in words if re.match(r"^[a-z]+$", w.lower())]
        if not clean:
            return 0.0, 1.0, 0.0, 1.0
        v_size = len(self.vocab) + 1
        
        bi_pad = ["<s>"] + clean + ["</s>"]
        bi_log = sum(math.log(max((self.bigram_counts.get((bi_pad[i-1], bi_pad[i]), 0) + self.k) /
                                  (self.bigram_history.get(bi_pad[i-1], 0) + self.k * v_size), 1e-15))
                     for i in range(1, len(bi_pad)))
        bi_ppl = math.exp(-bi_log / len(clean))

        tri_pad = ["<s>", "<s>"] + clean + ["</s>"]
        tri_log = sum(math.log(max((self.trigram_counts.get((tri_pad[i-2], tri_pad[i-1], tri_pad[i]), 0) + self.k) /
                                   (self.trigram_history.get((tri_pad[i-2], tri_pad[i-1]), 0) + self.k * v_size), 1e-15))
                      for i in range(2, len(tri_pad)))
        tri_ppl = math.exp(-tri_log / len(clean))
        return round(bi_log, 2), round(bi_ppl, 2), round(tri_log, 2), round(tri_ppl, 2)


# --- Part 3: Live Background Typing Engine ---
class BackgroundTypingEngine:
    def __init__(self, pcfg: PCFGConstituencyParser, shared_lm: SharedNgramLM):
        self.pcfg = pcfg
        self.lm = shared_lm

        # Load Q1 segmenter dictionary
        with open(Q1_CACHE, "rb") as f:
            q1_data = pickle.load(f)
        self.q1_vocab = set(q1_data["lm_vocab"])
        self.q1_tri = q1_data["lm_tri"]
        self.q1_bi = q1_data["lm_bi"]
        self.q1_uni = q1_data["lm_uni"]
        self.q1_h_bi = q1_data["lm_hist_bi"]
        self.q1_h_uni = q1_data["lm_hist_uni"]
        self.q1_tot = q1_data["lm_total_tokens"]

        # Load Q3 corrector dictionary
        with open(Q3_CACHE, "rb") as f:
            q3_data = pickle.load(f)
        self.q3_vocab = set(q3_data["vocab"])
        self.q3_unigram = q3_data["unigram_counts"]
        self.q3_bigram = q3_data["bigram_counts"]
        self.q3_unigram_h = q3_data["unigram_history"]

        # Precompute SymSpell map
        self.q3_deletes = defaultdict(set)
        for w in self.q3_vocab:
            w_low = w.lower()
            for i in range(len(w_low)):
                self.q3_deletes[w_low[:i] + w_low[i+1:]].add(w_low)

    def _q1_prob(self, w1, w2, w3):
        mle_tri = self.q1_tri.get((w1, w2, w3), 0) / self.q1_h_bi.get((w1, w2), 1) if (w1, w2) in self.q1_h_bi else 0.0
        mle_bi = self.q1_bi.get((w2, w3), 0) / self.q1_h_uni.get(w2, 1) if w2 in self.q1_h_uni else 0.0
        mle_uni = self.q1_uni.get(w3, 0) / self.q1_tot
        p_unk = 1e-10 if w3 not in self.q1_vocab else 1.0 / self.q1_tot
        return max(0.5 * mle_tri + 0.3 * mle_bi + 0.18 * mle_uni + 0.02 * p_unk, 1e-15)

    def segment_token(self, text: str) -> List[str]:
        N = len(text)
        dp = [{} for _ in range(N + 1)]
        dp[0][("<s>", "<s>")] = (0.0, None)
        for j in range(N):
            curr = dp[j]
            if not curr:
                continue
            cands = []
            for l in range(1, min(25, N - j + 1)):
                sub = text[j : j + l]
                if sub in self.q1_vocab:
                    cands.append((j + l, sub))
            sub_1 = text[j : j + 1]
            if not any(k == j + 1 and w == sub_1 for k, w in cands):
                cands.append((j + 1, sub_1))
            for k, w_next in cands:
                for (w_prev, w_curr), (score, _) in curr.items():
                    tot = score + math.log(self._q1_prob(w_prev, w_curr, w_next))
                    pair = (w_curr, w_next)
                    if pair not in dp[k] or tot > dp[k][pair][0]:
                        dp[k][pair] = (tot, (j, w_prev))
        if not dp[N]:
            return [text]
        best_end = max(dp[N].items(), key=lambda x: x[1][0])
        res = []
        curr_pos = N
        curr_u, curr_v = best_end[0]
        curr_bp = best_end[1][1]
        while curr_pos > 0 and curr_bp is not None:
            prev_pos, prev_u = curr_bp
            res.append(curr_v)
            curr_pos, curr_v, curr_u = prev_pos, curr_u, prev_u
            if curr_pos > 0:
                curr_bp = dp[curr_pos].get((curr_u, curr_v), (None, None))[1]
        res.reverse()
        return res

    def get_edit1_candidates(self, word: str) -> Set[str]:
        w = word.lower()
        cands = set()
        for i in range(len(w)):
            d = w[:i] + w[i+1:]
            if d in self.q3_vocab and d != w:
                cands.add(d)
            if d in self.q3_deletes:
                for target in self.q3_deletes[d]:
                    if target != w and abs(len(target) - len(w)) <= 1:
                        cands.add(target)
        if w in self.q3_deletes:
            for target in self.q3_deletes[w]:
                if len(target) == len(w) + 1:
                    cands.add(target)
        return cands

    def check_token(self, token: str) -> Tuple[List[str], List[Dict[str, Any]]]:
        alerts = []
        clean = re.sub(r"[^\w]", "", token).lower()
        if not clean or not clean.isalpha():
            return [token], alerts

        # 1. [SEGMENT-ALERT]
        if clean not in self.q3_vocab and len(clean) >= 6:
            splits = self.segment_token(clean)
            if len(splits) > 1 and all(w in self.q3_vocab for w in splits):
                alerts.append({
                    "type": "SEGMENT-ALERT",
                    "original": token,
                    "replacement": " ".join(splits),
                    "details": f"Split merged token into: {' + '.join(splits)}",
                })
                return splits, alerts

        # 2. [SPELL-ALERT]
        if clean not in self.q3_vocab:
            cands = self.get_edit1_candidates(clean)
            if cands:
                fixed = max(cands, key=lambda c: self.q3_unigram.get(c, 0))
                fixed_formatted = fixed.capitalize() if token[0].isupper() else fixed
                alerts.append({
                    "type": "SPELL-ALERT",
                    "original": token,
                    "replacement": fixed_formatted,
                    "details": f"Corrected '{token}' &rarr; '{fixed_formatted}'",
                })
                return [fixed_formatted], alerts

        return [token], alerts

    def check_grammar_window(self, window_tokens: List[str]) -> Optional[Dict[str, Any]]:
        clean_words = [w.lower() for w in window_tokens if w.isalpha()]
        if len(clean_words) < 2:
            return None

        # Check real-word error on last pair
        prev_w, curr_w = clean_words[-2], clean_words[-1]
        p_orig = (self.q3_bigram.get((prev_w, curr_w), 0) + 0.05) / (self.q3_unigram_h.get(prev_w, 0) + 0.05 * (len(self.q3_vocab) + 1))
        cands = self.get_edit1_candidates(curr_w)
        if cands:
            scored = []
            for c in cands:
                p_c = (self.q3_bigram.get((prev_w, c), 0) + 0.05) / (self.q3_unigram_h.get(prev_w, 0) + 0.05 * (len(self.q3_vocab) + 1))
                scored.append((c, p_c))
            best_c, best_p = max(scored, key=lambda x: x[1])
            if best_p >= 15.0 * p_orig:
                return {
                    "type": "GRAMMAR-ALERT",
                    "details": f"Contextual error in '{prev_w} {curr_w}' &rarr; suggested '{prev_w} {best_c}'",
                }

        # Perplexity check if 4+ tokens
        if len(clean_words) >= 4:
            _, _, _, tri_ppl = self.lm.score_sentence(clean_words)
            if tri_ppl > 1200.0:
                return {
                    "type": "GRAMMAR-ALERT",
                    "details": f"Unusual local phrasing '{' '.join(clean_words)}' (PPL = {tri_ppl:.1f})",
                }
        return None


# --- Part 4: Passage Analyzer & Helpers ---
def sample_passage_from_corpus(corpus_name: str = "gutenberg", min_sentences: int = 5, max_sentences: int = 6) -> List[str]:
    try:
        sents = gutenberg.sents() if corpus_name == "gutenberg" else brown.sents()
    except LookupError:
        nltk.download(corpus_name, quiet=True)
        sents = gutenberg.sents() if corpus_name == "gutenberg" else brown.sents()
    n = random.randint(min_sentences, max_sentences)
    start = random.randint(100, max(100, len(sents) - n - 10))
    return [" ".join(s) for s in sents[start : start + n]]

def introduce_fast_typing_merges(text: str, p: float = 0.08) -> str:
    tokens = text.split(" ")
    merged = [tokens[0]]
    for i in range(1, len(tokens)):
        if random.random() < p and merged[-1].isalpha() and tokens[i].isalpha():
            merged[-1] = merged[-1] + tokens[i]
        else:
            merged.append(tokens[i])
    return " ".join(merged)

class PassageAnalyzer:
    def __init__(self, pcfg: PCFGConstituencyParser, shared_lm: SharedNgramLM):
        self.pcfg = pcfg
        self.lm = shared_lm

    def analyze_passage(self, passage: str, num_seg_merges: int = 0, num_spell_corrs: int = 0) -> List[Dict[str, Any]]:
        sentences = [s.strip() for s in re.split(r"(?<=[.!?])\s+", passage) if s.strip()]
        results = []
        for idx, sent in enumerate(sentences):
            words = re.findall(r"[A-Za-z]+", sent)
            if not words:
                continue
            pcfg_log, pcfg_ok = self.pcfg.parse_log_probability(words)
            pcfg_res = f"{pcfg_log:.2f}" if pcfg_ok else "unparseable"
            bi_log, bi_ppl, tri_log, tri_ppl = self.lm.score_sentence(words)

            if pcfg_ok:
                chosen = "PCFG"
                verdict = "Grammatical (Valid Treebank Structure)"
            elif tri_ppl < 800:
                chosen = "Trigram LM"
                verdict = "Grammatical (Plausible Word Sequence)"
            else:
                chosen = "Bigram LM"
                verdict = "Flagged (High Perplexity / Atypical Structure)"

            results.append({
                "Sentence": sent,
                "PCFG Result": pcfg_res,
                "Bigram Score": f"{bi_log:.1f} (PPL: {bi_ppl:.1f})",
                "Trigram Score": f"{tri_log:.1f} (PPL: {tri_ppl:.1f})",
                "Chosen Method": chosen,
                "Final Verdict": verdict,
                "Seg Merges Resolved": num_seg_merges if idx == 0 else 0,
                "Spell Corrections": num_spell_corrs if idx == 0 else 0,
            })
        return results


# --- Streamlit UI Setup ---
st.set_page_config(
    page_title="Integrated Background NLP Editor",
    page_icon="✍️",
    layout="wide",
    initial_sidebar_state="expanded",
)

st.markdown("""
<style>
    @import url('https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&family=JetBrains+Mono:wght@400;500&display=swap');
    html, body, [class*="css"] { font-family: 'Inter', sans-serif; }
    .main-title {
        font-size: 2.2rem; font-weight: 800;
        background: linear-gradient(135deg, #6366f1 0%, #a855f7 50%, #ec4899 100%);
        -webkit-background-clip: text; -webkit-text-fill-color: transparent;
        margin-bottom: 0.2rem;
    }
    .subtitle { color: #94a3b8; font-size: 1.05rem; margin-bottom: 1.5rem; }
    .alert-card { padding: 10px 14px; border-radius: 6px; margin-bottom: 8px; font-size: 0.90rem; font-family: 'JetBrains Mono', monospace; border-left: 4px solid; }
    .alert-segment { background-color: #1e1b4b; border-color: #6366f1; color: #c7d2fe; }
    .alert-spell { background-color: #451a03; border-color: #f59e0b; color: #fde68a; }
    .alert-grammar { background-color: #4c0519; border-color: #f43f5e; color: #fecdd3; }
</style>
""", unsafe_allow_html=True)


@st.cache_resource(show_spinner=False)
def load_editor_models():
    pcfg = PCFGConstituencyParser()
    pcfg.train()
    shared_lm = SharedNgramLM()
    shared_lm.train()
    engine = BackgroundTypingEngine(pcfg=pcfg, shared_lm=shared_lm)
    analyzer = PassageAnalyzer(pcfg=pcfg, shared_lm=shared_lm)
    return pcfg, shared_lm, engine, analyzer


pcfg, shared_lm, engine, analyzer = load_editor_models()

st.markdown('<div class="main-title">Integrated Background NLP Editor</div>', unsafe_allow_html=True)
st.markdown(
    '<div class="subtitle">Live Word Segmentation, Symmetric-Delete Spelling Correction, and PCFG Constituency Grammar Checking</div>',
    unsafe_allow_html=True,
)

st.sidebar.header("⚙️ Editor Configuration")
app_mode = st.sidebar.radio("Choose Mode:", ["Simulated Background Streaming", "Live Interactive Typing"])

st.sidebar.markdown("---")
st.sidebar.subheader("Parameters")
p_merge = st.sidebar.slider("Fast-Typing Merge Probability (p)", 0.0, 0.20, 0.08, 0.01)
trigger_N = st.sidebar.slider("Grammar Trigger Interval (N)", 2, 8, 4, 1)

if app_mode == "Simulated Background Streaming":
    st.sidebar.markdown("---")
    st.sidebar.subheader("Corpus Sampler")
    corpus_choice = st.sidebar.selectbox("Sample Passage From:", ["Gutenberg (Jane Austen)", "Brown (News/Fiction)"])
    corpus_key = "gutenberg" if "Gutenberg" in corpus_choice else "brown"
    min_sents = st.sidebar.slider("Min Sentences", 4, 8, 5)
    max_sents = st.sidebar.slider("Max Sentences", min_sents, 10, 6)
    typing_delay = st.sidebar.slider("Typing Delay (ms/word)", 0, 100, 20)

    col1, col2 = st.columns([1.3, 1.0])

    with col1:
        st.subheader("📄 Simulated Passage Stream")
        if st.button("🎲 Sample New Passage & Run Live Simulation", type="primary"):
            st.session_state["passage"] = sample_passage_from_corpus(corpus_name=corpus_key, min_sentences=min_sents, max_sentences=max_sents)
            st.session_state["run_simulation"] = True

        passage_sents = st.session_state.get("passage", [
            "The quickbrown fox jumped over the lazy dog.",
            "I would like to sea the beautiful world.",
            "Please meat me at the train station tomorrow.",
            "She had a goodfeeling about this entire journey.",
            "Their respectability was as dear to her as her own.",
        ])
        passage_text = " ".join(passage_sents)
        st.info(f"**Loaded Passage ({len(passage_sents)} sentences):**\n\n{passage_text}")

    with col2:
        st.subheader("⚡ Live Diagnostics & Latency")
        m_col1, m_col2, m_col3 = st.columns(3)
        tok_metric = m_col1.empty()
        gram_metric = m_col2.empty()
        alerts_metric = m_col3.empty()

    if st.session_state.get("run_simulation", False):
        st.markdown("### 🔴 Real-Time Stream & Live Alert Feed")
        corrupted = introduce_fast_typing_merges(passage_text, p=p_merge)
        stream_tokens = corrupted.split(" ")

        stream_box = st.empty()
        alert_container = st.container()

        displayed = []
        alerts_log = []
        window = []

        total_tok_lat = 0.0
        total_gram_lat = 0.0
        tok_count = 0
        gram_count = 0

        for i, raw_tok in enumerate(stream_tokens):
            t0 = time.perf_counter()
            resolved, tok_alerts = engine.check_token(raw_tok)
            total_tok_lat += (time.perf_counter() - t0) * 1000.0
            tok_count += 1

            for a in tok_alerts:
                alerts_log.append(a)
                if a["type"] == "SEGMENT-ALERT":
                    alert_container.markdown(f'<div class="alert-card alert-segment"><b>[SEGMENT-ALERT]</b> {a["original"]} &rarr; {a["replacement"]} | {a["details"]}</div>', unsafe_allow_html=True)
                elif a["type"] == "SPELL-ALERT":
                    alert_container.markdown(f'<div class="alert-card alert-spell"><b>[SPELL-ALERT]</b> {a["original"]} &rarr; {a["replacement"]} | {a["details"]}</div>', unsafe_allow_html=True)

            displayed.extend(resolved)
            window.extend(resolved)

            if (i + 1) % trigger_N == 0:
                t0 = time.perf_counter()
                g_alert = engine.check_grammar_window(window[-trigger_N:])
                total_gram_lat += (time.perf_counter() - t0) * 1000.0
                gram_count += 1

                if g_alert:
                    alerts_log.append(g_alert)
                    alert_container.markdown(f'<div class="alert-card alert-grammar"><b>[GRAMMAR-ALERT]</b> {g_alert["details"]}</div>', unsafe_allow_html=True)

            stream_box.text_area("Accumulated Streamed Text:", value=" ".join(displayed), height=120)
            avg_tok = total_tok_lat / max(tok_count, 1)
            avg_gram = total_gram_lat / max(gram_count, 1)
            tok_metric.metric("Per-Token Latency", f"{avg_tok:.2f} ms")
            gram_metric.metric("Grammar Latency", f"{avg_gram:.2f} ms")
            alerts_metric.metric("Alerts Fired", f"{len(alerts_log)}")

            if typing_delay > 0:
                time.sleep(typing_delay / 1000.0)

        st.session_state["run_simulation"] = False
        st.session_state["final_passage"] = " ".join(displayed)
        st.session_state["alerts_log"] = alerts_log

    if "final_passage" in st.session_state:
        st.markdown("---")
        st.subheader("📊 Part 4: Final Passage Analysis — Method Comparison Table")
        seg_c = sum(1 for a in st.session_state["alerts_log"] if a["type"] == "SEGMENT-ALERT")
        spl_c = sum(1 for a in st.session_state["alerts_log"] if a["type"] == "SPELL-ALERT")
        table_rows = analyzer.analyze_passage(st.session_state["final_passage"], num_seg_merges=seg_c, num_spell_corrs=spl_c)
        df = pd.DataFrame(table_rows)
        st.dataframe(df, use_container_width=True)

elif app_mode == "Live Interactive Typing":
    st.subheader("⌨️ Live Interactive Typing Editor")
    st.write("Type or paste any text below. The background checker will incrementally analyze your input as you type.")

    user_text = st.text_area("Enter Text:", value="The quickbrown fox jumped over the lazydog. I would like to sea the world.", height=150)

    if user_text:
        words = user_text.split(" ")
        alerts_live = []
        clean_stream = []
        t0 = time.perf_counter()

        for i, w in enumerate(words):
            resolved, a_list = engine.check_token(w)
            alerts_live.extend(a_list)
            clean_stream.extend(resolved)
            if (i + 1) % trigger_N == 0:
                g_a = engine.check_grammar_window(clean_stream[-trigger_N:])
                if g_a:
                    alerts_live.append(g_a)

        tot_ms = (time.perf_counter() - t0) * 1000.0
        st.write(f"⚡ *Analyzed {len(words)} tokens in {tot_ms:.2f} ms ({tot_ms/max(len(words), 1):.2f} ms/token)*")

        if alerts_live:
            st.markdown("### 🔔 Detected Issues:")
            for a in alerts_live:
                if a["type"] == "SEGMENT-ALERT":
                    st.markdown(f'<div class="alert-card alert-segment"><b>[SEGMENT-ALERT]</b> {a["original"]} &rarr; {a["replacement"]} | {a["details"]}</div>', unsafe_allow_html=True)
                elif a["type"] == "SPELL-ALERT":
                    st.markdown(f'<div class="alert-card alert-spell"><b>[SPELL-ALERT]</b> {a["original"]} &rarr; {a["replacement"]} | {a["details"]}</div>', unsafe_allow_html=True)
                else:
                    st.markdown(f'<div class="alert-card alert-grammar"><b>[GRAMMAR-ALERT]</b> {a["details"]}</div>', unsafe_allow_html=True)
        else:
            st.success("No segmentation, spelling, or grammar issues detected!")

        st.markdown("---")
        st.subheader("📊 Sentence-Level Structural Analysis")
        cleaned_str = " ".join(clean_stream)
        table_rows = analyzer.analyze_passage(cleaned_str)
        st.dataframe(pd.DataFrame(table_rows), use_container_width=True)
