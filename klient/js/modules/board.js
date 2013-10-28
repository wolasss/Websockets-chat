var MODboard = function(sb){
	var container, show, sentMessage;

	show = function() {
		sb.addClass(sb.CSSchat, 'expanded');
		sb.toggleModule();
	}
	sentMessage = function(data) {

	}
	return {
	    init: function() {
	    	container = sb.find(sb.CSSmessagesContainer)[0];
	    	sb.on('loggedIn', show);
	    	sb.on('WSsendMessage', sentMessage);
	    },
	    destroy: function() { 
	    	sb.off('loggedIn');
	    	container = null;	
	    }
	};
};