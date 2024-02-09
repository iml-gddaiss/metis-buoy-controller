"""
Tag String Structure

[INIT]Buoy_Name,Date,Time,Latitude,Longitude,Heading,Pitch,Roll,COG,SOG,Magnetic_Variation,Water_Detection_Main
[POWR]VBatt1,ABatt1,VBatt2,ABatt2,VSolar,ASolar,AMain,ATurbine,AWinch,PM_RH,Relay_State
[ECO1]Scattering,Chlorophyll,FDOM
[CTD]Temperature,Conductivity,Salinity,Density
[PH]Ext_pH_Calc,Int_pH_Calc,Error_Flag,Ext_pH,Int_pH
[NO2]Dark_Nitrate,Light_Nitrate,Dark_Nitrogen_in_Nitrate,Light_Nitrogen_in_Nitrate,Dark_Bromide,Light_Bromide
[Wind]Source,Wind_Dir_Min,Wind_Dir_Ave,Wind_Dir_Max,Wind_Spd_Min,Wind_Spd_Ave,Wind_Spd_Max
                Source: 7: wmt700, 5: wxt536
[ATMS]Air_Temp,Air_Humidity,Air_Pressure,PAR,Rain_Total,Rain_Duration,Rain_Intensity
[WAVE]Wave_Date,Wave_Time,Wave_Period,Wave_Hm0,Wave_H13,Wave_Hmax
[ADCP]ADCPDate,ADCPTime,EW,NS,Vert,Err
[PCO2]CO2_Air,CO2_Water,Pressure_Air,Pressure_Water,Air_Humidity
[WNCH]messages
    messages:
        Air temperature is too low
        Waves are too high
        Wave period is too short
        Buoy is moving too fast
        Voltage is too low
        Mission Completed
        No Mission in Progress
        Mission in Progress
        Mission Started
        No String received from CTD
        Interval not reach
END]

Max String Length:

    WNCH : 36
"""

import os
import re
from typing import Dict, List
from pathlib import Path

# Tag found in transmitted file.
INSTRUMENTS_TAG = ['INIT', 'POWR', 'ECO1', 'CTD', 'PH', 'NO2', 'WIND', 'ATMS', 'WAVE', 'ADCP', 'PCO2', 'WNCH']

DATA_TAG_REGEX = re.compile(rf"\[({'|'.join(INSTRUMENTS_TAG)})]((?:(?!\[).)*)", re.DOTALL)

TAG_VARIABLES = {
    'init': ['buoy_name', 'date', 'time', 'latitude', 'longitude', 'heading', 'pitch', 'roll', 'cog', 'sog',
             'magnetic_declination', 'water_detection'],
    'powr': ['volt_batt_1', 'amp_batt_1', 'volt_batt_2', 'amp_batt_2', 'volt_solar', 'amp_solar', 'amp_main',
             'amp_turbine', 'amp_winch', 'pm_rh', 'relay_state'],
    'eco1': ['scattering', 'chlorophyll', 'fdom'],
    'ctd': ['temperature', 'conductivity', 'salinity', 'density'],
    'ph': ['ext_ph_calc', 'int_ph_calc', 'error_flag', 'ext_ph', 'int_ph'],
    'no2': ['dark_nitrate', 'light_nitrate', 'dark_nitrogen_in_nitrate', 'light_nitrogen_in_nitrate', 'dark_bromide',
            'light_bromide'],
    'wind': ['source', 'wind_dir_min', 'wind_dir_ave', 'wind_dir_max', 'wind_spd_min', 'wind_spd_ave', 'wind_spd_max'],
    'atms': ['air_temperature', 'air_humidity', 'air_pressure', 'par', 'rain_total', 'rain_duration', 'rain_intensity'],
    'wave': ['date', 'time', 'period', 'hm0', 'h13', 'hmax'],
    'adcp': ['date', 'time', 'u', 'v', 'w', 'err'],
    'pco2': ['co2_air', 'co2_water', 'gas_pressure_air', 'gas_pressure_water', 'air_humidity'],
    'wnch': ['message']
}


