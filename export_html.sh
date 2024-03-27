#!/bin/bash

# Define the output file
output_file="../hemcs/html.h"

# Delete the output file if it already exists
rm -f "$output_file"

# Iterate over each file in the directory
cd html
rm "$output_file"
echo "#ifndef HTML_H" >> "$output_file"
echo "#define HTML_H" >> "$output_file"
echo "" >> "$output_file"
for file in *; do
    # Check if it's a regular file
    if [ -f "$file" ]; then
        # Convert the file to hexadecimal using xxd and append it to the output file
        xxd -i "$file" >> "$output_file"
        echo "" >> "$output_file"  # Add an empty line between each file
    fi
done
echo "" >> "$output_file"
echo "#endif //HTML_H" >> "$output_file"

echo "Conversion completed. Output written to $output_file"
