import os
import sys

# Hosts file path (Windows)
if os.name == 'nt':  # Windows
    hosts_path = r"C:\Windows\System32\drivers\etc\hosts"

redirect_ip = "127.0.0.1"

def load_websites_from_file(filename):
    try:
        with open(filename, 'r') as f:
            # Read lines, strip whitespace, and ignore empty lines
            websites = [line.strip() for line in f if line.strip()]
        return websites
    except IOError:
        print("File not found: {}".format(filename))
        sys.exit(1)
    except Exception as e:
        print("An error occurred while reading {}: {}".format(filename, e))
        sys.exit(1)

def block_websites(websites):
    try:
        with open(hosts_path, 'r+') as file:
            content = file.read()
            for website in websites:
                entry = "{} {}".format(redirect_ip, website)
                if entry not in content:
                    file.write("\n{}".format(entry))
                    print("Blocked {}".format(website))
                else:
                    print("{} is already blocked.".format(website))
    except IOError:
        print("Permission denied: Run this script as administrator/root.")
        sys.exit(1)
    except Exception as e:
        print("An error occurred: {}".format(e))
        sys.exit(1)

if __name__ == "__main__":
    website_list = raw_input("Input path to website list: ")
    websites_to_block = load_websites_from_file(website_list)
    block_websites(websites_to_block)
