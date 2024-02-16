"""
 <buoy name>_SD_<date>.dat

    #1 Name of the buoy
    #2 date buoy = 2017/02/28
    #3 hour buoy = 00:00:46
    #4 latitude buoy = 48 28.97N
    #5 longitude buoy - 68 29.99W
    #6 speed of the wind in km/h = 5
    #7 maximal speed of the wind in km/h = 6
    #8 Wind direction in degree = 177
    #9 air temperature in degree Celsius = 24.1
    #10 air humidity relative in % = 31
    #11 air pressure in millibar = 1028.6
    #12 water temperature in degree Celsius = 2.3
    #13 water salinity in ppm = 12.0
    #14 Density of water =1016.3 in Kg/cubic meter
    #15 Fluorescent at 700nm (Scattering) in m-1 = 7.208E-03
    #16 Fluorescent at 695nm (Chlorophyll) in µg/L = 1.098E+01
    #17 Fluorescent at 460nm (Fluorescent Dissolved Organic Matter (FDOM)) in ppb = 2.822E+02
    #18 PAR value in µmol photons•m-2•s-1 = 20
    #19 Co2 in the water in ppm = 103.6 ppm
    #20 Co2 in the air in ppm = 103.6 ppm
    #21 pH = 7.6543
    #22 Wave, period in second = 8.0
    #23 Wave, average height in meter = 1.2
    #24 Wave, height of the biggest wave in meter = 2.3
    #25 Voltage of the batteries in Volt = 13.0
    #26 Power of the charging solar in ampere = 0.2
    #27 Power of the charging Wind turbine in ampere = 0.5
    #28 Power consuming in ampere = 0.9
    #29 pitch in degree (compass) = 0
    #30 roll in degree (compass) = 0
    #31 Power flow of the surface measured in m/s = 2.1
    #32 Heading buoy (compass) in degree = 128
    #33 Moving speed (GPS) in m/s = 0.0
    #34 Moving direction (GPS) in degree =264
    #35 Rain accumulation since midnight = #.#
    #36 Current (ADCP RTI or RDI) bin #1 in m/s = 2.4
    #37 Current direction (ADCP RTI ou RDI) in degree = 358
    #38 Water presence in the buoy controller box (0= no water, 1= water)
    #39 Water presence in the Power controller box (0= no water, 1= water)
    #40 Water presence in the Winch controller box (0= no water, 1= water)

Notes
-----
    Wind velocities are in knots,
    Velocities in "mm/s"


"""
import re
import json
import math
from typing import Dict, List
from pathlib import Path

from mitis_python_tools.TAG_reader import read_TAG_file

_KNOTS_TO_KPH = 1.852
_MMS_TO_MS = 1 / 1000


SD_PADDINGS = {
    0: 0,
    1: 0,
    2: 0,
    3: 0,
    4: 0,
    5: 2,
    6: 2,
    7: 3,
    8: 4,
    9: 3,
    10: 6,
    11: 5,
    12: 5,
    13: 5,
    14: 9,
    15: 6,
    16: 5,
    17: 4,
    18: 5,  #6?
    19: 5,  #6?
    20: 6,
    21: 4,
    22: 4,
    23: 4,
    24: 4,
    25: 4,
    26: 4,
    27: 4,
    28: 3,
    29: 3,
    30: 0,
    31: 3,
    32: 4,
    33: 3,
    34: 4,
    35: 3,
    36: 3,
    37: 0,
    38: 0,
    38: 0
}


class FilePointer:
    """Allows to read and update pointer value in pointer file."""
    pointer_key = "pointer"

    def __init__(self, filename: str):
        self.filename = filename

        self._ensure_file_exist()

    @property
    def value(self):
        return self._get_value()

    def increment(self, increment=1):
        self._update(self.value + increment)

    def _get_value(self):
        with open(self.filename, 'r') as f:
            value = json.load(f)[self.pointer_key]
        return value

    def _update(self, value):
        with open(self.filename, "w") as f:
            json.dump({self.pointer_key: value}, f, indent=4)

    def _ensure_file_exist(self):
        if not Path(self.filename).exists():
            self._update(0)


def process_SD(input_file: str, target_directory: str, sd_padding: bool, pointer: FilePointer):
    """Append SD String to the target_directory/SD_file.

    :param: filename : str
    :param: target_directory : str
    :param: sd_padding : If True, SD string's values will be padded.
    :param: pointers_file : str

    """
    data = read_TAG_file(filename=input_file, pointer_location=pointer.value)

    if data:
        station_name = data[0]['init']['buoy_name']
        station_directory = Path(target_directory).joinpath(station_name)
        Path(station_directory).mkdir(parents=True, exist_ok=True)

        for d in data:
            SD_data_string = _make_SD_string(data=d, sd_padding=sd_padding)
            SD_filename = f"{station_name}_SD_{d['init']['date'].replace('-', '')}.dat"
            SD_target_file = station_directory.joinpath(SD_filename)

            _write_SD(dest_file=SD_target_file, sd_string=SD_data_string)

            pointer.increment()


