cmake_minimum_required(VERSION 3.10)
project(drone_hover)

# 1. Find ROS 2 Packages
# These must be found before we can use the 'ament' macros
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# 2. Find Gazebo Harmonic Libraries
find_package(gz-transport13 REQUIRED)
find_package(gz-msgs10 REQUIRED)

# 3. Find LibSerial (using PkgConfig)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSERIAL REQUIRED libserial)

# 4. Define the executable (using your original names)
add_executable(hover_bin hover.cpp)

# 5. Include headers
# We need the LibSerial include paths and the Gazebo paths
target_include_directories(hover_bin PRIVATE 
    ${LIBSERIAL_INCLUDE_DIRS}
)

# 6. THE FIX: Link ROS 2 Dependencies using Ament
# This macro automatically handles the 'Type Support' handles that were missing
ament_target_dependencies(hover_bin
    rclcpp
    std_msgs
)

# 7. Link Gazebo and LibSerial
# These are standard libraries, so we use target_link_libraries
target_link_libraries(hover_bin
    gz-transport13::core
    gz-msgs10::gz-msgs10
    ${LIBSERIAL_LIBRARIES}
)

# 8. Set the C++ standard (Required for ROS 2 and Gazebo)
set_target_properties(hover_bin PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
