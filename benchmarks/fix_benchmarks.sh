#!/bin/bash
# Script to fix all benchmark files: update include path and add get_benchmark_name()

for file in $(find . -name "lib*.c" -type f); do
    # Extract benchmark name from file path (e.g., ./bubblesort/libbubblesort.c -> bubblesort)
    benchmark_name=$(basename $(dirname "$file"))
    
    echo "Processing $file (benchmark: $benchmark_name)"
    
    # Create a temporary file
    tmp_file="${file}.tmp"
    
    # Fix the include path from "support.h" to "../../support.h"
    sed 's|#include "support.h"|#include "../../support.h"|g' "$file" > "$tmp_file"
    
    # Check if get_benchmark_name already exists
    if ! grep -q "char\* get_benchmark_name" "$tmp_file"; then
        # Add get_benchmark_name() function at the end of the file
        cat >> "$tmp_file" << EOF

char* get_benchmark_name(void) {
    return "$benchmark_name";
}
EOF
    fi
    
    # Replace original file with updated version
    mv "$tmp_file" "$file"
done

echo "Done! All benchmark files updated."
