cmake_minimum_required(VERSION 3.10)

project(practical-ffmpeg-examples)

set(CMAKE_CXX_STANDARD 17)
set(FFMPEG_PATH "./ffmpeg")

find_library(AVCODEC_LIBRARY NAMES avcodec HINTS "${FFMPEG_PATH}/lib")
find_library(AVFORMAT_LIBRARY NAMES avformat HINTS "${FFMPEG_PATH}/lib")
find_library(AVUTIL_LIBRARY NAMES avutil HINTS "${FFMPEG_PATH}/lib")
find_library(AVDEVICE_LIBRARY NAMES avdevice HINTS "${FFMPEG_PATH}/lib")

add_executable(video2jpgs video2jpgs.cpp)
target_link_libraries(video2jpgs ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${AVDEVICE_LIBRARY})
target_include_directories(video2jpgs PUBLIC "${FFMPEG_PATH}/include")

file(GLOB FFPMEG_DLLS "${FFMPEG_PATH}/bin/*.dll")
foreach(FFPMEG_DLL IN LISTS FFPMEG_DLLS)
    add_custom_command(
                TARGET video2jpgs POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                ${FFPMEG_DLL}
                ${CMAKE_SOURCE_DIR}/build/)
endforeach()