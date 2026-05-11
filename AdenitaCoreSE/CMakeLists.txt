SET ( PROJECT_FOLDER_NAME AdenitaCoreSE )
SET( OUTPUT_UUID DDA2A078-1AB6-96BA-0D14-EE1717632D7A )

set(PROJECT_PATH ${ELEMENT_NAME})
set(OUTPUT_NAME "SE${PROJECT_FOLDER_NAME}")

message (STATUS "CREATING PROJECT OR MAKEFILE ${OUTPUT_UUID} : ${OUTPUT_NAME}")

file(GLOB_RECURSE ${OUTPUT_UUID}_headers CONFIGURE_DEPENDS include/*.hpp)
file(GLOB_RECURSE ${OUTPUT_UUID}_sources CONFIGURE_DEPENDS source/*.cpp)
file(GLOB_RECURSE ${OUTPUT_UUID}_resources CONFIGURE_DEPENDS resource/*.qrc)
file(GLOB_RECURSE ${OUTPUT_UUID}_form CONFIGURE_DEPENDS form/*.ui)
file(GLOB_RECURSE ${OUTPUT_UUID}_ts CONFIGURE_DEPENDS resource/translations/*.ts)

file(GLOB_RECURSE ${OUTPUT_UUID}_DAS_headers modules/DAS/include/*.hpp)
file(GLOB_RECURSE ${OUTPUT_UUID}_MSV_headers modules/MSV/include/*.hpp)
file(GLOB_RECURSE ${OUTPUT_UUID}_PI_headers modules/PI/include/*.hpp)
file(GLOB_RECURSE ${OUTPUT_UUID}_DAS_sources modules/DAS/source/*.cpp)
file(GLOB_RECURSE ${OUTPUT_UUID}_MSV_sources modules/MSV/source/*.cpp)
file(GLOB_RECURSE ${OUTPUT_UUID}_PI_sources modules/PI/source/*.cpp)

set(${OUTPUT_UUID}_tests
  tests/AdenitaStandaloneTests.cpp
)

add_library(${OUTPUT_NAME} SHARED)

target_sources(${OUTPUT_NAME}
  PRIVATE
    ${${OUTPUT_UUID}_headers}
    ${${OUTPUT_UUID}_sources}
    ${${OUTPUT_UUID}_resources}
    ${${OUTPUT_UUID}_form}
	${${OUTPUT_UUID}_DAS_headers}
	${${OUTPUT_UUID}_MSV_headers}
	${${OUTPUT_UUID}_PI_headers}
	${${OUTPUT_UUID}_DAS_sources}
	${${OUTPUT_UUID}_MSV_sources}
	${${OUTPUT_UUID}_PI_sources}
)

target_compile_definitions(${OUTPUT_NAME}
  PRIVATE
    ELEMENT_NAME="${OUTPUT_NAME}"
    SB_ELEMENT_UUID="${OUTPUT_UUID}"
)

target_include_directories(${OUTPUT_NAME}
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  PRIVATE
	"${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/rapidjson/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/DAS/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/MSV/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/PI/include"
)

# Qt6 use auto tools
set_target_properties(${OUTPUT_NAME} PROPERTIES
  AUTOMOC ON
  AUTOUIC ON
  AUTORCC ON
  AUTOUIC_SEARCH_PATHS
    "${CMAKE_CURRENT_SOURCE_DIR}/form"
)

# Qt6 translations
if(${OUTPUT_UUID}_ts)
  qt6_add_translations(${OUTPUT_NAME}
    TS_FILES ${${OUTPUT_UUID}_ts}
    QM_FILES_OUTPUT_VARIABLE ${OUTPUT_UUID}_QM
 )
endif()

source_group(include FILES ${${OUTPUT_UUID}_headers})
source_group(source FILES ${${OUTPUT_UUID}_sources})
source_group(resource FILES ${${OUTPUT_UUID}_resources})
source_group(form FILES ${${OUTPUT_UUID}_form})
source_group(translationFiles FILES ${${OUTPUT_UUID}_ts} ${${OUTPUT_UUID}_QM})
source_group(include/DAS FILES ${${OUTPUT_UUID}_DAS_headers})
source_group(include/MSV FILES ${${OUTPUT_UUID}_MSV_headers})
source_group(include/PI FILES ${${OUTPUT_UUID}_PI_headers})
source_group(source/DAS FILES ${${OUTPUT_UUID}_DAS_sources})
source_group(source/MSV FILES ${${OUTPUT_UUID}_MSV_sources})
source_group(source/PI FILES ${${OUTPUT_UUID}_PI_sources})
source_group(tests FILES ${${OUTPUT_UUID}_tests})


####################################################

target_link_libraries (${OUTPUT_NAME}
  PRIVATE
    ${QT_LIBRARIES}
    ${OPENGL_LIBRARY}
    ${SAMSON_SDK_LIBRARIES}
)

set_target_properties(${OUTPUT_NAME} PROPERTIES OUTPUT_NAME ${OUTPUT_UUID})

if(UNIX)
  set_target_properties (${OUTPUT_NAME} PROPERTIES PREFIX "")
endif(UNIX)

set_property(TARGET ${OUTPUT_NAME} PROPERTY FOLDER "Elements")

SAMSON_INSTALL_ELEMENT_DIRECTORY(data/ ${OUTPUT_UUID}/Data)
SAMSON_INSTALL_ELEMENT_DIRECTORY(resource/ ${OUTPUT_UUID}/Resource)
SAMSON_INSTALL_ELEMENT_DIRECTORY(doc/ ${OUTPUT_UUID}/Documentation)

SAMSON_INSTALL_ELEMENT (${OUTPUT_NAME} ${OUTPUT_UUID})

if(${OUTPUT_UUID}_tests)
  enable_testing()

  set(ADENITA_STANDALONE_TEST_TARGET ${OUTPUT_NAME}StandaloneTests)

  if(WIN32)
    get_filename_component(ADENITA_SAMSON_SDK_VERSION "${CMAKE_SAMSON_SDK_PATH}" NAME)
    get_filename_component(ADENITA_SAMSON_SDK_PARENT "${CMAKE_SAMSON_SDK_PATH}" DIRECTORY)
    get_filename_component(ADENITA_SAMSON_ROOT_DIR "${ADENITA_SAMSON_SDK_PARENT}" DIRECTORY)

    set(ADENITA_SAMSON_RUNTIME_DIR "${ADENITA_SAMSON_ROOT_DIR}/SAMSON/${ADENITA_SAMSON_SDK_VERSION}/Binaries")
    set(ADENITA_SAMSON_DEBUG_RUNTIME_DIR "${CMAKE_SAMSON_SDK_PATH}/SAMSON-Debug/Binaries")

    get_property(ADENITA_QT6_PREFIX GLOBAL PROPERTY QT6_PREFIX)
    if(NOT ADENITA_QT6_PREFIX)
      set(ADENITA_QT6_PREFIX "${_qt6_prefix}")
    endif()
    set(ADENITA_QT_RUNTIME_DIR "${ADENITA_QT6_PREFIX}/bin")
    set(ADENITA_STANDALONE_TEST_RUNTIME_DIR "$<IF:$<CONFIG:Debug>,${ADENITA_SAMSON_DEBUG_RUNTIME_DIR},${ADENITA_SAMSON_RUNTIME_DIR}>")
  endif()

  add_executable(${ADENITA_STANDALONE_TEST_TARGET})
  target_sources(${ADENITA_STANDALONE_TEST_TARGET}
    PRIVATE
      ${${OUTPUT_UUID}_tests}
      include/ADNConfig.hpp
  )

  target_include_directories(${ADENITA_STANDALONE_TEST_TARGET}
    PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/include"
      "${CMAKE_CURRENT_SOURCE_DIR}/external/rapidjson/include"
      "${CMAKE_CURRENT_SOURCE_DIR}/modules/DAS/include"
      "${SAMSON_INCLUDE_PATH}"
  )

  target_link_libraries(${ADENITA_STANDALONE_TEST_TARGET}
    PRIVATE
      ${OUTPUT_NAME}
      ${QT_LIBRARIES}
      ${SAMSON_SDK_LIBRARIES}
  )

  set_property(TARGET ${ADENITA_STANDALONE_TEST_TARGET} PROPERTY FOLDER "Elements/Tests")
  set_target_properties(${ADENITA_STANDALONE_TEST_TARGET} PROPERTIES AUTOMOC ON)

  if(WIN32)
    set_property(
      TARGET ${ADENITA_STANDALONE_TEST_TARGET}
      PROPERTY VS_DEBUGGER_ENVIRONMENT
        "PATH=${ADENITA_STANDALONE_TEST_RUNTIME_DIR}$<SEMICOLON>${ADENITA_QT_RUNTIME_DIR}$<SEMICOLON>%PATH%"
    )
  endif()

  add_test(
    NAME ${OUTPUT_NAME}.Standalone
    COMMAND $<TARGET_FILE:${ADENITA_STANDALONE_TEST_TARGET}>
  )

  if(WIN32)
    set_tests_properties(${OUTPUT_NAME}.Standalone PROPERTIES
      ENVIRONMENT_MODIFICATION
        "PATH=path_list_prepend:${ADENITA_QT_RUNTIME_DIR};PATH=path_list_prepend:${ADENITA_STANDALONE_TEST_RUNTIME_DIR}"
    )
  endif()
endif()
