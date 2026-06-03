# Adenita Architecture

This document is a developer-oriented map of Adenita's source code. For the detailed SAMSON node hierarchy, see [AdenitaDataGraph.md](AdenitaDataGraph.md).

## Major Subsystems

- App shell: `SEAdenitaCoreSEApp` coordinates the SAMSON app entry point, document integration, import/export actions, sequence generation, visual model management, and editor-facing operations.
- GUI shell: `SEAdenitaCoreSEAppGUI` and the `.ui` files in `AdenitaCoreSE/form` define the main Adenita panel and editor controls.
- Data model: `ADNPart`, `ADNDoubleStrand`, `ADNSingleStrand`, `ADNBaseSegment`, `ADNCell`, `ADNNucleotide`, `ADNBackbone`, `ADNSidechain`, `ADNAtom`, and `ADNLoop` implement Adenita's DNA graph on top of SAMSON nodes.
- Visual model: `SEAdenitaVisualModel` renders Adenita structures at atom, nucleotide, single-strand, double-strand, and object scales. `SEAdenitaVisualModelProperties` exposes visual settings.
- Editors: `SE*Editor` classes implement interactive tools such as break, delete, connect, twist, tag, dsDNA creation, lattice creation, nanotube creation, merge, and wireframe creation. Matching `SE*EditorGUI` classes own the Qt controls.
- Import/export: `SEAdenitaImporterAdn`, `SEAdenitaImporterAdnpart`, `SEAdenitaImporterJson`, and `SEAdenitaImporterPly` expose file importers. Save/load and export helpers live in `ADNSaveAndLoad` and app-level export methods.
- Configuration and validation: `SEConfig`, `ADNConfig*`, `ADNJsonValidation`, and `ADNNodeValidation` handle settings, JSON serialization support, and defensive checks around persisted graph references.
- DAS module: `modules/DAS` contains design and structure algorithms for Cadnano import, Daedalus wireframe generation, lattices, routing, polyhedra, complex operations, and back-to-the-atom conversion.
- MSV module: `modules/MSV` contains color and display helpers used by the visual model.
- PI module: `modules/PI` contains primer, crossover, and binding-region helpers, including Primer3 / ntthal-facing logic.
- Standalone tests: `AdenitaCoreSE/tests/AdenitaStandaloneTests.cpp` contains smoke and regression tests for code paths that can run without launching SAMSON.

## Common Change Locations

- Change the DNA graph shape or invariants in the `ADN*` model classes, especially `ADNPart`, `ADNNucleotide`, `ADNBaseSegment`, and `ADNCell`.
- Change create/delete/connect/split/merge behavior in `ADNBasicOperations` and the relevant editor class.
- Change app-level commands, file workflows, scaffold sequence handling, or visual model lifecycle in `SEAdenitaCoreSEApp`.
- Change rendering scale, coloring, highlighting, visibility, or OpenGL display paths in `SEAdenitaVisualModel`, `MSVColors`, and `MSVDisplayHelper`.
- Change editor controls in the matching `.ui` file and `SE*EditorGUI` class; change editor behavior in the matching `SE*Editor` class.
- Change file parsing or writing in the importer classes, `ADNSaveAndLoad`, `DASCadnano`, or the app export methods.
- Change settings, default options, or JSON schema handling in `SEConfig`, `ADNConfigFileIO`, `ADNConfigJson`, and `ADNJsonValidation`.
- Change wireframe, lattice, routing, nanotube, or atomistic generation in `modules/DAS`.
- Change binding-region or primer calculations in `modules/PI`.
- Change frame or geometry synchronization with [FrameSynchronization.md](FrameSynchronization.md) open; related code lives in `ADNFrameAdapters`, `ADNFrameUtils`, and `ADNGeometrySynchronization`.

## SAMSON Integration Notes

Adenita classes are exposed to SAMSON through `SB_CLASS`, `SB_REGISTER_TARGET_TYPE`, descriptor headers, and the element UUID configured in `AdenitaCoreSE/CMakeLists.txt`. These registrations are compatibility-sensitive and should only be changed when intentionally changing SAMSON-facing identity or descriptors.

`AdenitaCoreSE/CMakeLists.txt` builds one shared library target named `SEAdenitaCoreSE`, installs data, resources, documentation, and the compiled SAMSON element, and wires `SEAdenitaCoreSE.Standalone` into CTest. The top-level `CMakeLists.txt` resolves SAMSON SDK, Qt, OpenGL, and Boost dependencies before adding `AdenitaCoreSE`.

## Testing

After a successful release build, run:

```powershell
ctest --test-dir build_win_x64_qt6.10.2 -C Release --output-on-failure
```

Use `-C Debug` after a debug build.

## Development Notes

- Prefer existing paired header/source patterns and avoid broad rewrites in generated-style SAMSON extension files.
- Use `ADNPart` registration and deregistration helpers when changing graph topology.
- Keep UI behavior split between `SE*Editor` classes and their `SE*EditorGUI` classes.
- Keep user-facing tutorials in the SAMSON Documentation Center. Repository docs should focus on building, maintaining, and understanding the code.
- Keep vendored dependencies under `AdenitaCoreSE/external` unchanged unless the dependency update itself is the task.
