import os
import sys

# Hosts file path (Windows and Unix-based systems)
if os.name == 'nt':  # Windows
    hosts_path = r"C:\Windows\System32\drivers\etc\hosts"
else:  # Linux, macOS
    hosts_path = "/etc/hosts"

redirect_ip = "127.0.0.1"

def load_websites_from_file(filename):
    try:
        with open(filename, 'r') as f:
            # Read lines, strip whitespace, and ignore empty lines
            websites = [line.strip() for line in f if line.strip()]
        return websites
    except FileNotFoundError:
        print(f"File not found: {filename}")
        sys.exit(1)
    except Exception as e:
        print(f"An error occurred while reading {filename}: {e}")
        sys.exit(1)

def block_websites(websites):
    try:
        with open(hosts_path, 'r+') as file:
            content = file.read()
            for website in websites:
                entry = f"{redirect_ip} {website}"
                if entry not in content:
                    file.write(f"\n{entry}")
                    print(f"Blocked {website}")
                else:
                    print(f"{website} is already blocked.")
    except PermissionError:
        print("Permission denied: Run this script as administrator/root.")
        sys.exit(1)
    except Exception as e:
        print(f"An error occurred: {e}")
        sys.exit(1)

if __name__ == "__main__":
    # Change 'websites.txt' to your actual filename
    website_list = str(input("Input path to website list: "))
    websites_to_block = load_websites_from_file(website_list)
    block_websites(websites_to_block)
