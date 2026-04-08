# Quackle's 24-bit ceiling blocks large lexicons

Quackle's GADDAG/DAWG binary format packs each node into exactly **4 bytes**, using 3 of those bytes (24 bits) as a relative pointer to child nodes. This hard caps the structure at **16,777,215 addressable nodes**. A 639,294-word Spanish lexicon blows past that limit because the GADDAG representation multiplies each word into multiple rotated paths, easily generating tens of millions of nodes. The issue was reported on GitHub as **Issue #44** and closed as "wontfix" with no developer discussion, no patch, and no known fork that fixes it. Practical solutions exist, however: trimming the word list to roughly 300,000–350,000 words, modifying the source code to widen the pointer field, or switching to the modern Macondo/Wolges engine ecosystem that does not share this constraint.

## Inside the 4-byte node format that creates the bottleneck

The constraint lives in `gaddag.h`, where each `GaddagNode` stores all its data in `unsigned char data[4]`. The first three bytes encode a **relative offset to the first child node**, and the fourth byte encodes the letter (6 bits, mask `0x3F`), a terminal flag (bit 6, mask `0x40`), and a last-sibling flag (bit 7, mask `0x80`). The `firstChild()` method reconstructs the pointer as:

```cpp
unsigned int p = (data[0] << 16) + (data[1] << 8) + (data[2]);
if (p == 0) return 0;
else return this + p;
```

Since **3 bytes can represent at most 16,777,215**, that is the absolute maximum number of nodes the structure can address. The value 0 is reserved for null (no children), so valid offsets span 1 to 16,777,215. The same 24-bit pointer scheme appears in the standalone DAWG builder (`makedawg.cpp`), confirming it applies to both data structures.

A commented-out variant in the source code handles signed 24-bit offsets (allowing backward references), but even that would only yield ±8.3 million addressable positions — still insufficient for large lexicons.

## Why 639,294 words overwhelms 16.7 million nodes

A GADDAG is fundamentally different from a standard trie or DAWG. For every word of length *N*, the GADDAG creates *N* distinct paths — each one representing a different "anchor point" within the word, with the prefix reversed. This design enables Quackle's move generator to efficiently find plays in any direction, but it makes the data structure roughly **5–10× larger** than an equivalent DAWG.

English tournament dictionaries fit comfortably within the limit: **TWL/NWL contains ~180,000 words** and **Collins Scrabble Words (CSW) contains ~279,000 words**. Spanish, however, is a heavily inflected language with **34–35 conjugated forms per verb** plus gender and number agreement on adjectives and nouns. A comprehensive Spanish Scrabble dictionary at 639,294 words generates a GADDAG whose node count far exceeds 16.7 million. The same problem was first reported with a Polish word list from SJP.pl, another highly inflected language.

## GitHub Issue #44 and the absence of any official fix

The limitation was reported on **August 3, 2016** by user @alkamid as [Issue #44](https://github.com/quackle/quackle/issues/44), titled "Lexicon can't be built from large word lists." The reproduction steps used a Polish Scrabble word list. The maintainers **labeled it "wontfix" and closed it** without any substantive response. No pull request has ever been submitted to address the format limitation, and no fork in the Quackle ecosystem has widened the pointer field. The project's development activity has been minimal in recent years, making an upstream fix unlikely.

A community discussion about Indonesian language support did note that "you may have to increase the hardcoded memory allocations" in `lexiconparameters.cpp` and `loaddawg.cpp`, but this refers to runtime buffer sizes, not the fundamental 24-bit pointer constraint.

## Modifying the source code is feasible but non-trivial

Widening the node pointer from 24 bits to 32 bits would raise the limit to **~4.29 billion nodes**, more than sufficient for any Scrabble dictionary. The changes would touch several files:

- **`gaddag.h`**: Expand `data[4]` to `data[5]` (or larger), rewrite `firstChild()` to read 4 bytes instead of 3, and shift the letter/flag byte accordingly
- **GADDAG builder** (`quackleio/gaddagfactory.cpp` or `makegaddag/makegaddag.cpp`): Update serialization to write the wider pointer
- **GADDAG loader** (`quackleio/` files): Update deserialization to read the new format
- **DAWG builder** (`makedawg.cpp`): Apply the same widening if DAWG support is needed
- **Memory allocation constants** in `lexiconparameters.cpp`: Increase buffer sizes for larger structures

The resulting nodes would grow from 4 to 5 bytes each — a **25% memory increase** — and the on-disk `.gaddag` format would become incompatible with unmodified Quackle builds. Cache performance may degrade slightly due to the larger node size. For someone comfortable with C++, the actual code changes are mechanical: the logic is straightforward bit-packing, and the GADDAG traversal code is well-isolated. The harder part is testing thoroughly across the move generator, simulation engine, and endgame solver to ensure nothing depends on the 4-byte node assumption.

## Trimming the word list is the fastest practical fix

If modifying source code is not an option, reducing the Spanish word list to approximately **300,000–350,000 words** should bring it within Quackle's capacity. Effective trimming strategies include:

- **Remove words longer than 15 letters**, since the Scrabble board is 15×15 and longer words are unplayable
- **Prune rare verb conjugations** such as the future subjunctive (*cantare*, *cantares*) and vosotros imperative forms, which are archaic or regional — this alone can cut 30–50% of entries
- **Remove words containing K or W** if using the standard Castilian FISE tile set, which lacks those tiles entirely
- **Drop words longer than 10–12 letters**, which are statistically almost never played in competitive games
- **Prioritize keeping 2–8 letter words**, which represent the vast majority of actual Scrabble plays

The Collins English dictionary works well at 279,000 words, so a Spanish list trimmed to that range should produce a fully functional GADDAG while retaining all competitively relevant words.

## Macondo and Wolges offer a modern alternative without this limit

The most robust long-term solution is switching to the **Macondo** engine, developed in Go by César Del Solar and used as the backend for **Woogles.io**, the leading competitive Scrabble platform. Macondo uses the **KWG (Kurnia Word Graph)** format built by **Andy Kurnia's wolges library** (written in Rust), which is described as "small and fast" and is not subject to the same 24-bit limitation.

**Wolges** (https://github.com/andy-k/wolges) can build KWG word graphs from plain text word lists and can also output Quackle-compatible DAWG/GADDAG files — though those output files would still be in Quackle's 24-bit format. Macondo itself is considered more capable than Quackle's Championship Player, achieving roughly **56–57% simulated win rates** against equivalent Quackle algorithms. It features multi-core Monte Carlo simulation and an exhaustive minimax endgame solver.

A newer project called **MAGPIE**, a C rewrite of Macondo with contributions from John O'Laughlin (Quackle's original author), Andy Kurnia, and César Del Solar, promises 5–10× speed improvements and may become the definitive open-source Scrabble engine.

## Conclusion

The 24-bit pointer in Quackle's 4-byte node format is a clean engineering choice for English-sized dictionaries but an impassable wall for morphologically rich languages. The **16,777,215-node ceiling** was reasonable when Quackle was designed for 180K–280K-word English lexicons, but Spanish's 639,294 words generate a GADDAG that overshoots by a wide margin. With Issue #44 closed as "wontfix" and no active forks addressing it, the three viable paths forward are **trimming the word list** (fastest, loses completeness), **widening the pointer to 32 bits in source** (mechanically straightforward, requires C++ and testing), or **migrating to Macondo/Wolges** (best long-term investment, actively maintained, no format limitations). For competitive Spanish Scrabble analysis today, trimming to ~300K words gives an immediate working solution while a Macondo migration would provide the most capable engine without compromise.