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
