# Changelog

All notable changes to Adenita are documented in this file.

## [0.26.0] - 2026-05-29

### Summary

This release fixes rotation-related geometry reconstruction issues in Adenita. After a DNA structure is moved or rotated with SAMSON, Adenita editors now preserve the transformed geometry more reliably instead of reconstructing parts of the model from stale local frames. The changes particularly affect *Rotate DNA*, *Twist / Untwist* DNA strand, *Create base pairs*, complementary-strand placement, and atomic model generation.

Adenita remains distributed as a SAMSON extension source release, not as a standalone executable.

**Full Changelog**: <https://github.com/1A-OneAngstrom/Adenita/compare/0.25.0...v0.26.0>

### Fixed

- Fixed rotated DNA structures being flattened or partially reset when using Adenita geometry editors after a SAMSON move / rotate operation.
- Fixed stale cached nucleotide and base-segment frames that could remain mathematically valid while no longer matching the current geometry.
- Fixed *Rotate DNA* behavior so current geometry and frames are rotated consistently, avoiding reconstruction from stale template frames.
- Fixed *Twist / Untwist DNA* strand behavior on transformed structures by synchronizing affected geometry and using target-state reconstruction.
- Fixed *Create base pairs* behavior on transformed or one-sided structures by separating geometry-aligned frames from template reconstruction frames.
- Fixed complementary-strand placement so existing selected strands are preserved and newly created complementary nucleotides are placed from the current anchor geometry.
- Fixed one-sided complementary placement when the stored base-segment frame is misleading or inconsistent with the anchor side.
- Fixed side-aware atom template selection so right-strand and one-sided atomic generation uses the appropriate ideal nucleotide templates.
- Fixed atomic model generation so it no longer rewrites coarse frame state unnecessarily during atom placement.
- Fixed several single-strand / one-sided atom placement edge cases, including pair-direction ambiguity and generated-residue side consistency.

### Changed

- Added explicit frame synchronization barriers before and after Adenita geometry edits that reconstruct positions.
- Deferred structural geometry synchronization after SAMSON atom position / transform events so affected owning parts are synchronized once after the current event pass.
- Introduced frame-alignment validation that checks whether cached frames agree with reconstructible nucleotide and base-segment geometry, not only whether they are orthonormal.
- Introduced phase-aware conversion between canonical base-segment template frames and side-specific nucleotide frames.
- Reworked DNA rotation paths to operate on current geometry instead of rebuilding geometry from template state when that would discard user-applied transformations.
- Reworked complementary-strand and atom-placement paths to distinguish three frame conventions:
  - geometry-aligned synchronization frames;
  - canonical template reconstruction frames;
  - delta-rotation frames used for direct geometry rotation.
- Improved debug diagnostics for stale reconstruction frames, one-sided placement decisions, complementary placement, and atom template selection.
- Updated frame synchronization documentation to describe the editor synchronization boundary and frame-convention invariants.

### Added

- Added standalone regression coverage for stale nucleotide and base-segment frame detection.
- Added standalone regression coverage for rotated geometry edits, including *Rotate DNA*, *Twist DNA* behavior, and transformed atom placement.
- Added tests for phase-aware template-frame round trips, handedness, and base-plane consistency.
- Added tests for complementary placement from left, right, and rotated anchors.
- Added tests for one-sided atomic stacking, pair-level atom transforms, and side consistency.
- Added validation helpers for generated single-strand atom geometry in debug builds.

### Developer notes

- The release introduces `ADNGeometrySynchronization` helpers for frame / geometry analysis, synchronization, template preparation, and owning-part collection.
- `DASBackToTheAtom` now uses side-aware and pair-level placement helpers to avoid mixing geometry-aligned frames with template reconstruction frames.
- `SEAdenitaCoreSEApp` now batches structural synchronization and requests visual / viewport updates after deferred synchronization.
- `SEDNATwisterEditor` now collects affected base segments and owning parts before applying local twist / untwist operations.
- `SEAdenitaVisualModel` update handling was adjusted to react to atom additions / removals as well as atom position changes.

### Version

- Updated Adenita extension version from `0.25.0` to `0.26.0`.

### Compatibility / distribution

- Requires the normal Adenita developer build environment for this repository: SAMSON SDK, Qt, CMake, Boost, and a compatible compiler.
- This release is intended as a SAMSON extension source release.
