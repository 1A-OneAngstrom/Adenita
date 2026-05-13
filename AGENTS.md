# Guidance for Coding Agents

This repository is the `./adenita` project in the SAMSON extensions workspace.

## Repository Shape

- `CMakeLists.txt` configures the SAMSON extension and adds `AdenitaCoreSE`.
- `CMakePresets.json` contains the current Windows preset used by local builds.
- `AdenitaCoreSE/include` contains public headers for the SAMSON element classes, data model, editors, importers, and visual models.
- `AdenitaCoreSE/source` contains the corresponding implementations.
- `AdenitaCoreSE/form` contains Qt Designer `.ui` files used by the editor and app GUIs.
- `AdenitaCoreSE/resource` contains icons and Qt resources.
- `AdenitaCoreSE/data` contains bundled sample data, scaffolds, PDB fragments, meshes, and JSON examples installed with the element.
- `AdenitaCoreSE/modules/DAS` contains design and structure generation code, including Cadnano, Daedalus, lattice, routing, and atomistic conversion helpers.
- `AdenitaCoreSE/modules/MSV` contains visualization/color/display helpers.
- `AdenitaCoreSE/modules/PI` contains primer and binding-region helpers.
- `docs` contains developer notes and historical user documentation.

## Preferred Commands

- Search files and symbols with `rg` first, for example `rg "ADNPart" AdenitaCoreSE`.
- Configure the current Windows build with:

  ```powershell
  cmake --preset win_x64_qt6.10.2
  ```

- Build and install the release element with:

  ```powershell
  cmake --build --preset build_win_x64_qt6.10.2_release
  ```

- Build and install the debug element with:

  ```powershell
  cmake --build --preset build_win_x64_qt6.10.2_debug
  ```

## Editing Guidelines

- Keep changes scoped. Avoid broad formatting sweeps in generated or legacy SAMSON extension code.
- Do not change SAMSON element UUIDs, descriptor registrations, target names, or class registration macros unless the task explicitly requires it.
- Treat Qt `.ui` files, SAMSON descriptor boilerplate, and generated-style extension scaffolding as compatibility-sensitive.
- Prefer existing patterns in paired header/source files and editor/GUI classes.
- Do not rewrite unrelated includes, whitespace, or naming while fixing a focused issue.
- `ElementVersionHash.cmake` and `build*` directories are generated artifacts. Do not edit or commit generated build output.
- `CMakePresets.json` contains local path defaults. Update paths only when the task is specifically about build configuration.
