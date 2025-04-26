import threading
import datetime
import traceback
import sys
import time
import os
import BaseHTTPServer

PORT = 80

class WarningHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def do_GET(self):
        # Log the requested path and client IP
        try:
            with open("access_log.txt", "a") as log_file:
                log_file.write("{} - {} requested {}\n".format(
                    datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
                    self.client_address[0],
                    self.path
                ))
        except Exception as e:
            # Optional: log file write errors to error_log.txt
            with open("error_log.txt", "a") as f:
                f.write("{} - Failed to write access log: {}\n".format(
                    datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"), str(e)
                ))

        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        with open("warning.html", "rb") as f:
            self.wfile.write(f.read())

def start_server():
    httpd = BaseHTTPServer.HTTPServer(("", PORT), WarningHandler)
    print "Serving warning page on port {}".format(PORT)
    httpd.serve_forever()

def start_server_thread():
    server_thread = threading.Thread(target=start_server)
    server_thread.daemon = True
    server_thread.start()

def main():
    if os.name == 'nt' and not is_admin():
        print "\nADMIN REQUIRED"
        print "=========================="
        print "This script needs Administrator rights to modify the hosts file."
        print "\nTo run properly:"
        print "1. Close this window"
        print "2. Right-click the script"
        print "3. Select 'Run as administrator'"
        time.sleep(5)
        sys.exit(1)

    # Start local server to serve warning.html
    start_server_thread()

    try:
        website_list = raw_input("Input path to website list: ")  # Python 2 input
        websites_to_block = load_websites_from_file(website_list)
        block_websites(websites_to_block)
        
        print "\nOperation completed. Press Enter to exit."
        raw_input()
    except KeyboardInterrupt:
        print "\nOperation cancelled by user"
        sys.exit(0)
    except Exception as e:
        with open("error_log.txt", "a") as f:
            timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            f.write("{} - {}\n".format(timestamp, traceback.format_exc()))
        print "\nAn error occurred. Details written to error_log.txt"
        raw_input("Press Enter to exit...")
        sys.exit(1)

# You still need to define or import is_admin(), load_websites_from_file(), block_websites() functions.

if __name__ == "__main__":
    try:
        main()
    except Exception:
        with open("error_log.txt", "a") as f:
            f.write(traceback.format_exc())
        print "\nCritical startup error. Details written to error_log.txt"
        raw_input("Press Enter to exit...")
        sys.exit(1)
