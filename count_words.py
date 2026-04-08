import sys
from collections import Counter

def count_words(filepath):
    length_counts = Counter()
    total = 0

    with open(filepath, encoding='utf-8') as f:
        for line in f:
            # Replace spaces with Ñ BEFORE stripping (spaces encode Ñ)
            word = line.rstrip('\n\r').replace(' ', 'Ñ')
            if not word:
                continue
            # Skip comment lines (first char before 'A' and not Ñ)
            first = word[0]
            if first != 'Ñ' and first < 'A':
                continue

            # Count tiles using mixed-case notation:
            # uppercase char = 1 tile, consecutive lowercase chars = 1 tile
            tiles = 0
            i = 0
            while i < len(word):
                ch = word[i]
                if ch.isdigit():
                    break  # trailing playability digits
                if ch.islower():
                    # consume consecutive lowercase as one tile
                    while i < len(word) and word[i].islower():
                        i += 1
                    tiles += 1
                else:
                    tiles += 1
                    i += 1
            length_counts[tiles] += 1
            total += 1

    print(f"Total words: {total}")
    print(f"\nTiles  Count")
    print(f"-----  -----")
    for length in sorted(length_counts):
        print(f"{length:5d}  {length_counts[length]:5d}")

if __name__ == '__main__':
    path = sys.argv[1] if len(sys.argv) > 1 else 'FISE2022A.txt'
    count_words(path)
