import os
import json
from pathlib import Path

from mitis_python_tools.SD_handling import process_SD, FilePointer
from mitis_python_tools.WINCH_handling import process_winch

CONFIG_FILE = Path("config.json")
POINTERS_DIR = Path("pointers")

with open(CONFIG_FILE) as f:
    config = json.load(f)

SOURCE_DIR = config['source_dir']
TARGET_DIR = config['target_dir']
WINCH_DIR = config['winch_dir']
SD_PADDING = config['sd_padding']

POINTERS_DIR.mkdir(parents=True, exist_ok=True)


def main():
    for filename in os.listdir(SOURCE_DIR):
        if "WDATA" in filename:
            process_winch(
                input_file=Path(SOURCE_DIR).joinpath(filename),
                target_dir=TARGET_DIR,
                winch_dir=WINCH_DIR
            )

        elif filename.endswith('.dat'):
            pointer = FilePointer(filename=POINTERS_DIR.joinpath(Path(filename).with_suffix('.json')))
            process_SD(
                input_file=Path(SOURCE_DIR).joinpath(filename),
                target_directory=TARGET_DIR,
                sd_padding=SD_PADDING,
                pointer=pointer
                )


if __name__ == "__main__":
    main()
