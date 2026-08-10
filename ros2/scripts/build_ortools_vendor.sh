#!/usr/bin/env bash
set -euo pipefail

ROS_DISTRO="${ROS_DISTRO:-lyrical}"
ORTOOLS_VENDOR_REF="${ORTOOLS_VENDOR_REF:-1ec2e6d4dfe39711b7acc3aba57f4f54610aa063}"
ORTOOLS_VENDOR_PREFIX="${ORTOOLS_VENDOR_PREFIX:-/opt/ortools_vendor}"
ORTOOLS_VENDOR_JOBS="${ORTOOLS_VENDOR_JOBS:-$(nproc)}"
ORTOOLS_VENDOR_REPOSITORY="${ORTOOLS_VENDOR_REPOSITORY:-https://github.com/Fields2Cover/ortools_vendor.git}"

ros_setup="/opt/ros/${ROS_DISTRO}/setup.bash"
for requirement in git colcon cmake; do
  command -v "${requirement}" >/dev/null || {
    echo "build_ortools_vendor.sh: missing required command: ${requirement}" >&2
    exit 1
  }
done
if [[ ! -f "${ros_setup}" ]]; then
  echo "build_ortools_vendor.sh: ROS setup not found: ${ros_setup}" >&2
  exit 1
fi

workspace="$(mktemp -d)"
cleanup() {
  rm -rf "${workspace}"
}
trap cleanup EXIT

git clone --filter=blob:none "${ORTOOLS_VENDOR_REPOSITORY}" \
  "${workspace}/src/ortools_vendor"
git -C "${workspace}/src/ortools_vendor" checkout "${ORTOOLS_VENDOR_REF}"

cmake_file="${workspace}/src/ortools_vendor/CMakeLists.txt"
if ! grep -q 'USE_SCIP=OFF' "${cmake_file}"; then
  sed -i '/^[[:space:]]*CMAKE_ARGS[[:space:]]*$/a\\    -DUSE_SCIP:BOOL=OFF' \
    "${cmake_file}"
fi
if ! grep -q 'CMAKE_POLICY_VERSION_MINIMUM' "${cmake_file}"; then
  sed -i '/^[[:space:]]*CMAKE_ARGS[[:space:]]*$/a\\    -DCMAKE_POLICY_VERSION_MINIMUM:STRING=3.5' \
    "${cmake_file}"
fi

# setup.bash is generated code and may inspect unset variables.
set +u
# shellcheck source=/dev/null
source "${ros_setup}"
set -u

cd "${workspace}"
colcon build \
  --merge-install \
  --install-base "${ORTOOLS_VENDOR_PREFIX}" \
  --packages-select ortools_vendor \
  --parallel-workers "${ORTOOLS_VENDOR_JOBS}" \
  --cmake-args \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF

test -f "${ORTOOLS_VENDOR_PREFIX}/setup.bash"
