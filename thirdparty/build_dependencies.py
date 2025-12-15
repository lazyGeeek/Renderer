import glob
import os
import re
import subprocess
import shutil

from zipfile import ZipFile

script_dir = os.path.dirname(os.path.abspath(__file__))
temp_dir = os.path.join(script_dir, "temp")
bins_dir = os.path.join(script_dir, "bins")
include_dir = os.path.join(script_dir, "include")
libs_dir = os.path.join(script_dir, "libs")

def delete_temp_folder():
    if os.path.exists(temp_dir):
        shutil.rmtree(temp_dir)

def delete_libs_folders():
    if os.path.exists(bins_dir):
        shutil.rmtree(bins_dir)
    
    if os.path.exists(include_dir):
        shutil.rmtree(include_dir)

    if os.path.exists(libs_dir):
        shutil.rmtree(libs_dir)

def copy_libs(source: str):
    for path in glob.glob(source + '/**', recursive=True):
        if os.path.isfile(path):
            if re.search(r'\.(lib|dylib|a)$', path):
                shutil.copy(path, libs_dir, follow_symlinks=False)
            elif re.search(r'\.(pdb|dll|so|so(\.\d{0,}){0,})$', path):
                shutil.copy(path, bins_dir, follow_symlinks=False)

def glfw_build():
    print("\n============================================================================")
    print("Start Building GLFW")
    print("============================================================================\n")

    glfw_dir = os.path.join(temp_dir, "glfw-3.3.8")
    glfw_build_dir = os.path.join(glfw_dir, "build")
    
    with ZipFile(os.path.join(script_dir, "glfw-3.3.8.zip"), "r") as glfw_zip:
        glfw_zip.extractall(temp_dir)
        
        glfw_cmake_config_args = ['-B' + glfw_build_dir,
                                  '-DGLFW_BUILD_EXAMPLES=OFF',
                                  '-DGLFW_BUILD_TESTS=OFF',
                                  '-DGLFW_BUILD_DOCS=OFF',
                                  '-DGLFW_INSTALL=OFF',
                                  '-DGLFW_VULKAN_STATIC=OFF',
                                  '-DBUILD_SHARED_LIBS=ON']
        glfw_cmake_build_args = []
        if os.name == 'nt':
            glfw_cmake_build_args.append('--config Release')
        subprocess.run(['cmake', glfw_dir] + glfw_cmake_config_args, check=True)
        subprocess.run(['cmake', '--build', glfw_build_dir], check=True)

        print("Copying include files")

        shutil.copytree(os.path.join(glfw_dir, "include"), include_dir, dirs_exist_ok=True)
        shutil.copy(os.path.join(glfw_dir, "build/src/glfw_config.h"), os.path.join(include_dir, "GLFW/glfw_config.h"))

        print("Copying libraries")
        copy_libs(glfw_build_dir)

    print("\n============================================================================")
    print("Finish Building GLFW")
    print("============================================================================\n")

def main():
    try:
        print("\n============================================================================")
        print("Creating directories")
        print("============================================================================\n")

        delete_temp_folder()
        delete_libs_folders()

        os.mkdir(temp_dir)
        os.mkdir(bins_dir)
        os.mkdir(include_dir)
        os.mkdir(libs_dir)

        glfw_build()

    except Exception as e:
        print(f"An error occurred: {e}")
        delete_libs_folders()

    finally:
        delete_temp_folder()

if __name__ == '__main__':
    main()