def process_transmitted_data(filename: str):
    data = _get_last_line_of_file(filename)
    unpacked_data = _unpack_data(data)

    return unpacked_data


def _unpack_data(data: str) -> list:
    """Returns Data as a dictionary of {TAG:DATA}"""

    unpacked_data = {}
    for data_sequence in DATA_TAG_REGEX.finditer(data):
        tag = data_sequence.group(1).lower()
        data = data_sequence.group(2).split(",")

        unpacked_data[tag] = {key: value for key, value in zip(TAG_VARIABLES[tag], data)}

    return unpacked_data


def _get_last_line_of_file(filename: str) -> str:
    with open(filename, "rb") as f:
        try:
            f.seek(-2, os.SEEK_END)
            while f.read(1) != b'\n':
                f.seek(-2, os.SEEK_CUR)
        except OSError:
            f.seek(0)
        return f.readline().decode()


def _format_lonlat(value: str):
    """
    48°38.459'N -> 48 38.46N
    068°09.406'W -> 68 09.406W
    """
    _match = re.match("(\d+)°(\d+.\d+)\'([A-z])", value)
    if _match:
        _deg = float(_match.group(1))
        _min = float(_match.group(2))
        _hem = _match.group(3)
        lat_minutes = round(_min, 2)

        if lat_minutes == 60:
            _deg += + 1
            _min = 0
        return f"{_deg:0.0f} {_min:05.2f}{_hem}"
    return ""


def _make_SD_string(data: Dict[str, List[str]]) -> str:
    v = ["#" for x in range(37)]  # Rename to SD String FIXME what is the null value

    if "init" in data:
        v[0] = data["init"]['buoy_name']  # Buoy_Name
        v[1] = data["init"]['data'].replace("-", "/")
        v[2] = data["init"]['time']  # Buoy_Hour

        v[3] = _format_lonlat(data['init']['latitude'])
        v[4] = _format_lonlat(data['init']['longitude'])

        try:
            v[31] = f"{float(data['init']['heading']):.0f}"
            v[28] = f"{float(data['init']['pitch']):.1f}"
            v[29] = f"{float(data['init']['roll']):.1f}"
        except ValueError:
            pass
        try:
            v[33] = f"{round(float(data['init']['cog'])) % 360:.0f}"
            v[32] = f"{float(data['init']['sog']):.1f}"
        except ValueError:
            pass

        try:
            _water_detection = float(data['init']['water_detection'])
            v[37] = f"{0 if _water_detection < 2000 else _water_detection:.0f}"
        except ValueError:
            pass

    if "powr" in data:
        try:
            v[24] = f"{round(max(float(data['POWR']['volt_batt_1']), float(data['POWR']['volt_batt_2'])), 1)}"
            v[25] = f"{round(float(data['POWR']['amp_solar']), 1)}"
            v[26] = f"{round(float(data['POWR']['amp_turbine']), 1)}"
            v[27] = f"{round(float(data['POWR']['amp_main']) + float(data['POWR']['amp_winch']), 1)}"
        except ValueError:
            pass

        try:
            v[38] = data['POWR']['relay_state'][7]
        except IndexError:
            pass

    if "eco1" in data: #FIXME fiill value "#"
        try:
            v[14] = f"{float(data['eco1']['scattering']):.7f}"
        except ValueError:
            v[14] = "#"
        try:
            v[15] = f"{float(data['eco1']['chlorophyll']):.4f}"
        except ValueError:
            v[15] = "#"
        try:
            v[16] = f"{float(data['eco1']['fdom']):.2f}"
        except ValueError:
            v[16] = "#"

    if "ctd" in data:
        try:
            v[11] = f"{float(data['ctd']['temperature']):.2f}"
        except ValueError:
            v[11] = "#"
        try:
            v[12] = f"{float(data['ctd']['salinity']):.2f}"
        except ValueError:
            v[12] = "#"
        try:
            v[13] = f"{float(data['ctd']['density']):.2f}"
        except ValueError:
            v[13] = "#"

    if "ph" in data:  # fixme Is calc_ph ok ?
        #         if v[20] < 3 or v[20] > 11: # and why this ?? fixme
        #             v[20] = "#"
        #         else:
        #             v[20] = f"{v[20]:.4f}"
        try:
            v[20] = f"{round(float(data['ph']['ext_ph_calc']), 4)}"
        except ValueError:
            v[20] = "#"

    if "wind" in data:
        try: # FIXME what are wind units ????????
            v[5] = f"{float(data['wind']['wind_spd_ave']):.0f}"
            v[6] = f"{float(data['wind']['wind_spd_max']):.0f}"
            v[7] = f"{float(data['wind']['wind_dir_ave']):.0f}"
        except ValueError:
            pass

    if "atms" in data:
        try:
            v[8] = f"{data['atms']['air_temperature']:.1f}"
        except ValueError:
            v[8] = "#"
        try:
            v[9] = f"{data['atms']['air_humidity']:.0f}"
        except ValueError:
            v[9] = "#"
        try:
            v[10] = f"{data['atms']['air_pressure']:.1f}"
        except ValueError:
            v[10] = "#"

        try:
            v[17] = f"{data['atms']['par']:.0f}"
        except ValueError:
            v[17] = "#"

        try:
            v[34] = f"{float(data['atms']['rain_total']):.1f}"
        except ValueError:
            v[34] = "#"

    if "wave" in data:

