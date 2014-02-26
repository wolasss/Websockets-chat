# Websockets chat app

Chat written in C and JavaScript. Only for learning purposes. Own and not quite full implementation of websockets protocol in C.  

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

## I've Used

1. [ScaleApp](https://github.com/flosse/scaleApp) javascript architecture framework
2. [FlatUI](http://designmodo.github.io/Flat-UI/) 
3. [Nprogress](http://ricostacruz.com/nprogress/)
4. [Handlebars](http://handlebarsjs.com/)
5. [Modernizr](http://modernizr.com/)


## Live demo:
Try it out: [adamwolski.com/chat](http://adamwolski.com/chat)

Choose demo server.


### To do list:
 * rewrite server to C++ (OO)
 * logging via facebook, google+
 * sending/receiving files
