# fsm_state.hpp

A helper class to track FSM states, reasons for transitions, and arbitrary variables for telemetry.
It automatically handles ROS topic publishing and throttling for the Web UI.
The class will publish a JSON string to /states/<name> with the current state, previous state, reason, and variables.

Note: This is designed for use in a ROS environment and assumes you have set up ROS properly.

## Usage:
1. Create an instance of FSMState in your FSM class (e.g., FSMState fsmState("fsmState");)
2. Call fsmState.updateState("state_name") inside every case of your switch statement.
3. Optionally call fsmState.setReason("reason_for_transition") before the state change to log the reason.
4. Optionally call fsmState.setVariable("var_name", value) to include custom variables in the telemetry.
