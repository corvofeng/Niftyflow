'use strict';
var $ = require("jquery");
window.jQuery = require('jquery');
require('bootstrap/dist/js/bootstrap.bundle.js');   // for bootstrap
// var trace_page_js = require('./module/trace_data');
import {trace_data} from "./module/trace_data";
import {counter_data} from "./module/counter_data";
import {performance} from "./module/performance";

// require('bootstrap-timepicker/js/bootstrap-timepicker.min.js')


if (process.env.NODE_ENV !== 'production') {
  require('./index.html');
}



// $(document).ready(function(){
//   console.log('hello world');

//   $('#btn_trace_search').on("click", function() {
//     let input_trace_filter = $('#input_trace_search').val();
//     console.log(input_trace_filter);
//     console.log('btn click');

//     $("#list_trace_simple").append(
//       '<a href="/user/messages"><span class="tab">Message Center</span></a>'
//     );
    
//   });
//   // $('#timepicker2').timepicker();
// })



trace_data.bind_div(
  $('#btn_trace_search'),
  $('#input_trace_search'),
  $('#list_trace_simple'),
  $('#trace_detail')
);

counter_data.bind_div(
  $('#counter_select_rule_id'),
  $('#counter_btn_submit'),
  document.getElementById('counter_charts')
);
counter_data.init_couter_id();


performance.bind_div(

  document.getElementById('performance_chart')
);

//# counter_data.run_charts();