var MODuserlist = function(sb){
	var list, reactor, showList, show;

	show = function() {
		sb.slideToggle();
	};
	showList = function(users) {
		sb.clear(list);
		for(var i=0, len=users.length; i<len; i++){
			sb.append(list, "<li class=\"user\">"+users[i]+"</li>");
		}
	};
	reactor = function(data, topic) {
		if(data.status==104) {
			showList(data.message);
		}
	};
	return {
	    init: function() {
	    	list = sb.find(sb.CSSuserList)[0];
	    	sb.on('loggedIn', show);
	    	sb.on('WSresponse', reactor);
	    },
	    destroy: function() { 
	    	sb.off('WSresponse');
	    	sb.off('loggedIn');
	    	list = null;	
	    }
	};
};