var MODuserlist = function(sb){
	"use strict";

	var list, reactor, showList, toggle, username, privateMessage;

	toggle = function(data) {
		if(!username) username = data;
		sb.slideToggle();
	};
	privateMessage = function(e) {
		var user = e.originalEvent.target.innerHTML;
		if(user!=username) {
			sb.emit('newPrivateRoom', user);
		}
	};
	showList = function(users) {
		sb.clear(list);
		var addClass = '';
		for(var i=0, len=users.length; i<len; i++){
			if(users[i]==="thefox") {
				addClass="fox";
			}
			sb.append(list, "<li class=\"user "+addClass+"\">"+users[i]+"</li>");
			addClass = '';
		}
	};
	reactor = function(data) {
		if(data.status==104) {
			showList(data.message);
		}
	};
	return {
	    init: function() {
			list = sb.find(sb.CSSuserList)[0];
			sb.on('loggedIn', toggle);
			sb.on('loggedOut', toggle);
			sb.on('WSresponse', reactor);
			sb.addEvent(list, 'click', privateMessage);
	    },
	    destroy: function() { 
			sb.off('WSresponse');
			sb.off('loggedIn');
			list = null;	
	    }
	};
};