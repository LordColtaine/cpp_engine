cmake_minimum_required(VERSION 3.10)
project(Engine CXX)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# --- RAYLIB ---
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# --- ENET (Reliable UDP Networking) ---
FetchContent_Declare(
    enet
    GIT_REPOSITORY https://github.com/lsalzman/enet.git
    GIT_TAG master 
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(enet)

# --- ENGINE (100% Reusable) ---
set(ENGINE_SOURCES
    engine/src/memory/memorypool.cpp
    engine/src/memory/binnedallocator.cpp
    engine/src/core/world.cpp
    engine/src/core/threadpool.cpp
    engine/src/core/spatialgrid.cpp
    engine/src/logger/logger.cpp
    engine/src/behaviour/behaviourtree.cpp
    engine/src/network/client.cpp
    engine/src/network/server.cpp
    
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
    engine/include/network/client.h
    engine/include/network/server.h
)

add_library(core_engine STATIC ${ENGINE_SOURCES})
target_include_directories(core_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/engine/include ${enet_SOURCE_DIR}/include)

# Link ENet to the engine so all games inherit the networking capability
target_link_libraries(core_engine PUBLIC enet)

# --- GAME SELECTION ---
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
        games/rts/include/unit.h
        games/rts/include/unitmanager.h
        
	games/rts/src/main.cpp
        games/rts/src/unit.cpp
        games/rts/src/unitmanager.cpp
    )
    
    add_executable(game ${GAME_SOURCES})
    target_include_directories(game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/games/rts/include)
    target_link_libraries(game PRIVATE core_engine raylib)
    
    # ---  RTS STANDALONE SERVER ---
set(SERVER_SOURCES
    games/rts/server/main.cpp
)
add_executable(server ${SERVER_SOURCES})
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/games/rts/include)
target_link_libraries(server PRIVATE core_engine)
endif()

# --- CLANG FORMAT ---
find_program(CLANG_FORMAT_EXE clang-format)
if(CLANG_FORMAT_EXE)
    add_custom_target(
        FormatCode
        # --- UPDATED: Added ${SERVER_SOURCES} to the list ---
        COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${ENGINE_SOURCES} ${GAME_SOURCES} ${SERVER_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running clang-format on all source files..."
    )
    add_dependencies(core_engine FormatCode)
endif()
