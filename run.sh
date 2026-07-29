#!/bin/bash
set -e

JOBS=$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 2)

function delete {
    echo "[RUN SCRIPT] Delete build directory"
    rm -rf build
}

function build {
    echo "[RUN SCRIPT] Build project"
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build . -j"$JOBS"
    cd ..
}

function rebuild {
    delete
    build
}

run=true
built=0
function norun {
    echo "[RUN SCRIPT] Build without run"
    run=
}

function help {
    echo "[RUN SCRIPT] Usage: ./run [ARGUMENT]..."
    echo "[RUN SCRIPT] Arguments:"
    echo "[RUN SCRIPT]     -d, --delete     Delete build directory"
    echo "[RUN SCRIPT]     -b, --build      Build project"
    echo "[RUN SCRIPT]     -r, --rebuild    Rebuild project"
    echo "[RUN SCRIPT]     -R, --norun      Build without run"
    echo "[RUN SCRIPT]     -h, --help       Print this page"
}

while [ -n "$1" ]; do
    case "$1" in
        -d | --delete) delete ;;
        -b | --build) build; built=1; norun ;;
        -r | --rebuild) rebuild; built=1; norun ;;
        -R | --norun) norun ;;
        -h | --help) help
                     built=1
                     norun
                     break ;;
        *) echo "[RUN SCRIPT] Unknown argument: $1"
           help
           norun
           break ;;
    esac
    shift
done

if [ $built -eq 0 ]; then
    build
fi

if [ -n "$run" ]; then
    echo "[RUN SCRIPT] Run project"
    ./build/ChromaForge
fi
