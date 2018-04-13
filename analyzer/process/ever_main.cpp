#include "ever_main.h"
#include "reader.h"

EverflowMain::EverflowMain() {
    LOG_D("EverflowMain Init\n");
    this->processer_cnt = 3;
    this->reader_cnt = 1;   // 目前为单线程读入

    std::string file = "/home/corvo/Dropbox/课程文件/毕业设计/grecap.cap";
    char errbuff[PCAP_ERRBUF_SIZE];

    pcap_t * p = pcap_open_offline(file.c_str(), errbuff);
    pcap_vec.push_back(p);

    for(int i = 0; i < this->processer_cnt; i++) {
        auto p = shared_ptr<Processer>(new Processer());
        shared_ptr<PKT_QUEUE> q = shared_ptr<PKT_QUEUE>(new PKT_QUEUE);

        p->bind_queue(q);
        processer_vec.push_back(p);
        queue_vec.push_back(q);
    }

    for(int i = 0; i < reader_cnt && i < pcap_vec.size(); i++) {
        auto r = shared_ptr<Reader>(new Reader());
        r->bind_queue_vec(&this->queue_vec);
        r->bind_pcap(pcap_vec[i]);
        r->bind_counter_map(&this->counter_map);
        reader_vec.push_back(r);
    }
}

void EverflowMain::run() {
    for(auto p : processer_vec)
        p->run();
    for(auto r: reader_vec)
        r->run();
}

void EverflowMain::join() {
    for(auto p : processer_vec)
        p->join();

    for(auto r: reader_vec)
        r->join();
}

EverflowMain::~EverflowMain() {
    for(auto p: pcap_vec) {
        pcap_close(p);
    }
}

void EverflowMain::add_rules(vector<CounterRule>& rules) {
    for(auto rule : rules) {
        //this->counter_map.insert(std::make_pair<CounterRule, Counter>
        //    (CounterRule(rule), Counter(0)));
        //    一旦获取计数器, 使用shared_ptr, 防止内存泄露
        this->counter_map[CounterRule(rule)] = shared_ptr<Counter>(new Counter(0));
    }
}
void EverflowMain::del_rules(vector<CounterRule>& rules) {
    for(auto rule: rules) {
        this->counter_map.erase(rule);
    }
}

void EverflowMain::reader_pause() {
    LOG_I("Pause reader start !!\n");
    for(auto r: reader_vec) {
        r->do_pause();
    }
    bool allPause = false;

    while(!allPause) {  // 确保每个读入线程均暂停
        allPause = true;
        for(auto r: reader_vec) {
           if(!r->is_pause_ok())	// 只要有一个线程没有暂停成功, 程序就会一直循环
               allPause = false;
        }
    }
    LOG_I("Pause all reader ok !!\n");
    return ;
}

void EverflowMain::reader_active() {
    for(auto r: reader_vec)
        r->cancel_pause();
    LOG_I("All reader restart !!\n");
}




