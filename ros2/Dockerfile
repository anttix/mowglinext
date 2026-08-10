# syntax=docker/dockerfile:1.7
# =============================================================================
# Stage 0: gtsam-builder
# Builds GTSAM 4.3a1 from source. The Ubuntu Noble apt package
# (libgtsam-dev 4.2) ships a broken GTSAMConfig.cmake (references files
# that aren't installed) AND uses the legacy boost::optional<Matrix&>
# API for custom factors. There is no Lyrical GTSAM binary.
# Building from source gives us a clean install + the modern
# OptionalMatrixType API + a single pinned version we control.
#
# Install lands in /opt/gtsam; runtime stages COPY it across, set up
# ldconfig, and export CMAKE_PREFIX_PATH so find_package(GTSAM) works.
# =============================================================================
ARG ROS_BASE_IMAGE=ros:lyrical-ros-base@sha256:122a787d0eb7e1b2c18370e35ef394332db5c11d242bea6f05f2b2911e26b04d
ARG NAV2_COMMIT=0e69ba8a30259b827de5169817f6991d46374465
ARG ORTOOLS_VENDOR_COMMIT=1ec2e6d4dfe39711b7acc3aba57f4f54610aa063
ARG GRID_MAP_COMMIT=7ae24725d7effa0c22a23524b91b46bada5b2728
ARG WEBOTS_ROS2_COMMIT=a5a9c3f82b0056809e89c976b9e9f45137c0fd57

# =============================================================================
# Stage 0: nav2-base
# Build the pinned Nav2 source in the same base lineage used by grid_map and
# the final image. This preserves apt ownership of its dependency closure.
#
# Removal path after the packages reach the stable repository: delete this stage
# and install:
#   ros-lyrical-nav2-bringup
#   ros-lyrical-nav2-bt-navigator
#   ros-lyrical-nav2-controller
#   ros-lyrical-nav2-planner
#   ros-lyrical-nav2-behaviors
#   ros-lyrical-nav2-regulated-pure-pursuit-controller
#   ros-lyrical-nav2-smac-planner
#   ros-lyrical-nav2-costmap-2d
#   ros-lyrical-nav2-map-server
#   ros-lyrical-nav2-lifecycle-manager
#   ros-lyrical-nav2-waypoint-follower
#   ros-lyrical-nav2-velocity-smoother
#   ros-lyrical-nav2-rotation-shim-controller
#   ros-lyrical-nav2-core
#   ros-lyrical-nav2-util
#   ros-lyrical-nav2-msgs
#   ros-lyrical-nav2-ros-common
#   ros-lyrical-opennav-docking
# =============================================================================
FROM ${ROS_BASE_IMAGE} AS nav2-base

ARG NAV2_COMMIT
ARG DEBIAN_FRONTEND=noninteractive

SHELL ["/bin/bash", "-c"]

