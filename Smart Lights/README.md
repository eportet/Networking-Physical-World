# Smart Lights
Nodes of LED lights connected to arduinos take HTTP requests from a server hosted at a Raspberry Pi. You can toggle specific lights using a website or the Alexa Skill.

## Video Overview
[![Smart Lights](http://img.youtube.com/vi/u1Jl-YBPr2g/0.jpg)](https://youtu.be/u1Jl-YBPr2g "Smart Lights")

## Setup
Start by SSH'ing into the Raspberry Pi  
Download this repository into the Pi  
Run `node xbeeChat.js /port/name` to read data from the XbeeCoordinator  
You can visit the Raspbery's Public IP address at Port 9000 to view the UI
