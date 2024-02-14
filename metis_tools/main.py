import os
import json

from metis_tools.SD_handling import process_SD

CONFIG_FILE = "SD_config.json"

with open(CONFIG_FILE) as f:
    config = json.load(f)

SOURCE_DIR = config['source_dir']
SD_DIR = config['SD_dir']

SD_PADDING = config['sd_padding']


def main():
    for file_name in os.listdir(SOURCE_DIR):
        if "WDATA" in file_name:
            pass

        elif file_name.endswith('.dat'):
            process_SD(
                filename=file_name,
                source_dir=SOURCE_DIR,
                sd_directory=SD_DIR,
                sd_padding=SD_PADDING
                )


if __name__ == "__main__":
    main()