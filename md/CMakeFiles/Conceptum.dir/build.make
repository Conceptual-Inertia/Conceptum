# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.6

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.6.2/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.6.2/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/derros/Projects/Conceptum

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/derros/Projects/Conceptum/md

# Include any dependencies generated for this target.
include CMakeFiles/Conceptum.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Conceptum.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Conceptum.dir/flags.make

CMakeFiles/Conceptum.dir/src/main.c.o: CMakeFiles/Conceptum.dir/flags.make
CMakeFiles/Conceptum.dir/src/main.c.o: ../src/main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/derros/Projects/Conceptum/md/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/Conceptum.dir/src/main.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/Conceptum.dir/src/main.c.o   -c /Users/derros/Projects/Conceptum/src/main.c

CMakeFiles/Conceptum.dir/src/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Conceptum.dir/src/main.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/derros/Projects/Conceptum/src/main.c > CMakeFiles/Conceptum.dir/src/main.c.i

CMakeFiles/Conceptum.dir/src/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Conceptum.dir/src/main.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/derros/Projects/Conceptum/src/main.c -o CMakeFiles/Conceptum.dir/src/main.c.s

CMakeFiles/Conceptum.dir/src/main.c.o.requires:

.PHONY : CMakeFiles/Conceptum.dir/src/main.c.o.requires

CMakeFiles/Conceptum.dir/src/main.c.o.provides: CMakeFiles/Conceptum.dir/src/main.c.o.requires
	$(MAKE) -f CMakeFiles/Conceptum.dir/build.make CMakeFiles/Conceptum.dir/src/main.c.o.provides.build
.PHONY : CMakeFiles/Conceptum.dir/src/main.c.o.provides

CMakeFiles/Conceptum.dir/src/main.c.o.provides.build: CMakeFiles/Conceptum.dir/src/main.c.o


CMakeFiles/Conceptum.dir/src/memman.c.o: CMakeFiles/Conceptum.dir/flags.make
CMakeFiles/Conceptum.dir/src/memman.c.o: ../src/memman.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/derros/Projects/Conceptum/md/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/Conceptum.dir/src/memman.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/Conceptum.dir/src/memman.c.o   -c /Users/derros/Projects/Conceptum/src/memman.c

CMakeFiles/Conceptum.dir/src/memman.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Conceptum.dir/src/memman.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/derros/Projects/Conceptum/src/memman.c > CMakeFiles/Conceptum.dir/src/memman.c.i

CMakeFiles/Conceptum.dir/src/memman.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Conceptum.dir/src/memman.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/derros/Projects/Conceptum/src/memman.c -o CMakeFiles/Conceptum.dir/src/memman.c.s

CMakeFiles/Conceptum.dir/src/memman.c.o.requires:

.PHONY : CMakeFiles/Conceptum.dir/src/memman.c.o.requires

CMakeFiles/Conceptum.dir/src/memman.c.o.provides: CMakeFiles/Conceptum.dir/src/memman.c.o.requires
	$(MAKE) -f CMakeFiles/Conceptum.dir/build.make CMakeFiles/Conceptum.dir/src/memman.c.o.provides.build
.PHONY : CMakeFiles/Conceptum.dir/src/memman.c.o.provides

CMakeFiles/Conceptum.dir/src/memman.c.o.provides.build: CMakeFiles/Conceptum.dir/src/memman.c.o


# Object files for target Conceptum
Conceptum_OBJECTS = \
"CMakeFiles/Conceptum.dir/src/main.c.o" \
"CMakeFiles/Conceptum.dir/src/memman.c.o"

# External object files for target Conceptum
Conceptum_EXTERNAL_OBJECTS =

Conceptum: CMakeFiles/Conceptum.dir/src/main.c.o
Conceptum: CMakeFiles/Conceptum.dir/src/memman.c.o
Conceptum: CMakeFiles/Conceptum.dir/build.make
Conceptum: CMakeFiles/Conceptum.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/derros/Projects/Conceptum/md/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable Conceptum"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Conceptum.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Conceptum.dir/build: Conceptum

.PHONY : CMakeFiles/Conceptum.dir/build

CMakeFiles/Conceptum.dir/requires: CMakeFiles/Conceptum.dir/src/main.c.o.requires
CMakeFiles/Conceptum.dir/requires: CMakeFiles/Conceptum.dir/src/memman.c.o.requires

.PHONY : CMakeFiles/Conceptum.dir/requires

CMakeFiles/Conceptum.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Conceptum.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Conceptum.dir/clean

CMakeFiles/Conceptum.dir/depend:
	cd /Users/derros/Projects/Conceptum/md && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/derros/Projects/Conceptum /Users/derros/Projects/Conceptum /Users/derros/Projects/Conceptum/md /Users/derros/Projects/Conceptum/md /Users/derros/Projects/Conceptum/md/CMakeFiles/Conceptum.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Conceptum.dir/depend
