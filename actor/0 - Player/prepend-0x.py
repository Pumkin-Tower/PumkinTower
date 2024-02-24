import sys

if len(sys.argv) != 2:
    print("Usage: python3 prepend-0x.py <filename>")
    sys.exit(1)

file_path = sys.argv[1]

with open(file_path, 'r') as file:
    content = file.read().strip()

if content:
    with open(file_path, 'w') as file:
        file.write(f"#define COMMIT 0x{content}")
else:
    print("Error: File is empty.")

