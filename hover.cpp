#include <gz/msgs/altimeter.pb.h>
#include <gz/msgs/actuators.pb.h>
#include <gz/transport/Node.hh>
#include <libserial/SerialPort.h>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float64.hpp>
#include <iostream>
#include <string>
#include <thread>

// Global State
gz::transport::Node::Publisher motor_pub;
LibSerial::SerialPort esp32_serial;
double current_ros_target = 2.0; 
bool first_run = true;
double alt_offset = 0.0;

// ROS 2 Callback
void onRosTarget(const std_msgs::msg::Float64::SharedPtr msg) {
    current_ros_target = msg->data;
}

// Gazebo Callback:
void onAltitudeReceived(const gz::msgs::Altimeter &_msg) {
    if (first_run) {
        alt_offset = _msg.vertical_position();
        first_run = false;
        return;
    }

    
    double final_thrust = 0.0; 
    double current_alt = _msg.vertical_position() - alt_offset;
    double current_vel = _msg.vertical_velocity();

    std::string packet = std::to_string(current_alt) + ":" + 
                         std::to_string(current_vel) + ":" + 
                         std::to_string(current_ros_target) + "\n";

    try {
        esp32_serial.Write(packet);
        std::string response;
        esp32_serial.ReadLine(response);

        if (!response.empty() && std::isdigit(response[0])) {
    
            final_thrust = std::stod(response);

            gz::msgs::Actuators motor_msg;
            for (int i = 0; i < 4; ++i) motor_msg.add_velocity(final_thrust);
            motor_pub.Publish(motor_msg);
        }
    } catch (...) {
        // Serial error handling
    }

    std::printf("Alt: %6.2fm | Vel: %6.2fm/s | Target: %6.2fm | Thrust: %7.1f\n", 
                current_alt, current_vel, current_ros_target, final_thrust);
    std::fflush(stdout); 
}

int main(int argc, char **argv) {
    // A. Initialize ROS 2
    rclcpp::init(argc, argv);
    auto ros_node = rclcpp::Node::make_shared("drone_hil_bridge");
    auto target_sub = ros_node->create_subscription<std_msgs::msg::Float64>(
        "/drone/target_alt", 10, onRosTarget);

    // B. Initialize Serial
    try {
        esp32_serial.Open("/dev/ttyUSB0");
        esp32_serial.SetBaudRate(LibSerial::BaudRate::BAUD_921600);
    } catch (...) { return -1; }

    // C. Initialize Gazebo
    gz::transport::Node gz_node;
    gz_node.Subscribe("/model/x500/altimeter", onAltitudeReceived);
    motor_pub = gz_node.Advertise<gz::msgs::Actuators>("/x500/command/motor_speed");

    // D. Multi-threading: Spin ROS 2 in background, Gazebo in foreground
    std::thread ros_thread([&]() { rclcpp::spin(ros_node); });

    std::cout << "HIL + ROS 2 Node Running..." << std::endl;
    gz::transport::waitForShutdown();

    rclcpp::shutdown();
    ros_thread.join();
    return 0;
}
