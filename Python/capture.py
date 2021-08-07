import socket
import sacn
import argparse
import json

version = "1.0.1"

def check_positive_int(value):
    ivalue = int(value)
    if ivalue <= 0:
        raise argparse.ArgumentTypeError("{} is an invalid positive int value".format(value))
    return ivalue


# program options
parser = argparse.ArgumentParser(description='record sACN E1.31 led data, and store it to file')
parser.add_argument('config_file', type=str,
                    help='config file that describe which univers to map to which strip')
parser.add_argument('out_file', type=str,
                    help='file to write frames into')
parser.add_argument('-f, --frames_to_capture', dest='frames_to_capture', action='store', type=check_positive_int,
                    help='if set, app will exit after capturing this amount of frames')
parser.add_argument('-n, --pixels_per_string', dest='pixels_per_string', action='store', type=check_positive_int,
                    default=1000, help='number of pixels on every string')
parser.add_argument('--number_of_strings', dest='number_of_strings', action='store', type=check_positive_int,
                    default=8, help='number of strings in the controller')
parser.add_argument('--port', dest='port', action='store', type=check_positive_int,
                    default=5568, help='port to listen for sACN data')
parser.add_argument('--addr', dest='addr', action='store', type=str,
                    default='0.0.0.0',
                    help='addr of interface to listen for sACN data (use 0.0.0.0 to bind to all interfaces)')
parser.add_argument('--version', action='version', version=version)
args = parser.parse_args()

# initialize rgb_data to store universe data until a full frame is received
channels_per_pixel = 3
total_pixels = args.number_of_strings * args.pixels_per_string
total_channels = total_pixels * channels_per_pixel
rgb_data = bytearray([0] * total_channels)

# read config data into uni_to_range
uni_to_range = {}
with open(args.config_file) as json_file:
    json_data = json.loads(json_file.read())
    for uni, uni_config in json_data.items():

        string_id = uni_config['string_id']
        if string_id < 0 or string_id >= args.number_of_strings:
            raise ValueError(
                "string_id read from config file on universe {} is not in valid range [0, {}], received: {}".format(uni, args.number_of_strings - 1, string_id))

        num_of_pixels = uni_config['num_of_pixels']
        if num_of_pixels > 170:
            raise ValueError(
                "num_of_pixels read from config file on universe {} is too high for sACN. "
                "should be <= 170, received: {}".format(uni, num_of_pixels))

        pixel_in_string = uni_config['pixel_in_string']
        last_pixel_index = pixel_in_string + num_of_pixels
        if pixel_in_string < 0:
            raise ValueError("pixel_in_string for universe {} is negative, received: {}".format(uni, pixel_in_string))
        if last_pixel_index >= args.pixels_per_string:
            raise ValueError(
                "pixel_in_string read from config on universe {} file is not in valid range."
                "the value is {}, and after adding num_of_pixels which is {}, we receive {}, which is >= than "
                "total number of pixels in string {}"
                .format(uni, pixel_in_string, num_of_pixels, last_pixel_index, args.pixels_per_string))

        start_index = (string_id * args.pixels_per_string + pixel_in_string) * channels_per_pixel

        num_of_channels = num_of_pixels * channels_per_pixel
        uni_to_range[int(uni)] = (start_index, num_of_channels)
print("read config file {}, will monitor the following universes: {}".format(args.config_file, list(uni_to_range.keys())))
if args.frames_to_capture:
    print("will read {} frames and then quit. you can quit earlier if you want.".format(args.frames_to_capture))

# open udp recv socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # UDP
sock.bind((args.addr, args.port))

# handle write to file
f = open(args.out_file, "w+b")
recv_uni = set()
total_frames = 0
non_manged_uni = set()

while True:
    raw_data, _= sock.recvfrom(1144)  # 1144 because the longest possible packet
    try:
        sacn_packet = sacn.DataPacket.make_data_packet(raw_data)
    except:  # try to make a DataPacket. If it fails just go over it
        continue

    current_uni = sacn_packet.universe

    if current_uni not in uni_to_range:
        if current_uni not in non_manged_uni:
            print("recived a universe '{}' which is not used by the application. ignoring it. this might be a config mistake".format(current_uni))
            non_manged_uni.add(current_uni)
        continue

    arr_range = uni_to_range[current_uni]
    rgb_data[arr_range[0] : arr_range[0] + arr_range[1]] = bytearray(sacn_packet.dmxData[0 : arr_range[1]])

    #try to find config errors (universe in config which is not reported on network)
    if current_uni in recv_uni:
        missing_universe = [u for u in uni_to_range.keys() if u not in recv_uni]
        print("received universe {} while other universes are not found: {}. "
              "if you see this message too often it means you set these universe in the config file but do not output them to network. "
              "remove these universe from the config, or make sure they are sent correctly."
              .format(current_uni, missing_universe))
    recv_uni.add(current_uni)

    if len(recv_uni) == len(uni_to_range):
        f.write(rgb_data)
        total_frames += 1
        if args.frames_to_capture and total_frames >= args.frames_to_capture:
            print("captured {} frames. that's it".format(args.frames_to_capture))
            break
        if total_frames % 100 == 0:
            print('wrote {} frames to file so far'.format(total_frames))
        recv_uni.clear()
