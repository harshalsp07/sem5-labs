"""
Data Downloader Script
Downloads:
1. NLTK Brown Corpus (English)
2. Universal Dependencies Spanish-GSD (Spanish)
3. Universal Dependencies German-GSD (German, optional)
"""

import os
import sys
import urllib.request
import nltk

DATA_DIR = os.path.dirname(os.path.abspath(__file__))
SPANISH_DIR = os.path.join(DATA_DIR, "ud_spanish")
GERMAN_DIR = os.path.join(DATA_DIR, "ud_german")

UD_SPANISH_FILES = {
    "es_gsd-ud-train.conllu": "https://raw.githubusercontent.com/UniversalDependencies/UD_Spanish-GSD/master/es_gsd-ud-train.conllu",
    "es_gsd-ud-dev.conllu": "https://raw.githubusercontent.com/UniversalDependencies/UD_Spanish-GSD/master/es_gsd-ud-dev.conllu",
    "es_gsd-ud-test.conllu": "https://raw.githubusercontent.com/UniversalDependencies/UD_Spanish-GSD/master/es_gsd-ud-test.conllu",
}

UD_GERMAN_FILES = {
    "de_gsd-ud-train.conllu": "https://raw.githubusercontent.com/UniversalDependencies/UD_German-GSD/master/de_gsd-ud-train.conllu",
    "de_gsd-ud-dev.conllu": "https://raw.githubusercontent.com/UniversalDependencies/UD_German-GSD/master/de_gsd-ud-dev.conllu",
    "de_gsd-ud-test.conllu": "https://raw.githubusercontent.com/UniversalDependencies/UD_German-GSD/master/de_gsd-ud-test.conllu",
}


def download_file(url: str, dest_path: str):
    if os.path.exists(dest_path) and os.path.getsize(dest_path) > 1000:
        print(f"File already exists: {dest_path} ({os.path.getsize(dest_path)} bytes)")
        return
    print(f"Downloading {url} -> {dest_path} ...")
    os.makedirs(os.path.dirname(dest_path), exist_ok=True)
    req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
    with urllib.request.urlopen(req) as resp, open(dest_path, "wb") as f:
        while chunk := resp.read(65536):
            f.write(chunk)
    print(f"Saved {dest_path} ({os.path.getsize(dest_path)} bytes)")


def setup_brown():
    print("\n--- Downloading Brown Corpus via NLTK ---")
    nltk.download("brown")
    nltk.download("universal_tagset")
    print("Brown corpus downloaded successfully.")


def setup_ud_spanish():
    print("\n--- Downloading UD Spanish-GSD ---")
    os.makedirs(SPANISH_DIR, exist_ok=True)
    for filename, url in UD_SPANISH_FILES.items():
        dest = os.path.join(SPANISH_DIR, filename)
        download_file(url, dest)
    print("UD Spanish-GSD downloaded successfully.")


def setup_ud_german():
    print("\n--- Downloading UD German-GSD ---")
    os.makedirs(GERMAN_DIR, exist_ok=True)
    for filename, url in UD_GERMAN_FILES.items():
        dest = os.path.join(GERMAN_DIR, filename)
        download_file(url, dest)
    print("UD German-GSD downloaded successfully.")


def main():
    setup_brown()
    setup_ud_spanish()
    setup_ud_german()
    print("\nAll data downloads completed successfully!")


if __name__ == "__main__":
    main()
