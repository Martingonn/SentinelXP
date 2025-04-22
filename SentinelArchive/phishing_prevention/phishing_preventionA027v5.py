import os
import sys
import ctypes
import time
import traceback

# Hosts file path (Windows)
if os.name == 'nt':  # Windows
    hosts_path = r"C:\Windows\System32\drivers\etc\hosts"

redirect_ip = "127.0.0.1"

def is_admin():
    """Check if running as admin"""
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

def block_websites(websites):
    try:
        # First read current content
        with open(hosts_path, 'r') as f:
            existing_content = f.read()
        
        # Then append new entries if needed
        with open(hosts_path, 'a') as file:
            for website in websites:
                entry = "\n{} {}".format(redirect_ip, website)
                if entry not in existing_content:
                    file.write(entry)
                    print("Blocked {}".format(website))
                else:
                    print("Already blocked: {}".format(website))
    
    except IOError as e:
        error_msg = "FATAL ERROR: Hosts file modification failed\nDetails: {}\n".format(e)
        error_msg += "Possible causes:\n"
        error_msg += "- Hosts file is read-only\n"
        error_msg += "- Antivirus blocking modifications\n"
        error_msg += "- File permissions corrupted\n"
        print(error_msg)
        raise  # Re-raise for logging
    except Exception as e:
        print("Critical error: {}".format(e))
        raise  # Re-raise for logging

def load_websites_from_file(filename):
    try:
        with open(filename, 'r') as f:
            websites = [line.strip() for line in f if line.strip()]
        return websites
    except IOError:
        print("File not found: {}".format(filename))
        raise
    except Exception as e:
        print("Error reading {}: {}".format(filename, e))
        raise

def main():
    if os.name == 'nt' and not is_admin():
        print("\nADMIN REQUIRED")
        print("==========================")
        print("This script needs Administrator rights to modify the hosts file.")
        print("\nTo run properly:")
        print("1. Close this window")
        print("2. Right-click the script")
        print("3. Select 'Run as administrator'")
        time.sleep(5)
        sys.exit(1)

    try:
        website_list = raw_input("Input path to website list: ")
        websites_to_block = load_websites_from_file(website_list)
        block_websites(websites_to_block)
        
        print("\nOperation completed. Press Enter to exit.")
        raw_input()
    except KeyboardInterrupt:
        print("\nOperation cancelled by user")
        sys.exit(0)
    except Exception as e:
        with open("error_log.txt", "a") as f:
            timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            f.write(f"{timestamp} - {traceback.format_exc()}\n")
        print("\nAn error occurred. Details written to error_log.txt")
        input("Press Enter to exit...")
        sys.exit(1)

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        with open("error_log.txt", "w") as f:
            f.write(traceback.format_exc())
        print("\nCritical startup error. Details written to error_log.txt")
        raw_input("Press Enter to exit...")
        sys.exit(1)