var MODrooms = function(sb){
	var rooms, newPrivateRoom, switchRoom, currentRoom, newPublicRoom;

	switchRoom = function(e) {
		var t = e.originalEvent.target, 
			name = t.innerHTML;
		var data = {};
		sb.addClass(t, 'active');
		sb.removeClass(currentRoom, 'active');
		currentRoom = t;
		if(sb.hasClass(currentRoom, 'private_'+name)) {
			data.type = 'private';
		} else {
			data.type = 'public';
		}
		data.name = name;
		sb.emit('switchRoom', data);
	};
	newPrivateRoom = function(username) {
		if(sb.find('.private_'+username).length==0) {
			var tpl = "<li class=\"private_"+username+"\">"+username+"</li>";
			sb.append(rooms, tpl);			
		}
	};
	newPublicRoom = function(name) {
		console.log(name);
		if(sb.find('.room_'+name).length==0) {
			var tpl = "<li class=\"room_"+name+"\">"+name+"</li>";
			sb.append(rooms, tpl);		
		}
	};
	toggle = function() {
		sb.slideToggle();
	};
	return {
	    init: function() {
	    	rooms = sb.find(sb.CSSrooms)[0];
	    	currentRoom = sb.find('.room_main')[0];
	    	sb.on('loggedIn', toggle);
	    	sb.on('newPrivateRoom', newPrivateRoom);
	    	sb.on('WSnewPublicRoom', newPublicRoom);
	    	sb.addEvent(rooms, 'click', switchRoom);
	    	sb.on('loggedOut', toggle);
	    },
	    destroy: function() { 
	    	sb.off('loggedIn');
	    	sb.off('loggedOut');
	    	sb.off('newPrivateRoom');
	    	sb.off('newPublicRoom')
	    	sb.off('newPrivateRoom');
	    	rooms = null;	
	    }
	};
};