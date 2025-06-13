#!/bin/bash
#
# SPDX-License-Identifier: LicenseRef-TridentMSLA
# SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>

# Check if the correct number of arguments is provided
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <input_file1> <input_file2> <output_file>"
    exit 1
fi

# Assign arguments to variables for clarity
input_file1="$1"
input_file2="$2"
output_file="$3"

# Extract just the filename without the path for error messages
filename1=$(basename "$input_file1")
filename2=$(basename "$input_file2")
outputname=$(basename "$output_file")

# Check if input_file1 exists
if [ ! -f "$input_file1" ]; then
    echo "$filename1 does not exist, skipping $outputname creation"
    exit 0
fi

# Check if input_file2 exists
if [ ! -f "$input_file2" ]; then
    echo "$filename2 does not exist, skipping $outputname creation"
    exit 0
fi

# Combine the files into the output file
cat "$input_file1" "$input_file2" > "$output_file"

# Check if the operation was successful
if [ $? -eq 0 ]; then
    echo "$outputname created successfully"
else
    echo "Error: Failed to create $outputname"
    exit 1
fi

exit 0 
