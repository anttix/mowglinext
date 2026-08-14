# Copyright 2026 Mowgli Project
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

from pathlib import Path
import xml.etree.ElementTree as ET


TREE_PATH = Path(__file__).resolve().parents[1] / 'trees' / 'main_tree.xml'


def test_terminal_docked_status_requires_charger_detection():
    root = ET.parse(TREE_PATH).getroot()
    parent_by_child = {
        child: parent
        for parent in root.iter()
        for child in parent
    }
    docked_statuses = root.findall(
        ".//PublishHighLevelStatus[@state_name='IDLE_DOCKED']"
    )

    assert len(docked_statuses) == 4
    for status in docked_statuses:
        charging_sequence = parent_by_child[status]
        assert charging_sequence.tag == 'Sequence'
        assert list(charging_sequence)[0].tag == 'IsCharging'

        status_fallback = parent_by_child[charging_sequence]
        assert status_fallback.tag == 'Fallback'
        assert list(status_fallback)[-1].tag == 'AlwaysSuccess'
