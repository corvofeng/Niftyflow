var $ = require("jquery");

if (process.env.NODE_ENV !== 'production') {
  require('./index.html')
}

console.log('hello world');
$("#button").on('click', function(event){
});



