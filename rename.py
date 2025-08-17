import os

def replace_c_file_content(directory):
    """Recursively replace 'jaguar' with 'void' in .c file contents"""
    for root, dirs, files in os.walk(directory):
        for file_name in files:
            if file_name.endswith(".c"):
                file_path = os.path.join(root, file_name)
                try:
                    with open(file_path, 'r') as file:
                        file_content = file.read()
                    if "jaguar" in file_content:
                        new_content = file_content.replace("jaguar", "void")
                        with open(file_path, 'w') as file:
                            file.write(new_content)
                        print(f"Replaced content in file: {file_path}")
                except Exception as e:
                    print(f"Error processing file: {file_path} - {str(e)}")

if __name__ == "__main__":
    current_directory = os.getcwd()
    replace_c_file_content(current_directory)
