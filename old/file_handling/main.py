import os

from metis_file_handling.ConvertWinch import process_winch_file
from metis_file_handling.TAGtoSD import process_SD_file


SOURCE_PATH = 'C:\Campbellsci\LoggerNet\RECEIVED'


def main():
    for file_name in os.listdir(SOURCE_PATH):
        if "WDATA" in file_name:
            process_winch_file(file_name)

        elif file_name.endswith('.dat'):
            process_SD_file(file_name)


if __name__ == '__main__':
    main()


