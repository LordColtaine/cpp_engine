cmake_minimum_required(VERSION 3.10)
project(Engine CXX)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- RAYLIB ---
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
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
    
    engine/include/memory/memorypool.h
    engine/include/memory/binnedallocator.h
    engine/include/core/gameobject.h
    engine/include/core/world.h
    engine/include/core/threadpool.h
    engine/include/core/spatialgrid.h
    engine/include/core/typeid.h
    engine/include/logger/logger.h
)

add_library(core_engine STATIC ${ENGINE_SOURCES})
target_include_directories(core_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/engine/include)

# --- 2. GAME: ANTS ---
set(GAME_ANTS_SOURCES
    games/ants/src/main.cpp
    games/ants/src/ant.cpp
    games/ants/src/pheromonegrid.cpp
    
    games/ants/include/ant.h
    games/ants/include/pheromonegrid.h
    games/ants/include/food.h
)

add_executable(game_ants ${GAME_ANTS_SOURCES})
target_include_directories(game_ants PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/games/ants/include)
target_link_libraries(game_ants PRIVATE core_engine raylib)

# --- CLANG FORMAT ---
find_program(CLANG_FORMAT_EXE clang-format)
if(CLANG_FORMAT_EXE)
    add_custom_target(
        FormatCode
        COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${ENGINE_SOURCES} ${GAME_ANTS_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running clang-format on all source files..."
    )
    add_dependencies(core_engine FormatCode)
endif()
