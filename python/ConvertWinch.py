import os

# Set the path to the RECEIVED folder
received_folder = r'C:\Campbellsci\LoggerNet\RECEIVED'

# Loop through all the files in the RECEIVED folder
for file_name in os.listdir(received_folder):
    if "WDATA" in file_name:
        # Extract the station name from the filename
        station_name = file_name.split('_')[1]

        # Set the path to the DATA/station_name/WINCH_MISSIONS folder
        winch_folder = os.path.join(r'C:\DATA', station_name, 'WINCH_MISSIONS')

        # Create the DATA/station_name/WINCH_MISSIONS folder if it doesn't exist
        if not os.path.exists(winch_folder):
            os.makedirs(winch_folder)

        # Set the path to the output file
        output_file = os.path.join(winch_folder, file_name)

        # Create the output file
        with open(output_file, 'w') as f:
            # Read the input file
            input_file_path = os.path.join(received_folder, file_name)
            with open(input_file_path, 'r') as input_file:
                lines = input_file.readlines()

                # Extract the header lines
                header = lines[:4]

                # Write the header and filtered lines to the output file
                f.writelines(header)
                for line in lines[4:]:
                    if line.startswith('[S]') or line.startswith('[W]'):
                        f.writelines(line)

    # Move the input file to the station_name/WINCH_MISSIONS folder
    station_winch_folder = os.path.join(rf'C:\Campbellsci\LoggerNet\{station_name}', 'WINCH_MISSIONS')
    if not os.path.exists(station_winch_folder):
        os.makedirs(station_winch_folder)
    destination_file = os.path.join(station_winch_folder, file_name)
    if os.path.exists(destination_file):
        os.replace(input_file_path, destination_file)
    else:
        os.rename(input_file_path, destination_file)
