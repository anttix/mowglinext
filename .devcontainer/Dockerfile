# =============================================================================
# MowgliNext Development Container
#
# All-in-one image for GitHub Codespaces and local devcontainer usage.
# Includes: ROS2 Lyrical, the pinned source navigation underlay,
# Fields2Cover 3 + ORTools source prefixes,
# Gazebo Harmonic (headless), Go 1.24, Node 22, Python linting, build tools,
# and developer utilities. Full-stack builds use ros2/Dockerfile.
#
# Gazebo runs headless (CPU rendering) — connect Foxglove Studio to
# ws://localhost:8765 for visualization.
#
# The default ros:lyrical-ros-base image keeps Codespaces and local
# devcontainer builds on the same pinned distribution base.
# =============================================================================
# ---------------------------------------------------------------------------
# Stage 0: gtsam-builder
# Mirrors ros2/Dockerfile stage 0. Builds GTSAM 4.3a1 from source —
# the Ubuntu libgtsam-dev package ships a broken GTSAMConfig.cmake
# and uses the legacy boost::optional<Matrix&> custom-factor API, and
# there is no ros-lyrical-gtsam package. Source build gives the modern
# OptionalMatrixType API needed by fusion_graph's GnssLeverArmFactor.
# ---------------------------------------------------------------------------
ARG BASE_IMAGE=ros:lyrical-ros-base

FROM ros:lyrical-ros-base AS gtsam-builder

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

FROM ${BASE_IMAGE} AS nav2-base

