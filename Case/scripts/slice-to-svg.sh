#!/usr/bin/env bash


DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR/../dist" || exit 1

/Applications/Slic3r.app/Contents/MacOS/Slic3r ./case-8x8-top.stl --export-svg -o case-8x8-top.svg
/Applications/Slic3r.app/Contents/MacOS/Slic3r ./case-8x8-bottom.stl --export-svg -o case-8x8-bottom.svg

/Applications/Slic3r.app/Contents/MacOS/Slic3r ./case-16x8-top.stl --export-svg -o case-16x8-top.svg
/Applications/Slic3r.app/Contents/MacOS/Slic3r ./case-16x8-bottom.stl --export-svg -o case-16x8-bottom.svg

