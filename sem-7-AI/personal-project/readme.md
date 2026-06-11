# Reading Guide — PlantHealthMonitor
A guide to what this project contains and the recommended order to read it in.

PlantHealthMonitor is an embedded plant monitoring system: an ESP32 runs a TinyML model to classify plant health on-device, syncs results to Supabase, and a Flutter app (iOS + Android) shows live readings and sends alerts.

NOTE: Documents are written in AsciiDoc. Every top-level document has a rendered PDF next to it, *read the PDFs*; the `.adoc` files in subfolders are source includes.

## Structure

[cols="1,3"]
|===
| Folder | Contents

| `docs/`
| All project documentation (proposal through final report)

| `src/`
| Source code: firmware, ML model pipeline, Flutter app, Supabase backend, releases

| `datasets/`
| Training/validation datasets, logs, and data documentation
|===

## Documentation — recommended order

Read in the order the documents were produced; this follows the project lifecycle:

. `docs/proposal/proposol-1.1.pdf` — what the project is and why.
. `docs/project-plan/project-plan.pdf` — goals, research questions, approach, planning, risk analysis.
. `docs/research/background/background-research.pdf` — related work, edge AI, datasets, sensor data.
. `docs/research/dev-board.pdf` — development board comparison and choice.
. `docs/proof-of-concept/` — three iterative PoC reports: `pof-01.pdf` → `pof-02.pdf` → `poc-03.pdf`.
. `docs/system-design/system-design.pdf` — the realized architecture (requirements, use cases, component/class/sequence diagrams; reflects firmware v1.1).
. `docs/manual/manual.pdf` — user manual for the finished system.
. `docs/final-project-report/final-project-report.pdf` — the wrap-up; ties everything together.

### Supporting documents (read as needed)

* `docs/potential-impact-assessment/impact-assessment.pdf` — societal impact, future scenarios, reflection.
* `docs/legal-ethical/AI-Act-Compliance.pdf` — EU AI Act compliance analysis.
* `docs/psychological/psycho-aspets.pdf` — psychological aspects.
* `docs/ai-augmented/ai-augmented.pdf` — how AI tools were used during development.
* `docs/reflection/reflection.pdf` — personal reflection.

## Source code

Start with `src/README.md` — it explains the full system architecture (ESP32 ↔ Supabase ↔ Flutter) with diagrams. Then:

* `src/firmware/` — ESP32 production firmware (PlatformIO): sensors, TinyML inference, BLE provisioning, Supabase sync. See its `README.md`.
* `src/model/` — Python ML pipeline: training data, scripts, and logs for the TinyML model.
* `src/app/` — Flutter app; `android_/` is the main cross-platform app, `ios_only/` an iOS variant.
* `src/supabase/` — database/backend mitigations.
* `src/releases/` — packaged release artifacts (firmware binaries, app builds), each with a README.
* `src/legacy/` — earlier prototypes matching the PoC reports (`pof-02`, `pof-03`, `prototype-local`, `prototype-non-local`). Historical context only.

## Datasets

`datasets/` holds the raw and processed datasets, validation data, and old logs used for model training, with accompanying documentation in `datasets/docs/`.

## Generated artifacts (safe to ignore)

* `src/firmware/.pio/` — PlatformIO build output and library dependencies.
* `src/app/build/` — Flutter build output.
* `docs/system-design/related/html/` — generated Doxygen output.
* `.asciidoctor/` folders — diagram render caches.
