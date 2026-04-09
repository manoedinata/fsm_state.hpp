/*
 * fsm_state.hpp
 * A helper class to track FSM states, reasons for transitions, and arbitrary variables for telemetry.
 * 
 * Author: Hendra Manudinata
 *
 * (C) 2026 IRIS Robotics, Institut Teknologi Sepuluh Nopember
*/

#ifndef FSM_STATE_HPP
#define FSM_STATE_HPP

#include <map>
#include <ros/ros.h>
#include <sstream>
#include <std_msgs/String.h>
#include <string>

class FSMState
{
private:
    ros::NodeHandle nh;
    ros::Publisher state_pub;
    std::string fsm_name;
    std::string current_state;
    std::string previous_state;
    std::string transition_reason;

    // Telemetry storage and throttling
    std::map<std::string, double> variables;
    ros::Time last_publish_time;
    double publish_rate_sec;

    // Internal publish function
    inline void publishData()
    {
        std::stringstream ss;
        ss << "{";
        ss << "\"previous_state\": \"" << previous_state << "\", ";
        ss << "\"current_state\": \"" << current_state << "\", ";
        ss << "\"reason\": \"" << transition_reason << "\", ";
        ss << "\"variables\": {";

        bool first = true;
        for (const auto &kv : variables)
        {
            if (!first)
                ss << ", ";
            ss << "\"" << kv.first << "\": " << kv.second;
            first = false;
        }
        ss << "}}";

        std_msgs::String msg;
        msg.data = ss.str();
        state_pub.publish(msg);

        last_publish_time = ros::Time::now();
    }

public:
    /**
     * @brief Construct a new FSMState tracker
     * @param name The name of the FSM (used for the ROS topic: /fsm/<name>)
     * @param publish_rate The telemetry throttle rate in seconds (default 10Hz)
     */
    inline FSMState(std::string name, double publish_rate = 0.1)
        : fsm_name(name),
          current_state("UNKNOWN"),
          previous_state("NONE"),
          transition_reason(""),
          publish_rate_sec(publish_rate)
    {
        state_pub = nh.advertise<std_msgs::String>("/states/" + fsm_name, 10);
        last_publish_time = ros::Time::now();
    }

    /**
     * @brief Track a variable's value for the Web UI telemetry
     */
    inline void setVariable(std::string name, double value)
    {
        variables[name] = value;
    }

    /**
     * @brief Set the reason for the NEXT state transition
     */
    inline void setReason(std::string reason)
    {
        transition_reason = reason;
    }

    /**
     * @brief Update the state. Automatically handles logging, publishing, and throttling.
     * Call this inside every case of your switch statement.
     */
    inline void updateState(std::string new_state)
    {
        bool state_changed = (new_state != current_state);

        if (state_changed)
        {
            previous_state = current_state;
            current_state = new_state;

            ROS_INFO("[%s] Transitioned: %s -> %s (Reason: %s)",
                     fsm_name.c_str(), previous_state.c_str(), current_state.c_str(), transition_reason.c_str());
        }

        // Publish if the state just changed, OR if enough time has passed for a telemetry update
        if (state_changed || (ros::Time::now() - last_publish_time).toSec() >= publish_rate_sec)
        {
            publishData();

            // CRITICAL: Wipe the reason clean AFTER publishing a state jump
            // so it doesn't accidentally bleed into the next transition.
            if (state_changed)
            {
                transition_reason = "";
            }
        }
    }
};

#endif // FSM_STATE_HPP

