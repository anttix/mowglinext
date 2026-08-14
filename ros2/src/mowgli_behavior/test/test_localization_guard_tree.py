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
