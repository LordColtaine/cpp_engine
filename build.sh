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
    # Force the CACHE update so the switch always happens
    cmake .. -DACTIVE_GAME=$GAME -DFETCHCONTENT_QUIET=OFF
    make -j$(nproc)
    
    if [ $? -ne 0 ]; then
        echo "COMPILATION FAILED"
        exit 1
    fi
    cd .. 
fi

if [ "$DO_RUN" = true ]; then
    if [ ! -f "build/game" ]; then
        echo "ERROR: EXECUTABLE MISSING"
        exit 1
    fi
    cd build && ./game
fi
