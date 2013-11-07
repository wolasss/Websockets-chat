var MODboard = function(sb){
	var container, show, receiveMessage, hide, username, receiveNotification, newPrivateRoom, currentRoom, receivePrivMessage, AUTOnewPrivateRoom, newPublicRoom;

	show = function(data, topic) {
		var mainRoom = "<ul class=\"room room_main active\"></ul>";
		sb.clear(container);
		sb.append(container, mainRoom);
		currentRoom = sb.find('.room_main')[0];
		console.log(currentRoom);
		username = data;
		sb.addClass(sb.CSSchat, 'expanded');
		setTimeout(sb.fadeToggleModule,600);
	};
	hide = function(data, topic) {
		sb.removeClass(sb.CSSchat, 'expanded');
		sb.toggleModule();
	};
	newPrivateRoom = function(user) {
		if(sb.find('.room_private_'+user).length==0) {
			var tpl = "<ul class=\"room room_private_"+user+"\"></ul>";
			sb.append(container, tpl);
		}
	};
	newPublicRoom = function(name) {
		if(sb.find('.room_'+name).length==0) {
			var tpl = "<ul class=\"room room_"+name+"\"></ul>";
			sb.append(container, tpl);
		}
	};
	switchRoom = function(data) {
		var type = data.type, room = data.name,
		target = (type==='private') ? sb.find('.room_private_'+room)[0] : sb.find('.room_'+room)[0];
		if(target.length!=0) {
			sb.removeClass(currentRoom, 'active');
			currentRoom = target;
			sb.addClass(target, 'active');
		}
	};
	AUTOnewPrivateRoom = function(name) {
		sb.emit('newPrivateRoom', name);
		var room = sb.find('.room_private_'+name)[0];
		return room;
	};
	receivePrivMessage = function(data, topic) {
		var message = data.message,
			room = data.room,
			sender = data.sender
			now = new Date(),
			additionalClass = '';
			if(username === sender) { additionalClass+='mine'; }
			if(data.sender === "thefox") { additionalClass+=' fox'; }
			var msgTemplate = '<li class="msgContainer clearfix"><div class="avatar '+additionalClass+'"><div class="nick">'+sender+'</div></div><div class="message '+additionalClass+'"><div class="bubble">'+message+'<div class="info">'+now.toString().match(/\d\d:\d\d:\d\d/)[0]+'</div></div></div></li>',
            room = sb.find('.room_private_'+room);
            console.log('room:', room);
            if(room.length!=0) {
            	sb.append(room, msgTemplate);
            	sb.scrollTop(room, room.scrollHeight);
            } else {
            	console.log('trzeba stworzyc')
            	room = AUTOnewPrivateRoom(data.sender);
            	console.log(room);
            	sb.append(room, msgTemplate);
            	sb.scrollTop(room, room.scrollHeight);
            }
	};
	receiveMessage = function(data, topic) {
		var message = data.message,
			room = data.room,
			sender = data.sender
			now = new Date(),
			additionalClass = '';
			if(username === sender) { additionalClass+='mine'; }
			if(data.sender === "thefox") { additionalClass+=' fox'; }
			var msgTemplate = '<li class="msgContainer clearfix"><div class="avatar '+additionalClass+'"><div class="nick">'+sender+'</div></div><div class="message '+additionalClass+'"><div class="bubble">'+message+'<div class="info">'+now.toString().match(/\d\d:\d\d:\d\d/)[0]+'</div></div></div></li>',
            room = sb.find('.room_'+room)[0];
            
            if(room.length!=0) {
            	sb.append(room, msgTemplate);
            	sb.scrollTop(room, room.scrollHeight);
            } else {
            	//add and append
            }
	};
	receiveNotification = function(data, topic){
		console.log('not:', data);
		var message = data.message,
		room = data.room,
		now = new Date(),
		msgTemplate = '<li class="controlMessage clearfix"><span class="prompt">\>\>\>\>  </span>'+message+'<div class="date">['+now.toString().match(/\d\d:\d\d:\d\d/)[0]+']</div></li>';

		room = sb.find('.room_'+room)[0];
		if(room) {
            	sb.append(room, msgTemplate);
            	sb.scrollTop(room, room.scrollHeight);
        }
	};
	return {
	    init: function() {
	    	container = sb.module()[0];
	    	sb.on('loggedIn', show);
	    	sb.on('loggedOut', hide);
	    	sb.on('newPrivateRoom', newPrivateRoom);
	    	sb.on('WSnewPublicRoom', newPublicRoom);
	    	sb.on('switchRoom', switchRoom);
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