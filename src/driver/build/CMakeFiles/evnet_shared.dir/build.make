# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.4

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/zwyao/code/krpc/src/core

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zwyao/code/krpc/src/core/build

# Include any dependencies generated for this target.
include CMakeFiles/evnet_shared.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/evnet_shared.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/evnet_shared.dir/flags.make

CMakeFiles/evnet_shared.dir/ev.cpp.o: CMakeFiles/evnet_shared.dir/flags.make
CMakeFiles/evnet_shared.dir/ev.cpp.o: ../ev.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zwyao/code/krpc/src/core/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/evnet_shared.dir/ev.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/evnet_shared.dir/ev.cpp.o -c /home/zwyao/code/krpc/src/core/ev.cpp

CMakeFiles/evnet_shared.dir/ev.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/evnet_shared.dir/ev.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zwyao/code/krpc/src/core/ev.cpp > CMakeFiles/evnet_shared.dir/ev.cpp.i

CMakeFiles/evnet_shared.dir/ev.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/evnet_shared.dir/ev.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zwyao/code/krpc/src/core/ev.cpp -o CMakeFiles/evnet_shared.dir/ev.cpp.s

CMakeFiles/evnet_shared.dir/ev.cpp.o.requires:

.PHONY : CMakeFiles/evnet_shared.dir/ev.cpp.o.requires

CMakeFiles/evnet_shared.dir/ev.cpp.o.provides: CMakeFiles/evnet_shared.dir/ev.cpp.o.requires
	$(MAKE) -f CMakeFiles/evnet_shared.dir/build.make CMakeFiles/evnet_shared.dir/ev.cpp.o.provides.build
.PHONY : CMakeFiles/evnet_shared.dir/ev.cpp.o.provides

CMakeFiles/evnet_shared.dir/ev.cpp.o.provides.build: CMakeFiles/evnet_shared.dir/ev.cpp.o


CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.o: CMakeFiles/evnet_shared.dir/flags.make
CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.o: ../ev_process_internal.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zwyao/code/krpc/src/core/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.o -c /home/zwyao/code/krpc/src/core/ev_process_internal.cpp

CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zwyao/code/krpc/src/core/ev_process_internal.cpp > CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.i

CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zwyao/code/krpc/src/core/ev_process_internal.cpp -o CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.s

CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.o.requires:

.PHONY : CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.o.requires

CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.o.provides: CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.o.requires
	$(MAKE) -f CMakeFiles/evnet_shared.dir/build.make CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.o.provides.build
.PHONY : CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.o.provides

CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.o.provides.build: CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.o


CMakeFiles/evnet_shared.dir/ev_io.cpp.o: CMakeFiles/evnet_shared.dir/flags.make
CMakeFiles/evnet_shared.dir/ev_io.cpp.o: ../ev_io.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zwyao/code/krpc/src/core/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/evnet_shared.dir/ev_io.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/evnet_shared.dir/ev_io.cpp.o -c /home/zwyao/code/krpc/src/core/ev_io.cpp

CMakeFiles/evnet_shared.dir/ev_io.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/evnet_shared.dir/ev_io.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zwyao/code/krpc/src/core/ev_io.cpp > CMakeFiles/evnet_shared.dir/ev_io.cpp.i

CMakeFiles/evnet_shared.dir/ev_io.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/evnet_shared.dir/ev_io.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zwyao/code/krpc/src/core/ev_io.cpp -o CMakeFiles/evnet_shared.dir/ev_io.cpp.s

CMakeFiles/evnet_shared.dir/ev_io.cpp.o.requires:

.PHONY : CMakeFiles/evnet_shared.dir/ev_io.cpp.o.requires

CMakeFiles/evnet_shared.dir/ev_io.cpp.o.provides: CMakeFiles/evnet_shared.dir/ev_io.cpp.o.requires
	$(MAKE) -f CMakeFiles/evnet_shared.dir/build.make CMakeFiles/evnet_shared.dir/ev_io.cpp.o.provides.build
.PHONY : CMakeFiles/evnet_shared.dir/ev_io.cpp.o.provides

CMakeFiles/evnet_shared.dir/ev_io.cpp.o.provides.build: CMakeFiles/evnet_shared.dir/ev_io.cpp.o


CMakeFiles/evnet_shared.dir/ev_timer.cpp.o: CMakeFiles/evnet_shared.dir/flags.make
CMakeFiles/evnet_shared.dir/ev_timer.cpp.o: ../ev_timer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zwyao/code/krpc/src/core/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/evnet_shared.dir/ev_timer.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/evnet_shared.dir/ev_timer.cpp.o -c /home/zwyao/code/krpc/src/core/ev_timer.cpp

