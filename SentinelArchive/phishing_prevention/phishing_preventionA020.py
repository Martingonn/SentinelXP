import os
import sys

# List of websites to block
websites_to_block = 

# Hosts file path (Windows and Unix-based systems)
if os.name == 'nt':  # Windows
    hosts_path = r"C:\Windows\System32\drivers\etc\hosts"
else:  # Linux, macOS
    hosts_path = "/etc/hosts"

redirect_ip = "127.0.0.1"

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
    block_websites(websites_to_block)