CHAT.register("websocket", MODwebsocket);
CHAT.register("board", MODboard);
CHAT.register("shouter", MODshouter);

CHAT.start(function(){
	$('#loading').hide();
});