ARG NAV2_COMMIT=0e69ba8a30259b827de5169817f6991d46374465
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
      git \
      libboost-all-dev \
      ros-dev-tools \
 && rm -rf /var/lib/apt/lists/*

COPY ros2/scripts/build_nav2.sh /usr/local/bin/build_nav2
RUN NAV2_REF="${NAV2_COMMIT}" NAV2_JOBS="$(nproc)" build_nav2

FROM nav2-base AS grid-map-builder

ARG GRID_MAP_COMMIT=7ae24725d7effa0c22a23524b91b46bada5b2728
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
      libopencv-dev \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp/grid-map-ws/src
RUN git clone --filter=blob:none https://github.com/ANYbotics/grid_map.git grid_map && \
    git -C grid_map checkout "${GRID_MAP_COMMIT}"

COPY ros2/patches/grid_map-lyrical.patch /tmp/grid-map-lyrical.patch
RUN git -C grid_map apply --check /tmp/grid-map-lyrical.patch && \
    git -C grid_map apply /tmp/grid-map-lyrical.patch

WORKDIR /tmp/grid-map-ws
RUN apt-get update && \
    . /opt/ros/lyrical/setup.sh && \
    . /opt/nav2/setup.sh && \
    rosdep install \
      --from-paths \
        src/grid_map/grid_map_cmake_helpers \
        src/grid_map/grid_map_msgs \
        src/grid_map/grid_map_core \
        src/grid_map/grid_map_cv \
        src/grid_map/grid_map_ros \
      --ignore-src \
      --rosdistro lyrical \
      --skip-keys nav2_msgs \
      -y && \
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
        -DCMAKE_CXX_FLAGS=-Wno-deprecated-declarations && \
    rm -rf /var/lib/apt/lists/* /tmp/grid-map-ws

FROM grid-map-builder AS webots-ros2-builder

ARG WEBOTS_ROS2_COMMIT=a5a9c3f82b0056809e89c976b9e9f45137c0fd57
ARG DEBIAN_FRONTEND=noninteractive

WORKDIR /tmp/webots-ws/src
RUN git clone --filter=blob:none https://github.com/cyberbotics/webots_ros2.git \
      webots_ros2 && \
    git -C webots_ros2 checkout "${WEBOTS_ROS2_COMMIT}" && \
    git -C webots_ros2 submodule update --init --recursive

COPY ros2/patches/webots_ros2-lyrical.patch /tmp/webots-ros2-lyrical.patch
RUN git -C webots_ros2 apply --check /tmp/webots-ros2-lyrical.patch && \
    git -C webots_ros2 apply /tmp/webots-ros2-lyrical.patch

WORKDIR /tmp/webots-ws
RUN apt-get update && \
    . /opt/ros/lyrical/setup.sh && \
    rosdep install \
      --from-paths \
        src/webots_ros2/webots_ros2_msgs \
        src/webots_ros2/webots_ros2_importer \
        src/webots_ros2/webots_ros2_driver \
        src/webots_ros2/webots_ros2_control \
      --ignore-src \
      --rosdistro lyrical \
      -y && \
    colcon build \
      --merge-install \
      --install-base /opt/webots_ros2 \
      --packages-up-to webots_ros2_driver webots_ros2_control \
      --cmake-args \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTING=OFF && \
    rm -rf /var/lib/apt/lists/* /tmp/webots-ws

FROM webots-ros2-builder

ARG DEBIAN_FRONTEND=noninteractive
ARG ORTOOLS_VENDOR_COMMIT=1ec2e6d4dfe39711b7acc3aba57f4f54610aa063

# ---------------------------------------------------------------------------
# 1. ROS2 runtime + build + simulation dependencies
#    (apt layer — heaviest; cache by keeping this mostly stable)
# ---------------------------------------------------------------------------
RUN apt-get update && apt-get install -y --no-install-recommends \
    # ── Navigation ──
    # ros-lyrical-nav2-costmap-2d \
    # ros-lyrical-nav2-util \
    # ros-lyrical-nav2-msgs \
    # ros-lyrical-nav2-ros-common \
    # ── DDS (Cyclone is required — FastRTPS has stale shm issues on ARM) ──
    ros-lyrical-rmw-cyclonedds-cpp \
    # ── Motion control ──
    ros-lyrical-twist-mux \
    ros-lyrical-topic-tools \
    # ── Behavior trees ──
    ros-lyrical-behaviortree-cpp \
    # ── Robot description / TF ──
    ros-lyrical-xacro \
    ros-lyrical-robot-state-publisher \
    ros-lyrical-joint-state-publisher \
    ros-lyrical-tf2-ros \
    ros-lyrical-tf2-eigen \
    ros-lyrical-tf2-geometry-msgs \
    # ── Diagnostics ──
    ros-lyrical-diagnostic-msgs \
    ros-lyrical-diagnostic-updater \
    ros-lyrical-rclcpp-components \
    ros-lyrical-rclcpp-lifecycle \
    # ── Bridges ──
    ros-lyrical-foxglove-bridge \
    ros-lyrical-rosbridge-suite \
    # ── Message definitions ──
    ros-lyrical-ublox-msgs \
    ros-lyrical-nmea-msgs \
    ros-lyrical-rtcm-msgs \
    ros-lyrical-geometry-msgs \
    ros-lyrical-nav-msgs \
    ros-lyrical-sensor-msgs \
    ros-lyrical-std-msgs \
    # ── Gazebo Harmonic (headless simulation) ──
    ros-lyrical-ros-gz-sim \
    ros-lyrical-ros-gz-bridge \
    ros-lyrical-ros-gz-interfaces \
    # ── Build tools ──
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-vcstool \
    ros-lyrical-ament-cmake \
    ros-lyrical-ament-cmake-vendor-package \
    ros-lyrical-rosidl-default-generators \
    build-essential \
    cmake \
    git \
    libcurl4-openssl-dev \
    libusb-1.0-0-dev \
    # ── Fields2Cover 3 build dependencies ──
    libeigen3-dev \
    libgdal-dev \
    libgeos++-dev \
    libgeos-dev \
    libgsl-dev \
    libpython3-dev \
    libtbb-dev \
    nlohmann-json3-dev \
    pkg-config \
    # ── Runtime deps ──
    libtinyxml2-dev \
    python3-argcomplete \
    # ── GTSAM runtime deps — libgtsam itself is COPYed from gtsam-builder.
    #    libmetis-dev provides both libmetis5 runtime and the unversioned
    #    libmetis.so symlink GTSAM's exported CMake target links against.
    #    libboost-* runtime libs come in transitively. ──
    libmetis-dev \
    # ── Developer tools ──
    gdb \
    htop \
    vim \
    curl \
    wget \
    bash-completion \
    sudo \
    clang-format \
    cppcheck \
    # ── Python runtime + linting (matches mowgli_localization runtime deps) ──
    python3 \
    python3-pip \
    python3-venv \
    python3-numpy \
    python3-yaml \
    # ── Headless rendering (Gazebo sensors need a virtual display on ARM) ──
    xvfb \
    # ── GitHub CLI ──
    gpg \
    ca-certificates \
 && rm -rf /var/lib/apt/lists/* \
 && curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg \
      | gpg --dearmor -o /usr/share/keyrings/githubcli-archive-keyring.gpg \
 && echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" \
      > /etc/apt/sources.list.d/github-cli.list \
 && apt-get update && apt-get install -y --no-install-recommends gh \
 && rm -rf /var/lib/apt/lists/*

# Keep the ORTools workaround in one shared script so deleting it after the
# Lyrical binary release is a two-line Dockerfile cleanup.
# Removal package: ros-lyrical-ortools-vendor
COPY ros2/scripts/build_ortools_vendor.sh /usr/local/bin/build_ortools_vendor
RUN ORTOOLS_VENDOR_REF="${ORTOOLS_VENDOR_COMMIT}" build_ortools_vendor

# Fields2Cover v3 remains pinned to the same revision as the production image.
# ros-lyrical-fields2cover is the future package name, but currently provides
# F2C 2 and is not a drop-in replacement for this pinned F2C 3 build.
RUN git clone https://github.com/Fields2Cover/Fields2Cover.git /tmp/Fields2Cover && \
    git -C /tmp/Fields2Cover checkout 884d895b59192882476e986ba44ea9143a06a6a9 && \
    sed -i '1i #include <iomanip>' /tmp/Fields2Cover/src/fields2cover/utils/visualizer.cpp && \
    sed -i '1i #include <algorithm>' /tmp/Fields2Cover/src/fields2cover/types/Graph.cpp && \
    . /opt/ros/lyrical/setup.sh && \
    . /opt/ortools_vendor/setup.sh && \
    cmake -S /tmp/Fields2Cover -B /tmp/Fields2Cover/build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/opt/fields2cover-300 \
      -DBUILD_TESTS=OFF \
      -DBUILD_PYTHON=OFF \
      -DBUILD_TUTORIALS=OFF \
      -DBUILD_DOC=OFF \
      -DALLOW_PARALLELIZATION=ON \
      -DBUILD_SHARED_LIBS=ON && \
    cmake --build /tmp/Fields2Cover/build -j$(nproc) && \
    cmake --install /tmp/Fields2Cover/build && \
    printf '%s\n' /opt/ortools_vendor/lib /opt/fields2cover-300/lib \
      > /etc/ld.so.conf.d/coverage-source-prefixes.conf && \
    rm -rf /tmp/Fields2Cover && \
    ldconfig

ENV CMAKE_PREFIX_PATH=/opt/fields2cover-300:/opt/ortools_vendor

# ---------------------------------------------------------------------------
# 2. Python linters + pre-commit (system-wide, PEP 668 override OK inside
#    a dev container). Versions track .pre-commit-config.yaml if the repo
#    adds one later.
# ---------------------------------------------------------------------------
RUN python3 -m venv /opt/dev-tools && \
    /opt/dev-tools/bin/pip install --no-cache-dir \
      ruff==0.8.0 \
      pre-commit==4.0.1 && \
    ln -s /opt/dev-tools/bin/ruff /usr/local/bin/ruff && \
    ln -s /opt/dev-tools/bin/pre-commit /usr/local/bin/pre-commit

# ---------------------------------------------------------------------------
# 3. Go 1.24 — matches gui/go.mod + gui/Dockerfile build stage.
#    Installed under /usr/local/go, put on PATH.
# ---------------------------------------------------------------------------
ENV GO_VERSION=1.24.0
RUN set -eux; \
    arch="$(dpkg --print-architecture)"; \
    case "$arch" in \
      amd64) go_arch=amd64 ;; \
      arm64) go_arch=arm64 ;; \
      *) echo "Unsupported arch: $arch" >&2; exit 1 ;; \
    esac; \
    curl -fsSL "https://go.dev/dl/go${GO_VERSION}.linux-${go_arch}.tar.gz" \
      | tar -C /usr/local -xz
ENV PATH="/usr/local/go/bin:${PATH}"

# ---------------------------------------------------------------------------
# 4. Node 22 + yarn — matches gui/Dockerfile build stage (gui/web build).
# ---------------------------------------------------------------------------
RUN curl -fsSL https://deb.nodesource.com/setup_22.x | bash - \
 && apt-get install -y --no-install-recommends nodejs \
 && rm -rf /var/lib/apt/lists/* \
 && npm install -g --no-fund --no-audit yarn

# ---------------------------------------------------------------------------
# 5. Pull source-built GTSAM 4.3a1 into /opt/gtsam, register its lib
#    directory with ldconfig, and expose the CMake config dir so
#    find_package(GTSAM) resolves without extra hints. Mirrors
#    ros2/Dockerfile so the devcontainer matches production exactly.
# ---------------------------------------------------------------------------
COPY --from=gtsam-builder /opt/gtsam /opt/gtsam
RUN echo '/opt/gtsam/lib' > /etc/ld.so.conf.d/gtsam.conf && ldconfig
ENV CMAKE_PREFIX_PATH=/opt/gtsam:${CMAKE_PREFIX_PATH}

RUN printf '%s\n' \
      'set +u' \
      'source /opt/ros/lyrical/setup.bash' \
      'source /opt/nav2/setup.bash' \
      'source /opt/ortools_vendor/setup.bash' \
      'source /opt/grid_map/setup.bash' \
      'source /opt/webots_ros2/setup.bash' \
      'set -u' \
      'export CMAKE_PREFIX_PATH="/opt/fields2cover-300:/opt/gtsam:${CMAKE_PREFIX_PATH:-}"' \
      'export LD_LIBRARY_PATH="/opt/fields2cover-300/lib:/opt/gtsam/lib:${LD_LIBRARY_PATH:-}"' \
      > /opt/mowgli_underlay.sh

RUN test "$(dpkg --print-architecture)" = amd64 && \
    apt-get update && apt-get install -y --no-install-recommends wget && \
    wget -q https://github.com/cyberbotics/webots/releases/download/R2025a/webots_2025a_amd64.deb && \
    apt-get install -y ./webots_2025a_amd64.deb && \
    rm -f webots_2025a_amd64.deb && \
    rm -rf /var/lib/apt/lists/*
ENV WEBOTS_HOME=/usr/local/webots

# ---------------------------------------------------------------------------
# 6. rosdep init (needed for resolving deps on first build)
# ---------------------------------------------------------------------------
RUN rosdep update --rosdistro lyrical || true

# ---------------------------------------------------------------------------
# 7. Workspace directory
# ---------------------------------------------------------------------------
RUN echo 'ubuntu ALL=(root) NOPASSWD:ALL' > /etc/sudoers.d/ubuntu && \
    chmod 0440 /etc/sudoers.d/ubuntu && \
    mkdir -p /ros2_ws && \
    chown ubuntu:ubuntu /ros2_ws

WORKDIR /ros2_ws

# ---------------------------------------------------------------------------
# 8. Environment setup — auto-source ROS2 + workspace in every shell
# ---------------------------------------------------------------------------
RUN echo 'source /opt/mowgli_underlay.sh' >> /etc/bash.bashrc && \
    echo '[ -f /ros2_ws/install/setup.bash ] && source /ros2_ws/install/setup.bash' >> /etc/bash.bashrc && \
    echo 'export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp' >> /etc/bash.bashrc && \
    echo 'export RCUTILS_COLORIZED_OUTPUT=1' >> /etc/bash.bashrc

# ---------------------------------------------------------------------------
# 9. Cyclone DDS config (localhost-only discovery for containers)
# ---------------------------------------------------------------------------
COPY install/config/cyclonedds.xml /etc/cyclonedds.xml
ENV CYCLONEDDS_URI=file:///etc/cyclonedds.xml
ENV RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
ENV ROS_DOMAIN_ID=0
ENV ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST
ENV DISPLAY=:99
ENV USER=ubuntu

USER ubuntu

# ---------------------------------------------------------------------------
# NOT installed by default (opt in if you need them):
#   PlatformIO      — firmware builds (~500 MB). Install with:
#                       pip install --break-system-packages platformio
#                     Works on x86 for ARM-target cross-compile.
#   Docker-in-Docker — multi-container sim/hardware topologies. Add the
#                     docker-in-docker feature to devcontainer.json.
# ---------------------------------------------------------------------------
