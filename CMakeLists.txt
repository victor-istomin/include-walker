cmake_minimum_required (VERSION 3.11)
 
# --- Fetch FTXUI --------------------------------------------------------------
include(FetchContent)
 
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v5.0.0
)
 
FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
 
# ------------------------------------------------------------------------------

project(include_walker
  LANGUAGES CXX
  VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable parallel compilation
if(MSVC)
  add_compile_options(/MP)
else()
  add_compile_options(-j)
endif()

set(SOURCES
  src/main.cpp
  src/model.cpp
  src/config.cpp
  src/msvcParser.cpp
  src/ui.cpp
)

set(HEADERS
  src/model.h
  src/error.h
  src/msvcParser.h
  src/config.h
  src/collapsible-colorful.hpp
  src/ui.h
)

add_executable(include_walker ${SOURCES} ${HEADERS})
target_include_directories(include_walker PRIVATE src)
target_include_directories(include_walker PRIVATE submodules/argh) # include: argh!
 
target_link_libraries(include_walker
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component # Not needed for this example.
)
