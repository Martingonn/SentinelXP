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
 <br>-->malware_hashes
    <br>-->malware_hashes_list.txt
 <br>-->packet_inspection
 <br>-->port_scanning
 <br>-->phishing_prevention
    <br>-->phishing_website_list.txt
 <br>-->logs
    <br>-->malware_on_device.txt
    <br>-->malicious_packets.txt
    <br>-->connections.txt
<br>**Build tools**
You will need:
<br>MinGW,
<br>simhash-2.1.2 (uncompressed)
<br>python3.4
1. Put these on USB/combine into .iso file.
2. On Windows XP, put MinGW folder into C: folder.
3. Add ";C:\mingw32\bin;C:\mingw32" to system "Path" variable.
4. Get into Python library folder (for example: simhash).
5. Run setup.py


# Future Additions
* Packet inspection
* Monitoring port connections
* Opening .exe and inspecting contents
* Anti-phishing website module

# Downloads
![GitHub All Releases](https://img.shields.io/github/downloads/Martingonn/SentinelXP/total)
