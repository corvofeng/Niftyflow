import { debug } from 'util';

var Conf = require('../conf');
var $ = require("jquery");


String.prototype.printf = function (obj) {
  var useArguments = false;
  var _arguments = arguments;
  var i = -1;
  if (typeof _arguments[0] == "string") {
    useArguments = true;
  }
  if (obj instanceof Array || useArguments) {
    return this.replace(/\%s/g,
      function (a, b) {
        i++;
        if (useArguments) {
          if (typeof _arguments[i] == 'string') {
            return _arguments[i];
          }
          else {
            throw new Error("Arguments element is an invalid type");
          }
        }
        return obj[i];
      });
  }
  else {
    return this.replace(/{([^{}]*)}/g,
      function (a, b) {
        var r = obj[b];
        return typeof r === 'string' || typeof r === 'number' ? r : a;
      });
  }
};

/**
 * fdate: 20170320
 * fms: 25595309 一天内的偏移毫秒数
 */
function date_parse(fdate, fms) {
  if (!/^(\d){8}$/.test(fdate)) return "invalid date";
  fdate = "" + fdate;
  var y = fdate.substr(0, 4),
    m = fdate.substr(4, 2),
    d = fdate.substr(6, 2);

  let hours = Math.floor((fms / 1000) / 3600);
  let minutes = Math.floor(((fms / 1000) % 3600) / 60);
  let seconds = Math.floor((fms / 1000) % 60);
  let ms = fms % 1000;

  return new Date(y, m, d, hours, minutes, seconds, ms);
}

/**
 * 打印时间
 * Date() =>  2018-04-12 15:00:23
 */
function date_print(m) {
  var dateString =
    m.getUTCFullYear() + "-" +
    ("0" + (m.getUTCMonth() + 1)).slice(-2) + "-" +
    ("0" + m.getUTCDate()).slice(-2) + " " +
    ("0" + m.getUTCHours()).slice(-2) + ":" +
    ("0" + m.getUTCMinutes()).slice(-2) + ":" +
    ("0" + m.getUTCSeconds()).slice(-2) + ":" +
    ("0" + m.getMilliseconds()).slice(-3)
    ;
  return dateString;
}

/**
 * Get nums in str
 * abcd123de45 => 12345
 */
function retnum(str) {
  var num = str.replace(/[^0-9]/g, '');
  return parseInt(num, 10);
}

// trace数据页中的主类, 用于搜索行为的控制以及输出的展示.
var trace_data = {

  _div_input_search: undefined,   // 搜索框, 用于获取filter
  _div_btn_sarch: undefined,      // 搜索按钮

  _div_list_trace_simple: undefined,  // 左侧的缩略列表
  _div_trace_detail: undefined,   //  右侧的详细信息

  _trace_arr: [],       // 每次请求所得的数据

  // 绑定div成员
  bind_div: function () {
    if (arguments.length < 4) {
      throw "Not enough arguments";
      return;
    }
    // 并不优雅, 有更好的方法请提出
    this._div_btn_search = arguments[0];
    this._div_input_search = arguments[1];
    this._div_list_trace_simple = arguments[2];
    this._div_trace_detail = arguments[3];

    let self = this;
    this._div_btn_search.on("click", function () {
      self.search_event();
    });
  },

  search_event: function () {
    let self = this;
    let input_trace_filter = self._div_input_search.val();

    console.log(input_trace_filter);

    this.refresh_list_trace_div();

    console.log("In Search event");
    $.ajax({
      type: "POST",
      url: Conf.Conf['url_prefix'] + '/v1/tracce_filter',
      data: {
        'start_time': '2018-04-14 11:30:00',
        'end_time': '2018-04-14 11:50:00'
      },
      dataType: "text",
      success: function (resultData) {
        let jData = JSON.parse(resultData);
        if (jData['code'] != 200) {
          alert(jData['msg']);
        }
        console.log(jData)
        self._trace_arr = jData['data'];
        self.refresh_list_trace_div();
      }
    });
  },

  refresh_list_trace_div: function () {   // 刷新网页视图
    let self = this;

    self._trace_arr.forEach((element, idx) => {
      console.log(idx, element);
      let id = element['id'];
      let timer = date_parse(element['fdate'], element['generate_time']);
      element['timer'] = timer;
      let t_str = date_print(timer);
      let s_ip = element['s_ip'];
      let d_ip = element['d_ip'];

      self._div_list_trace_simple.append($("<li />")
        .html(`${t_str} ${s_ip} => ${d_ip}`)
        .attr("id", `myDivId-${idx}`)
        .addClass(
          "list-group-item list-group-item-action list-group-item-secondary"
        ));

      $(`#myDivId-${idx}`).on('click', function () {
        let idx = retnum(arguments[0].target.id);
        self.refresh_trace_detail(idx);
      });

    });
  },

  /**
   *   获取到idx后, 修改右侧的信息
   * 下面的函数, 通过idx确定数组元素, 继而进行渲染
   */
  refresh_trace_detail: function (idx) {
    let self = this;

    self._div_trace_detail[0].innerHTML = '';
    if (idx > this._trace_arr.length) {
      throw "Not valid lenght";
      return;
    }
    let item = self._trace_arr[idx];

    let up_left_elements = $("<div />")
      .html(
        `产生时间: ${date_print(item.timer)} <br>` + 
        `协议类型: ${item.protocol}   <br>` + 
        `是否环路: ${item.is_loop} <br> ` +
        `是否丢包: ${item.is_drop} <br> ` +
        `是否探针: ${item.is_probe}`
      )
      .addClass("col-5");
    let up_right_elements = $("<div />")
      .html(
        `源IP: ${item.s_ip} <br>` + 
        `目的IP: ${item.d_ip} <br>`
      )
      .addClass("col-5");

    let table_element = $("<table />").html(
      "<thead> " +
      "<tr> " +
      " <th scope=\"col\">交换机ID</th> " +
      "  <th scope=\"col\">接受到的报文数</th> " +
      "  <th scope=\"col\">偏移时间</th> " +
      " </tr>" +
      " </thead>"
    ).addClass("table table-hover");

    let jData = JSON.parse(item['trace_data']);
    let t_info = jData['trace_info']
    let t_html;
    for(let i = 0; i < t_info.length; i++) {
      t_html = t_html +
        "<tr>"+
          `<td>${t_info[i]['switch_id']}</td>` +
          `<td>${t_info[i]['hop_rcvd']}</td>` +
          `<td>${t_info[i]['hop_timeshift']}</td>` +
        "</tr>";
    }
    

    let table_body = $("<tbody />").html(t_html);
    table_element.append(table_body);


    //let up_table_elements = $("<table />").a
    let table = $('<div />')
      .addClass("col-9").append(table_element);

    let up_elements = $('<div />')
      .addClass("row")
      .append(up_left_elements)
      .append(up_right_elements)
      .append(table);

    self._div_trace_detail.append(up_elements);

    console.log(self._trace_arr[idx]);
  }
};

// module.exports.trace_data = _trace_data;
export { trace_data };