CMakeFiles/evnet_shared.dir/ev_timer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/evnet_shared.dir/ev_timer.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zwyao/code/krpc/src/core/ev_timer.cpp > CMakeFiles/evnet_shared.dir/ev_timer.cpp.i

CMakeFiles/evnet_shared.dir/ev_timer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/evnet_shared.dir/ev_timer.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zwyao/code/krpc/src/core/ev_timer.cpp -o CMakeFiles/evnet_shared.dir/ev_timer.cpp.s

CMakeFiles/evnet_shared.dir/ev_timer.cpp.o.requires:

.PHONY : CMakeFiles/evnet_shared.dir/ev_timer.cpp.o.requires

CMakeFiles/evnet_shared.dir/ev_timer.cpp.o.provides: CMakeFiles/evnet_shared.dir/ev_timer.cpp.o.requires
	$(MAKE) -f CMakeFiles/evnet_shared.dir/build.make CMakeFiles/evnet_shared.dir/ev_timer.cpp.o.provides.build
.PHONY : CMakeFiles/evnet_shared.dir/ev_timer.cpp.o.provides

CMakeFiles/evnet_shared.dir/ev_timer.cpp.o.provides.build: CMakeFiles/evnet_shared.dir/ev_timer.cpp.o


CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.o: CMakeFiles/evnet_shared.dir/flags.make
CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.o: ../reactor/reactor_epoll.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zwyao/code/krpc/src/core/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.o -c /home/zwyao/code/krpc/src/core/reactor/reactor_epoll.cpp

CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zwyao/code/krpc/src/core/reactor/reactor_epoll.cpp > CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.i

CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zwyao/code/krpc/src/core/reactor/reactor_epoll.cpp -o CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.s

CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.o.requires:

.PHONY : CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.o.requires

CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.o.provides: CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.o.requires
	$(MAKE) -f CMakeFiles/evnet_shared.dir/build.make CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.o.provides.build
.PHONY : CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.o.provides

CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.o.provides.build: CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.o


# Object files for target evnet_shared
evnet_shared_OBJECTS = \
"CMakeFiles/evnet_shared.dir/ev.cpp.o" \
"CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.o" \
"CMakeFiles/evnet_shared.dir/ev_io.cpp.o" \
"CMakeFiles/evnet_shared.dir/ev_timer.cpp.o" \
"CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.o"

# External object files for target evnet_shared
evnet_shared_EXTERNAL_OBJECTS =

libevnet_core.so: CMakeFiles/evnet_shared.dir/ev.cpp.o
libevnet_core.so: CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.o
libevnet_core.so: CMakeFiles/evnet_shared.dir/ev_io.cpp.o
libevnet_core.so: CMakeFiles/evnet_shared.dir/ev_timer.cpp.o
libevnet_core.so: CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.o
libevnet_core.so: CMakeFiles/evnet_shared.dir/build.make
libevnet_core.so: CMakeFiles/evnet_shared.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/zwyao/code/krpc/src/core/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX shared library libevnet_core.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/evnet_shared.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/evnet_shared.dir/build: libevnet_core.so

.PHONY : CMakeFiles/evnet_shared.dir/build

CMakeFiles/evnet_shared.dir/requires: CMakeFiles/evnet_shared.dir/ev.cpp.o.requires
CMakeFiles/evnet_shared.dir/requires: CMakeFiles/evnet_shared.dir/ev_process_internal.cpp.o.requires
CMakeFiles/evnet_shared.dir/requires: CMakeFiles/evnet_shared.dir/ev_io.cpp.o.requires
CMakeFiles/evnet_shared.dir/requires: CMakeFiles/evnet_shared.dir/ev_timer.cpp.o.requires
CMakeFiles/evnet_shared.dir/requires: CMakeFiles/evnet_shared.dir/reactor/reactor_epoll.cpp.o.requires

.PHONY : CMakeFiles/evnet_shared.dir/requires

CMakeFiles/evnet_shared.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/evnet_shared.dir/cmake_clean.cmake
.PHONY : CMakeFiles/evnet_shared.dir/clean

CMakeFiles/evnet_shared.dir/depend:
	cd /home/zwyao/code/krpc/src/core/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zwyao/code/krpc/src/core /home/zwyao/code/krpc/src/core /home/zwyao/code/krpc/src/core/build /home/zwyao/code/krpc/src/core/build /home/zwyao/code/krpc/src/core/build/CMakeFiles/evnet_shared.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/evnet_shared.dir/depend
