# Proofreading access — what to do

Two separate routes. They are not alternatives; you can run both.

---

## Route A — BANC (fly brain + nerve cord).  Do this one first.

The entire gate is a three-field form.

**https://flywire.ai/banc_access**

Fields: **Name**, **Email**, **Affiliation**.
For affiliation write `Independent researcher` — there is no affiliation
requirement in this field and unaffiliated people are already named authors
on BANC papers.

What happens next, in their words: *"We'll schedule a meeting to show you
around, go over FlyWire/BANC Principles, and set up access to production
data and tools."*

Read before the meeting:
- FlyWire/BANC Principles —
  https://docs.google.com/document/d/1XKNbBv0AjiVLwUBtRKj2f1NOEMKNKyQXBEsfoiPeAgQ/view
- The dataset in Codex — https://codex.flywire.ai/banc

Worth mentioning in the meeting: you have the FAFB challenge connectome
loaded locally and have written your own solver against it. That is more
hands-on experience with this data than most applicants arrive with.

---

## Route B — EyeWire II / Pyr (mouse retina).  The 10-edit test.

Page: https://connectome.quest/access.html

1. Read the principles: https://edit.flywire.ai/principles.html
2. Email **support@eyewire.org**
   - Subject: `early access request`
   - Body: `I'd like early access to (dataset): `
     plus `I've read and agree to the community principles.`
3. Practice in the sandbox: **https://edit.flywire.ai/**
   (needs a Google sign-in; has a built-in tutorial)
4. Produce **10 links** — 7 mergers, 3 splits. Annotate only; do not
   execute the edits. Use the Share button to save a link for each.
5. Email the 10 links to **support@eyewire.org**
   - Subject: `dataset access edits`

No limit on attempts. They send back personalised feedback either way.

EyeWire players get priority, and the original game is still running at
https://eyewire.org — playing it builds standing for the Pyr beta.

---

## The two error types

Automatic segmentation makes exactly two kinds of mistake. The whole task
is telling them apart.

**Split** — one real neuron cut into two or more pieces.
Looks like: a branch that stops dead mid-air while another segment carries
on in the same direction with the same calibre. Fix: select both pieces so
they join. Cheap to find, cheap to fix, and detectable from shape alone.

**Merge** — two different neurons fused into one object.
Looks like: an object that no single cell could be. Two cell bodies. A
branch leaving the region the rest of the cell respects. A sudden change in
thickness at the join. Fix: Multicut — place points on each side of the bad
join to mark the cut.

Merges are the dangerous ones. A split costs you one neuron's completeness;
a merge invents a connection that does not exist and corrupts the graph
around it. This asymmetry is why merge detection is the open research
problem in the field, and why human review still gates everything.

For the application you mark where you *would* cut, you do not cut.

---

## One thing I will not do

I am not producing your 10 example edits. They are an assessment of your
judgement, and submitting my work under your name would misrepresent you to
the people deciding whether to trust you with the production dataset.

I can explain anything you see, talk through a case you are unsure about,
and check your reasoning. The calls are yours.

---

## What credit actually looks like

From the FlyWire Principles, verbatim: contributors *"may request
co-authorship or acknowledgments"* — and *"many days or months of work most
likely would"* constitute authorship, while a few hours or days may not.

So it is opt-in and effort-scaled. You have to ask. Nobody hands it to you.
Seven people with a non-academic affiliation cleared that bar on the June
2026 BANC paper.
