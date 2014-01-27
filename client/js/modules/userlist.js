var MODuserlist = function(sb){
	"use strict";

	var list, generateUserList, userListTemplate, reactor, showList, toggle, username, privateMessage, notification, clearNotification, currentRoom;

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
	generateUserList = function(name, addClass) {
		var user;

		user = userListTemplate({
			name: name,
			additionalClass: (addClass ? addClass : '')
		});

		return user;
	}
	showList = function(users) {
		sb.clear(list);
		//dodac escape html na usernamach.

		for(var i=0, len=users.length; i<len; i++){
			//easter egg
			
			var name = sb.escapeHTML(users[i]);

			
			addClass = '';
		}
		sb.append(list, userListTemplate(name));
		//to powinno byc inaczej zrobione trzeba dodac jeszcze obiekt z tymi userami... za kazdym razem gdy odbierana jest wiadomosc z serwera 
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
		var count = (messages) ? parseInt(messages.innerHTML,10) : 0;
		count++;
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
			userListTemplate = sb.templates.compile(sb.find('#user-list-template')[0].innerHTML);

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