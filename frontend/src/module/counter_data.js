'use strict';

var Conf = require('../conf');
var $ = require("jquery");
var echarts = require('echarts');

// http://echarts.baidu.com/examples/editor.html?c=line-smooth
var option = {
  xAxis: {
      type: 'category',
      data: [] // ['Mon', 'Sat', 'Sun']
  },
  yAxis: {
      type: 'value'
  },
  series: [{
      data: [], //  [820, 1330, 1320],
      type: 'line',
      smooth: true
  }]
};



var counter_data = {

  // 三个div控件
  _div_counter_select_rule_id: null,
  _div_counter_btn_submit: null,
  _div_counter_charts: null,

  _counter_chart: null, 
  _rule_arr: null,  // 保存所有规则
  _counts_arr: null,  // 保存请求到的数据

  bind_div: function () {
    let self = this;
    if (arguments.length < 3) {
      throw "Not enough arguments";
      return;
    }

    this._div_counter_select_rule_id = arguments[0];
    this._div_counter_btn_submit = arguments[1];
    this._div_counter_charts = arguments[2];

    this._counter_chart = echarts.init(this._div_counter_charts);
    this._div_counter_btn_submit.on("click", function () {
      self.get_counts();
    });
  },

  // 点击提交之后获取rule_id计数器值
  get_counts: function () {
    let self = this;
    let idx = self._div_counter_select_rule_id.val();

    let rule_id = self._rule_arr[idx]['rule_id'];

    $.ajax({
      type: "POST",
      url: Conf.Conf['url_prefix'] + '/v1/counter_filter',
      data: {
        'start_time': '2018-04-01 03:00:00',
        'end_time': '2018-04-01 23:00:00',
        'rule_id': rule_id
      },
      dataType: "text",
      success: function (resultData) {
        let jData = JSON.parse(resultData);
        if (jData['code'] != 200) {
          alert(jData['msg']);
          return
        }
        console.log(jData);
        self._counts_arr = jData['data'];
        self.run_charts();
      }
    });
  },

  // 请求所有的计数器规则
  init_couter_id: function () {
    console.log("init counter id");
    let self = this;
    $.ajax({
      type: "GET",
      url: Conf.Conf['url_prefix'] + '/v1/rules',
      dataType: "text",
      success: function (resultData) {
        let jData = JSON.parse(resultData);
        if (jData['code'] != 200) {
          alert(jData['msg']);
          return;
        }
        self._rule_arr = jData.data;
        self.refresh_counter_id();
      }
    });
  },
  // 获取所有规则后渲染到选择器中
  refresh_counter_id: function() {
    let self = this;
    self._rule_arr.forEach((element, idx) => {
      if (element['is_valid'] == 1) {
        self._div_counter_select_rule_id.append(
          `<option value="${idx}">${element['rule_name']}</option>`
        );
      }
    });
  },

  run_charts: function () {
    let self = this;

    // 这里使用了深拷贝, 基于option
    let tmpOption = $.extend(true, {}, option);

    console.log(tmpOption);
    self._counts_arr.forEach((element, idx) => {
      tmpOption['xAxis']['data'].push(element['generate_time']);
      tmpOption['series'][0]['data'].push(element['cnt']);
    });


    self._counter_chart.setOption(tmpOption);
  }
};

export { counter_data };
