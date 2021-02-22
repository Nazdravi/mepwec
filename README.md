# mepwec

MEdia Player WEb Control (control photo, audio and video play from local sources via web interface)

## mepwec consists of three main components:
* a service component `"playersvc"` that starts, controls and kills players and is controllable via a pseudo terminal
* server-side lua code for the web interface, running under nginx web server
* web-page (Bootstrap4-based) with embedded javascript

> In the development installation this setup is installed on a raspberry Pi4 connected to a TV screen.

![alt text](https://github.com/Nazdravi/mepwec/blob/main/Media_Play_Web_Control.png "Screenshot with active slideshow and audio")

### The service component `"playersvc"`
is startet with autostart as user "pi" from the graphical user user interface (`lxterminal --command="<PATH TO APPLICATION>/playersvc"`). The service component exposes its command interface over a pseudo terminal thats name can be read from `/var/run/playersvc.pts`. These commands can be issued to the service component:

```
Commands summary
 exit|ex|x|quit|q
 help|?
 open foto|photo|audio|video <FILE>
 enqueue|enq audio <FILE>
 close foto|photo|audio|video
 playerctl|play foto|photo|audio|video start|play|stop|pause|playerctl|play|previous|prev|next
 playerctl|play list
 playerctl|play status
 playerctl|play metadata
 playerctl|play allstatus
 playerctl|play slideshow|slides[ stop]
 display|disp on|off
 paths|path
```
`playersvc` uses Gwenview for photos, SMplayer for audio and VLC for video. `/etc/playersvc.conf` defines the root paths to media content which can be browsed through the web interface, e.g.:
```
pi@media-pi:~ $ cat /etc/playersvc.conf 
/<PATH TO PHOTOS DIRECTORY>/
/<PATH TO AUDIO CONTENT>/
/<PATH TO VIDEO DIRECTORIES>/
```
For separate testing the server components pseudo terminal can be connected to by some terminal application (e.g. minicom) and commands (as described above) can be issued.

### The web server lua application (runs within nginx)
The server-side program installation consists of then `file-walk` and `pts-interact` lua-scripts. The path to the lua scripts has to be specified in the nginx configuration file `nginx.conf`: `lua_package_path "/var/www/lua/?.lua;;";`. The nginx default server configuration `default` defines the two locations `/` and `/media` which deliver the index page and the command interface.\
The server-side lua scripts add file browsing functionality `ls`-command to the service component's functions.

### Index page / user interface
`index.shtml` contains all client-side presentation stuff. This page uses SSI and is built on Bootstrap4 (at the moment nearly no work has been spent on design issues).
