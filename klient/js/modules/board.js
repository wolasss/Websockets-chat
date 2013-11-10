var MODboard = function(sb){

    "use strict";
    var container, show, receiveMessage, hide, username, leaveRoom, switchRoom, receiveNotification, newPrivateRoom, currentRoom, receivePrivMessage, newPublicRoom;

    show = function(data) {
        var mainRoom = '<ul class=\"room room_main active\"></ul>';
        sb.clear(container);
        sb.append(container, mainRoom);
        currentRoom = sb.find('.room_main')[0];
        receiveNotification({ room:'__CURRENT__', message: '<strong>Welcome to Websockets chat!</strong><br>Now, you are in the main room. If you need any help, just type: /help' });
        username = data;
        sb.addClass(sb.CSSchat, 'expanded');
        setTimeout(sb.fadeToggleModule,600);
    };
    hide = function() {
        sb.removeClass(sb.CSSchat, 'expanded');
        sb.toggleModule();
    };
    newPrivateRoom = function(user) {
        user = sb.escapeHTML(user);
        if(sb.find('.room_private_'+user).length===0) {
            var tpl = "<ul class=\"room room_private_"+user+"\"></ul>";
            sb.append(container, tpl);
        }
    };
    newPublicRoom = function(name) {
        name = sb.escapeHTML(name);
        if(sb.find('.room_'+name).length===0) {
            var tpl = "<ul class=\"room room_"+name+"\"></ul>";
            sb.append(container, tpl);
        }
    };
    leaveRoom = function(name) {
        var room = sb.find('.room_'+name);
        if(room.length!==0) {
            sb.remove(room[0]);
        }
    };
    switchRoom = function(data) {
        var type = data.type, room = data.name,
        target = (type==='private') ? sb.find('.room_private_'+room)[0] : sb.find('.room_'+room)[0];
        if(target.length!==0) {
            sb.removeClass(currentRoom, 'active');
            currentRoom = target;
            sb.addClass(target, 'active');
            if(type==='private') {
                sb.emit('PrivateReadMessages', room);
            } else {
                sb.emit('PublicReadMessages', room);
            }
            sb.scrollTop(currentRoom, currentRoom.scrollHeight);
        }
    };
    receivePrivMessage = function(data) {
        var message = sb.escapeHTML(data.message),
            roomDOM,
            room = data.room,
            sender = sb.escapeHTML(data.sender),
            now = new Date(),
            additionalClass = '';
            if(username === sender) { additionalClass+='mine'; }
            if(data.sender === "thefox") { additionalClass+=' fox'; }
            var msgTemplate = '<li class="msgContainer clearfix"><div class="avatar '+additionalClass+'"><div class="nick">'+sender+'</div></div><div class="message '+additionalClass+'"><div class="bubble">'+message+'<div class="info">'+now.toString().match(/\d\d:\d\d:\d\d/)[0]+'</div></div></div></li>';
            roomDOM = sb.find('.room_private_'+room);
            if(roomDOM.length!==0) {
                if((roomDOM[0].scrollTop + roomDOM[0].offsetHeight)>435 && (roomDOM[0].scrollTop + roomDOM[0].offsetHeight)<roomDOM[0].scrollHeight ) {
                    sb.append(roomDOM[0], msgTemplate);
                } else {
                    sb.append(roomDOM[0], msgTemplate);
                    sb.scrollTop(roomDOM[0], roomDOM[0].scrollHeight);
                }
                if( (!roomDOM[0].isEqualNode(currentRoom)) && (sender != username)) {
                    sb.emit('PrivateUnreadMessage', sender);
                }
            } else {
                newPrivateRoom(data.room);
                roomDOM = sb.find('.room_private_'+room);
                sb.append(roomDOM, msgTemplate);
                sb.scrollTop(roomDOM, roomDOM.scrollHeight);
                sb.emit('PrivateUnreadMessage', sender);
            }
    };
    receiveMessage = function(data) {
        var message = sb.escapeHTML(data.message),
            roomDOM,
            room = data.room,
            sender = sb.escapeHTML(data.sender),
            now = new Date(),
            additionalClass = '';
            if(username === sender) { additionalClass+='mine'; }
            if(data.sender === "thefox") { additionalClass+=' fox'; }
            var msgTemplate = '<li class="msgContainer clearfix"><div class="avatar '+additionalClass+'"><div class="nick">'+sender+'</div></div><div class="message '+additionalClass+'"><div class="bubble">'+message+'<div class="info">'+now.toString().match(/\d\d:\d\d:\d\d/)[0]+'</div></div></div></li>';
            roomDOM = sb.find('.room_'+room);
            if(roomDOM.length!==0) {
                if((roomDOM[0].scrollTop + roomDOM[0].offsetHeight)>435 && (roomDOM[0].scrollTop + roomDOM[0].offsetHeight)<roomDOM[0].scrollHeight ) {
                    sb.append(roomDOM[0], msgTemplate);
                } else {
                    sb.append(roomDOM[0], msgTemplate);
                    sb.scrollTop(roomDOM[0], roomDOM[0].scrollHeight);
                }
                if( (!roomDOM[0].isEqualNode(currentRoom)) && (sender != username)) {
                    sb.emit('PublicUnreadMessage', room);
                }
            }
    };
    receiveNotification = function(data){
        var message = data.message,
        room = data.room,
        now = new Date(),
        msgTemplate = '<li class="controlMessage clearfix"><span class="prompt">>>>>  </span><span class="message">'+message+'</span><div class="date">['+now.toString().match(/\d\d:\d\d:\d\d/)[0]+']</div></li>';
        if(room==="__CURRENT__") {
            if(currentRoom) {
                room = currentRoom;
            } else {
                room = sb.find('.room_main')[0];
            }
        } else {
            room = sb.find('.room_'+room)[0];
            if(!room) {
                room = currentRoom;
            }
        }
        //if response show only error messages
        if( data.status>=500 || data.status==195) {
            sb.append(room, msgTemplate);
            sb.scrollTop(room, room.scrollHeight);
        }
    };
    return {
        init: function() {
            container = sb.module()[0];
            sb.on('loggedIn', show);
            sb.on('loggedOut', hide);
            sb.on('newPrivateRoom', newPrivateRoom);
            sb.on('WSnewPublicRoom', newPublicRoom);
            sb.on('switchRoom', switchRoom);
            sb.on('WSleftRoom', leaveRoom);
            sb.on('WSreceivedMessage', receiveMessage);
            sb.on('WSreceivedPrivMessage', receivePrivMessage);
            sb.on('WSreceivedNotification', receiveNotification);
            sb.on('WSresponse', receiveNotification);
        },
        destroy: function() { 
            sb.off('loggedIn');
            container = null;   
        }
    };
};
