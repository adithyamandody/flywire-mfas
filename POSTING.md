# Posting both preprints

Both papers go up together. Paper 2 cites paper 1 for the method and the
six-basin result, and each lists the other as reference [14] / [16]; posting
one alone leaves a dangling citation.

## 0. Before you start

- Repo is public: https://github.com/adithyamandody/flywire-mfas ✓
- Both manuscripts are built and current ✓
- Decide the licence (step 3) — it is awkward to change afterwards.

## 1. Make the two PDFs

Open each file in Chrome or Safari and print to PDF:

| | file |
|---|---|
| paper 1 | `preprint/manuscript.html` |
| paper 2 | `preprint2/manuscript.html` |

Settings that matter:

- Destination **Save as PDF**
- Paper **Letter**, Margins **Default**, Scale **100%**
- **Background graphics ON.** Without it the figure bars print white.

Then check each PDF before uploading: figures appear in order, no table is
split badly across a page, and **the document ends at the references** —
there must be no "Notes for the author" section. (The builder strips it, but
look anyway.)

## 2. Metadata for each submission

**Paper 1**
- Title: *A maximally feedforward ordering of the Drosophila connectome
  recovers its processing hierarchy and replicates across two brains*
- Abstract: paste `preprint/abstract.txt` (291 words)
- Category: **New Results**
- Subject area: **Neuroscience**

**Paper 2**
- Title: *Nervous systems are wired less recurrently than their own degree
  sequences require*
- Abstract: paste `preprint2/abstract.txt` (241 words)
- Category: **New Results** — it confirms Zhao & Zhou's direction [16] but
  the certificates, the weighting and the degree-preserving null are new.
  If you would rather be conservative, Confirmatory Results is defensible.
- Subject area: **Neuroscience** (Bioinformatics is the alternative)

Both: author *Adithya Mandody, independent researcher*, sole and
corresponding. Competing interests: none.

## 3. The licence

bioRxiv asks for one at submission. CC-BY is the conventional choice and the
one most funders and journals prefer; CC-BY-NC-ND is the most restrictive on
offer. Either is compatible with later journal submission. For work whose
whole argument is that anyone can re-run the checks, CC-BY is the consistent
choice.

## 4. If the form asks about AI use

Many submission systems now ask directly. The full statement is §6 of paper
1 and §7 of paper 2. Short form:

> Generative AI (Claude, Anthropic) was used as a research tool in this
> work, not only for writing assistance: it wrote the solver and the
> analysis code, designed and implemented the cycle-packing bound, ran the
> experiments, produced the figures, proposed several of the analyses and
> drafted the manuscripts. The author set the direction, decided which lines
> to pursue, ran the verification scripts, read the code and the cited
> sources, checked the figures and quoted numbers against the data, and
> takes full responsibility. No AI system is an author.

Do not shorten this to "AI was used for language editing". That would be
false, and the disclosure is what places the work inside what the major
publishers permit rather than outside it.

## 5. After the DOIs arrive

1. Fill in the companion reference in each paper — paper 1's [14] and paper
   2's [14] currently say "companion manuscript, 2026" and need the other's
   DOI.
2. Rebuild both: `python3 -I mfas_code/build_preprint.py PAPER.md figures preprint`
   and `python3 -I mfas_code/build_preprint.py RECURRENCE_PAPER.md figures preprint2 figures/recurrence_figs.tsv`
3. Post the updated PDFs as a revision to each (bioRxiv versions preprints).
4. Add both DOIs to the repo README.

## 6. Optional, afterwards

- **Re-send the leaderboard entry.** The board shows 35,551,497, the
  ordering emailed on 18 November; the repo's best is 35,551,569. Only the
  best is kept and submissions are unlimited, so re-sending stops a reader
  finding two numbers under one name.
- **A note to Arie Matsliah** at Princeton, who verified the submission
  personally. Two lines: the work behind the entry is now written up, here
  are the links.
