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
CMAKE_SOURCE_DIR = /home/lhz/server

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/lhz/server/build

# Include any dependencies generated for this target.
include CMakeFiles/myserver.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/myserver.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/myserver.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/myserver.dir/flags.make

CMakeFiles/myserver.dir/src/main.cpp.o: CMakeFiles/myserver.dir/flags.make
CMakeFiles/myserver.dir/src/main.cpp.o: ../src/main.cpp
CMakeFiles/myserver.dir/src/main.cpp.o: CMakeFiles/myserver.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lhz/server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/myserver.dir/src/main.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/myserver.dir/src/main.cpp.o -MF CMakeFiles/myserver.dir/src/main.cpp.o.d -o CMakeFiles/myserver.dir/src/main.cpp.o -c /home/lhz/server/src/main.cpp

CMakeFiles/myserver.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/myserver.dir/src/main.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lhz/server/src/main.cpp > CMakeFiles/myserver.dir/src/main.cpp.i

CMakeFiles/myserver.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/myserver.dir/src/main.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lhz/server/src/main.cpp -o CMakeFiles/myserver.dir/src/main.cpp.s

CMakeFiles/myserver.dir/src/server.cpp.o: CMakeFiles/myserver.dir/flags.make
CMakeFiles/myserver.dir/src/server.cpp.o: ../src/server.cpp
CMakeFiles/myserver.dir/src/server.cpp.o: CMakeFiles/myserver.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lhz/server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/myserver.dir/src/server.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/myserver.dir/src/server.cpp.o -MF CMakeFiles/myserver.dir/src/server.cpp.o.d -o CMakeFiles/myserver.dir/src/server.cpp.o -c /home/lhz/server/src/server.cpp

CMakeFiles/myserver.dir/src/server.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/myserver.dir/src/server.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lhz/server/src/server.cpp > CMakeFiles/myserver.dir/src/server.cpp.i

CMakeFiles/myserver.dir/src/server.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/myserver.dir/src/server.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lhz/server/src/server.cpp -o CMakeFiles/myserver.dir/src/server.cpp.s

CMakeFiles/myserver.dir/src/logger.cpp.o: CMakeFiles/myserver.dir/flags.make
CMakeFiles/myserver.dir/src/logger.cpp.o: ../src/logger.cpp
CMakeFiles/myserver.dir/src/logger.cpp.o: CMakeFiles/myserver.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lhz/server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/myserver.dir/src/logger.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/myserver.dir/src/logger.cpp.o -MF CMakeFiles/myserver.dir/src/logger.cpp.o.d -o CMakeFiles/myserver.dir/src/logger.cpp.o -c /home/lhz/server/src/logger.cpp

CMakeFiles/myserver.dir/src/logger.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/myserver.dir/src/logger.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lhz/server/src/logger.cpp > CMakeFiles/myserver.dir/src/logger.cpp.i

CMakeFiles/myserver.dir/src/logger.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/myserver.dir/src/logger.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lhz/server/src/logger.cpp -o CMakeFiles/myserver.dir/src/logger.cpp.s

# Object files for target myserver
myserver_OBJECTS = \
"CMakeFiles/myserver.dir/src/main.cpp.o" \
"CMakeFiles/myserver.dir/src/server.cpp.o" \
"CMakeFiles/myserver.dir/src/logger.cpp.o"

# External object files for target myserver
myserver_EXTERNAL_OBJECTS =

myserver: CMakeFiles/myserver.dir/src/main.cpp.o
myserver: CMakeFiles/myserver.dir/src/server.cpp.o
myserver: CMakeFiles/myserver.dir/src/logger.cpp.o
myserver: CMakeFiles/myserver.dir/build.make
myserver: CMakeFiles/myserver.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/lhz/server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable myserver"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/myserver.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/myserver.dir/build: myserver
.PHONY : CMakeFiles/myserver.dir/build

CMakeFiles/myserver.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/myserver.dir/cmake_clean.cmake
.PHONY : CMakeFiles/myserver.dir/clean

CMakeFiles/myserver.dir/depend:
	cd /home/lhz/server/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/lhz/server /home/lhz/server /home/lhz/server/build /home/lhz/server/build /home/lhz/server/build/CMakeFiles/myserver.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/myserver.dir/depend

