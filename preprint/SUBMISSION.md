# bioRxiv submission package

Built by `mfas_code/build_preprint.py` from `PAPER.md` and `figures/`.
Rebuild after any edit to the paper:

```
python3 -I mfas_code/build_preprint.py PAPER.md figures preprint
```

The builder refuses to run if it cannot find the "Notes for the author"
heading, because everything from that heading down is working notes and must
not reach a submitted manuscript. It also fails rather than guess if the
paper grows a maths expression it has not been taught to render.

## Files here

| file | what it is |
|---|---|
| `manuscript.html` | the whole paper, figures inlined, styled for print |
| `abstract.txt` | plain text, 274 words, for pasting into the form |
| `SUBMISSION.md` | this checklist |

## 1. Make the PDF

bioRxiv's own submission guide says: "The simplest way to deposit a
manuscript on bioRxiv is to upload a single PDF including the text and any
figures/tables." That is the route this package is built for.

Open `preprint/manuscript.html` in Chrome or Safari and print to PDF:

- Destination: **Save as PDF**
- Paper: **Letter** (the stylesheet sets its own 18 mm margins)
- Margins: **Default**, Scale **100%**
- **Background graphics: on.** Without it the figure bars print white.

Check before uploading: figures appear as Figure 1, 2, 3 in that order, no
table is split awkwardly across a page break, and the paper ends at the
references — there must be no "Notes for the author" section.

## 2. Metadata the form will ask for

| field | value |
|---|---|
| Title | A maximally feedforward ordering of the *Drosophila* connectome recovers its processing hierarchy and replicates across two brains |
| Author | Adithya Mandody, independent researcher (sole author, corresponding) |
| Abstract | paste `abstract.txt` |
| Article category | **New Results** (the options are New Results, Confirmatory Results, Contradictory Results) |
| Subject area | **Neuroscience**; Bioinformatics is the reasonable alternative |
| Competing interests | none, unless you consider otherwise |

## 3. The one decision that is yours

**Licence.** bioRxiv asks you to choose one at submission and it is awkward
to change later. CC-BY is the most permissive and the one most funders and
journals prefer; CC-BY-NC-ND is the most restrictive bioRxiv offers. Either
is compatible with later journal submission. If you have no particular
reason to restrict reuse, CC-BY is the conventional choice for work whose
whole argument is that anyone can re-run the checks.

## 4. Before you post

- [ ] **Make the repository public.** The manuscript's header links to
      `github.com/adithyamandody/flywire-mfas`. The paper's case rests on a
      reader being able to re-run `score.py` and `check_bound.py` themselves;
      a preprint pointing at a 404 undercuts exactly that. These two
      decisions are now coupled.
- [ ] Confirm the licence choice above.
- [ ] Consider re-sending the improved ordering. The submission is now
      listed first (26 November 2026) as 35,551,497, while the paper reports
      35,551,576; submissions are unlimited and only the best is shown, so
      re-sending would stop a reader finding two numbers under one name. No
      claim in the paper depends on it.

## 5. If the form asks about AI use

Many submission systems now ask directly. The paper's own §6 is the full
statement; this is the short form:

> Generative AI (Claude, Anthropic) was used as a research tool in this
> work, not only for writing assistance: it wrote the solver and the
> analysis code, designed and implemented the cycle-packing bound, ran the
> experiments, produced the figures, proposed several of the analyses and
> drafted the manuscript. The author set the direction, decided which lines
> to pursue, ran the verification scripts, read the code and the cited
> sources, checked the figures and quoted numbers against the data, and
> takes full responsibility for the paper. No AI system is an author. Full
> detail is in Methods §5 and in the AI use declaration, §6.

Do not shorten this to "AI was used for language editing". That would be
false, and the disclosure is what places the work inside what the major
publishers permit rather than outside it.

## 6. What this package does not do

- **It does not submit anything.** The bioRxiv account and the submission
  are yours.
- Three references ([15] Meier & Borst, [18] Waddell, [19] Keene) are
  confirmed only to the level of their abstracts, because no free full text
  is reachable. Methods says so plainly. If you have library access, the
  PDFs would close the last gap.
