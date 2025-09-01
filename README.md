# Adenita SAMSON Extension

[![Maintained by SAMSON Team at OneAngstrom](https://img.shields.io/badge/Maintained%20by-SAMSON%20Team%20at%20OneAngstrom-blue.svg)](https://www.oneangstrom.com/)
[![SAMSON Compatible](https://img.shields.io/badge/Compatible%20with-SAMSON%20(latest)-brightgreen.svg)](https://www.samson-connect.net/)
[![License](https://img.shields.io/badge/license-BSD%203--Clause-lightgrey.svg)](LICENSE)

This repository provides an updated version of the **Adenita SAMSON Extension**, forked from the original [Adenita SAMSON Edition repository](https://github.com/edellano/Adenita-SAMSON-Edition), which is no longer maintained.

## Origin

The Adenita SAMSON Extension was initially created by [Elisa de Llano](https://github.com/edellano) and [Haichao Miao](https://github.com/mhaichao) at the [Austrian Institute of Technology](https://www.ait.ac.at/), [University of Vienna](https://www.univie.ac.at/en/), and [TU Wien](https://www.tuwien.at/).  
It was released for **SAMSON v0.7.0** and **v0.8.5**, both of which are no longer available.

## Current Status

This repository is forked from the original [Adenita repository](https://github.com/edellano/Adenita-SAMSON-Edition) and updated to work with the latest SAMSON versions.  
Maintenance and support are handled by the [SAMSON team](https://www.samson-connect.net/) at [OneAngstrom](https://www.oneangstrom.com/).

## Users Documentation

**Documentation for users**: [SAMSON Documentation Center: Adenita](https://documentation.samson-connect.net/tutorials/adenita/adenita/).

## Developers Documentation

Please refer to the [SAMSON Developer Guide](https://documentation.samson-connect.net/developers/latest/) to learn more about the use of SAMSON SDK.

### Quick Start

1. **Install the latest SAMSON and SAMSON SDK**
   Download from the [SAMSON website](https://www.samson-connect.net/).

2. **Install [requirements](#requirements)**

3. **Clone this repository**
   ```bash
   git clone https://github.com/1A-OneAngstrom/Adenita.git
   ```

4. **Build and install the extension**: see [SAMSON Developer Guide - Building a SAMSON Extension](https://documentation.samson-connect.net/developers/latest/building/).

### Requirements

- [SAMSON and SAMSON SDK](https://www.samson-connect.net/) v.8.0.0 or above
- Qt 6.8.2 and Qt Creator
- CMake version 3.15 or above
- On Windows: Visual Studio 2022 or above. On macOS and Linux: IDE of your choice, e.g. Qt Creator.
- Boost library (version 1.87 or above)

### How to build

Please refer to the [SAMSON Developer Guide - Building a SAMSON Extension](https://documentation.samson-connect.net/developers/latest/building/) and follow the same instructions.

For example, to create a Visual Studio project on Windows, run the following command in the PowerShell terminal in the `build` subfolder (change the paths):

```shell
cmake -G"Visual Studio 17" -DSAMSON_SDK_PATH="path/to/SAMSON/SDK/8.0.0" -DQT5_CMAKE_INCLUDE_DIR="path/to/Qt/6.8.2/msvc2022_64/lib/cmake" -DBOOST_PATH="path/to/boost_1_87_0"
```

### Other docs

See the [docs](docs) subfolder:

- [Adenita's Data Graph](docs/AdenitaDataGraph.md)
