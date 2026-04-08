import os

def bundle_codebase(output_filename="codebase_dump.txt"):
    # Directories we absolutely do not want to crawl
    ignore_dirs = {'.git', 'build', 'bin', 'out', '.cache', '.vscode'}
    
    # File extensions we actually care about
    valid_extensions = {'.h', '.cpp', '.hpp'}
    valid_files = {'CMakeLists.txt'}
    
    with open(output_filename, 'w', encoding='utf-8') as outfile:
        # Walk through the directory tree
        for root, dirs, files in os.walk('.'):
            # Modify the 'dirs' list in-place to skip ignored folders
            dirs[:] = [d for d in dirs if d not in ignore_dirs]
            
            for file in files:
                # Check if it's a C++ file or our CMake script
                if any(file.endswith(ext) for ext in valid_extensions) or file in valid_files:
                    filepath = os.path.join(root, file)
                    
                    # Create a highly visible header for each file
                    outfile.write(f"{'='*60}\n")
                    outfile.write(f"FILE: {filepath}\n")
                    outfile.write(f"{'='*60}\n\n")
                    
                    try:
                        with open(filepath, 'r', encoding='utf-8') as infile:
                            outfile.write(infile.read())
                    except Exception as e:
                        outfile.write(f"[Error reading file: {e}]\n")
                    
                    outfile.write("\n\n")
    
    print(f"Success! Your codebase has been bundled into '{output_filename}'.")

if __name__ == "__main__":
    bundle_codebase()