#     elif data[1:5] == "WAVE":
#         data = data[6:]
#         WAVE = data.split(",")
#         try:
#             v[21] = f"{float(WAVE[2]):.1f}"  # Wave Period
#             v[22] = f"{float(WAVE[4]):.1f}"  # Wave Ave. Height (H1/3)
#             v[23] = f"{float(WAVE[5]):.1f}"  # Wave Max Height
#         except ValueError:
#             pass
#
#     elif data[1:5] == "PCO2":
#         data = data[6:]
#         PCO2 = data.split(",")
#         v[18] = f"{float(PCO2[0]):.1f}"  # PCO2 Water
#         v[19] = f"{float(PCO2[1]):.1f}"  # PCO2 Air
#
#     elif data[1:4] == "RDI":
#         data = data[5:]
#         RDI = data.split(",")
#         try:
#             v[35] = f"{(float(RDI[3]) / 1000):.1f}"  # Bin 1 mm/s to m/s
#             v[36] = int(round(float(RDI[2]), 0))  # Bin 1 Heading
#             if v[36] == 360:
#                 v[36] = 0  # FIXME c'était == j'imagine que c'était juste égale ?
#         except ValueError:
#             v[35] = "#"
#             v[36] = "#"

#        # join the array values with ',' and write to the output file
#        with open(output_file, 'a') as f:
#            v_str = []
#             for idx, val in enumerate(v):
#                 if idx in [5, 6]:
#                     v_str.append(str(val).rjust(2))
#
#                 elif idx in [7, 9, 28, 29, 31, 33, 35, 36]:
#                     v_str.append(str(val).rjust(3))
#
#                 elif idx in [8, 17, 21, 22, 23, 24, 25, 26, 27, 32, 34]:
#                     v_str.append(str(val).rjust(4))
#
#                 elif idx in [11, 12, 13, 16, 18, 19]:
#                     v_str.append(str(val).rjust(5))
#
#                 elif idx in [10, 15, 20, ]:
#                     v_str.append(str(val).rjust(6))
#
#                 elif idx in [14]:
#                     v_str.append(str(val).rjust(9))
#                 else:
#                     v_str.append(str(val))
#        f.write(','.join(v_str) + '\n')

if __name__ == "__main__":
    PATH = "/home/jeromejguay/ImlSpace/Projects/mitis-buoy-controller/tests/"

    FN = "PMZA-RIKI_FileTAGS.dat"

    d = process_transmitted_data(PATH + FN)
