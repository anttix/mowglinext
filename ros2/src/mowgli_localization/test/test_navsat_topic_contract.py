# Copyright 2026 Mowgli Project
# SPDX-License-Identifier: GPL-3.0

from pathlib import Path


def test_factor_input_keeps_antenna_position() -> None:
    source = (
        Path(__file__).resolve().parents[1]
        / 'src'
        / 'navsat_to_absolute_pose_node.cpp'
    ).read_text()

    absolute_pose_block = source.split(
        '// Publish /gps/absolute_pose', 1
    )[1].split('// RTK-aware gate', 1)[0]
    factor_input_block = source.split(
        '// Factor-graph GNSS measurement', 1
    )[1].split('// Inflate variance', 1)[0]

    assert 'out.pose.pose.position.x = base_x;' in absolute_pose_block
    assert 'out.pose.pose.position.y = base_y;' in absolute_pose_block
    assert 'twin.pose.pose.position.x = east;' in factor_input_block
    assert 'twin.pose.pose.position.y = north;' in factor_input_block
    assert 'twin.pose.pose.position.x = base_x;' not in factor_input_block
    assert 'twin.pose.pose.position.y = base_y;' not in factor_input_block
