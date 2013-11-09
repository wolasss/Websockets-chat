var sandbox = function(core, instanceId, options, moduleId) {
  "use strict";
  core._mediator.installTo(this);

  //private vars
  var DOMid = instanceId ? '#'+instanceId : '';
  
  var entityMap = {
    "&": "&amp;",
    "<": "&lt;",
    ">": "&gt;",
    '"': '&quot;',
    "'": '&#39;',
    "/": '&#x2F;'
  };
  //selectors
  this.CSSloginButton = '.login-button';
  this.CSSusernameField = '.nick-field';
  this.CSSportField = '.port-field';
  this.CSShostnameField = '.host-field';
  this.CSSdefaultServerField = '.default-server-check';
  this.CSSerror = '.error';
  this.CSSchat = '.chat';
  this.CSSmessageField = '.message-field';
  this.CSSmessagesContainer = '.room';
  this.CSSuserList = '.list';
  this.CSSlogoutButton = '.logout';
  this.CSSrooms = '.room-list';


  //functions
  this.escapeHTML = function(string) {
    return String(string).replace(/[&<>"'\/]/g, function (s) {
      return entityMap[s];
    });
  };
  this.append = function(selector, what) {
    return $(selector).append(what);
  };
  this.is = function(target, what) {
    return $(target).is(what);
  };
  this.height = function(selector) {
    return $(selector).height();
  };
  this.scrollTop = function(selector, how) {
    return $(selector).scrollTop(how);
  };
  this.hasClass = function(selector, cl) {
    return $(selector).hasClass(cl);
  };
  this.addClass = function(selector, cl) {
    return $(selector).addClass(cl);
  };
  this.remove = function(selector) {
    return $(selector).remove();
  };
  this.removeClass = function(selector, cl) {
    return $(selector).removeClass(cl);
  };
  this.clear = function(selector) {
    return $(selector).html('');
  };
  this.hide = function(selector) {
    return $(selector).hide();
  };
  this.show = function(selector) {
    return $(selector).show();
  };
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
  };
  this.find = function(selector) {
    return $(DOMid).find(selector);
  };
  this.addEvent = function(elem, event, callback) {
    return $(elem).on(event, callback);
  };
  this.removeEvent = function(elem, event, callback) {
    return $(elem).off(event, callback);
  };
  this.module = function() {
    return $(DOMid);
  };
  this.fadeToggleModule = function() {
    return $(DOMid).fadeToggle();
  };
  this.toggleModule = function() {
    return $(DOMid).toggle();
  };
  this.slideToggle = function(cb) {
    return $(DOMid).slideToggle(cb);
  };
  this.id = instanceId;
  
  return this;
};

sandbox.prototype.foo = function() {  };

var CHAT = new scaleApp.Core(sandbox);
