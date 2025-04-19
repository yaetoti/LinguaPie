cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(LinguaPie)

# Definitions
add_definitions(
  -DUNICODE
  -DWIN32
  -DWIN32_LEAN_AND_MEAN
  -DNOMINMAX
)

# Parameters
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Parsing sources
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp")

# Define executable
add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
set_target_properties(
  ${PROJECT_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}/$<CONFIG>/${PROJECT_NAME}"
)

# Link libraries
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
  ConsoleLib
  dxgi
  d3d11
  dcomp
  dwrite
  d3dcompiler
)

# Add includes
target_include_directories(
  ${PROJECT_NAME}
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/

  # Vendors
  ${CMAKE_SOURCE_DIR}/vendor/ConsoleLib/src/
)