# Match the temporary repository used by the official image for dependencies
# that have not reached the stable sync yet.
RUN sed -i 's|packages.ros.org/ros2/|packages.ros.org/ros2-testing/|g' \
      /etc/apt/sources.list.d/*

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get install -y --no-install-recommends \
      git \
      ros-dev-tools

COPY ros2/scripts/build_nav2.sh /usr/local/bin/build_nav2
RUN NAV2_REF="${NAV2_COMMIT}" build_nav2 && \
    sed -i 's|packages.ros.org/ros2-testing/|packages.ros.org/ros2/|g' \
      /etc/apt/sources.list.d/*

# =============================================================================
# Stage 0a: ortools-vendor-builder
# Lyrical has no ortools_vendor binary. Build the Fields2Cover vendor package
# once and reuse the pinned prefix in both Fields2Cover builders and runtime.
# Removal package: ros-lyrical-ortools-vendor
# =============================================================================
FROM ${ROS_BASE_IMAGE} AS ortools-vendor-builder

ARG ORTOOLS_VENDOR_COMMIT
ARG DEBIAN_FRONTEND=noninteractive

SHELL ["/bin/bash", "-c"]

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get install -y --no-install-recommends \
      git \
      ros-lyrical-ament-cmake-vendor-package \
      ros-dev-tools

COPY ros2/scripts/build_ortools_vendor.sh /usr/local/bin/build_ortools_vendor
RUN ORTOOLS_VENDOR_REF="${ORTOOLS_VENDOR_COMMIT}" build_ortools_vendor

# =============================================================================
# Stage 0b: grid-map-builder
# grid_map is absent from Lyrical. Build only the closure used by mowgli_map,
# using the published Nav2 message package.
# Removal packages:
#   ros-lyrical-grid-map-core
#   ros-lyrical-grid-map-msgs
#   ros-lyrical-grid-map-cv
#   ros-lyrical-grid-map-ros
# =============================================================================
FROM nav2-base AS grid-map-builder

ARG GRID_MAP_COMMIT
ARG DEBIAN_FRONTEND=noninteractive

SHELL ["/bin/bash", "-c"]

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get install -y --no-install-recommends \
      git \
      libboost-all-dev \
      libopencv-dev \
      ros-dev-tools

WORKDIR /ws/src
RUN git clone --filter=blob:none https://github.com/ANYbotics/grid_map.git grid_map && \
    git -C grid_map checkout "${GRID_MAP_COMMIT}"

COPY ros2/patches/grid_map-lyrical.patch /tmp/grid-map-lyrical.patch
RUN git -C grid_map apply --check /tmp/grid-map-lyrical.patch && \
    git -C grid_map apply /tmp/grid-map-lyrical.patch && \
    grep -q "rosbag2_cpp::rosbag2_cpp" grid_map/grid_map_ros/CMakeLists.txt

WORKDIR /ws
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && \
    source /opt/ros/lyrical/setup.bash && \
    source /opt/nav2/setup.bash && \
    rosdep install \
      --from-paths \
        src/grid_map/grid_map_cmake_helpers \
        src/grid_map/grid_map_msgs \
        src/grid_map/grid_map_core \
        src/grid_map/grid_map_cv \
        src/grid_map/grid_map_ros \
      --ignore-src \
      --rosdistro lyrical \
      --dependency-types build \
      --dependency-types build_export \
      --dependency-types buildtool \
      --dependency-types exec \
      --skip-keys nav2_msgs \
      -y && \
    rm -rf /var/lib/apt/lists/* && \
    colcon build \
      --merge-install \
      --install-base /opt/grid_map \
      --packages-select \
        grid_map_cmake_helpers \
        grid_map_msgs \
        grid_map_core \
        grid_map_cv \
        grid_map_ros \
      --cmake-args \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTING=OFF \
        -DCMAKE_CXX_FLAGS=-Wno-deprecated-declarations

# =============================================================================
# Stage 0c: webots-ros2-builder
# Simulation-only source build. The pinned Webots ROS release predates Lyrical,
# so teach its distro/Python switch about Ubuntu 26.04's Python 3.14.
# Removal packages:
#   ros-lyrical-webots-ros2-driver
#   ros-lyrical-webots-ros2-control
# =============================================================================
FROM ${ROS_BASE_IMAGE} AS webots-ros2-builder

ARG WEBOTS_ROS2_COMMIT
ARG DEBIAN_FRONTEND=noninteractive

SHELL ["/bin/bash", "-c"]

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get install -y --no-install-recommends \
      git \
      ros-dev-tools

WORKDIR /ws/src
RUN git clone --filter=blob:none https://github.com/cyberbotics/webots_ros2.git \
      webots_ros2 && \
    git -C webots_ros2 checkout "${WEBOTS_ROS2_COMMIT}" && \
    git -C webots_ros2 submodule update --init --recursive

COPY ros2/patches/webots_ros2-lyrical.patch /tmp/webots-ros2-lyrical.patch
RUN git -C webots_ros2 apply --check /tmp/webots-ros2-lyrical.patch && \
    git -C webots_ros2 apply /tmp/webots-ros2-lyrical.patch && \
    grep -q 'ROS_DISTRO} MATCHES "lyrical"' \
      webots_ros2/webots_ros2_driver/CMakeLists.txt && \
    grep -q 'Python 3.14 EXACT REQUIRED' \
      webots_ros2/webots_ros2_driver/CMakeLists.txt

WORKDIR /ws
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && \
    source /opt/ros/lyrical/setup.bash && \
    rosdep install \
      --from-paths \
        src/webots_ros2/webots_ros2_msgs \
        src/webots_ros2/webots_ros2_importer \
        src/webots_ros2/webots_ros2_driver \
        src/webots_ros2/webots_ros2_control \
      --ignore-src \
      --rosdistro lyrical \
      --dependency-types build \
      --dependency-types build_export \
      --dependency-types buildtool \
      --dependency-types exec \
      -y && \
    rm -rf /var/lib/apt/lists/* && \
    colcon build \
      --merge-install \
      --install-base /opt/webots_ros2 \
      --packages-up-to webots_ros2_driver webots_ros2_control \
      --cmake-args \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTING=OFF

FROM ${ROS_BASE_IMAGE} AS gtsam-builder

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake git \
      libboost-all-dev libeigen3-dev libmetis-dev \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp
RUN git clone --depth 1 -b 4.3a1 https://github.com/borglab/gtsam.git

RUN cd gtsam && mkdir build && cd build && \
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/gtsam \
        -DGTSAM_BUILD_TESTS=OFF \
        -DGTSAM_BUILD_EXAMPLES_ALWAYS=OFF \
        -DGTSAM_BUILD_PYTHON=OFF \
        -DGTSAM_BUILD_UNSTABLE=OFF \
        -DGTSAM_USE_SYSTEM_EIGEN=ON \
        -DGTSAM_USE_SYSTEM_METIS=ON \
        -DGTSAM_WITH_TBB=OFF \
        -DGTSAM_WITH_EIGEN_MKL=OFF \
        -DGTSAM_INSTALL_MATLAB_TOOLBOX=OFF \
        -DBUILD_SHARED_LIBS=ON \
 && make -j$(nproc) install

# =============================================================================
# Stage 0b: fields2cover-v3-builder
# Builds Fields2Cover v3 into /opt/fields2cover-300. v3 is UNRELEASED (no tag/
# release); it lives on the moving `v3.0` branch, so we PIN a commit SHA. The win
# is native headland routing (rp::RoutePlannerBase::genRoute, graph via OR-Tools)
# to fix the headland-perimeter freeze. OR-Tools comes from
# the source-built ortools_vendor prefix;
# nlohmann-json3-dev is an added v3 dependency. mowgli_coverage finds 3.0.0 at
# /opt/fields2cover-300 and adapts the generateHeadlandSwaths API.
#
# ros-lyrical-fields2cover is the future package name, but currently provides
# F2C 2 and is not a drop-in replacement for this pinned F2C 3 build.
# =============================================================================
FROM ${ROS_BASE_IMAGE} AS fields2cover-v3-builder

ARG DEBIAN_FRONTEND=noninteractive

COPY --from=ortools-vendor-builder /opt/ortools_vendor /opt/ortools_vendor

RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake git pkg-config \
      libeigen3-dev libgdal-dev libgeos++-dev libgeos-dev libgsl-dev \
      libtinyxml2-dev libpython3-dev python3-dev libtbb-dev nlohmann-json3-dev \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp
# Pinned to v3.0 @ 884d895 (2026-03-02) — do NOT track the moving branch head.
# Patch an upstream bug in this commit: visualizer.cpp uses std::setw/setfill
# without #include <iomanip> (modern GCC/libstdc++ no longer pulls it in
# transitively), which fails the build with "'setw' is not a member of 'std'".
RUN git clone https://github.com/Fields2Cover/Fields2Cover.git f2c3 && \
    cd f2c3 && git checkout 884d895b59192882476e986ba44ea9143a06a6a9 && \
    sed -i '1i #include <iomanip>' src/fields2cover/utils/visualizer.cpp && \
    sed -i '1i #include <algorithm>' src/fields2cover/types/Graph.cpp

RUN . /opt/ros/lyrical/setup.sh && \
    . /opt/ortools_vendor/setup.sh && \
    cd f2c3 && mkdir build && cd build && \
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/fields2cover-300 \
        -DBUILD_TESTS=OFF \
        -DBUILD_PYTHON=OFF \
        -DBUILD_TUTORIALS=OFF \
        -DBUILD_DOC=OFF \
        -DALLOW_PARALLELIZATION=ON \
        -DBUILD_SHARED_LIBS=ON \
 && make -j$(nproc) install

# =============================================================================
# Stage 0c: ublox-msgs-builder
# Builds ONLY the ublox_ubx_msgs + ublox_ubx_interfaces interface packages
# from the cedbossneo/ublox_dgnss fork. The GPS driver runs in the separate
# sensors/gps image and publishes ~25 /ubx_* topics typed with
# ublox_ubx_msgs/* and ublox_dgnss services typed with ublox_ubx_interfaces/*.
# foxglove_bridge in THIS image introspects every topic/service on the graph
# and was logging one "package not found" error per topic on every reconnect
# because the schemas weren't installed here. We only need the .msg/.srv
# typesupport for schema resolution, NOT the driver node — so build just the
# two interface packages with --packages-up-to (pulls in nothing else; both
# depend only on builtin_interfaces/std_msgs/rosidl, present in the base).
#
# The pinned ref MUST stay identical to sensors/gps/Dockerfile's
# UBLOX_DGNSS_REF/SHA — a schema mismatch between what the driver publishes
# and what foxglove loads here would be worse than the current log spam.
# Bump both Dockerfiles together when the fork moves.
# =============================================================================
FROM ${ROS_BASE_IMAGE} AS ublox-msgs-builder

ARG DEBIAN_FRONTEND=noninteractive
ARG UBLOX_DGNSS_REF=fix/hp-fix-stale-status-cov
ARG UBLOX_DGNSS_SHA=5e1d0cf1e8ec0e2638c44a59f57d15f5dd04c9af
ARG UBLOX_DGNSS_URL=https://github.com/cedbossneo/ublox_dgnss.git

RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake git \
      python3-colcon-common-extensions \
      ros-lyrical-rosidl-default-generators \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /ws/src
RUN git clone --branch ${UBLOX_DGNSS_REF} ${UBLOX_DGNSS_URL} ublox_dgnss \
 && cd ublox_dgnss && git checkout ${UBLOX_DGNSS_SHA}

WORKDIR /ws
RUN . /opt/ros/lyrical/setup.sh \
 && colcon build --merge-install \
      --packages-up-to ublox_ubx_msgs ublox_ubx_interfaces \
      --cmake-args -DCMAKE_BUILD_TYPE=Release -Wno-dev \
 && rm -rf /ws/build /ws/log /ws/src

# =============================================================================
# Stage 1: base
# Installs all runtime ROS2 dependencies. Both runtime and simulation stages
# inherit from here, keeping the image lean.
# =============================================================================
FROM nav2-base AS base

ARG DEBIAN_FRONTEND=noninteractive

SHELL ["/bin/bash", "-c"]

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get install -y --no-install-recommends \
    # Motion control
    ros-lyrical-twist-mux \
    # ros2_control (Webots simulation drivetrain)
    ros-lyrical-ros2-control \
    ros-lyrical-ros2-controllers \
    ros-lyrical-controller-manager \
    ros-lyrical-diff-drive-controller \
    ros-lyrical-joint-state-broadcaster \
    ros-lyrical-pid-controller \
    # Robot description / TF
    ros-lyrical-xacro \
    ros-lyrical-robot-state-publisher \
    ros-lyrical-joint-state-publisher \
    ros-lyrical-tf2-ros \
    ros-lyrical-tf2-eigen \
    ros-lyrical-tf2-geometry-msgs \
    # Diagnostics
    ros-lyrical-diagnostic-msgs \
    ros-lyrical-diagnostic-updater \
    # Foxglove Bridge (WebSocket bridge for GUI and Foxglove Studio)
    ros-lyrical-foxglove-bridge \
    # RTCM message definitions for the public /rtcm topic
    ros-lyrical-rtcm-msgs \
    # tinyxml2 (required at runtime by BehaviorTree.CPP)
    libtinyxml2-dev \
    # Topic tools (relay, throttle — used in simulation for topic remapping)
    ros-lyrical-topic-tools \
    # DDS middleware — Cyclone DDS is more reliable than FastRTPS on ARM
    # (no shared memory, no stale /dev/shm segments, better service discovery)
    ros-lyrical-rmw-cyclonedds-cpp \
    # GTSAM runtime deps — libgtsam itself is COPYed from the
    # gtsam-builder stage below. libmetis-dev provides both the
    # runtime libmetis5 and the unversioned libmetis.so symlink
    # that GTSAM's exported CMake target references at link time. GTSAM's
    # exported config also requires Boost.Serialization while configuring
    # downstream packages, so keep that component's development config.
    libmetis-dev \
    libboost-serialization-dev libboost-filesystem-dev libboost-thread-dev \
    libboost-program-options-dev libboost-date-time-dev libboost-regex-dev \
    libboost-timer-dev \
    libboost-chrono-dev \
    # Fields2Cover runtime deps. libFields2Cover.so links
    # against libgdal, libgeos_c, libortools, libtinyxml2 + bundles
    # its own libsteering_functions and libmatplot under
    # /opt/fields2cover-300/lib. -dev variants are needed because
    # mowgli_coverage's find_package pulls in transitive headers via
    # the exported Fields2Cover CMake target.
    libgdal-dev libgeos++-dev libtinyxml2-dev libtbb12 \
    # Utilities
    python3-argcomplete \
    python3-rosdep \
    # WebSocket server for cmd_vel_ws_relay (manual mowing low-latency path)
    python3-websockets \
 && rm -rf /var/lib/apt/lists/*

# Source-built ROS dependencies that do not have Lyrical binaries.
COPY --from=ortools-vendor-builder /opt/ortools_vendor /opt/ortools_vendor
COPY --from=grid-map-builder /opt/grid_map /opt/grid_map

# Install the external runtime dependencies declared by the exact source
# packages built above. Source trees are bind-mounted from builder stages so
# they do not become runtime image layers.
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=from=grid-map-builder,source=/ws/src/grid_map,target=/tmp/grid-map-src,ro \
    apt-get update && \
    rosdep install \
      --from-paths \
        /tmp/grid-map-src/grid_map_cmake_helpers \
        /tmp/grid-map-src/grid_map_msgs \
        /tmp/grid-map-src/grid_map_core \
        /tmp/grid-map-src/grid_map_cv \
        /tmp/grid-map-src/grid_map_ros \
      --ignore-src \
      --rosdistro lyrical \
      --dependency-types exec \
      --skip-keys nav2_msgs \
      -y && \
    rm -rf /var/lib/apt/lists/*

RUN printf '%s\n' \
      /opt/ortools_vendor/lib \
      /opt/nav2/lib \
      /opt/grid_map/lib \
      > /etc/ld.so.conf.d/mowgli-source-prefixes.conf && \
    ldconfig

ENV CMAKE_PREFIX_PATH=/opt/grid_map:/opt/nav2:/opt/ortools_vendor

RUN printf '%s\n' \
      '#!/bin/bash' \
      'source /opt/ros/lyrical/setup.bash' \
      'source /opt/ortools_vendor/setup.bash' \
      'source /opt/nav2/setup.bash' \
      'source /opt/grid_map/setup.bash' \
      > /opt/mowgli_underlay.sh && \
    chmod +x /opt/mowgli_underlay.sh

# Pull source-built GTSAM 4.3a1 into /opt/gtsam, register its lib
# directory with ldconfig, and expose the CMake config dir so
# find_package(GTSAM) resolves without extra hints. GTSAM's runtime
# .so links against libboost-* and libmetis5, both already present.
COPY --from=gtsam-builder /opt/gtsam /opt/gtsam
RUN echo '/opt/gtsam/lib' > /etc/ld.so.conf.d/gtsam.conf && ldconfig
ENV CMAKE_PREFIX_PATH=/opt/gtsam:${CMAKE_PREFIX_PATH}

# Coverage runs on Fields2Cover v3. Registering
# /opt/fields2cover-300/lib lets the runtime loader resolve libFields2Cover.so
# + libsteering_functions.so + libmatplot.so.1; libortools comes from the
# source-built ortools_vendor prefix. mowgli_coverage's
# CMakeLists pins PATHS /opt/fields2cover-300 + INSTALL_RPATH, so find_package
# and the binary both resolve v3.
COPY --from=fields2cover-v3-builder /opt/fields2cover-300 /opt/fields2cover-300
RUN echo '/opt/fields2cover-300/lib' > /etc/ld.so.conf.d/fields2cover-300.conf && ldconfig

# =============================================================================
# Stage 2: deps
# Installs build tools and resolves rosdep dependencies. Cached as long as
# package.xml files don't change.
# =============================================================================
FROM base AS deps

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-vcstool \
    ros-lyrical-ament-cmake \
    ros-lyrical-rosidl-default-generators \
    build-essential \
    cmake \
    git \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /ros2_ws

# Copy only package.xml + CMakeLists.txt to resolve rosdep deps (cache layer).
# This ensures the expensive rosdep install is only re-run when package
# definitions change, NOT when source code changes. The Docker build context
# is the monorepo root so we can also pull the sidecar tools/motor package.
COPY ros2/src/mowgli_interfaces/package.xml    src/mowgli_interfaces/package.xml
COPY ros2/src/mowgli_interfaces/CMakeLists.txt src/mowgli_interfaces/CMakeLists.txt
COPY ros2/src/mowgli_hardware/package.xml      src/mowgli_hardware/package.xml
COPY ros2/src/mowgli_hardware/CMakeLists.txt   src/mowgli_hardware/CMakeLists.txt
COPY ros2/src/mowgli_localization/package.xml  src/mowgli_localization/package.xml
COPY ros2/src/mowgli_localization/CMakeLists.txt src/mowgli_localization/CMakeLists.txt
COPY ros2/src/mowgli_map/package.xml           src/mowgli_map/package.xml
COPY ros2/src/mowgli_map/CMakeLists.txt        src/mowgli_map/CMakeLists.txt
COPY ros2/src/mowgli_nav2_plugins/package.xml  src/mowgli_nav2_plugins/package.xml
COPY ros2/src/mowgli_nav2_plugins/CMakeLists.txt src/mowgli_nav2_plugins/CMakeLists.txt
COPY ros2/src/mowgli_behavior/package.xml      src/mowgli_behavior/package.xml
COPY ros2/src/mowgli_behavior/CMakeLists.txt   src/mowgli_behavior/CMakeLists.txt
COPY ros2/src/mowgli_monitoring/package.xml    src/mowgli_monitoring/package.xml
COPY ros2/src/mowgli_monitoring/CMakeLists.txt src/mowgli_monitoring/CMakeLists.txt
COPY ros2/src/mowgli_simulation/package.xml    src/mowgli_simulation/package.xml
COPY ros2/src/mowgli_simulation/CMakeLists.txt src/mowgli_simulation/CMakeLists.txt
COPY ros2/src/mowgli_bringup/package.xml       src/mowgli_bringup/package.xml
COPY ros2/src/mowgli_bringup/CMakeLists.txt    src/mowgli_bringup/CMakeLists.txt
COPY ros2/src/fusion_graph/package.xml         src/fusion_graph/package.xml
COPY ros2/src/fusion_graph/CMakeLists.txt      src/fusion_graph/CMakeLists.txt
# fusion_graph is the opt-in factor-graph replacement for ekf_map_node;
# depends on libgtsam-dev installed in the base stage.
COPY ros2/src/mowgli_coverage/package.xml      src/mowgli_coverage/package.xml
COPY ros2/src/mowgli_coverage/CMakeLists.txt   src/mowgli_coverage/CMakeLists.txt
# opennav_coverage submodule provides opennav_coverage_msgs (action types
# consumed by mowgli_coverage). Pulled by checkout submodules: recursive.
COPY ros2/src/opennav_coverage/opennav_coverage_msgs/package.xml    src/opennav_coverage/opennav_coverage_msgs/package.xml
COPY ros2/src/opennav_coverage/opennav_coverage_msgs/CMakeLists.txt src/opennav_coverage/opennav_coverage_msgs/CMakeLists.txt
# universal-gnss is vendored as a git submodule under ros2/src/external.
# gnss_ros2 is the only colcon package; its sibling gnss_* libraries are
# plain CMake subdirectories pulled in at build time from the same repo root.
COPY ros2/src/external/universal-gnss/gnss_ros2/package.xml    src/external/universal-gnss/gnss_ros2/package.xml
COPY ros2/src/external/universal-gnss/gnss_ros2/CMakeLists.txt src/external/universal-gnss/gnss_ros2/CMakeLists.txt
# operator-side drivetrain tools live outside ros2/src but must still be built
# into the runtime workspace as the ROS 2 package mowgli_tools.
COPY tools/motor/package.xml    src/mowgli_tools/package.xml
COPY tools/motor/CMakeLists.txt src/mowgli_tools/CMakeLists.txt

# Resolve rosdep dependencies (cached until package.xml changes)
RUN source /opt/mowgli_underlay.sh \
 && rosdep update --rosdistro lyrical \
 && rosdep install \
      --from-paths src \
      --ignore-src \
      --rosdistro lyrical \
      --skip-keys \
        "grid_map_core grid_map_msgs grid_map_ros nav2_behaviors nav2_bringup nav2_bt_navigator nav2_controller nav2_core nav2_costmap_2d nav2_lifecycle_manager nav2_map_server nav2_mppi_controller nav2_msgs nav2_planner nav2_regulated_pure_pursuit_controller nav2_ros_common nav2_smac_planner nav2_util nav2_waypoint_follower opennav_docking ortools_vendor webots_ros2_driver" \
      -y

# =============================================================================
# Stage 3: build-interfaces
# Build only mowgli_interfaces (msg/srv/action definitions). These rarely
# change and are a dependency for all other packages.
# =============================================================================
FROM deps AS build-interfaces

# Copy the full mowgli_interfaces package
COPY ros2/src/mowgli_interfaces/ src/mowgli_interfaces/

RUN /bin/bash -c "\
    source /opt/ros/lyrical/setup.bash && \
    colcon build \
      --packages-select mowgli_interfaces \
      --cmake-args -DCMAKE_BUILD_TYPE=Release \
      --parallel-workers \$(nproc) \
    "

# =============================================================================
# Stage 4: build
# Compiles all remaining workspace packages from source.
# =============================================================================
FROM build-interfaces AS build

ARG BUILD_TYPE=Release
ARG BUILD_TESTING=OFF

# Now copy the rest of the source tree plus the sidecar tools package
COPY ros2/src/ src/
COPY tools/motor/ src/mowgli_tools/
COPY ros2/cmake/ cmake/
COPY --from=webots-ros2-builder /opt/webots_ros2 /opt/webots_ros2

# Skip every opennav_coverage subpackage except _msgs. The upstream server
# is pinned to F2C 1.2.1 (no TrapezoidalDecomp / generateHeadlandSwaths /
# planPathForConnection); we use mowgli_coverage with pinned F2C 3 instead, and
# only need the action types from opennav_coverage_msgs. Marker files
# aren't tracked in the submodule, so create them here.
RUN touch \
      src/opennav_coverage/opennav_coverage/COLCON_IGNORE \
      src/opennav_coverage/opennav_coverage_bt/COLCON_IGNORE \
      src/opennav_coverage/opennav_coverage_demo/COLCON_IGNORE \
      src/opennav_coverage/opennav_coverage_navigator/COLCON_IGNORE \
      src/opennav_coverage/opennav_row_coverage/COLCON_IGNORE

RUN /bin/bash -c "\
    f2c_config=\$(find /opt/fields2cover-300 -type f \( -name 'Fields2CoverConfig.cmake' -o -name 'fields2cover-config.cmake' \) | head -n 1) && \
    test -n \"\${f2c_config}\" && \
    echo \"Fields2Cover config: \${f2c_config}\" \
    "

# Build all packages (mowgli_interfaces is already built and cached)
RUN /bin/bash -c "\
    source /opt/mowgli_underlay.sh && \
    source /opt/webots_ros2/setup.bash && \
    source install/setup.bash && \
    export CMAKE_PREFIX_PATH=/opt/fields2cover-300:/opt/gtsam:\${CMAKE_PREFIX_PATH:-} && \
    export MAKEFLAGS=-j3 && \
    colcon build \
      --cmake-args \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
        -DBUILD_TESTING=${BUILD_TESTING} \
        -DCMAKE_PROJECT_INCLUDE_BEFORE=/ros2_ws/cmake/ament_target_dependencies_compat.cmake \
      --parallel-workers 2 \
      --event-handlers console_cohesion+ \
    "

# Run tests only for validation builds; failures are fatal.
RUN /bin/bash -c "\
    if [ '${BUILD_TESTING}' = 'ON' ]; then \
      source /opt/mowgli_underlay.sh && \
      source /opt/webots_ros2/setup.bash && \
      source install/setup.bash && \
      colcon test --return-code-on-test-failure \
        --ctest-args -E '^cpplint\$' \
        --event-handlers console_cohesion+ && \
      colcon test-result --verbose; \
    fi \
    "

# =============================================================================
# Stage 5: runtime
# Minimal image containing only what is needed to run the robot.
# =============================================================================
FROM base AS runtime

WORKDIR /ros2_ws

# Ensure libtinyxml2 .so.9 and .so.10 both resolve. The build stage may
# compile against a different minor version than the runtime base provides
# when Docker layer caching spans a base image update.
RUN reallib=$(find /usr/lib -name 'libtinyxml2.so.*.*.*' | head -1) && \
    dir=$(dirname "$reallib") && \
    [ ! -e "$dir/libtinyxml2.so.9" ] && ln -sf "$(basename "$reallib")" "$dir/libtinyxml2.so.9" || true && \
    [ ! -e "$dir/libtinyxml2.so.10" ] && ln -sf "$(basename "$reallib")" "$dir/libtinyxml2.so.10" || true && \
    ldconfig

# Pull compiled install tree from the build stage
COPY --from=build /ros2_ws/install/ install/

# Pull the ublox_ubx_msgs + ublox_ubx_interfaces typesupport built from the
# cedbossneo/ublox_dgnss fork (see ublox-msgs-builder stage). The GPS driver
# runs in the separate sensors/gps image, so these message/service packages
# aren't otherwise present here — without them foxglove_bridge logs one
# "package not found" schema error per /ubx_* topic + ublox_dgnss service on
# every client reconnect. Sourced by ros2_entrypoint.sh as a ROS overlay.
COPY --from=ublox-msgs-builder /ws/install /opt/ublox_msgs

# Copy all launch files and config from every package
COPY ros2/src/mowgli_bringup/launch/    /ros2_ws/install/mowgli_bringup/share/mowgli_bringup/launch/
COPY ros2/src/mowgli_bringup/config/    /ros2_ws/install/mowgli_bringup/share/mowgli_bringup/config/
# mowgli_localization has no launch files; nodes are brought up from
# mowgli_bringup/navigation.launch.py. Keep the config copy so the YAML
# files still land in the install tree.
COPY ros2/src/mowgli_localization/config/  /ros2_ws/install/mowgli_localization/share/mowgli_localization/config/

# fusion_graph ships its own launch + config used by navigation.launch.py
COPY ros2/src/fusion_graph/launch/  /ros2_ws/install/fusion_graph/share/fusion_graph/launch/
COPY ros2/src/fusion_graph/config/  /ros2_ws/install/fusion_graph/share/fusion_graph/config/

# Use Cyclone DDS — FastRTPS shared memory causes stale /dev/shm segments
# and unreliable service discovery on ARM boards
ENV RMW_IMPLEMENTATION=rmw_cyclonedds_cpp

# Entrypoint script sources both ROS and workspace setups before exec'ing CMD
COPY ros2/scripts/ros2_entrypoint.sh /ros2_entrypoint.sh
RUN chmod +x /ros2_entrypoint.sh

# Mow-session monitor — runs alongside the live stack to record a full
# state timeline per mowing test (see CLAUDE.md "Mowing Session Monitoring").
COPY ros2/scripts/mow_session_monitor.py /ros2_ws/scripts/mow_session_monitor.py
RUN chmod +x /ros2_ws/scripts/mow_session_monitor.py

ENTRYPOINT ["/ros2_entrypoint.sh"]
CMD ["ros2", "launch", "mowgli_bringup", "mowgli.launch.py"]

# =============================================================================
# Stage 6: simulation
# Extends runtime with the Webots simulator for headless/web simulation.
#
# Webots R2025a is the minimum version required by the ROS 2 Kilted driver.
# Pin the vendor package so a clean checkout builds the same simulator image.
# Cyberbotics publishes this Linux package for amd64 only; the regular runtime
# target remains architecture-neutral, while this simulator target fails early
# and clearly on unsupported Linux architectures.
# =============================================================================
FROM runtime AS simulation

ARG DEBIAN_FRONTEND=noninteractive

COPY --from=webots-ros2-builder /opt/webots_ros2 /opt/webots_ros2

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=from=webots-ros2-builder,source=/ws/src/webots_ros2,target=/tmp/webots_ros2,ro \
    apt-get update && \
    rosdep install \
      --from-paths \
        /tmp/webots_ros2/webots_ros2_msgs \
        /tmp/webots_ros2/webots_ros2_importer \
        /tmp/webots_ros2/webots_ros2_driver \
        /tmp/webots_ros2/webots_ros2_control \
      --ignore-src \
      --rosdistro lyrical \
      --dependency-types exec \
      -y && \
    rm -rf /var/lib/apt/lists/*

# webots_ros2_importer imports PIL at runtime, but upstream declares it only
# as a test dependency, so rosdep does not install it.
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get install -y --no-install-recommends \
    # Headless rendering (sensors need a virtual display when no GPU)
    xvfb \
    # VNC + lightweight desktop for sim GUI access from macOS
    tigervnc-standalone-server \
    tigervnc-common \
    novnc \
    websockify \
    openbox \
    xterm \
    x11-utils \
    libsndio7.0 \
    python3-pil

# Install Webots R2025a if not pulled in by the ROS package
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    if [ ! -d /usr/local/webots ] && [ ! -d /usr/share/webots ]; then \
      apt-get update && apt-get install -y --no-install-recommends wget && \
      wget -q https://github.com/cyberbotics/webots/releases/download/R2025a/webots_2025a_amd64.deb && \
      apt-get install -y ./webots_2025a_amd64.deb && \
      rm -f webots_2025a_amd64.deb; \
    fi

ENV WEBOTS_HOME=/usr/local/webots

# Fail the image build if the pinned Webots binary gains another undeclared
# system dependency or its Python importer cannot load in the final image.
RUN missing="$(LD_LIBRARY_PATH=/usr/local/webots/lib/webots \
      ldd /usr/local/webots/bin/webots-bin | grep 'not found' || true)" && \
    if [ -n "${missing}" ]; then \
      printf '%s\n' "${missing}" >&2; \
      exit 1; \
    fi && \
    source /opt/webots_ros2/setup.bash && \
    python3 -c "from PIL import Image; import collada" && \
    ros2 pkg prefix webots_ros2_control >/dev/null && \
    test -f /opt/webots_ros2/lib/libwebots_ros2_control.so && \
    test -f /opt/webots_ros2/lib/libwebots_ros2_control_system.so && \
    test -f /opt/webots_ros2/share/webots_ros2_control/webots_ros2_control.xml && \
    test -f /opt/webots_ros2/share/webots_ros2_control/webots_ros2_control_system.xml && \
    control_missing="$(ldd \
      /opt/webots_ros2/lib/libwebots_ros2_control.so \
      /opt/webots_ros2/lib/libwebots_ros2_control_system.so | \
      grep 'not found' || true)" && \
    if [ -n "${control_missing}" ]; then \
      printf '%s\n' "${control_missing}" >&2; \
      exit 1; \
    fi

# Patch webots_ros2_driver's WSL detection: inside a Docker container the
# host WSL kernel leaks through /proc/version, causing is_wsl() to return
# True even though we run native Linux Webots.  Replace the check with a
# constant False.
RUN find /opt/webots_ros2 -name utils.py -path '*/webots_ros2_driver/*' \
      -exec sed -i "s/return 'microsoft-standard' in uname().release/return False  # patched for Docker-in-WSL/" {} +

# Copy VNC startup script
COPY ros2/scripts/start_vnc.sh /ros2_ws/scripts/start_vnc.sh
RUN chmod +x /ros2_ws/scripts/start_vnc.sh


# Copy Foxglove layout for easy import
COPY ros2/foxglove/ /ros2_ws/foxglove/

# Foxglove Bridge WebSocket port
EXPOSE 8765
# noVNC web port (sim GUI via browser)
EXPOSE 6080

CMD ["ros2", "launch", "mowgli_bringup", "sim_full_system.launch.py", "headless:=true", "use_rviz:=false"]
