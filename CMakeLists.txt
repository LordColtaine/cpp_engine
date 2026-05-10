cmake_minimum_required(VERSION 3.10)
project(Engine CXX)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- RAYLIB ---
# Using FetchContent to pull raylib directly from GitHub.
# GIT_SHALLOW and GIT_PROGRESS help manage the download size and visibility.
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# --- 1. ENGINE (100% Reusable) ---
set(ENGINE_SOURCES
    engine/src/memory/memorypool.cpp
    engine/src/memory/binnedallocator.cpp
    engine/src/core/world.cpp
    engine/src/core/threadpool.cpp
    engine/src/core/spatialgrid.cpp
    engine/src/logger/logger.cpp
    engine/src/behaviour/behaviourtree.cpp
    
    engine/include/memory/memorypool.h
    engine/include/memory/binnedallocator.h
    engine/include/core/gameobject.h
    engine/include/core/world.h
    engine/include/core/threadpool.h
    engine/include/core/spatialgrid.h
    engine/include/core/typeid.h
    engine/include/logger/logger.h
    engine/include/logger/profiler.h
    engine/include/behaviour/behaviourtree.h
    engine/include/behaviour/btagent.h
)

add_library(core_engine STATIC ${ENGINE_SOURCES})
target_include_directories(core_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/engine/include)

# --- 2. GAME SELECTION ---
# We use CACHE STRING so the build script can override this via -DACTIVE_GAME=
set(ACTIVE_GAME "rts" CACHE STRING "The game project to build")

if(ACTIVE_GAME STREQUAL "ants")
    set(GAME_SOURCES
        games/ants/src/main.cpp
        games/ants/src/ant.cpp
        games/ants/src/pheromonegrid.cpp
        games/ants/src/nest.cpp
        games/ants/src/predator_nodes.cpp
        
        games/ants/include/ant.h
        games/ants/include/pheromonegrid.h
        games/ants/include/food.h
        games/ants/include/nest.h
        games/ants/include/predator.h
        games/ants/include/predator_nodes.h
    )

    add_executable(game ${GAME_SOURCES})
    target_include_directories(game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/games/ants/include)
    target_link_libraries(game PRIVATE core_engine raylib)
    
elseif(ACTIVE_GAME STREQUAL "rts")
    set(GAME_SOURCES
        games/rts/src/main.cpp
    )
    
    add_executable(game ${GAME_SOURCES})
    target_include_directories(game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/games/rts/include)
    target_link_libraries(game PRIVATE core_engine raylib)
endif()

# --- CLANG FORMAT ---
# This target automatically formats your code based on the .clang-format file.
find_program(CLANG_FORMAT_EXE clang-format)
if(CLANG_FORMAT_EXE)
    add_custom_target(
        FormatCode
        COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${ENGINE_SOURCES} ${GAME_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running clang-format on all source files..."
    )
    add_dependencies(core_engine FormatCode)
endif()
