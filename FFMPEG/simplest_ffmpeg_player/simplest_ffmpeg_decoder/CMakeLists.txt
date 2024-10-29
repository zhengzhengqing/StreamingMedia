# 设置CMake的最低版本要求  
cmake_minimum_required(VERSION 3.10)  
  
# 设置项目名称和版本号  
project(simplest_ffmpeg_decoder VERSION 1.0)  
  
# 指定C++标准（例如C++11）  
set(CMAKE_CXX_STANDARD 14)  
set(CMAKE_CXX_STANDARD_REQUIRED True)  

#添加include目录
#include_directories(${CMAKE_SOURCE_DIR}/include)

# 指定库目录（相对于 CMakeLists.txt 的路径）  
#link_directories(${CMAKE_SOURCE_DIR}/lib)  
  
# 添加一个可执行文件，并链接之前定义的库  
add_executable(simplest_ffmpeg_decoder 
    src/simplest_ffmpeg_decoder.cpp
)  

# 添加一个可执行文件，并链接之前定义的库  
add_executable(ffmpeg_decoder
    src/ffmpeg_decoder.cc
)

target_link_libraries(simplest_ffmpeg_decoder 
    avcodec
    avdevice
    avfilter
    avformat
    avutil
    swresample
    swscale
    postproc
)

target_link_libraries(ffmpeg_decoder 
    avcodec
    avdevice
    avfilter
    avformat
    avutil
    swresample
    swscale
    postproc
)