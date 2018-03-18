var $ = require("jquery");
require('bootstrap/dist/js/bootstrap.bundle.js')    // for bootstrap

if (process.env.NODE_ENV !== 'production') {
  require('./index.html');
}

console.log('hello world');

$("#btn").on("click", function(){
    console.log("ok");
});
