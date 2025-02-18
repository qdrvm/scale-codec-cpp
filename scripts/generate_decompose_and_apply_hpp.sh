#!/bin/sh

# This script modifies a specified C++ source file by replacing a generated section.
# It reads the file, finds the markers '-BEGIN-GENERATED-SECTION-' and '-END-GENERATED-SECTION-',
# generates C++ code based on a given integer N, and writes the modified content to the output file.
# If the markers are not found, the script exits with an error.
# If output file contents matches generated file contents, it won't be modified.
# Usage: ./generate_decompose_and_apply_hpp.sh <PATH_HPP_IN> <N> <PATH_HPP_OUT>
# Where <PATH_HPP_IN> is the path to the template C++ source file, <N> is an integer between 0 and 1000,
# and <PATH_HPP_OUT> is a path to the output file.

# Validate input
if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
    echo "Usage: $0 <PATH_HPP_IN> <N> <PATH_HPP_OUT>"
    exit 1
fi

PATH_HPP_IN="$1"
N="$2"
PATH_HPP_OUT="$3"
mkdir -p $(dirname "$PATH_HPP_OUT")

if [ ! -f "$PATH_HPP_IN" ]; then
    echo "Error: File '$PATH_HPP_IN' not found."
    exit 1
fi

if echo "$N" | grep -q "[^0-9]" || [ "$N" -lt 0 ] || [ "$N" -gt 1000 ]; then
    echo "N must be an integer between 0 and 1000"
    exit 1
fi

# Find the section markers
BEGIN_LINE=$(awk '/-BEGIN-GENERATED-SECTION-/ {print NR; exit}' "$PATH_HPP_IN")
END_LINE=$(awk '/-END-GENERATED-SECTION-/ {print NR; exit}' "$PATH_HPP_IN")

if [ -z "$BEGIN_LINE" ] || [ -z "$END_LINE" ]; then
    echo "Error: Required markers not found in the file."
    exit 1
fi

# Create temporary file
tempfile=$(mktemp) || { echo "Failed to create temporary file"; exit 1; }

# Trap to ensure the temporary file is deleted in case of errors
trap 'rm -f "$tempfile"' EXIT

# Write the modified content to the temporary file
awk -v begin="$BEGIN_LINE" -v end="$END_LINE" -v n="$N" '
BEGIN { in_generated_section = 0; }
{
    if (NR == begin) {
        print; # Print the BEGIN marker
        in_generated_section = 1;
        for (i = 1; i <= n; i++) {
            printf "    } else if constexpr (N == %d) {\n", i;
            printf "      auto &[";
            for (j = 1; j <= i; j++) {
                printf "v%d%s", j, (j < i ? ", " : "");
            }
            printf "] = v;\n";
            printf "      return f(";
            for (j = 1; j <= i; j++) {
                printf "v%d%s", j, (j < i ? ", " : "");
            }
            printf ");\n";
        }
        next;
    }
    if (NR == end) {
        print; # Print the END marker
        in_generated_section = 0;
        next;
    }
    if (!in_generated_section) {
        print;
    }
}' "$PATH_HPP_IN" > "$tempfile" || { echo "Error processing file"; exit 1; }

# Check if the file has changed before overwriting
if ! cmp -s "$tempfile" "$PATH_HPP_OUT"; then
    mv "$tempfile" "$PATH_HPP_OUT"
    echo "File '$PATH_HPP_OUT' successfully updated."
else
    rm -f "$tempfile"
    echo "No changes detected."
fi

# Remove trap on success
trap - EXIT

exit 0
