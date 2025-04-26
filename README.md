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
 <br>-->-->malware_log.txt
 <br>-->malware_hashes
    <br>-->malware_list.txt
 <br>-->packet_inspection
 <br>-->connection_monitor.exe
 <br>-->-->connection_monitor.exe
 <br>-->-->whitelisted_specific.txt
 <br>-->-->whitelisted_ips.txt
 <br>-->-->whitelisted_ports.txt
 <br>-->-->blocked_ips.txt
 <br>-->-->blocked_ports.txt
 <br>-->-->connection_logs.txt
 <br>-->phishing_prevention
    <br>phishing_prevention.exe
    <br>-->phishing_website_list.txt
    <br>warning.html
 <br>-->log_viewer.exe
<br>**Set up venv for compilation**
<br>Before you start, set up folder with required libraries for PyInstaller 3.5. You can get them here (as locating them took A LOT of time): **<a href=https://github.com/Martingonn/SentinelXP/tree/main/Build/Libraries/pyinstaller> (still on repo) </a>**
<br>For Pyinstaller 3.3 (better) download this: <a href="https://github.com/Martingonn/SentinelXP/blob/main/Build/Build%20Tools/pyinstaller33withLibs.iso">(still on repo)</a>
<br>Make sure you also have a folder with virtualenv: **<a href=https://github.com/Martingonn/SentinelXP/tree/main/Build/Libraries/virtualenv> (still on repo) </a>**
<br>__1.__ Download Python 2.7 on your device.
<br>__2.__ Run the installer.
<br>__3.__ Make sure Python path is C:\Python27 for ease. Otherwise you will have to change the path in commands.
<br>__4.__ Enter the uncompressed folder with virtualenv (where setup.py is located).
<br>__5.__ Run "*C:\Python27\python.exe setup.py install".* If you have a different Python 2.7 path, change "*C:\Python27\python.exe*" from now on.
<br>__6.__ Once virtualenv is installed, run these commands in the directory that you want your venv in:
<br>
<br>__*C:\Python27\Scripts\virtualenv.exe --python=C:\Python27\python.exe myenv*__ 
<br>
<br>__*.\myenv\Scripts\activate*__ or __*myenv\Scripts\activate*__
<br>
<br>To run a script in the venv, run *__C:\Python27\python.exe script_name.py__*
<br>__7.__ Go into folder with Pyinstaller libraries (still in venv).
<br>__8.__ Install the libraries furthest on the right first:
<br>**For example, Pyinstaller required Pefile, which requires Future**
<br>**Pyinstaller 3.5 (will be abandoned due to A LOT of dependencies and countless problems)**
<br>*|---setuptools-42.0.2
<br>*|---dis3-0.1.3
<br>*|---pywin32-ctypes-0.2.0*
<br>*|---altgraph-0.16.1*
<br>*|---pefile-2019.4.18*
<br>*|---|---future-0.18.2*
<br>**Pyinstaller 3.3 (less dependencies, still Python 2.7 though)**
<br>*|---|Setuptools(42.0.2 works)
<br>*|---|Pywin32-ctypes-0.2.0
<br>*|---|Macholib-1.8
<br>*|---|Pefile-2017.8.1
<br>***Note: install all of these packages manually***, as Python 2.7 has broken SSL compatibility and is unable to install the packages itself. This is how I did it.
<br>Test using *__pyinstaller --version__*
<br>NOTE: As of writing, it is unknown if Python 2.7 scripts compiled using Pyinstaller 3.5 on Windows 11 work on Windows XP.
<br>**Build tools on Windows XP**
<br>You will need:
<br>Pyinstaller (for compiling .py into .exe),
<br>Libraries for the script that you will be compiling (like compare_hashes)
<br>------------------------------------------
<br>***1. ***Install BuildTools.iso to your Windows XP machine. Here is the <a href=https://github.com/Martingonn/SentinelXP/blob/main/Build/Build%20Tools/BuildTools.iso>link (still on repo)</a>
<br>***2.*** Run the Python 2.7 installer, add it to System Path.
<br>***3.*** Extract all of the libraries.
<br>***4.*** Run "python setup.py install" in the library folder to install it.
<br>Install the libraries furthest on the right first:
<br>**Pyinstaller 3.5 (abandoned due to A LOT of dependencies and countless problems)**
<br>*|---pywin32-ctypes-0.2.0*
<br>*|---altgraph-0.17.4*
<br>*|---pefile-2017.8.1*
<br>*|---|---future-0.18.2*
<br>No worries about dependencies with dependencies with Pyinstaller 3.3:
<br>**Pyinstaller 3.3 (less dependencies, still Python 2.7 though)**
<br>*|---|Setuptools(42.0.2 works)
<br>*|---|Pywin32-ctypes-0.2.0
<br>*|---|Macholib-1.8
<br>*|---|Pefile-2017.8.1
<br>***5.*** Once you installed everything, run "pyinstaller --version"
<br>If Pyinstaller says you don't have a library that you actually have, make sure Python 2.7 is added to System Path and doesn't have compatibility issues with other Python versions (like Python 3.4 in my case).
<br>***6.*** Done! You can use Pyinstaller to compile .py into .exe. Make sure you have all the libraries required for that script, or the .exe will crash immediately!


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
