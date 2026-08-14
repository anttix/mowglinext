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


def test_localization_guard_stays_blocking_until_covariance_recovers():
    root = ET.parse(TREE_PATH).getroot()
    handler = root.find(
        ".//Sequence[@name='LocalizationDegradedHandler']"
    )

    assert handler is not None
    assert [child.tag for child in handler][-1] == 'AlwaysFailure'

    status = handler.find('PublishHighLevelStatus')
    assert status is not None
    assert status.attrib['state_name'] == 'LOCALIZATION_DEGRADED'
