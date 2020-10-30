# CMake

## Requirements:

- Qt 5.12.5
- CMake (Version 3.15 or above)
- Visual Studio
- Boost library (version 1.69 or above)
- Rapidjson
- SAMSON SDK 1.0.0 or above

# CMake Command

To create the developing environment use CMake:

```
mkdir build
cd build
```
If it is being build based on a public SAMSON SDK:
```
cmake -G"Visual Studio 15 Win64" -DSAMSON_SDK_PATH="path/to/SAMSON/SDK/1.0.0" -DQT5_CMAKE_INCLUDE_DIR="path/to/Qt/5.12.5/msvc2017_64/lib/cmake" -DBOOST_PATH="path/to/boost_1_69_0" -DRAPIDJSON_PATH="path/to/rapidjson" ..
```
If it is being build based on a built SAMSON Base:
```
cmake -G"Visual Studio 15 Win64" -DSAMSON_SDK_PATH="path/to/SAMSON/SDK/1.0.0/SAMSON-Debug/Binaries" -DQT5_CMAKE_INCLUDE_DIR="path/to/Qt/5.12.5/msvc2017_64/lib/cmake" -DBOOST_PATH="path/to/boost_1_69_0" -DRAPIDJSON_PATH="path/to/rapidjson" ..
```
