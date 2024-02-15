import os
import shutil
from pathlib import Path

WINCH_SUB_DIR = 'WINCH_MISSIONS'


def process_winch(input_file: str, target_dir: str, winch_dir: str):
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
    destination_path = Path(winch_dir).joinpath(station_name, WINCH_SUB_DIR)
    Path(destination_path).mkdir(parents=True, exist_ok=True)
    shutil.move(input_file, Path(destination_path).joinpath(Path(input_file).name))


def _write_winch_file(input_file: str, output_file: str):
    with open(input_file, 'r') as input_file:
        lines = input_file.readlines()

    # Create the output file
    with open(output_file, 'w') as f:
        # Extract the header lines
        header = lines[:4]

        # Write the header and filtered lines to the output file
        f.writelines(header)
        for line in lines[4:]:
            if line.startswith('[S]') or line.startswith('[W]'):
                f.writelines(line)
