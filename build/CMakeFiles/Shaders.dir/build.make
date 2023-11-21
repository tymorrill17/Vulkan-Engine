# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/tyler/Codes/vulkan-guide

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/tyler/Codes/vulkan-guide/build

# Utility rule file for Shaders.

# Include any custom commands dependencies for this target.
include CMakeFiles/Shaders.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/Shaders.dir/progress.make

CMakeFiles/Shaders: ../shaders/fragment.frag.spv
CMakeFiles/Shaders: ../shaders/fragment_red_tri.frag.spv
CMakeFiles/Shaders: ../shaders/tri_mesh.vert.spv
CMakeFiles/Shaders: ../shaders/triangle.vert.spv
CMakeFiles/Shaders: ../shaders/triangle_red_tri.vert.spv

../shaders/fragment.frag.spv: ../shaders/fragment.frag
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/tyler/Codes/vulkan-guide/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating ../shaders/fragment.frag.spv"
	/usr/bin/glslangValidator -V /home/tyler/Codes/vulkan-guide/shaders/fragment.frag -o /home/tyler/Codes/vulkan-guide/shaders/fragment.frag.spv

../shaders/fragment_red_tri.frag.spv: ../shaders/fragment_red_tri.frag
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/tyler/Codes/vulkan-guide/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Generating ../shaders/fragment_red_tri.frag.spv"
	/usr/bin/glslangValidator -V /home/tyler/Codes/vulkan-guide/shaders/fragment_red_tri.frag -o /home/tyler/Codes/vulkan-guide/shaders/fragment_red_tri.frag.spv

../shaders/tri_mesh.vert.spv: ../shaders/tri_mesh.vert
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/tyler/Codes/vulkan-guide/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Generating ../shaders/tri_mesh.vert.spv"
	/usr/bin/glslangValidator -V /home/tyler/Codes/vulkan-guide/shaders/tri_mesh.vert -o /home/tyler/Codes/vulkan-guide/shaders/tri_mesh.vert.spv

../shaders/triangle.vert.spv: ../shaders/triangle.vert
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/tyler/Codes/vulkan-guide/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Generating ../shaders/triangle.vert.spv"
	/usr/bin/glslangValidator -V /home/tyler/Codes/vulkan-guide/shaders/triangle.vert -o /home/tyler/Codes/vulkan-guide/shaders/triangle.vert.spv

../shaders/triangle_red_tri.vert.spv: ../shaders/triangle_red_tri.vert
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/tyler/Codes/vulkan-guide/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Generating ../shaders/triangle_red_tri.vert.spv"
	/usr/bin/glslangValidator -V /home/tyler/Codes/vulkan-guide/shaders/triangle_red_tri.vert -o /home/tyler/Codes/vulkan-guide/shaders/triangle_red_tri.vert.spv

Shaders: CMakeFiles/Shaders
Shaders: ../shaders/fragment.frag.spv
Shaders: ../shaders/fragment_red_tri.frag.spv
Shaders: ../shaders/tri_mesh.vert.spv
Shaders: ../shaders/triangle.vert.spv
Shaders: ../shaders/triangle_red_tri.vert.spv
Shaders: CMakeFiles/Shaders.dir/build.make
.PHONY : Shaders

# Rule to build all files generated by this target.
CMakeFiles/Shaders.dir/build: Shaders
.PHONY : CMakeFiles/Shaders.dir/build

CMakeFiles/Shaders.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Shaders.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Shaders.dir/clean

CMakeFiles/Shaders.dir/depend:
	cd /home/tyler/Codes/vulkan-guide/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/tyler/Codes/vulkan-guide /home/tyler/Codes/vulkan-guide /home/tyler/Codes/vulkan-guide/build /home/tyler/Codes/vulkan-guide/build /home/tyler/Codes/vulkan-guide/build/CMakeFiles/Shaders.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Shaders.dir/depend
