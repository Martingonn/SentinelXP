# SentinelXP
An antivirus for Windows XP focusing on diagnostics and user choice. Free, forever, no spyware, no bloatware.
# What is it?
There will be a few scripts combined by an interface.
# Original Author 
Development was started on April 13th, 2025, by Marcin Jacek Chmiel.
# Contributors 
 <br>***Code***<br>
As of now, there are no more contributors than the original author.
If you have any problems or suggestions, contact me: *martingonn-dev@outlook.com*
 <br>***Encouragement***
 <br><a href="https://www.xpforums.com/members/xperceniol.2881/">xperceniol</a> over on XP forums
 <br><a href="https://www.xpforums.com/members/cleverscreenname.3089/">cleverscreenname </a>on XP Forums
 <br><a href="https://www.xpforums.com/members/boss1968.3085/">boss1968 </a>on XP Forums
 # How can you contribute?
 Simply make a contribution here, on Github. Another, better way, would be to send me links to malware repositories so that I can train the antivirus.
 <br> Remember, you have to send me the links/files yourself, as I have no server to send the files to. When I will have, I will only send out file hashes. You will be able to see that, as the project will be open-source forever.
 <br>**You can also join the conversation here:** https://www.xpforums.com/conversations/windowsxp-antivirus.272/
# Disclaimer!!!
**We are not viable for any damages you sustain due to malware!!!**
**The best antivirus is staying vigilant with what you do in the internet!!!**
# How to use
  **Just run the executable!**
# Build from source
<br>**Structure**
<br>**C++**
<br>Located here: https://github.com/Martingonn/SentinelXP/blob/main/Sentinel_CPP_Archive/structure.txt
<br>Once finished, will be pasted over.
# C++ 98
<br>Use gcc 3.4.5 from MinGW on Windows XP.
# Archival - Python
<br>*Leaving this in because I spent over 35 hours on this.
<br>You will need:
<br>__Py2Exe__
<br>You can get Py2Exe as .iso file mountable to VM here: <a href="https://github.com/Martingonn/SentinelXP/blob/main/Build/Build%20Tools/py2exe-0.6.9.iso">
<br>*__Steps:__*
<br>**1.** Mount py2exe .iso into your Windows XP
<br>**2.** Run the installer.
<br>**3.** Once you installed Py2Exe, make a "setupy.py" that looks like this:
<br>from distutils.core import setup
<br>import py2exe
<br>
<br>setup(console=['yourscript.py'])
<br>**4.** In the same directory, run:
<br>*python setup.py py2exe
<br>**5.** Done! 
<br> I wasted over 35 hours on this... trying out different ways... until finally...

# Future Additions
* Packet inspection
* Monitoring port connections
* Opening .exe and inspecting contents
* Anti-phishing website module
* GUI version (maybe)
* Menu module to operate on other modules
* Sneaky download prevention
* A LOT MORE
For more, check out ideas.txt: https://github.com/Martingonn/SentinelXP/blob/main/Sentinel_CPP_Archive/ideas.txt

# Downloads
![GitHub All Releases](https://img.shields.io/github/downloads/Martingonn/SentinelXP/total)
