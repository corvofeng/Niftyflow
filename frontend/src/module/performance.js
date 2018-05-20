'use strict';

var Conf = require('../conf');
var $ = require("jquery");
var echarts = require('echarts');
var process_data = require("./process_data");
var queue_data = require("./queue_data");



var process_charts_option = {
    title: {
        text: '每个线程处理情况'
    },
    tooltip : {
        trigger: 'axis',
        axisPointer: {
            type: 'cross',
            label: {
                backgroundColor: '#6a7985'
            }
        }
    },
    // legend: {
    //     data:['邮件营销','联盟广告','视频广告','直接访问','搜索引擎']
    // },
    dataZoom: [
        {
            show: true,
            realtime: true,
            start: 65,
            end: 85
        },
        {
            type: 'inside',
            realtime: true,
            start: 65,
            end: 85
        }
    ],
    toolbox: {
        feature: {
            dataZoom: {
                yAxisIndex: 'none'
            },
            restore: {},
            saveAsImage: {}
        }
    },
    grid: {
        left: '3%',
        right: '4%',
        bottom: '3%',
        containLabel: true
    },

    // xAxis : [
    //     {
    //         type : 'category',
    //         boundaryGap : false,
    //         data : ['周一','周二','周三','周四','周五','周六','周日']
    //     }
    // ],

    // yAxis : [
    //     {
    //         type : 'value'
    //     }
    // ],

    // series : [
    //     {
    //         name:'邮件营销',
    //         type:'line',
    //         stack: '总量',
    //         areaStyle: {normal: {}},
    //         data:[120, 132, 101, 134, 90, 230, 210]
    //     },
    //     {
    //         name:'联盟广告',
    //         type:'line',
    //         stack: '总量',
    //         areaStyle: {normal: {}},
    //         data:[220, 182, 191, 234, 290, 330, 310]
    //     },
    //     {
    //         name:'视频广告',
    //         type:'line',
    //         stack: '总量',
    //         areaStyle: {normal: {}},
    //         data:[150, 232, 201, 154, 190, 330, 410]
    //     },
    //     {
    //         name:'直接访问',
    //         type:'line',
    //         stack: '总量',
    //         areaStyle: {normal: {}},
    //         data:[320, 332, 301, 334, 390, 330, 320]
    //     },
    //     {
    //         name:'搜索引擎',
    //         type:'line',
    //         stack: '总量',
    //         label: {
    //             normal: {
    //                 show: true,
    //                 position: 'top'
    //             }
    //         },
    //         areaStyle: {normal: {}},
    //         data:[820, 932, 901, 934, 1290, 1330, 1320]
    //     }
    // ]
};
var queue_charts_option = {
    // xAxis: {
    //     type: 'category',
    //     data: xAxis_vec
    // },
    title: {
        text: '队列情况'
    },
    dataZoom: [
        {
            show: true,
            realtime: true,
            start: 65,
            end: 85
        },
        {
            type: 'inside',
            realtime: true,
            start: 65,
            end: 85
        }
    ],
    tooltip: {
        trigger: 'axis',
        axisPointer: {
            type: 'cross',
            label: {
                backgroundColor: '#6a7985'
            }
        }
    },
    yAxis: {
        type: 'value'
    },

    toolbox: {
        feature: {
            dataZoom: {
                yAxisIndex: 'none'
            },
            restore: {},
            saveAsImage: {}
        }
    },
    // series: [{
    //     data: queue_vec,
    //     type: 'line',
    //     smooth: true
    // }]
};




var performance = {
    _div_performance_show: null,
    _performance_chart: null,

    bind_div: function () {
        let self = this;
        if (arguments.length < 1) {
            throw "Not enough arguments";
            return;
        }

        this._div_performance_show = arguments[0];
        this._performance_chart = echarts.init(this._div_performance_show);
        this.run_queue_charts();
        //this.run_process_charts();
    },

    refresh: function () {

    },
    run_queue_charts: function () {
        console.log("Run chats");

        // 队列中的详细信息
        var queue_vec = queue_data.queue_data_faster;

        var xAxis_vec = [];
        for (var i = 0; i < queue_vec.length; i++) {
            xAxis_vec.push(i * 20 + 's');
        }

        queue_charts_option['xAxis'] = {
            type: 'category',
            data: xAxis_vec
        }
        queue_charts_option['series'] = [{
            data: queue_vec,
            type: 'line',
            smooth: true
        }]

        this._performance_chart.setOption(queue_charts_option);
    },
    run_process_charts: function() {
        let process_dict = {};

        let orig_arr = process_data.process_data_faster;
        let GAP = 2;

        // 构造process_dict
        for(var i = 0; i < orig_arr.length; i++) {
            let k = orig_arr[i][0];
            let v = orig_arr[i][1];
            if(process_dict.hasOwnProperty(k)) {
                process_dict[k].push(v);
            } else {
                process_dict[k] = [];
                process_dict[k].push(v);
            }
        }
        console.log(process_dict);


        // 由于点太过密集, 决定每10个点只取一个
        for (let key of Object.keys(process_dict)){
            let small_values = [];
            let i = 0;
            for(let v of process_dict[key]) {
                if (i % GAP == 0)
                    small_values.push(v);
                i ++;
            }
            process_dict[key] = small_values;
        }

        let process_legend = [];
        let min_length = 2147483648;
        for (let key of Object.keys(process_dict)){
            process_legend.push(key);
            if(process_dict[key].length < min_length)
                min_length = process_dict[key].length;
        }

        let process_series = [];
        for(let process_id of process_legend) {
            process_series.push({
                name      : 'process' + process_id,
                type      : 'line',
                stack     : 'all_packets',
                areaStyle : {normal: {}},
                data      : process_dict[process_id].slice(0, min_length)
            });
        }
        let xAxis_data = [];
        for(var i = 0; i < min_length; i++){
            xAxis_data.push(i * GAP + 's');
        }
        console.log(process_series);
        process_charts_option['xAxis'] = [{
            type : 'category',
            boundaryGap : false,
            data : xAxis_data
        }];
        process_charts_option['yAxis'] = [{
            type : 'value'
        }];
        process_charts_option['series'] = process_series;
        process_charts_option['legend'] = {
            data: process_legend
        };

        console.log(process_charts_option);
        this._performance_chart.setOption(process_charts_option);
    },
}


export { performance };
