#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.1.0)

project(RGBDT)

# Save the command line compile commands in the build output
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Make project require C++11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pthread")
endif()

# Simple non robust way to find the librealsense library
if(WIN32)
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
        set(LIBRARY_DIR "C:\\Program Files (x86)\\Intel RealSense SDK 2.0\\lib\\x64")          # TODO: Update this variable to correct path - folder where realsense2.lib is found
        set(DLL_DIR "C:\\Program Files (x86)\\Intel RealSense SDK 2.0\\bin\\x64")              # TODO: Update this variable to correct path - folder where realsense2.dll is found
    else()
        set(LIBRARY_DIR "C:\\Program Files (x86)\\Intel RealSense SDK 2.0\\lib\\x86")          # TODO: Update this variable to correct path - folder where realsense2.lib is found
        set(DLL_DIR "C:\\Program Files (x86)\\Intel RealSense SDK 2.0\\bin\\x86")              # TODO: Update this variable to correct path - folder where realsense2.dll is found
    endif()
    set(PROJECT_BINARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/\$\(Configuration\)")                # TODO: Update this variable to correct path - folder to which your project will compile
    set(ADDITIONAL_INCLUDE_DIRS "C:\\Program Files (x86)\\Intel RealSense SDK 2.0\\include")   # TODO: Update this variable to correct path - folder where librealsense2 folder is found
else()
    set(ADDITIONAL_INCLUDE_DIRS "/usr/local/include")
    set(ADDITIONAL_LIBRARY_DIRS "/usr/local/lib")
endif()

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /usr/lib64)

find_library(REALSENSE2_FOUND realsense2 HINTS ${LIBRARY_DIR} REQUIRED)
if(NOT REALSENSE2_FOUND)
    SET(REALSENSE2_FOUND "realsense2")
    message(FATAL_ERROR "Failed to find_library(realsense2)")
endif()

find_library(LIBUVC_FOUND uvc HINTS ${LIBRARY_DIR} REQUIRED)
if(NOT LIBUVC_FOUND)
    SET(LIBUVC_FOUND "libuvc")
    message(FATAL_ERROR "Failed to find_library(libuvc)")
endif()

#find_library(OPENCV_FOUND opencv_core opencv_imgproc opencv_highgui opencv_ml opencv_video opencv_features2d opencv_calib3d opencv_objdetect opencv_contrib opencv_legacy opencv_flann opencv_imgcodecs PATHS ${CMAKE_LIBRARY_PATH} REQUIRED )
#if(NOT OPENCV_FOUND)
#    SET(OPENCV_FOUND "opencv")
#    message(FATAL_ERROR "Failed to find_library(opencv)")
#endif()

#find_library(BOOST_FOUND boost_system boost_filesystem HINTS ${LIBRARY_DIR} REQUIRED)
#if(NOT BOOST_FOUND)
#    SET(BOOST_FOUND "boost")
#    message(FATAL_ERROR "Failed to find_library(boost)")
#endif()
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem program_options)


# add_executable(capture-rgbt capture-rgbt.cpp pt_pipeline.hpp safe_queue.hpp pt_pipeline.hpp safe_queue.hpp utils/common.hpp utils/calibration.hpp detection.hpp)
# include_directories(capture-rgbt ${ADDITIONAL_INCLUDE_DIRS})
# target_link_libraries(capture-rgbt PRIVATE ${LIBUVC_FOUND} ${REALSENSE2_FOUND} ${Boost_LIBRARIES} ${OpenCV_LIBS})
# set_target_properties (capture-rgbt PROPERTIES FOLDER Examples)
add_subdirectory(src)

# add_executable(capture-rgbt-rls capture-rgbt-rls.cpp pt_pipeline.hpp safe_queue.hpp pt_pipeline.hpp safe_queue.hpp utils.hpp detection.hpp)
# include_directories(capture-rgbt-rls ${ADDITIONAL_INCLUDE_DIRS})
# target_link_libraries(capture-rgbt-rls PRIVATE ${LIBUVC_FOUND} ${REALSENSE2_FOUND} ${Boost_LIBRARIES} ${OpenCV_LIBS})

# add_executable(viewer-rgbt viewer-rgbt.cpp utils.hpp)
# include_directories(viewer-rgbt ${ADDITIONAL_INCLUDE_DIRS})
# target_link_libraries(viewer-rgbt PRIVATE ${Boost_LIBRARIES} ${OpenCV_LIBS})

# add_executable(calib-intrinsics calib-intrinsics.cpp utils.hpp)
# include_directories(calib-intrinsics ${ADDITIONAL_INCLUDE_DIRS})
# target_link_libraries(calib-intrinsics PRIVATE ${Boost_LIBRARIES} ${OpenCV_LIBS})

# add_executable(calib-extrinsics calib-extrinsics.cpp utils.hpp)
# include_directories(calib-extrinsics ${ADDITIONAL_INCLUDE_DIRS})
# target_link_libraries(calib-extrinsics PRIVATE ${Boost_LIBRARIES} ${OpenCV_LIBS})

# add_executable(test_chessboard_detection test_chessboard_detection.cpp utils.hpp)
# include_directories(test_chessboard_detection ${ADDITIONAL_INCLUDE_DIRS})
# target_link_libraries(test_chessboard_detection PRIVATE ${Boost_LIBRARIES} ${OpenCV_LIBS})

# add_executable(calib-corners calib-corners.cpp utils.hpp)
# include_directories(calib-corners ${ADDITIONAL_INCLUDE_DIRS})
# target_link_libraries(calib-corners PRIVATE ${Boost_LIBRARIES} ${OpenCV_LIBS})

# Post Build script to copy realsense2.dll
# if(WIN32)
# message(STATUS "Adding Post build script to copy realsense2.dll to project's binary folder")
# message(STATUS "Will try to copy from ${DLL_DIR} to ${PROJECT_BINARY_OUTPUT_PATH}")
# add_custom_command(TARGET capture-rgbt POST_BUILD        # Adds a post-build event to capture-rgbt
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different              # which executes "cmake - E copy_if_different..."
#         "${DLL_DIR}/realsense2.dll"                            # <--this is in-file
#         ${PROJECT_BINARY_OUTPUT_PATH})                 # <--this is out-file path
# endif()

# install(
#     TARGETS

#     RUNTIME DESTINATION
#     ${CMAKE_INSTALL_PREFIX}/bin
# )
