$(function(){
	$('.default-server-check').on('change', function() {
		var checked = this.checked,
			inputs = $('.hostname, .port'); 
		if(checked) {
			inputs.slideUp();
		} else {
			inputs.slideDown();
		}
	})
});

$(function(){
    $('.login-button').on('click', function() {
        NProgress.start();
        
        NProgress.done();
    });
});