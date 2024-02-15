import os
import json
from pathlib import Path

from mitis_python_tools.SD_handling import process_SD, FilePointer

CONFIG_FILE = Path("./SD_config.json")
POINTERS_DIR = Path("./pointers")

with open(CONFIG_FILE) as f:
    config = json.load(f)

SOURCE_DIR = config['source_dir']
OUTPUT_DIR = config['output_dir']
SD_PADDING = config['sd_padding']

POINTERS_DIR.mkdir(parents=True, exist_ok=True)


def main():
    for file_name in os.listdir(SOURCE_DIR):
        if "WDATA" in file_name:
            pass

        elif file_name.endswith('.dat'):
            pointer = FilePointer(filename=POINTERS_DIR.joinpath(Path(file_name).with_suffix('.json')))
            process_SD(
                filename=Path(SOURCE_DIR).joinpath(file_name),
                target_directory=OUTPUT_DIR,
                sd_padding=SD_PADDING,
                pointer=pointer
                )


if __name__ == "__main__":
    main()
