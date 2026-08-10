#!/usr/bin/env bash
set -euo pipefail

ROS_DISTRO="${ROS_DISTRO:-lyrical}"
NAV2_REF="${NAV2_REF:-0e69ba8a30259b827de5169817f6991d46374465}"
NAV2_PREFIX="${NAV2_PREFIX:-/opt/nav2}"
NAV2_JOBS="${NAV2_JOBS:-$(nproc)}"
NAV2_REPOSITORY="${NAV2_REPOSITORY:-https://github.com/ros-navigation/navigation2.git}"
NAV2_DEPS_ONLY="${NAV2_DEPS_ONLY:-0}"

ros_setup="/opt/ros/${ROS_DISTRO}/setup.bash"
for requirement in git rosdep colcon; do
  command -v "${requirement}" >/dev/null || {
    echo "build_nav2.sh: missing required command: ${requirement}" >&2
    exit 1
  }
done
if [[ ! -f "${ros_setup}" ]]; then
  echo "build_nav2.sh: ROS setup not found: ${ros_setup}" >&2
  exit 1
fi

apt_get=(apt-get)
if (( EUID != 0 )); then
  command -v sudo >/dev/null || {
    echo "build_nav2.sh: sudo is required to install dependencies as a non-root user" >&2
    exit 1
  }
  apt_get=(sudo apt-get)
fi

workspace="$(mktemp -d)"
cleanup() {
  rm -rf "${workspace}"
}
trap cleanup EXIT

git clone --filter=blob:none "${NAV2_REPOSITORY}" "${workspace}/src/navigation2"
git -C "${workspace}/src/navigation2" checkout "${NAV2_REF}"

if [[ ! -f /etc/ros/rosdep/sources.list.d/20-default.list ]]; then
  if (( EUID == 0 )); then
    rosdep init
  else
    sudo rosdep init
  fi
fi
rosdep update --rosdistro "${ROS_DISTRO}"
"${apt_get[@]}" update
rosdep install \
  --from-paths "${workspace}/src" \
  --ignore-src \
  --rosdistro "${ROS_DISTRO}" \
  --dependency-types build \
  --dependency-types build_export \
  --dependency-types buildtool \
  --dependency-types buildtool_export \
  --dependency-types exec \
  --skip-keys \
    "diff_drive_controller gazebo_ros_pkgs joint_state_broadcaster nav2_minimal_tb3_sim nav2_minimal_tb4_description nav2_minimal_tb4_sim nav2_system_tests ros_gz ros_gz_bridge ros_gz_sim rviz2 slam_toolbox turtlebot3_gazebo" \
  -y
"${apt_get[@]}" install -y --no-install-recommends libboost-serialization-dev
if (( EUID == 0 )); then
  rm -rf /var/lib/apt/lists/*
fi

if [[ "${NAV2_DEPS_ONLY}" == "1" ]]; then
  exit 0
fi

set +u
# shellcheck source=/dev/null
source "${ros_setup}"
set -u

cd "${workspace}"
colcon build \
  --merge-install \
  --install-base "${NAV2_PREFIX}" \
  --parallel-workers "${NAV2_JOBS}" \
  --packages-skip \
    nav2_minimal_tb3_sim \
    nav2_minimal_tb4_description \
    nav2_minimal_tb4_sim \
    nav2_system_tests \
  --cmake-args \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF

test -f "${NAV2_PREFIX}/setup.bash"
