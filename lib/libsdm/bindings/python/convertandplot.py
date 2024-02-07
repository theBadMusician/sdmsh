import numpy as np
import matplotlib.pyplot as plt

def load_and_convert_data(file_path, is_uint_data=True):
    """
    Load data from a text file, convert it from integers to a float range of -1.0 to 1.0,
    and return the converted data.
    """
    with open(file_path, 'r') as file:
        data_text = file.readlines()
    if is_uint_data:
        data_int = np.array([int(line.strip()) for line in data_text])
        # Convert and scale the data to the range -1.0 to 1.0
        data = (data_int / 65535) * 2 - 1
    else:
        data = np.array([float(line.strip()) for line in data_text])

    return data

def plot_data(data):
    """
    Plot the given data.
    """
    plt.figure(figsize=(10, 6))
    plt.plot(data)
    plt.title('Scaled Data (-1.0 to 1.0)')
    plt.xlabel('Sample Index')
    plt.ylabel('Scaled Value')
    plt.grid(True)
    plt.show()

def main():
    # Path to the .dat file
    file_path = "/home/beng/Projects/sdmsh/examples/rcv-test.dat"
    # file_path = "/home/beng/Projects/sdmsh/examples/1834_polychirp_re_up.dat"
    
    # Load and convert the data
    data_scaled = load_and_convert_data(file_path, True)[:1024]
    
    # Plot the data
    plot_data(data_scaled)

if __name__ == "__main__":
    main()
