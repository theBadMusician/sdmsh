import struct

def convert_to_int16(float_val):
    # Scale the float value to fit in int16
    return int(float_val * 32767)

def process_file(input_file, output_file):
    with open(input_file, 'r') as file:
        with open(output_file, 'wb') as bin_file:
            for line in file:
                # Convert line to float and then to int16
                float_val = float(line.strip())
                int_val = convert_to_int16(float_val)
                # Write to binary file
                bin_file.write(struct.pack('h', int_val))

# Example usage
input_file = "1834_polychirp_re_up.dat"
output_file = 'chirp.bin'
process_file(input_file, output_file)
