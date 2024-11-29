cmake_minimum_required(VERSION 3.14)

# Set the project name
project(EPITA_CODEFEST_WEEK)

# Set CMake to export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Find required packages
find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core highgui imgproc calib3d imgcodecs)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# OS-specific configurations
if(APPLE)
    # macOS specific settings
    message(STATUS "Configuring for macOS")

    # Manually specify the location of libfreenect
    set(FREENECT_INCLUDE_DIR /opt/homebrew/opt/libfreenect/include)
    set(FREENECT_LIB_DIR /opt/homebrew/opt/libfreenect/lib)

    # Add the include directory for libfreenect
    include_directories(${FREENECT_INCLUDE_DIR})

    # Link the freenect library manually
    link_directories(${FREENECT_LIB_DIR})
    set(FREENECT_LIB ${FREENECT_LIB_DIR}/libfreenect.dylib)
else()
    # Linux specific settings
    message(STATUS "Configuring for Linux")
    find_package(libfreenect REQUIRED)
    set(FREENECT_LIB libfreenect::libfreenect)
endif()

# Add the executable for glview
add_executable(glview src/glview.c)
target_link_libraries(glview PRIVATE ${FREENECT_LIB} GLUT::GLUT OpenGL::GL)

# Add the executable for test-cv
add_executable(test-cv src/test-cv.cpp)

# Add the library opencv_kinect
add_library(opencv_kinect src/capture-cv.hpp src/capture-cv.cpp src/utils.cpp src/calibration-utils.hpp src/calibration-utils.cpp)
target_link_libraries(opencv_kinect PRIVATE opencv_imgproc opencv_calib3d ${FREENECT_LIB} Qt6::Core)
target_link_libraries(opencv_kinect PUBLIC opencv_core opencv_imgcodecs)

# Link libraries for test-cv
target_link_libraries(test-cv PRIVATE opencv_highgui opencv_kinect)

# Add the executable for calibration
add_executable(calibration src/calibrate-qt.cpp src/calibrate-qt-main.cpp)
target_link_libraries(calibration PRIVATE Qt6::Gui Qt6::Widgets opencv_kinect)