def _write_SD(dest_file: str, sd_string: str):
    """Append data to the end of dest_file.

    :param dest_file: SD file to append to.
    :param sd_string: SD data string.
    """
    with open(dest_file, 'a') as f:
        f.write(sd_string + '\n')


def _format_lonlat(value: str):
    """ Round and format lon-lat string
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


def _make_SD_string(data: Dict[str, List[str]], sd_padding: bool) -> str:
    """Make SD string from unpacked Mitis Tag Data
    """
    sd_data = ["#"] * 40

    if "init" in data:
        sd_data[0] = data["init"]['buoy_name']  # Buoy_Name
        sd_data[1] = data["init"]['date'].replace("-", "/")
        sd_data[2] = data["init"]['time']  # Buoy_Hour

        sd_data[3] = _format_lonlat(data['init']['latitude'])
        sd_data[4] = _format_lonlat(data['init']['longitude'])

        sd_data[31] = f"{float(data['init']['heading']):.0f}"
        sd_data[28] = f"{float(data['init']['pitch']):.1f}"
        sd_data[29] = f"{float(data['init']['roll']):.1f}"

        sd_data[33] = f"{float(data['init']['cog']) % 360:.0f}"
        sd_data[32] = f"{float(data['init']['sog']):.1f}"

        _water_detection = float(data['init']['water_detection'])
        if _water_detection < 2000 and not math.isnan(_water_detection):
            _water_detection = 0
        sd_data[37] = f"{_water_detection:.0f}"

    if "powr" in data:
        _max_battery = max(float(data['powr']['volt_batt_1']), float(data['powr']['volt_batt_2']))
        _sum_amp = (float(data['powr']['amp_main']) + float(data['powr']['amp_winch']))

        sd_data[24] = f"{_max_battery:.1f}"
        sd_data[25] = f"{float(data['powr']['amp_solar']):.1f}"
        sd_data[26] = f"{float(data['powr']['amp_turbine']):.1f}"
        sd_data[27] = f"{_sum_amp:.1f}"
        sd_data[38] = data['powr']['relay_state'][7]

    if "eco1" in data:
        sd_data[14] = f"{float(data['eco1']['scattering']):.7f}"
        sd_data[15] = f"{float(data['eco1']['chlorophyll']):.4f}"
        sd_data[16] = f"{float(data['eco1']['fdom']):.2f}"

    if "ctd" in data:
        sd_data[11] = f"{float(data['ctd']['temperature']):.2f}"
        sd_data[12] = f"{float(data['ctd']['salinity']):.2f}"
        sd_data[13] = f"{float(data['ctd']['density']):.2f}"

    if "ph" in data:
        sd_data[20] = f"{float(data['ph']['ext_ph_calc']):.4f}"

    if "wind" in data:
        sd_data[5] = f"{float(data['wind']['wind_spd_ave']) * _KNOTS_TO_KPH:.0f}"
        sd_data[6] = f"{float(data['wind']['wind_spd_max']) * _KNOTS_TO_KPH:.0f}"
        sd_data[7] = f"{float(data['wind']['wind_dir_ave']):.0f}"

    if "atms" in data:
        sd_data[8] = f"{float(data['atms']['air_temperature']):.1f}"
        sd_data[9] = f"{float(data['atms']['air_humidity']):.0f}"
        sd_data[10] = f"{float(data['atms']['air_pressure']):.1f}"
        sd_data[17] = f"{float(data['atms']['par']):.0f}"
        sd_data[34] = f"{float(data['atms']['rain_total']):.1f}"

    if "wave" in data:
        sd_data[21] = f"{float(data['wave']['period']):.1f}"
        sd_data[22] = f"{float(data['wave']['h13']):.1f}"
        sd_data[23] = f"{float(data['wave']['hmax']):.1f}"

    if "pco2" in data:
        sd_data[18] = f"{float(data['pco2']['co2_water']):.1f}"
        sd_data[19] = f"{float(data['pco2']['co2_air']):.1f}"

    if 'adcp' in data:
        _u = float(data['adcp']['u'])
        _v = float(data['adcp']['v'])
        _uv = math.sqrt(_u ** 2 + _v ** 2) * _MMS_TO_MS
        _dir = math.atan2(_u, _v) % 360

        sd_data[35] = f"{_uv:.1f}"
        sd_data[36] = f"{_dir:.0f}"

    # fixme replace nan not need for try, except

    for index, value in enumerate(sd_data):
        if value in ['nan', 'NA']:
            sd_data[index] = '#'

    if sd_padding is True:
        for index, _just in SD_PADDINGS.items():
            sd_data[index] = sd_data[index].rjust(_just)

    return ','.join(sd_data)
