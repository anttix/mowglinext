#!/bin/bash
# =============================================================================
# ros2_entrypoint.sh
#
# Container entrypoint. Sources ROS2 Lyrical, the pinned source prefixes,
# and the workspace overlay,
# then exec's whatever was passed as CMD.
# =============================================================================
set -e

# Source the ROS, ortools_vendor, Nav2, and grid_map underlays.
# shellcheck source=/opt/mowgli_underlay.sh
source /opt/mowgli_underlay.sh

# Present only in the amd64 simulation image.
if [ -f /opt/webots_ros2/setup.bash ]; then
    # shellcheck source=/opt/webots_ros2/setup.bash
    source /opt/webots_ros2/setup.bash
fi

# Source the ublox interface overlay (ublox_ubx_msgs + ublox_ubx_interfaces),
# built from the cedbossneo/ublox_dgnss fork into /opt/ublox_msgs.
if [ -f /opt/ublox_msgs/setup.bash ]; then
    # shellcheck source=/opt/ublox_msgs/setup.bash
    source /opt/ublox_msgs/setup.bash
fi

# Source the workspace overlay if it has been built.
if [ -f /ros2_ws/install/setup.bash ]; then
    # shellcheck source=/ros2_ws/install/setup.bash
    source /ros2_ws/install/setup.bash
fi

exec "$@"
