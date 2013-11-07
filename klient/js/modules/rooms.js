var MODrooms = function(sb){
	"use strict";

	var rooms, newPrivateRoom, switchRoom, currentRoom, newPublicRoom, toggle, notification, clearNotification;

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
				sb.addClass(t, 'active');
				sb.removeClass(currentRoom, 'active');
				currentRoom = t;
				data.type = 'public';
				sb.emit("currentRoomChangedPublic");	
			}
		}
		data.name = name;
		console.log(data);
		sb.emit('switchRoom', data);
	};
	newPublicRoom = function(name) {
		if(sb.find('.room_'+name).length===0) {
			var tpl = "<li class=\"room room_"+name+"\" roomname=\""+name+"\">"+name+"<span class=\"messages\"></span></li>";
			sb.append(rooms, tpl);		
		}
	};
	toggle = function() {
		sb.slideToggle();
	};
	notification = function(room) {
		console.log(room);
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
	return {
	    init: function() {
			rooms = sb.find(sb.CSSrooms)[0];
			currentRoom = sb.find('.room_main')[0];
			console.log('current', currentRoom);
			sb.on('loggedIn', toggle);
			sb.on('newPrivateRoom', newPrivateRoom);
			sb.on('WSnewPublicRoom', newPublicRoom);
			sb.on('currentRoomChangedPrivate', function(){
				sb.removeClass(currentRoom, 'active');
				currentRoom = null;
			});
			sb.on('PublicUnreadMessage', notification);
			sb.on('PublicReadMessages', clearNotification);
			sb.addEvent(rooms, 'click', switchRoom);
			sb.on('loggedOut', toggle);
	    },
	    destroy: function() { 
			sb.off('loggedIn');
			sb.off('loggedOut');
			sb.off('newPrivateRoom');
			sb.off('newPublicRoom');
			sb.off('newPrivateRoom');
			rooms = null;
	    }
	};
};
