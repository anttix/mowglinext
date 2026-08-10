#!/bin/bash
# =============================================================================
# post-start.sh — Runs each time the devcontainer starts.
#
# Exposes the host's stable USB serial symlinks inside the container when the
# host provides them. Docker's generated /dev often omits /dev/serial/by-id,
# which makes GNSS receiver selection fall back to volatile tty numbering.
# =============================================================================
set -euo pipefail

if [ ! -S /tmp/.X11-unix/X99 ]; then
  mkdir -p /tmp/.X11-unix
  nohup Xvfb :99 -screen 0 1280x720x24 -nolisten tcp \
    >/tmp/mowgli-xvfb.log 2>&1 &
fi

HOST_DEV_ROOT="${HOST_DEV_ROOT:-/host-dev}"
HOST_SERIAL_BY_ID="${HOST_SERIAL_BY_ID:-${HOST_DEV_ROOT}/serial/by-id}"
CONTAINER_SERIAL_DIR="${CONTAINER_SERIAL_DIR:-/dev/serial}"
CONTAINER_SERIAL_BY_ID="${CONTAINER_SERIAL_BY_ID:-${CONTAINER_SERIAL_DIR}/by-id}"

if [ ! -d "$HOST_SERIAL_BY_ID" ]; then
  exit 0
fi

sudo mkdir -p "$CONTAINER_SERIAL_DIR"

if [ -L "$CONTAINER_SERIAL_BY_ID" ]; then
  current_target="$(readlink "$CONTAINER_SERIAL_BY_ID" || true)"
  if [ "$current_target" = "$HOST_SERIAL_BY_ID" ]; then
    exit 0
  fi
  sudo rm -f "$CONTAINER_SERIAL_BY_ID"
elif [ -d "$CONTAINER_SERIAL_BY_ID" ]; then
  # A native by-id directory already exists in the container. Keep it.
  exit 0
elif [ -e "$CONTAINER_SERIAL_BY_ID" ]; then
  sudo rm -f "$CONTAINER_SERIAL_BY_ID"
fi

sudo ln -s "$HOST_SERIAL_BY_ID" "$CONTAINER_SERIAL_BY_ID"
