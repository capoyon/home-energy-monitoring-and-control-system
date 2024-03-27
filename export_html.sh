#!/bin/bash

# Source and destination directories
source_dir="./html"
destination_dir="./hemcs"

# Create destination directory if it doesn't exist
mkdir -p "$destination_dir"

# Loop through all files in the source directory
for file in "$source_dir"/*; do
    if [ -f "$file" ]; then
        # Get the filename without the path#!/bin/bash
# This script is used to compress the html files to .gz in current directory
# Then converts it to .h file using xxd

# Script filename
script_filename="compress.sh"
source_dir=$(pwd)/html
output_dir=$(pwd)/hemcs

rm -r ${source_dir}/tmp
mkdir ${source_dir}/tmp

# Compress all html to gz
for file in $source_dir/*.html; do
    filename=$(basename "$file")
    echo "$filename"
    mv "$file" "${source_dir}/tmp/${filename%.*}"
done

cd $source_dir/tmp

for file in *; do
    filename=$(basename "$file")
    echo "$filename > $output_dir/$filename.h"
    xxd -i "$file" "$output_dir/$filename.h"
    rm "$file"
done

rm -r ${source_dir}/tmp

        filename=$(basename "$file")
        
        # Apply xxd -i to the file and save the output to a new file in the destination directory
        xxd -i "$file" > "$destination_dir/$filename.h"
        
        echo "Processed: $file"
    fi
done

echo "All files processed."
