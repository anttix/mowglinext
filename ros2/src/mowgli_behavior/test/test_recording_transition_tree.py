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


def test_recording_transition_is_bounded_across_safety_guards():
    root = ET.parse(TREE_PATH).getroot()

    for guard_name in (
        'SensorSafetyGuard',
        'BoundaryGuard',
        'LocalizationGuard',
    ):
        guard = root.find(f".//*[@name='{guard_name}']")
        assert guard is not None
        assert sum(child.tag == 'IsRecordingTransition' for child in guard) == 1
