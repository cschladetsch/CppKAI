# Training Consent Policy

KAI should generally train silently from local repository evidence.

## Silent By Default

The following should normally be ingested without a prompt:

- code under `Source/`
- tests under `Test/`
- console and workflow scripts under `Scripts/`
- runtime evidence under `Logs/`
- `.history` files
- README and documentation files
- curated lessons under `Scripts/Training/`

## Ask First

Ask only when a change has large impact, for example:

- adding a new external corpus
- replacing or rewriting a large portion of curated lessons
- pulling in user content with unclear provenance
- changing the corpus shape in a way that would materially alter model behavior

## Memory

Every training run should leave a durable record in the generated manifest and
dataset output. That record is the audit trail for what KAI learned from.
