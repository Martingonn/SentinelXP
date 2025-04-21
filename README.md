# SentinelXP
An antivirus for Windows XP. Free, forever, no spyware, no bloatware.
# What is it?
There will be a few scripts combined by an interface.
# Original Author 
Development was started on April 13th, 2025, by Marcin Jacek Chmiel.
# Contributors 
 <br>***Code***<br>
As of now, there are no more contributors than the original author.
If you have any problems or suggestions, contact me: *martingonn-dev@outlook.com*
 <br>***Encouragement***<br>
 xperceniol over on WindowsXP forums: https://www.xpforums.com/members/xperceniol.2881/
 # How can you contribute?
 Simply make a contribution here, on Github. Another, better way, would be to send me links to malware repositories so that I can train the antivirus.
 <br> Remember, you have to send me the links/files yourself, as I have no server to send the files to. When I will have, I will only send out file hashes. You will be able to see that, as the project will be open-source forever.
# Disclaimer!!!
**We are not viable for any damages you sustain due to malware!!!**
**The best antivirus is staying vigilant with what you do in the internet!!!**
# How to use
  **Just run the executable!**
# Build from source
<br>**Structure**
 <br>SentinelXP
 <br>-->SentinelInterface.exe
 <br>-->compare_hashes
 <br>-->-->detected_malware.txt
 <br>-->malware_hashes
    <br>-->malware_hashes_list.txt
 <br>-->packet_inspection
 <br>-->connection_monitor.exe
 <br>-->-->connection_monitor.exe
 <br>-->-->whitelisted_specific.txt
 <br>-->-->whitelisted_ips.txt
 <br>-->-->whitelisted_ports.txt
 <br>-->-->blocked_ips.txt
 <br>-->-->blocked_ports.txt
 <br>-->phishing_prevention
    <br>-->phishing_website_list.txt
    <br>warning.html
<br>**Build tools**
<br>You will need:
<br>Pyinstaller (for compiling .py into .exe),
<br>Libraries for the script that you will be compiling (like compare_hashes)
<br>------------------------------------------
<br>1. Install BuildTools.iso to your Windows XP machine. Here is the <a href=https://github.com/Martingonn/SentinelXP/blob/main/Build/Build%20Tools/BuildTools.iso>link (still on repo)</a>
<br>2. Run the Python 2.7 installer, add it to System Path.
<br>3. Extract all of the libraries.
<br>4. Run "python setup.py install" in the library folder to install it.
<br>Install the libraries furthest on the right first:
<br>**Pyinstaller 3.5**
<br>*|---pywin32-ctypes-0.2.0*
<br>*|---altgraph-0.17.4*
<br>*|---pefile-2017.8.1*
<br>*|---|---future-0.18.2*
<br>5. Once you installed everything, run "pyinstaller --version"
<br>If Pyinstaller says you don't have a library that you actually have, make sure Python 2.7 is added to System Path and doesn't have compatibility issues with other Python versions (like Python 3.4 in my case).
<br>6. Done! You can use Pyinstaller to compile .py into .exe. Make sure you have all the libraries required for that script, or the .exe will crash immediately!

# Future Additions
* Packet inspection
* Monitoring port connections
* Opening .exe and inspecting contents
* Anti-phishing website module
* GUI version (maybe)
* Menu module to operate on other modules
* Sneaky download prevention

# Downloads
![GitHub All Releases](https://img.shields.io/github/downloads/Martingonn/SentinelXP/total)
