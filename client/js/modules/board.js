var MODboard = function(sb){
    "use strict";
    var roomTemplate, messageTemplate, notificationTemplate, container, show, generateAdditionalClass, generateNotification, generateMessage, receivePublicMessage, hide, username, leaveRoom, switchRoom, receiveNotification, newPrivateRoom, currentRoom, receivePrivMessage, newPublicRoom;

    show = function(data) {
        var mainRoom = roomTemplate({ class: 'room room_main active'});

        sb.clear(container);
        sb.append(container, mainRoom);
        currentRoom = sb.find('.room_main')[0];
        receiveNotification({ status: 195, room:'__CURRENT__', title: 'Welcome to Websockets chat!', message: 'Now, you are in the main room. If you need any help, just type: /help' });
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
            var privateTemplate = roomTemplate({ class: 'room room_private_'+user+'' });
            sb.append(container, privateTemplate);
        }
    };
    newPublicRoom = function(name) {
        name = sb.escapeHTML(name);
        if(sb.find('.room_'+name).length===0) {
            var publicRoom = roomTemplate({ class: 'room room_'+name+'' });
            sb.append(container, publicRoom);
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
    generateAdditionalClass = function(data) {
        var addClass = '';

        if(data) {
            if(username === data.sender) { 
                addClass+='mine'; 
            }
            if(data.sender === "thefox") { 
                addClass+=' fox'; 
            }
        }
        
        return addClass;
    };
    generateMessage = function(data) {
        
        var now = new Date(),
            additionalClass = '',
            message;
        if(messageTemplate) {
            message = messageTemplate({
                additionalClass: generateAdditionalClass(data),
                sender: sb.escapeHTML(data.sender),
                message: sb.escapeHTML(data.message),
                date: now.toString().match(/\d\d:\d\d:\d\d/)[0]
            });
        } else {
            message = null;
        }
        

        return message;
    };
    generateNotification = function(message, title) {
        var now = new Date(),
            additionalClass = '',
            notification;

        if(notificationTemplate) {
            notification = notificationTemplate({
                additionalClass: generateAdditionalClass(),
                message: message,
                title: title,
                date: now.toString().match(/\d\d:\d\d:\d\d/)[0]
            });
        } else {
            notification = null;
        }

        return notification;
    };
    receivePrivMessage = function(data) {
        var roomDOM,
            room = data.room,
            msg = generateMessage(data);

        roomDOM = sb.find('.room_private_'+room);
        if(roomDOM.length!==0) {
            if((roomDOM[0].scrollTop + roomDOM[0].offsetHeight)>435 && (roomDOM[0].scrollTop + roomDOM[0].offsetHeight)<roomDOM[0].scrollHeight ) {
                sb.append(roomDOM[0], msg);
            } else {
                sb.append(roomDOM[0], msg);
                sb.scrollTop(roomDOM[0], roomDOM[0].scrollHeight);
            }
            if( (!roomDOM[0].isEqualNode(currentRoom)) && (data.sender != username)) {
                sb.emit('PrivateUnreadMessage', data.sender);
            }
        } else {
            newPrivateRoom(data.room);
            roomDOM = sb.find('.room_private_'+room);
            sb.append(roomDOM, msg);
            sb.scrollTop(roomDOM, roomDOM.scrollHeight);
            sb.emit('PrivateUnreadMessage', data.sender);
        }
    };
    receivePublicMessage = function(data) {
        var roomDOM,
            room = data.room,
            msg = generateMessage(data);

        roomDOM = sb.find('.room_'+room);
        if(roomDOM.length!==0) {
            if((roomDOM[0].scrollTop + roomDOM[0].offsetHeight)>435 && (roomDOM[0].scrollTop + roomDOM[0].offsetHeight)<roomDOM[0].scrollHeight ) {
                sb.append(roomDOM[0], msg);
            } else {
                sb.append(roomDOM[0], msg);
                sb.scrollTop(roomDOM[0], roomDOM[0].scrollHeight);
            }
            if( (!roomDOM[0].isEqualNode(currentRoom)) && (data.sender != username)) {
                sb.emit('PublicUnreadMessage', room);
            }
        }
    };
    receiveNotification = function(data){
        var message = data.message,
        room = data.room;

        var notification = generateNotification(message, data.title);
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
        //if response, or contorl message show only error messages
        if( data.status>=500 || data.status==195) {
            sb.append(room, notification);
            sb.scrollTop(room, room.scrollHeight);
        }
    };
    return {
        init: function() {
            container = sb.module()[0];
            roomTemplate = sb.templates.compile(sb.findTemplate('room-template'));
            messageTemplate = sb.templates.compile(sb.findTemplate('message-template'));
            notificationTemplate = sb.templates.compile(sb.findTemplate('notification-template'));
            sb.on('loggedIn', show);
            sb.on('loggedOut', hide);
            sb.on('newPrivateRoom', newPrivateRoom);
            sb.on('WSnewPublicRoom', newPublicRoom);
            sb.on('switchRoom', switchRoom);
            sb.on('WSleftRoom', leaveRoom);
            sb.on('WSreceivedMessage', receivePublicMessage);
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
