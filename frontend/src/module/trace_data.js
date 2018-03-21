
/** 示例trace数据 */
var trace_item = {
    time_start: '2017-02-05 18:23:00:1234',
    s_ip: '192.168.1.0',
    d_ip: '192.168.2.5',
    proto_type: 'TCP',
    is_loop: false,
    is_probe: false,
    is_drop: true,
    hops: [
    {
     switch_id: 12,
     hop_rcvd: 2,
     hop_timeshift: 10 
    },
    {
     switch_id: 12,
     hop_rcvd: 2,
     hop_timeshift: 10 
    },
    {
     switch_id: 12,
     hop_rcvd: 2,
     hop_timeshift: 10 
    },
    ],
  };

// trace数据页中的主类, 用于搜索行为的控制以及输出的展示.
var trace_data = {

    _div_input_search: undefined,   // 搜索框, 用于获取filter
    _div_btn_sarch: undefined,      // 搜索按钮

    _div_list_trace_simple: undefined,  // 左侧的缩略列表
    _div_trace_detail: undefined,   //  右侧的详细信息

    _trace_arr: [],       // 每次请求所得的数据

    // 绑定div成员
    bind_div: function() {
        if(arguments.length < 4) {
            throw "Not enough arguments";
            return;
        }
        // 并不优雅, 有更好的方法请提出
        this._div_btn_sarch = arguments[0];
        this._div_btn_sarch = arguments[1];
        this._div_list_trace_simple = arguments[2];
        this._div_trace_detail = arguments[3];
    },

    refresh_list_trace_div: function() {   // 刷新网页视图

    },

    on_trace_item_click: function() {

    },

    // 网页数据中点击每个缩略的list, 将会调用
    // 下面的函数, 通过idx确定数组元素, 继而进行渲染
    refresh_trace_detail: function(idx) {
        if(idx > this._trace_arr.length) {
            throw "Not valid lenght";
            return;
        }

    }


};



module.exports.trace_data = trace_data;