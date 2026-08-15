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
