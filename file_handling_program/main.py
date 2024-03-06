import os
import json
from pathlib import Path

from mitis_python_tools.DAT_handling import process_DAT, FilePointer
from mitis_python_tools.WINCH_handling import process_winch

CONFIG_FILE = Path("config.json")
POINTERS_DIR = Path("pointers")

with open(CONFIG_FILE) as f:
    config = json.load(f)

SOURCE_DIR = config['source_dir']
TARGET_DIR = config['target_dir']
MOVE_DIR = config['move_dir']
SD_PADDING = config['sd_padding']

POINTERS_DIR.mkdir(parents=True, exist_ok=True)


def main():
    for filename in os.listdir(SOURCE_DIR):
        if "WDATA" in filename:
            process_winch(
                input_file=Path(SOURCE_DIR).joinpath(filename),
                target_dir=TARGET_DIR,
                move_dir=MOVE_DIR
            )

        elif filename.endswith('.dat'):
            pointer = FilePointer(filename=POINTERS_DIR.joinpath(Path(filename).with_suffix('.json')))
            process_DAT(
                input_file=Path(SOURCE_DIR).joinpath(filename),
                target_dir=TARGET_DIR,
                move_dir=MOVE_DIR,
                sd_padding=SD_PADDING,
                pointer=pointer
                )


if __name__ == "__main__":
    main()
