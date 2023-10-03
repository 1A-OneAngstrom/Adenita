# CMake

## Requirements

- SAMSON SDK 5.0.0 or above
- Qt 6.5.2
- CMake version 3.15 or above
- Visual Studio 2019 or above
- Boost library (version 1.80 or above)
- Rapidjson

## How to build

To create the developing environment use CMake:

```shell
mkdir build
cd build
```

If you build based on a public SAMSON SDK:

```shell
cmake -G"Visual Studio 17" -DSAMSON_SDK_PATH="path/to/SAMSON/SDK/5.0.0" -DQT5_CMAKE_INCLUDE_DIR="path/to/Qt/6.5.2/msvc2019_64/lib/cmake" -DBOOST_PATH="path/to/boost_1_80_0" -DRAPIDJSON_PATH="path/to/rapidjson" ..
```

If you build based on a built SAMSON Base:

```shell
cmake -G"Visual Studio 17" -DSAMSON_SDK_PATH="path/to/SAMSON/SDK/5.0.0/SAMSON-Debug/Binaries" -DQT5_CMAKE_INCLUDE_DIR="path/to/Qt/6.5.2/msvc2019_64/lib/cmake" -DBOOST_PATH="path/to/boost_1_80_0" -DRAPIDJSON_PATH="path/to/rapidjson" ..
```
