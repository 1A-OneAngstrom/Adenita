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
- before and after Adenita editors reconstruct geometry from cached frames;
- when code explicitly requests manual frame repair.

The explicit editor boundary is required because editors such as Rotate DNA, Twister, and Create base pair call geometry reconstruction routines that use cached frame axes. Synchronizing the owning `ADNPart` before those calls makes SAMSON-rotated positions the source of truth; synchronizing again after the edit leaves the next operation with aligned frames.

Native `serialize` methods write sanitized frame copies without mutating object state.
