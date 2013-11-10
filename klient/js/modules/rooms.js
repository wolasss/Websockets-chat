var MODrooms = function(sb){
	"use strict";

	var rooms, newPrivateRoom, switchRoom, currentRoom, newPublicRoom, toggle, notification, clearNotification, leaveRoom;

	switchRoom = function(e) {
		var data={}, t, name;
		if(typeof(e)==="string") {
			name = e;
			sb.removeClass(currentRoom, 'active');
			data.type = 'private';
		} else {
			if( sb.is(e.originalEvent.target, 'li') ) {
				t = e.originalEvent.target;
				if(t.isEqualNode(currentRoom)) {
					return;
				}
				name = t.getAttribute('roomname');
				sb.removeClass(currentRoom, 'active');
				sb.addClass(t, 'active');
				currentRoom = t;
				data.type = 'public';
				sb.emit("currentRoomChangedPublic");	
			}
		}
		data.name = name;
		sb.emit('switchRoom', data);
	};
	newPublicRoom = function(name) {
		name = sb.escapeHTML(name);
		if(sb.find('.room_'+name).length===0) {
			var tpl = "<li class=\"room room_"+name+"\" roomname=\""+name+"\"><span class=\"name\">"+name+"</span><span class=\"messages\"></span></li>";
			sb.append(rooms, tpl);		
		}
	};
	toggle = function() {
		sb.clear(rooms);
		sb.append(rooms, '<li class="room room_main active" roomname="main"><span class=\"name\">main</span><span class="messages"></span></li>');
		currentRoom = sb.find('.room_main')[0];
		sb.slideToggle();
	};
	notification = function(room) {
		var item = sb.find('.room_'+room);
		if(item.length!==0) {
			item = item[0];
		}
		var messagesContainer = sb.find('.room_'+room+' .messages')[0];
		var messages = sb.find('.room_'+room+' .messages > span')[0];
		var count = (messages) ? parseInt(messages.innerHTML,10) : 0;
		count++;
		sb.clear(messagesContainer);
		sb.append(messagesContainer, '<span>'+count+'</span>');
	};
	clearNotification = function(room) {
		var messagesContainer = sb.find('.room_'+room+' .messages')[0];
		if(messagesContainer) {
			sb.clear(messagesContainer);
		}
	};
	leaveRoom = function(name) {
		var room = sb.find('.room_'+name);
		if(room.length!==0) {
			sb.remove(room[0]);
		}
	};
	return {
	    init: function() {
			rooms = sb.find(sb.CSSrooms)[0];
			
			sb.addEvent(rooms, 'click', switchRoom);
			
			sb.on('currentRoomChangedPrivate', function(){
				sb.removeClass(currentRoom, 'active');
				currentRoom = null;
			});
			sb.on('PublicUnreadMessage', notification);
			sb.on('PublicReadMessages', clearNotification);

			sb.on('newPrivateRoom', newPrivateRoom);
			sb.on('WSnewPublicRoom', newPublicRoom);
			sb.on('WSleftRoom', leaveRoom);
			sb.on('loggedIn', toggle);
			sb.on('loggedOut', toggle);
	    },
	    destroy: function() { 
			sb.off('loggedIn');
			sb.off('loggedOut');
			sb.off('newPrivateRoom');
			sb.off('WSnewPublicRoom');
			sb.off('PublicUnreadMessage');
			sb.off('PublicReadMessages');
			sb.off('currentRoomChangedPrivate');
			rooms = null;
			currentRoom = null;
	    }
	};
};
