# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/zzq/StreamingMedia/FFMPEG/simplest_ffmpeg_player/simplest_ffmpeg_player

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zzq/StreamingMedia/FFMPEG/simplest_ffmpeg_player/simplest_ffmpeg_player/build

# Include any dependencies generated for this target.
include CMakeFiles/simplest_ffmpeg_player.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/simplest_ffmpeg_player.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/simplest_ffmpeg_player.dir/flags.make

CMakeFiles/simplest_ffmpeg_player.dir/src/simplest_ffmpeg_player.cpp.o: CMakeFiles/simplest_ffmpeg_player.dir/flags.make
CMakeFiles/simplest_ffmpeg_player.dir/src/simplest_ffmpeg_player.cpp.o: ../src/simplest_ffmpeg_player.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zzq/StreamingMedia/FFMPEG/simplest_ffmpeg_player/simplest_ffmpeg_player/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/simplest_ffmpeg_player.dir/src/simplest_ffmpeg_player.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/simplest_ffmpeg_player.dir/src/simplest_ffmpeg_player.cpp.o -c /home/zzq/StreamingMedia/FFMPEG/simplest_ffmpeg_player/simplest_ffmpeg_player/src/simplest_ffmpeg_player.cpp

CMakeFiles/simplest_ffmpeg_player.dir/src/simplest_ffmpeg_player.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/simplest_ffmpeg_player.dir/src/simplest_ffmpeg_player.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zzq/StreamingMedia/FFMPEG/simplest_ffmpeg_player/simplest_ffmpeg_player/src/simplest_ffmpeg_player.cpp > CMakeFiles/simplest_ffmpeg_player.dir/src/simplest_ffmpeg_player.cpp.i

CMakeFiles/simplest_ffmpeg_player.dir/src/simplest_ffmpeg_player.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/simplest_ffmpeg_player.dir/src/simplest_ffmpeg_player.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zzq/StreamingMedia/FFMPEG/simplest_ffmpeg_player/simplest_ffmpeg_player/src/simplest_ffmpeg_player.cpp -o CMakeFiles/simplest_ffmpeg_player.dir/src/simplest_ffmpeg_player.cpp.s

# Object files for target simplest_ffmpeg_player
simplest_ffmpeg_player_OBJECTS = \
"CMakeFiles/simplest_ffmpeg_player.dir/src/simplest_ffmpeg_player.cpp.o"

# External object files for target simplest_ffmpeg_player
simplest_ffmpeg_player_EXTERNAL_OBJECTS =

simplest_ffmpeg_player: CMakeFiles/simplest_ffmpeg_player.dir/src/simplest_ffmpeg_player.cpp.o
simplest_ffmpeg_player: CMakeFiles/simplest_ffmpeg_player.dir/build.make
simplest_ffmpeg_player: CMakeFiles/simplest_ffmpeg_player.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/zzq/StreamingMedia/FFMPEG/simplest_ffmpeg_player/simplest_ffmpeg_player/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable simplest_ffmpeg_player"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/simplest_ffmpeg_player.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/simplest_ffmpeg_player.dir/build: simplest_ffmpeg_player

.PHONY : CMakeFiles/simplest_ffmpeg_player.dir/build

CMakeFiles/simplest_ffmpeg_player.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/simplest_ffmpeg_player.dir/cmake_clean.cmake
.PHONY : CMakeFiles/simplest_ffmpeg_player.dir/clean

CMakeFiles/simplest_ffmpeg_player.dir/depend:
	cd /home/zzq/StreamingMedia/FFMPEG/simplest_ffmpeg_player/simplest_ffmpeg_player/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zzq/StreamingMedia/FFMPEG/simplest_ffmpeg_player/simplest_ffmpeg_player /home/zzq/StreamingMedia/FFMPEG/simplest_ffmpeg_player/simplest_ffmpeg_player /home/zzq/StreamingMedia/FFMPEG/simplest_ffmpeg_player/simplest_ffmpeg_player/build /home/zzq/StreamingMedia/FFMPEG/simplest_ffmpeg_player/simplest_ffmpeg_player/build /home/zzq/StreamingMedia/FFMPEG/simplest_ffmpeg_player/simplest_ffmpeg_player/build/CMakeFiles/simplest_ffmpeg_player.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/simplest_ffmpeg_player.dir/depend

