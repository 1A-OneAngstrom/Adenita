# Adenita SAMSON Extension

[![Maintained by SAMSON Team at OneAngstrom](https://img.shields.io/badge/Maintained%20by-SAMSON%20Team%20at%20OneAngstrom-blue.svg)](https://www.oneangstrom.com/)
[![SAMSON Compatible](https://img.shields.io/badge/Compatible%20with-SAMSON%20(latest)-brightgreen.svg)](https://www.samson-connect.net/)
[![License](https://img.shields.io/badge/license-BSD%203--Clause-lightgrey.svg)](LICENSE)

Adenita is a SAMSON extension for modeling and visualizing DNA nanostructures. This repository contains the maintained OneAngstrom / SAMSON Team fork of the original [Adenita SAMSON Edition](https://github.com/edellano/Adenita-SAMSON-Edition), updated for current SAMSON SDK and Qt versions.

## For Adenita Users

If you want to use Adenita in SAMSON, start with the [Adenita tutorial in the SAMSON Documentation Center](https://documentation.samson-connect.net/tutorials/adenita/adenita/).

This README is primarily for developers who build, maintain, or extend Adenita's source code.

## Origin and Status

Adenita was initially created by [Elisa de Llano](https://github.com/edellano) and [Haichao Miao](https://github.com/mhaichao) at the [Austrian Institute of Technology](https://www.ait.ac.at/), [University of Vienna](https://www.univie.ac.at/en/), and [TU Wien](https://www.tuwien.at/). The original SAMSON Edition targeted SAMSON v0.7.0 and v0.8.5, which are no longer available.

Maintenance and support for this fork are handled by the [SAMSON team](https://www.samson-connect.net/) at [OneAngstrom](https://www.oneangstrom.com/). The code is distributed under the [BSD 3-Clause License](LICENSE); third-party license notes are in [LICENSE-3rdParty](LICENSE-3rdParty).

Please cite the original Adenita publication when appropriate:

> Elisa de Llano, Haichao Miao, Yasaman Ahmadi, Amanda J. Wilson, Morgan Beeby, Ivan Viola, Ivan Barisic. "Adenita: Interactive 3D modeling and visualization of DNA Nanostructures." bioRxiv 849976, 2019. <https://doi.org/10.1101/849976>

## Developer Quick Start

Install the development prerequisites:

- [SAMSON and the SAMSON SDK](https://www.samson-connect.net/) v11.0.0 or newer.
- Qt 6.10.2 with the modules required by `CMakeLists.txt`.
- CMake 4.0 or newer.
- Boost 1.87 or newer.
- On Windows, Visual Studio 2022 or newer.

Clone the repository:

```bash
git clone https://github.com/1A-OneAngstrom/Adenita.git
cd Adenita
```

Copy or rename `CMakePresets.json.example` as `CMakePresets.json` and review it before configuring. The preset paths are local defaults for the current Windows development setup, including `SAMSON_SDK_PATH`, Qt, Boost, RDKit, and Python paths. Adjust them or override them on the CMake command line for your machine.

Configure with the provided Windows preset:

```powershell
cmake --preset win_x64_qt6.10.2
```

Build and install the release extension:

```powershell
cmake --build --preset build_win_x64_qt6.10.2_release
```

For a debug build:

```powershell
cmake --build --preset build_win_x64_qt6.10.2_debug
```

For additional SAMSON SDK setup, packaging, and extension workflow details, see the [SAMSON Developer Guide](https://documentation.samson-connect.net/developers/latest/) and the guide to [building a SAMSON extension](https://documentation.samson-connect.net/developers/latest/building/).

Non-Windows builds should follow the same SAMSON SDK and CMake model, but may require a local CMake preset or explicit `-D` options for the SDK, Qt, Boost, and compiler paths.

## Repository Map

- `CMakeLists.txt` and `CMakePresets.json.example`: top-level build configuration.
- `AdenitaCoreSE/include`: SAMSON element headers, data model classes, app/editor/importer/visual-model declarations.
- `AdenitaCoreSE/source`: implementation files matching the public headers.
- `AdenitaCoreSE/form`: Qt Designer UI files.
- `AdenitaCoreSE/resource`: icons and Qt resources.
- `AdenitaCoreSE/data`: bundled sample structures, scaffolds, PDB fragments, meshes, and JSON examples.
- `AdenitaCoreSE/modules/DAS`: design and structure algorithms, including Cadnano, Daedalus, routing, lattices, and atomistic conversion.
- `AdenitaCoreSE/modules/MSV`: visualization color and display helpers.
- `AdenitaCoreSE/modules/PI`: primer and binding-region helpers.
- `docs`: developer notes plus historical user documentation.

## Developer Documentation

- [Architecture overview](docs/Architecture.md)
- [Adenita data graph](docs/AdenitaDataGraph.md)
- [Historical user documentation](docs/README.md)

Future coding agents should also read [AGENTS.md](AGENTS.md) before making changes.
