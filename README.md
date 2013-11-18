# Websockets chat app

## Client

Run client/index.html

## Server 

Works properly on MAC OS X >= 10.7, Linux (tested on Debian 6.0). On linux following packages are required:
* libimobiledevice-dev* libplist-dev* libusbmuxd-dev* libssl-dev

To run a server type following commands:

1. `cd server`
2. `./configure`
3. `make`
4. `./server [port]`

## Live demo:
Try it out: [adamwolski.com/chat](http://adamwolski.com/chat)

Choose demo server.

### To do list:
 * rewrite server to C++
 * logging via facebook, google+
 * sending/receiving files
