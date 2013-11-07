var MODuserlist = function(sb){
	"use strict";

	var list, reactor, showList, toggle, username, privateMessage, notification, clearNotification, currentRoom;

	toggle = function(data) {
		if(!username) username = data;
		sb.slideToggle();
	};
	privateMessage = function(e) {
		if( sb.is(e.originalEvent.target, 'li') ) {
			var t = e.originalEvent.target;
			if(t.isEqualNode(currentRoom)) {
				return;
			}
			var userNick = t.getAttribute('username');
			if(userNick!=username) {
				var data = {};
				data.type='private';
				data.name = userNick;
				sb.emit('newPrivateRoom', userNick);
				sb.emit('switchRoom', data);
				currentRoom = t;
				sb.addClass(t, 'active');
				sb.emit('currentRoomChangedPrivate');
			}
		}
	};
	showList = function(users) {
		sb.clear(list);
		var addClass = '';
		for(var i=0, len=users.length; i<len; i++){
			if(users[i]==="thefox") {
				addClass="fox";
			}
			sb.append(list, "<li class=\"user "+addClass+" user_"+users[i]+"\" username=\""+users[i]+"\">"+users[i]+"<span class=\"messages\"></span></li>");
			addClass = '';
		}
	};
	reactor = function(data) {
		if(data.status==104) {
			showList(data.message);
		}
	};
	notification = function(user) {
		var item = sb.find('.user_'+user);
		if(item.length!==0) {
			item = item[0];
		}
		var messagesContainer = sb.find('.user_'+user+' .messages')[0];
		var messages = sb.find('.user_'+user+' .messages > span')[0];
		console.log('span:', messages);
		var count = (messages) ? parseInt(messages.innerHTML,10) : 0;
		count++;
		console.log('mes, count', messagesContainer, count);
		sb.clear(messagesContainer);
		sb.append(messagesContainer, '<span>'+count+'</span>');
	};
	clearNotification = function(user) {
		var messagesContainer = sb.find('.user_'+user+' .messages')[0];
		if(messagesContainer) {
			sb.clear(messagesContainer);
		}
	};
	return {
	    init: function() {
			list = sb.find(sb.CSSuserList)[0];
			currentRoom = null;
			sb.on('loggedIn', toggle);
			sb.on('loggedOut', toggle);
			sb.on('WSresponse', reactor);
			sb.on('currentRoomChangedPublic', function(){
				sb.removeClass(currentRoom, 'active');
				currentRoom = null;
			});
			sb.on('PrivateUnreadMessage', notification);
			sb.on('PrivateReadMessages', clearNotification);
			sb.addEvent(list, 'click', privateMessage);
	    },
	    destroy: function() { 
			sb.off('WSresponse');
			sb.off('loggedIn');
			list = null;	
	    }
	};
};