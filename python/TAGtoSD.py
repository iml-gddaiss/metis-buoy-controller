import os

# Set the path to the RECEIVED folder
received_folder = r'C:\Campbellsci\LoggerNet\RECEIVED'

# Loop through all the files in the RECEIVED folder
for file_name in os.listdir(received_folder):
    if file_name.endswith('.dat'):
        # Extract the station name from the filename
        station_name = file_name.split('_')[0]

        # Set the path to the DATA folder
        data_folder = os.path.join(r'C:\DATA', station_name)

        # Create the DATA folder if it doesn't exist
        if not os.path.exists(data_folder):
            os.makedirs(data_folder)

        # Extract the year, month, day, hour, and minute from the filename
        datetime = file_name.split("_")[1]
        year = datetime[0:4]
        month = datetime[4:6]
        day = datetime[6:8]

        # Set the path to the output file
        output_file = os.path.join(data_folder, f"{station_name}_SD_{year}{month}{day}.dat")

        # Create the output file if it doesn't exist
        if not os.path.exists(output_file):
            with open(output_file, 'w') as f:
                f.write('')

        # Write data to output file
        v = ['#' for _ in range(40)]

        with open(os.path.join(received_folder, file_name), 'r') as f:
            for line in f:
                # Remove the first characters if the file is modified (because of the BOM garbage)
                if line[0] != "[":
                    line = line[3:]

                if line[1:5] == "INIT":
                    line = line[6:]
                    INIT = line.split(",")
                    v[0] = INIT[0]  # Buoy_Name
                    v[1] = "{}/{}/{}".format(year, month, day)
                    v[2] = INIT[2]  # Buoy_Hour

                    #Formating Latitude
                    lat_degrees = int(INIT[3][:2])
                    lat_minutes = round(float(INIT[3][4:-2]),2)
                    lat_hem = INIT[3][-1]
                    if lat_minutes > 59.94:
                        lat_degrees = lat_degrees + 1
                        lat_minutes = "00.00"
                    lat_minutes = "{:05.2f}".format(lat_minutes)
                    v[3] = "{} {}{}".format(lat_degrees,lat_minutes,lat_hem)

                    #Formating Longitude
                    long_degrees = int(INIT[4][:3])
                    long_minutes = round(float(INIT[4][5:-2]),2)
                    long_hem = INIT[4][-1]
                    if long_minutes > 59.94:
                        long_degrees = long_degrees + 1
                        long_minutes = "00.00"
                    long_minutes = "{:05.2f}".format(long_minutes)
                    v[4] = "{} {}{}".format(long_degrees,long_minutes,long_hem)
                    
                    try:
                        v[31] = int(round(float(INIT[5]),0))  # Heading
                        if v[31] == 360:
                            v[31] = 0
                        v[28] = int(round(float(INIT[6]),0))  # Pitch
                        v[29] = int(round(float(INIT[7]),0))  # Roll
                    except ValueError:
                        pass
                    v[33] = int(round(float(INIT[8]),0))  # Buoy_COG
                    if v[33] == 360:
                        v[33] = 0
                    v[32] = f"{float(INIT[9]):.1f}" # Buoy_SOG
                    v[37] = round(float(INIT[10]),0)  # WaterDetection
                    if v[37] < 2000:
                        v[37] = 0


                elif line[1:5] == "POWR":
                    line = line[6:]
                    POWR = line.split(",")
                    try:
                        v[24] = round(max(float(POWR[0]),float(POWR[2])),1)  #Higher voltage between VBatt1 & VBatt2
                        v[25] = round(float(POWR[1])+float(POWR[3]),1)  #Sum Amp Solar
                        v[26] = round(float(POWR[7]),1) #Amp Turbine
                        v[27] = round(float(POWR[6])+float(POWR[8]),1)  #Sum AMain & AWinch
                        v[38] = POWR[10][7]
                    except (ValueError, IndexError):
                        pass

                elif line[1:5] == "TRP1":
                    line = line[6:]
                    TRP1 = line.split(",")
                    try:
                        v[14] = f"{float(TRP1[0]):.7f}"
                        v[15] = f"{float(TRP1[1]):.4f}"
                        v[16] = f"{float(TRP1[2]):.2f}"
                    except ValueError:
                        v[14] = "#"
                        v[15] = "#"
                        v[16] = "#"

                elif line[1:4] == "CTD":
                    line = line[5:]
                    CTD = line.split(",")
                    try:
                        v[11] = f"{float(CTD[0]):.2f}" #Temperature
                    except ValueError:
                        v[11] = "#"
                    try:
                        v[12] = f"{float(CTD[2]):.2f}" #Salinity
                    except ValueError:
                        v[12] = "#"
                    try:
                        v[13] = f"{float(CTD[3]):.2f}" #Density
                    except ValueError:
                        v[13] = "#"

                    
                elif line[1:6] == "PHPRO":
                    line = line[7:]
                    PH = line.split(",")
                    v[20] = round(float(PH[0]),4) #First value is the Ext. pH
                    if v[20] < 3 or v[20] > 11 :
                        v[20] = "#"
                    else :
                        v[20] = f"{v[20]:.4f}"
                        
                elif line [1:5] == "WIND" or line[1:5] == "W700" or line[1:5] == "W536":
                    line = line[6:]
                    WIND = line.split(",")
                    try:
                        v[5] = int(round(float(WIND[4]),0)) #Average Wind Speed
                        v[6] = int(round(float(WIND[5]),0)) #Max Wind Speed
                        v[7] = int(round(float(WIND[1]),0)) #Average Wind Direction
                    except (ValueError, OverflowError):
                        pass
                
                elif line[1:5] == "ATMS":
                    line = line[6:]
                    ATMS = line.split(",")
                    v[8] = f"{float(ATMS[0]):.1f}" #Air Temperature
                    v[9] = int(round(float(ATMS[1]),0)) #Relative Humidity
                    v[10] = f"{float(ATMS[2]):.1f}" #Air Pressure

                    try:
                        v[17] = int(round(float(ATMS[3]),0)) #PAR
                    except ValueError:
                        v[17] = "#"
                    v[34] = f"{float(ATMS[4]):.1f}" #Rain Accumulation


                elif line[1:5] == "WAVE":
                    line = line[6:]
                    WAVE = line.split(",")
                    try:
                        v[21] = f"{float(WAVE[2]):.1f}" #Wave Period
                        v[22] = f"{float(WAVE[4]):.1f}" #Wave Ave. Height (H1/3)
                        v[23] =f"{float(WAVE[5]):.1f}" #Wave Max Height
                    except ValueError:
                        pass

                elif line[1:5] == "PCO2":
                    line = line[6:]
                    PCO2 = line.split(",")
                    v[18] = f"{float(PCO2[0]):.1f}" #PCO2 Water
                    v[19] = f"{float(PCO2[1]):.1f}" #PCO2 Air


                elif line[1:4] == "RDI":
                    line = line[5:]
                    RDI = line.split(",")
                    try:
                        v[35] = f"{(float(RDI[3])/1000):.1f}" #Bin 1 mm/s to m/s
                        v[36] = int(round(float(RDI[2]),0)) #Bin 1 Heading
                        if v[36] == 360:
                            v[36] == 0
                    except ValueError:
                        v[35] = "#"
                        v[36] = "#"
                    
                    

            # join the array values with ',' and write to the output file
            with open(output_file, 'a') as f:
                v_str = []
                for idx, val in enumerate(v):
                    if idx in [5,6]:
                        v_str.append(str(val).rjust(2))
                                     
                    elif idx in [7,9,28,29,31,33,35,36]:
                        v_str.append(str(val).rjust(3))
                                     
                    elif idx in [8,17,21,22,23,24,25,26,27,32,34]:
                        v_str.append(str(val).rjust(4))
                                     
                    elif idx in [11,12,13,16,18,19]:
                        v_str.append(str(val).rjust(5))
                                     
                    elif idx in [10,15,20,]:
                        v_str.append(str(val).rjust(6))
                                     
                    elif idx in [14]:
                        v_str.append(str(val).rjust(9))
                    else:
                        v_str.append(str(val))

                f.write(','.join(v_str)+'\n')
        
        # Move the input file to the SORTED folder
        sorted_folder = os.path.join(rf'C:\Campbellsci\LoggerNet\{station_name}', 'SORTED')
        if not os.path.exists(sorted_folder):
            os.makedirs(sorted_folder)
        destination_file = os.path.join(sorted_folder, file_name)
        if os.path.exists(destination_file):
            os.replace(os.path.join(received_folder, file_name), destination_file)
        else:
            os.rename(os.path.join(received_folder, file_name), destination_file)
