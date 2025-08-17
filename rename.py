import os

def replace_void_string(directory):
    """Recursively replace 'void' string in file contents"""
    for root, dirs, files in os.walk(directory):
        for file_name in files:
            file_path = os.path.join(root, file_name)
            try:
                with open(file_path, 'r') as file:
                    file_content = file.read()
                if "2025 TigerClips1 <spongebob1966@proton.me>" in file_content:
                    new_content = file_content.replace("2025 TigerClips1 <spongebob1966@proton.me>", "2025 TigerClips1 <spongebob1966@proton.me>")
                    with open(file_path, 'w') as file:
                        file.write(new_content)
                    print(f"Replaced string in file: {file_path}")
                elif "~/void" in file_content:
                    new_content = file_content.replace("~/void", "~/jaguar")
                    with open(file_path, 'w') as file:
                        file.write(new_content)
                    print(f"Replaced string in file: {file_path}")
                elif "void" in file_content:
                    new_content = file_content.replace("void", "jaguar")
                    with open(file_path, 'w') as file:
                        file.write(new_content)
                    print(f"Replaced string in file: {file_path}")
            except Exception as e:
                print(f"Error processing file: {file_path} - {str(e)}")

if __name__ == "__main__":
    current_directory = os.getcwd()
    replace_void_string(current_directory)
