# Adenita Frame Synchronization

Adenita nucleotides and base segments store both geometric positions and local orientation frames. The three `Orientable` axes must be treated as part of the geometry, not as display-only cache.

For every persisted or synchronized frame:

- `e1`, `e2`, and `e3` are finite unit vectors.
- The axes are mutually orthogonal.
- The frame is right-handed with positive determinant for columns `[e1 e2 e3]`.

Frame validity is necessary but not sufficient. A frame can remain orthonormal after a SAMSON move while no longer matching the current backbone, sidechain, base-pair, or strand tangent geometry. Adenita validates both the mathematical frame and its alignment with reconstructible geometry before using cached axes as geometry anchors.

Rigid transforms must update positions and frames together:

```text
p'  = R p + t
e1' = R e1
e2' = R e2
e3' = R e3
```

When SAMSON changes atom positions without exposing the transform matrix, Adenita reconstructs frames from current geometry through `ADNGeometrySynchronization`. Nucleotide frames use backbone/sidechain direction plus strand tangent. Base-segment frames use paired nucleotide geometry plus double-strand tangent, falling back to repaired cached axes when geometry is incomplete.

`Orientable::SetE1`, `SetE2`, and `SetE3` intentionally remain passive setters. Call `ADNFrameAdapters::sanitizeFrame`, `ADNFrameAdapters::sanitizedFrame`, or the geometry synchronization helpers at explicit model boundaries instead.

Current synchronization boundaries:

- after native SAMSON unserialization;
- before and after Adenita JSON save/load;
- after deferred Adenita structural position / transform events;
- before and after Adenita editors that reconstruct geometry from geometry-aligned cached frames;
- when code explicitly requests manual frame repair.

There are three frame conventions that should not be mixed:

- Geometry-aligned synchronization frames are reconstructed from the current nucleotide or base-segment geometry and are used as the durable in-memory state after SAMSON moves, serialization, and most editor edits.
- Canonical template reconstruction frames are phase-neutral base-segment frames consumed by `DASBackToTheAtom`. `Create base pair` and `Twister` prepare these frames explicitly with `prepareBaseSegmentFrameForTemplateReconstruction` before calling template reconstruction.
- Delta geometry rotations operate directly on current positions and frames. `Rotate DNA` uses this path so partial rotations compose without rebuilding from ideal templates.

Do not run a generic pre-edit part synchronization immediately before a `DASBackToTheAtom` template reconstruction call. That replaces the phase-neutral template frame with a geometry-aligned frame, after which `DASBackToTheAtom` applies the helical phase a second time. Post-edit synchronization remains appropriate because it returns the model to geometry-aligned frames after reconstruction.

Native `serialize` methods write sanitized frame copies without mutating object state.

## Implementation Pointers

- Public synchronization API: `AdenitaCoreSE/include/ADNGeometrySynchronization.hpp`.
- Frame math helpers: `AdenitaCoreSE/include/ADNFrameUtils.hpp` and `AdenitaCoreSE/include/ADNFrameAdapters.hpp`.
- Synchronization implementation: `AdenitaCoreSE/source/ADNGeometrySynchronization.cpp`.
- Editor and app synchronization boundaries: `SEAdenitaCoreSEApp`, `SEAdenitaCoreSEAppGUI`, `SEDNATwisterEditor`, and creation or reconstruction paths that call `DASBackToTheAtom`.
- Regression coverage: `AdenitaCoreSE/tests/AdenitaStandaloneTests.cpp`.

When adding a new editor or reconstruction path, decide whether it is a geometry-aligned synchronization boundary, a template reconstruction boundary, or a delta geometry rotation. Mixing those conventions is the main source of double-applied helical phase and stale-frame bugs.
