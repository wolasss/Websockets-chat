var MODlogoutbutton = function(sb){
	"use strict";
	var button, logout, toggle;
	
	toggle = function() {
		sb.toggleModule();
	};
	logout = function() {
		sb.emit('WSlogout');
	};
	return {
	    init: function() {
			button = sb.find(sb.CSSlogoutButton)[0];
			sb.addEvent(button, 'click', logout);
			sb.on('loggedIn', toggle);
			sb.on('loggedOut', toggle);
	    },
	    destroy: function() { 
			button = null;
	    }
	};
};