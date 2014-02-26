var MODuserlist = function(sb){
	"use strict";

	var users = [], updateUsers, list, generateUserList, userListTemplate, reactor, showList, toggle, username, privateMessage, notification, clearNotification, currentRoom;

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
				sb.removeClass(currentRoom, 'active');
				currentRoom = t;
				sb.addClass(t, 'active');
				sb.emit('currentRoomChangedPrivate');
			}
		}
	};
	showList = function() {
		sb.clear(list);
		sb.append(list, userListTemplate(users));
	};
	updateUsers = function(data) {
		users = [];
		for(var i=0, len=data.length; i<len; i++){			
			var name = sb.escapeHTML(data[i]);
			users.push({name: name, additionalClass: ''});
		}
	};
	reactor = function(data) {
		if(data.status==104) {
			updateUsers(data.message);
			showList(data.message);
		}
	};
	notification = function(user) {

		var item = sb.find('.user_'+user);
		if(item.length!==0) {
			item = item[0];
		}
		var messagesContainer = sb.find('.user_'+user+' .notification')[0];
		var messages = sb.find('.user_'+user+' .notification > span')[0];
		var count = (messages) ? parseInt(messages.innerHTML,10) : 0;
		count++;
		sb.clear(messagesContainer);
		sb.append(messagesContainer, '<span>'+count+'</span>');
	};
	clearNotification = function(user) {
		var messagesContainer = sb.find('.user_'+user+' .notification')[0];
		if(messagesContainer) {
			sb.clear(messagesContainer);
		}
	};
	return {
	    init: function() {
			list = sb.find(sb.CSSuserList)[0];
			userListTemplate = sb.templates.compile(sb.findTemplate('user-list-template'));

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