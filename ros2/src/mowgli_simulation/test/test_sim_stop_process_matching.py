import os
from pathlib import Path
import shlex
import subprocess


SIM_STOP = Path(__file__).resolve().parents[3] / 'scripts' / 'sim-stop.sh'


def _start_named_sleep(name: str) -> subprocess.Popen:
    return subprocess.Popen(['bash', '-c', 'exec -a "$1" sleep 30', 'bash', name])


def test_sim_stop_matches_ros_cli_without_matching_workspace_caller():
    ros_launch = _start_named_sleep('ros2 launch mowgli_bringup sim_full_system.launch.py')
    workspace_caller = _start_named_sleep(
        'devcontainer exec bash -lc cd ros2; make e2e-test'
    )

    try:
        env = os.environ.copy()
        env['SIM_STOP_LIST_ROS_PIDS'] = '1'
        result = subprocess.run(
            [str(SIM_STOP)],
            check=True,
            capture_output=True,
            text=True,
            env=env,
        )
        matched_pids = {int(pid) for pid in result.stdout.split()}

        assert ros_launch.pid in matched_pids
        assert workspace_caller.pid not in matched_pids
    finally:
        ros_launch.terminate()
        workspace_caller.terminate()
        ros_launch.wait()
        workspace_caller.wait()


def test_sim_stop_does_not_match_ancestor_with_ros_launch_in_command():
    command = (
        f'echo caller=$$; '
        f'SIM_STOP_LIST_ROS_PIDS=1 {shlex.quote(str(SIM_STOP))}'
    )
    result = subprocess.run(
        ['bash', '-c', command, 'make e2e-test ros2 launch'],
        check=True,
        capture_output=True,
        text=True,
    )
    lines = result.stdout.splitlines()
    caller_pid = int(lines[0].removeprefix('caller='))
    listed_pids = {int(line) for line in lines[1:]}

    assert caller_pid not in listed_pids
