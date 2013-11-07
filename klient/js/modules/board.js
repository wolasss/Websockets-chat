var MODboard = function(sb){

	"use strict";
	var container, show, receiveMessage, hide, username, leaveRoom, switchRoom, receiveNotification, newPrivateRoom, currentRoom, receivePrivMessage, AUTOnewPrivateRoom, newPublicRoom;

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
		console.log(user);
		if(sb.find('.room_private_'+user).length===0) {
			var tpl = "<ul class=\"room room_private_"+user+"\"></ul>";
			sb.append(container, tpl);
		} else {
			//activate it
		}
	};
	newPublicRoom = function(name) {
		if(sb.find('.room_'+name).length===0) {
			var tpl = "<ul class=\"room room_"+name+"\"></ul>";
			sb.append(container, tpl);
		}
	};
	leaveRoom = function(name) {
		name = 1;
	};
	switchRoom = function(data) {
		var type = data.type, room = data.name,
		target = (type==='private') ? sb.find('.room_private_'+room)[0] : sb.find('.room_'+room)[0];
		if(target.length!==0) {
			sb.removeClass(currentRoom, 'active');
			currentRoom = target;
			sb.addClass(target, 'active');
			var classes = currentRoom.getAttribute('class').match(/.room_private_\w+/);
			if(classes.length!==0) {
				var nick = classes[0].split('_')[2];
				sb.emit('readMessages', nick);
			}
		}
	};
	AUTOnewPrivateRoom = function(name) {
		sb.emit('newPrivateRoom', name);
		var room = sb.find('.room_private_'+name)[0];
		return room;
	};
	receivePrivMessage = function(data) {
		var message = data.message,
			roomDOM,
			room = data.room,
			sender = data.sender,
			now = new Date(),
			additionalClass = '';
			if(username === sender) { additionalClass+='mine'; }
			if(data.sender === "thefox") { additionalClass+=' fox'; }
			var msgTemplate = '<li class="msgContainer clearfix"><div class="avatar '+additionalClass+'"><div class="nick">'+sender+'</div></div><div class="message '+additionalClass+'"><div class="bubble">'+message+'<div class="info">'+now.toString().match(/\d\d:\d\d:\d\d/)[0]+'</div></div></div></li>';
     		if(room==="__CURRENT__") {
     			roomDOM = [];
     			roomDOM.push(currentRoom);
     		} else {
     			roomDOM = sb.find('.room_private_'+room);
     		}
            if(roomDOM.length!==0) {
				sb.append(roomDOM, msgTemplate);
				sb.scrollTop(roomDOM[0], roomDOM[0].scrollHeight);
				if( (!roomDOM[0].isEqualNode(currentRoom)) && (sender != username)) {
					sb.emit('unreadMessage', sender);
				}
            } else {
				roomDOM = new AUTOnewPrivateRoom(data.sender);
				sb.append(roomDOM, msgTemplate);
				sb.scrollTop(roomDOM, roomDOM.scrollHeight);
				sb.emit('unreadMessage', sender);
            }
	};
	receiveMessage = function(data) {
		var message = data.message,
			roomDOM,
			room = data.room,
			sender = data.sender,
			now = new Date(),
			additionalClass = '';
			if(username === sender) { additionalClass+='mine'; }
			if(data.sender === "thefox") { additionalClass+=' fox'; }
			var msgTemplate = '<li class="msgContainer clearfix"><div class="avatar '+additionalClass+'"><div class="nick">'+sender+'</div></div><div class="message '+additionalClass+'"><div class="bubble">'+message+'<div class="info">'+now.toString().match(/\d\d:\d\d:\d\d/)[0]+'</div></div></div></li>';
            
            if(room==="__CURRENT__") {
     			roomDOM = [];
     			roomDOM.push(currentRoom);
     		} else {
     			roomDOM = sb.find('.room_'+room);
     		}
            if(roomDOM.length!==0) {
				sb.append(roomDOM[0], msgTemplate);
				sb.scrollTop(roomDOM[0], roomDOM[0].scrollHeight);
            } else {
				//add and append
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
		}
		sb.append(room, msgTemplate);
		sb.scrollTop(room, room.scrollHeight);
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
	    },
	    destroy: function() { 
			sb.off('loggedIn');
			container = null;	
	    }
	};
};
