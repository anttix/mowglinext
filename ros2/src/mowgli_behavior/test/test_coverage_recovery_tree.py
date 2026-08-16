# Copyright 2026 Mowgli Project
# SPDX-License-Identifier: GPL-3.0-or-later

from pathlib import Path
import xml.etree.ElementTree as ET


def test_failed_coverage_transit_runs_bounded_backup_recovery() -> None:
    tree_path = Path(__file__).resolve().parents[1] / 'trees' / 'main_tree.xml'
    root = ET.parse(tree_path).getroot()
    sequence = root.find(".//Sequence[@name='CoverageTransitBackoff']")

    assert sequence is not None
    assert [child.tag for child in sequence] == [
        'WasCoverageTransitFailure',
        'PublishHighLevelStatus',
        'SetMowerEnabled',
        'BackUp',
        'ClearCostmap',
        'AlwaysFailure',
    ]
    backup = sequence.find('BackUp')
    assert backup is not None
    assert backup.attrib == {
        'backup_dist': '0.40',
        'backup_speed': '0.15',
    }


def test_coverage_transit_tree_has_no_physical_recovery() -> None:
    package_path = Path(__file__).resolve().parents[1]
    root = ET.parse(
        package_path / 'trees' / 'coverage_transit_to_pose.xml'
    ).getroot()

    assert root.findall('.//BackUp') == []
    assert root.findall('.//Spin') == []
    assert root.findall('.//DriveOnHeading') == []
    assert root.findall('.//ClearEntireCostmap')


def test_follow_strip_selects_recovery_free_tree_for_both_transit_goals() -> None:
    package_path = Path(__file__).resolve().parents[1]
    source = (package_path / 'src' / 'coverage_nodes.cpp').read_text()

    assert source.count(
        'nav_goal.behavior_tree = ctx->coverage_transit_bt_xml;'
    ) == 2
    node_source = (package_path / 'src' / 'behavior_tree_node.cpp').read_text()
    assert (
        'context_->coverage_transit_bt_xml = '
        'pkg_share + "/trees/coverage_transit_to_pose.xml";'
    ) in node_source
