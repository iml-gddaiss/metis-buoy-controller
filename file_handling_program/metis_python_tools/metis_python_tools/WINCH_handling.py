import os
import shutil
from pathlib import Path

WINCH_SUB_DIR = 'WINCH_MISSIONS'


CNV_COL_HEADER = [
    "# name 0 = tv290C: Temperature [ITS-90, deg C]",
    "# name 1 = c0S/m: Conductivity [S/m]",
    "# name 2 = prdM: Pressure, Strain Gauge [db]",
    "# name 3 = sal00: Salinity, Practical [PSU]",
    "# name 4 = flTC7: Fluorescence, Turner Cyclop-7F [micro-g/l]",
    "# name 5 = ox: Oxygen, JFE Advantech Co. Rinko Aro-FT [micromol/l]"
]


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
    _write_winch_cnv_file(input_file, winch_target_file)

    # Move the input file to the WINCH folder
    move_path = Path(move_dir).joinpath(station_name, WINCH_SUB_DIR)
    Path(move_path).mkdir(parents=True, exist_ok=True)
    shutil.move(input_file, Path(move_path).joinpath(Path(input_file).name))


def _write_winch_cnv_file(input_file: str, output_file: str):
    # Create the output file
    col_header = ""
    with open(output_file, 'w') as output_file:
        output_file.write("* Viking Buoy CTD file\n")
        with open(input_file, 'r') as input_file:
            header = []
            line = input_file.readline()
            while line.startswith("**"):
                header.append(line)
                line = input_file.readline()

            output_file.writelines(header)

            # Reading data to get the number of column
            for line in input_file:
                if line.startswith("D:") or line.startswith("**"):
                    continue

                if line.count(",") == 3:
                    for _h in CNV_COL_HEADER[0:4]:
                        output_file.write(_h+"\n")
                    break

                elif line.count(",") == 5:
                    for _h in CNV_COL_HEADER:
                        output_file.write(_h + "\n")
                    break

            output_file.write("# bad_flag = -9.990e-29\n")
            output_file.write("*END*\n")

            # Reading / writing data
            for line in input_file:
                if line.startswith("D:") or line.startswith("**"):
                    continue

                if line.count(",") > 2:
                    output_file.write(line.replace(",", " "))

