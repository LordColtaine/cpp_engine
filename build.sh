#!/bin/bash

# Read the first argument passed to the script. If none is provided, default to "all".
ACTION=${1:-"all"}

DO_BUILD=false
DO_RUN=false

# Parse the command line argument
case "$ACTION" in
    build)
        DO_BUILD=true
        ;;
    run)
        DO_RUN=true
        ;;
    all)
        DO_BUILD=true
        DO_RUN=true
        ;;
    *)
        echo "Invalid option: $1"
        echo "Usage: ./build.sh [build | run | all]"
        echo "  build : Only compile the project"
        echo "  run   : Only execute the game"
        echo "  all   : Compile and execute (Default)"
        exit 1
        ;;
esac

# Build Phase
if [ "$DO_BUILD" = true ]; then
    mkdir -p build
    cd build
    
    cmake ..
    make
    
    # If compilation fails, stop the script immediately
    if [ $? -ne 0 ]; then
        echo "--------------------------"
        echo "   COMPILATION FAILED     "
        echo "--------------------------"
        exit 1
    else
        echo "--------------------------"
        echo "  COMPILATION SUCCEEDED   "
        echo "--------------------------"
    fi
    
    # Go back to the root folder so the run phase has a consistent starting point
    cd .. 
fi

# Run Phase
if [ "$DO_RUN" = true ]; then
    # Safety check: make sure the executable actually exists before trying to run it
    if [ ! -f "build/game_ants" ]; then
        echo "--------------------------"
        echo " ERROR: EXECUTABLE MISSING"
        echo "--------------------------"
        echo "Please run './build.sh build' first."
        exit 1
    fi

    echo "--------------------------"
    echo "        APP OUTPUT        "
    echo "--------------------------"
    cd build && ./game_ants
fi
