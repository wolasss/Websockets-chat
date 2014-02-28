module.exports = function(grunt) {

  // Project configuration.
  grunt.initConfig({
    pkg: grunt.file.readJSON('package.json'),
    cssmin: {
      combine: {
        files: {
          'css/build/app.min.css': ['css/main.css']
        }
      }
    },
    compass: {
      dev: {
        options: {
          config: 'config.rb'
        }
      },
      dist: {
        options: {
          config: 'config.rb'
        }
      }
    },
    clean: ['js/build', 'css/*'],
    uglify: {
      options: {
        mangle: true,
        compress: true,
      },
      build: {
        src: ['js/vendor/*.js', 'js/plugins.js', 'js/sandbox.js', 'js/modules/*.js', 'js/core.js'],
        dest: 'js/build/app.min.js'
      }
    }
  });

  grunt.loadNpmTasks('grunt-contrib-uglify');
  grunt.loadNpmTasks('grunt-contrib-compass');
  grunt.loadNpmTasks('grunt-contrib-clean');
  grunt.loadNpmTasks('grunt-contrib-cssmin');

  grunt.registerTask('default', ['compass', 'uglify', 'cssmin']);
  grunt.registerTask('reboot', ['clean', 'default']);
};