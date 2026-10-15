# Bootstrap a host C++ tool for both native and cross builds.
set(HFC_FONT_DIR "$ENV{HFC_FONT_DIR}" CACHE PATH "Directory containing DejaVuSans.ttf and DejaVuSans-Bold.ttf")
if(NOT HFC_FONT_DIR)
  find_path(HFC_SYSTEM_FONT_DIR DejaVuSans.ttf
    PATHS /usr/share/fonts/truetype/dejavu /usr/local/share/fonts /usr/share/fonts/dejavu
    NO_CMAKE_FIND_ROOT_PATH)
  set(HFC_FONT_DIR "${HFC_SYSTEM_FONT_DIR}" CACHE PATH "DejaVu font directory" FORCE)
endif()
if(NOT EXISTS "${HFC_FONT_DIR}/DejaVuSans.ttf" OR NOT EXISTS "${HFC_FONT_DIR}/DejaVuSans-Bold.ttf")
  message(FATAL_ERROR "Set HFC_FONT_DIR to a directory containing DejaVuSans.ttf and DejaVuSans-Bold.ttf")
endif()
if(CMAKE_CROSSCOMPILING)
  find_program(HFC_HOST_CXX NAMES c++ g++ clang++ NO_CMAKE_FIND_ROOT_PATH)
  if(NOT HFC_HOST_CXX)
    message(FATAL_ERROR "Set HFC_HOST_CXX to a host C++17 compiler for font embedding")
  endif()
else()
  set(HFC_HOST_CXX "${CMAKE_CXX_COMPILER}")
endif()
set(HFC_HOST_TOOL "${CMAKE_CURRENT_BINARY_DIR}/host-tools/gptframes-embed-fonts")
if(CMAKE_HOST_WIN32)
  string(APPEND HFC_HOST_TOOL ".exe")
endif()
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/host-tools" "${CMAKE_CURRENT_BINARY_DIR}/generated")
if(MSVC AND NOT CMAKE_CROSSCOMPILING)
  execute_process(COMMAND "${HFC_HOST_CXX}" /nologo /std:c++17 /EHsc
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/embed_fonts.cpp" "/Fe:${HFC_HOST_TOOL}"
    RESULT_VARIABLE HFC_FONT_COMPILE_RESULT OUTPUT_VARIABLE HFC_FONT_OUTPUT ERROR_VARIABLE HFC_FONT_ERROR)
else()
  execute_process(COMMAND "${HFC_HOST_CXX}" -std=c++17 -O2
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/embed_fonts.cpp" -o "${HFC_HOST_TOOL}"
    RESULT_VARIABLE HFC_FONT_COMPILE_RESULT OUTPUT_VARIABLE HFC_FONT_OUTPUT ERROR_VARIABLE HFC_FONT_ERROR)
endif()
if(NOT HFC_FONT_COMPILE_RESULT EQUAL 0)
  message(FATAL_ERROR "Host font tool failed: ${HFC_FONT_OUTPUT}${HFC_FONT_ERROR}")
endif()
execute_process(COMMAND "${HFC_HOST_TOOL}" "${HFC_FONT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}/generated/font_embedded.hpp"
  RESULT_VARIABLE HFC_FONT_GENERATE_RESULT OUTPUT_VARIABLE HFC_FONT_OUTPUT ERROR_VARIABLE HFC_FONT_ERROR)
if(NOT HFC_FONT_GENERATE_RESULT EQUAL 0)
  message(FATAL_ERROR "Font generation failed: ${HFC_FONT_OUTPUT}${HFC_FONT_ERROR}")
endif()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/tools/embed_fonts.cpp"
  "${HFC_FONT_DIR}/DejaVuSans.ttf" "${HFC_FONT_DIR}/DejaVuSans-Bold.ttf")
