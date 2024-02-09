import os


def process_winch_file(file_name: str, source_path: str):
    station_name = _get_station_name(file_name)
    winch_path = _get_output_path(station_name)
    _ensure_target_path(winch_path)

    input_file = os.path.join(source_path, file_name)
    output_file = os.path.join(winch_path, file_name)
    _write_file(input_file, output_file)

    # Move the input file to the WINCH folder
    destination_path = _get_destination_path(station_name)
    _ensure_target_path(destination_path)

    destination_file = os.path.join(destination_path, file_name)
    _move_processed_file(input_file, destination_file)


def _write_file(input_file: str, output_file: str):
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


def _move_processed_file(input_file: str, destination_file: str):
    if os.path.exists(destination_file):
        os.replace(input_file, destination_file)
    else:
        os.rename(input_file, destination_file)


def _get_destination_path(station_name: str) -> str:
    return os.path.join('C:\Campbellsci\LoggerNet', station_name, 'WINCH_MISSIONS')


def _get_output_path(stations_name: str) -> str:
    return os.path.join('C:\DATA', stations_name, 'WINCH_MISSIONS')


def _get_station_name(file_name: str) -> str:
    """
    file_name:
        `WDATA_PMZA-RIKI_2023-06-24_163000.txt`
    """
    return file_name.split('_')[1]


def _ensure_target_path(path: str):
    if not os.path.exists(path):
        os.makedirs(path)



