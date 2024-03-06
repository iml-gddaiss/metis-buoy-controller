import os
import shutil
from pathlib import Path

WINCH_SUB_DIR = 'WINCH_MISSIONS'


def process_winch(input_file: str, target_dir: str, move_dir: str):
    """
    file_name:
        `WDATA_PMZA-RIKI_2023-06-24_163000.txt`
    """
    # Write Winch File from raw winch.
    station_name = input_file.split('_')[1]
    station_winch_directory = Path(target_dir).joinpath(station_name, WINCH_SUB_DIR)

    Path(station_winch_directory).mkdir(parents=True, exist_ok=True)
    winch_target_file = Path(station_winch_directory).joinpath(Path(input_file).name)
    _write_winch_file(input_file, winch_target_file)

    # Move the input file to the WINCH folder
    move_path = Path(move_dir).joinpath(station_name, WINCH_SUB_DIR)
    Path(move_path).mkdir(parents=True, exist_ok=True)
    shutil.move(input_file, Path(move_path).joinpath(Path(input_file).name))


def _write_winch_file(input_file: str, output_file: str):
    # Create the output file
    with open(output_file, 'w') as output_file:
        with open(input_file, 'r') as input_file:
            line = input_file.readline()
            # Extract the header lines
            if line.startswith("**") or line.count(",") == 5:
                output_file.write(line)
