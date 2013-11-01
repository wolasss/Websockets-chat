CHAT.register("websocket", MODwebsocket);
CHAT.register("board", MODboard);
CHAT.register("shouter", MODshouter);
CHAT.register("userlist", MODuserlist);

CHAT.start(function(){
	$('#loading').hide();
});
