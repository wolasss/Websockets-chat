

var sandbox = function(core, instanceId, options, moduleId) {
  core._mediator.installTo(this);

  //private vars
  var DOMid = instanceId ? '#'+instanceId : '';
  
  //selectors
  this.CSSloginButton = '.login-button';
  this.CSSusernameField = '.nick-field';
  this.CSSportField = '.port-field';
  this.CSShostnameField = '.host-field';
  this.CSSdefaultServerField = '.default-server-check';
  this.CSSerror = '.error';




  //functions
  this.append = function(selector, what) {
    return $(selector).append(what);
  };
  this.clear = function(selector) {
    return $(selector).html('');
  }
  this.hide = function(selector) {
    return $(selector).hide();
  };
  this.show = function(selector) {
    return $(selector).show();
  }
  this.toggle = function(selector) {
    return $(selector).toggle();
  };
  this.parent = function (selector) {
    return $(selector).parent();
  };
  this.slideUp = function(selector) {
    return $(selector).slideUp();
  };
  this.slideDown = function(selector) {
    return $(selector).slideDown();
  }
  this.find = function(selector) {
    return $(DOMid).find(selector);
  };
  this.addEvent = function(elem, event, callback) {
    return $(elem).on(event, callback);
  };
  this.toggleModule = function() {
    return $(DOMid).toggle();
  };
  this.prop = "bar";
  this.id = instanceId;
  
  return this;
};

sandbox.prototype.foo = function() {  };

var CHAT = new scaleApp.Core(sandbox);
