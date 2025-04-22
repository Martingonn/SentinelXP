def print_file_contents(filename):
    try:
        with open(filename, 'r') as f:
            print("Viewing contents of {}:".format(filename))
            print(f.read())
    except IOError as e:
        print("Error: Could not open file '{}'. Reason: {}".format(filename, e))

def main():
    files = {
        '1': ('Detected Malware', 'compare_hashes/malware_log.txt'),
        '2': ('Blocked Ports', 'connection_monitor/blocked_ports.txt'),
        '3': ('Blocked IPs', 'connection_monitor/blocked_ips.txt'),
        '4': ('Whitelisted Ports', 'connection_monitor/whitelisted_ports.txt'),
        '5': ('Whitelisted IPs', 'connection_monitor/whitelisted_ips.txt'),
        '6': ('Whitelisted IPs + Ports', 'connection_monitor/whitelisted_specific.txt'),
    }

    while True:
        print("\n---Log Viewer from Sentinel XP---")
        print("1. Detected Malware")
        print("2. Blocked Ports")
        print("3. Blocked IPs")
        print("4. Whitelisted Ports")
        print("5. Whitelisted IPs")
        print("6. Whitelisted IPs + Ports")
        print("0. Exit")

        user_input = raw_input("Enter number to view log (or 0 to exit): ").strip()

        if user_input == '0':
            print("Exiting Log Viewer. Goodbye!")
            break
        elif user_input in files:
            desc, filename = files[user_input]
            print_file_contents(filename)
            print("---------------------------------------")
        else:
            print("Invalid input. Please enter a number between 0 and 6.")

if __name__ == "__main__":
    main()