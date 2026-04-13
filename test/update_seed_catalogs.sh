#!/bin/bash
# Regenerate all seed catalogs after a dungeon generation change.
# Run from the repository root: ./test/update_seed_catalogs.sh

set -euo pipefail

BROGUE=./bin/brogue

if [ ! -x "$BROGUE" ]; then
    echo "Binary not found at $BROGUE — building..."
    make bin/brogue
fi

echo "Generating brogue seed catalog (depth 40)..."
"$BROGUE" --print-seed-catalog 1 25 40 2>/dev/null > test/seed_catalogs/seed_catalog_brogue.txt

echo "Generating rapid_brogue seed catalog (depth 10)..."
"$BROGUE" --variant rapid_brogue --print-seed-catalog 1 25 10 2>/dev/null > test/seed_catalogs/seed_catalog_rapid_brogue.txt

echo "Generating bullet_brogue seed catalog (depth 5)..."
"$BROGUE" --variant bullet_brogue --print-seed-catalog 1 25 5 2>/dev/null > test/seed_catalogs/seed_catalog_bullet_brogue.txt

echo "All seed catalogs updated."
