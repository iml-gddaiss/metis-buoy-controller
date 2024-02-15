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
[END]
"""

import re
from typing import Dict, List

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


def read_TAG_file(filename: str, pointer_location: int = 0) -> List[Dict[str, dict[str, str]]]:
    """
    :param pointer_location: Line at which to start unpacking data.
    :param filename: Path to file

    """

    unpacked_data = []
    with open(filename, 'r') as f:
        for _ in range(pointer_location):
            next(f)

        for line in f:
            unpacked_data.append(_unpack_data_from_tag_string(line))

    return unpacked_data


def _unpack_data_from_tag_string(data: str) -> Dict[str, dict[str, str]]:
    """Unpack Mitis Tag Data
    Returns Data as a dictionary of {TAG:DATA}
    """

    unpacked_data = {}
    for data_sequence in DATA_TAG_REGEX.finditer(data):
        tag = data_sequence.group(1).lower()
        data = data_sequence.group(2).split(",")

        unpacked_data[tag] = {key: value for key, value in zip(TAG_VARIABLES[tag], data)}

    return unpacked_data