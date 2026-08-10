from ament_index_python.packages import get_package_prefix


def test_ros2_control_plugins_are_installed():
    for package in ('diff_drive_controller', 'joint_state_broadcaster'):
        assert get_package_prefix(package)
