CHAT.register("websocket", MODwebsocket);
CHAT.register("board", MODboard);
CHAT.register("shouter", MODshouter);
CHAT.register("userlist", MODuserlist);
CHAT.register("logoutbutton", MODlogoutbutton);
CHAT.register("rooms", MODlogoutbutton);

CHAT.start(function(){
	$('#loading').hide();
});
