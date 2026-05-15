#!/bin/bash

# $1 is now GAME (default rts), $2 is now ACTION (default all)
GAME=${1:-"rts"}
ACTION=${2:-"all"}

DO_BUILD=false
DO_RUN=false

case "$ACTION" in
    build) DO_BUILD=true ;;
    run)   DO_RUN=true ;;
    all)   DO_BUILD=true; DO_RUN=true ;;
esac

if [ "$DO_BUILD" = true ]; then
    mkdir -p build
    cd build
    
    # 1. Run CMake and CATCH ERRORS
    cmake .. -DACTIVE_GAME=$GAME -DFETCHCONTENT_QUIET=OFF
    if [ $? -ne 0 ]; then
        echo "❌ CMAKE CONFIGURATION FAILED! Check the red text above."
        exit 1
    fi
    
    # 2. Run Make and CATCH ERRORS
    make -j$(nproc)
    if [ $? -ne 0 ]; then
        echo "❌ COMPILATION FAILED!"
        exit 1
    fi
    cd .. 
fi

if [ "$DO_RUN" = true ]; then
    # 3. Account for Windows .exe vs Linux binary
    EXE_NAME="build/game"
    if [ -f "build/game.exe" ]; then
        EXE_NAME="build/game.exe"
    fi

    if [ ! -f "$EXE_NAME" ]; then
        echo "❌ ERROR: EXECUTABLE MISSING"
        echo "Here is what is inside your build/ folder:"
        ls -la build/
        exit 1
    fi
    
    echo "✅ Launching $EXE_NAME..."
    ./$EXE_NAME
fi
