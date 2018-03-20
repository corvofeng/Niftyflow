/*
 *=======================================================================
 *    Filename:log.h
 *
 *     Version: 1.0
 *  Created on: March 18, 2018
 *
 *      Author: corvo
 *=======================================================================
 */
#ifndef LOG_H_EVNJHQW5
#define LOG_H_EVNJHQW5

#include <string.h>
#include "lock.h"
#include <pthread.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <cstdarg>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <iomanip>      // std::setw



#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

enum Level { ERROR, INFO, WARNING, DEBUG };

/**
 * https://stackoverflow.com/questions/3165563/flexible-logger-class-using-standard-streams-in-c
 */
class Logger
{
    std::ostream *m_out; // use pointer so you can change it at any point
    bool          m_owner;
    Level         lvl;
public:
    static Logger* instance() {
        static Logger l;
        return &l;
    }

    pthread_mutex_t mtx;
    void init(std::ostream* stream, Level lvl) {
        this->setStream(stream, false);
        this->setLevel(lvl);
        pthread_mutex_init(&mtx, NULL);
    }
    static const char* lName(Level l) {
        switch (l) {
            case Level::ERROR:
                return "[ ERROR ]";
                break;
            case Level::WARNING:
                return "[WARNING]";
                break;
            case Level::INFO:
                return "[  INFO ]";
                break;
            case Level::DEBUG:
                return "[ DEBUG ]";
                break;
            default:
                return "[ ERROR ]";
        }
    }

    // constructor is trivial (and ommited)
    virtual ~Logger()
    {
        setStream(0, false);
        pthread_mutex_destroy(&mtx);
    }
    void setLevel(Level lvl){
        this->lvl = lvl;
    }

    /**
     * @brief  
     *  假如当前设置输出Warning及以上的日志, 将会忽略Debug日志
     *    此时当l 为Debug时, 返回false
     * @param l
     *
     * @return 
     */
    bool isLvlValid(Level l) {
        if(this->lvl >= l) {
            return true;
        } else {
            return false;
        }
    }

    void setStream( std::ostream* stream, bool owner )
    {
        if(m_owner)
            delete m_out;
        m_out = stream;
        m_owner = owner;
    }
    template<typename T>
    Logger& operator << (const T& object)
    {
        if(!m_out)
            throw std::runtime_error("No stream set for Logger class");
        (*m_out) << object;
        return *this;
    }

};
/*
 * Logger logger;
 * logger.setStream( &std::cout, false ); // do not delete std::cout when finished
 * logger << "This will be logged to std::cout" << std::endl;
 * // ...
 * logger.setStream( 
 *     new std::ofstream("myfile.log", std::ios_base::ate|std::ios_base::app), 
 *     true ); // delete the file stream when Logger goes out of scope
 * logger << "This will be appended to myfile.log" << std::endl;
 */

/**
 * 线程首先获取锁, 之后调用单例对象进行. 这是线程安全的日志记录方式
 * 修改前请了解你的所作所为. 修改后几乎需要重新编译所有的库
 */
#define LOG_L(lvl, msg) do{                                                    \
    if(Logger::instance()->isLvlValid(lvl)) {                                  \
        Lock l(&(Logger::instance()->mtx));                                    \
        ((*Logger::instance())                                                 \
            << Logger::lName(lvl)                                              \
            << "[" <<  __FILENAME__  << ":" << std::setw(3) << __LINE__ << "] " \
            << syscall(SYS_gettid) << " "                                      \
            <<  msg);                                                          \
    }                                                                          \
  } while(0)

#define LOG_D(msg) LOG_L(Level::DEBUG   , msg)
#define LOG_W(msg) LOG_L(Level::WARNING , msg)
#define LOG_I(msg) LOG_L(Level::INFO    , msg)
#define LOG_E(msg) LOG_L(Level::ERROR   , msg)

//((*Logger::instance()) << "[" <<  __FILENAME__  << ":" <<__LINE__ << "] "<<  msg);\
// For quick access you could define a macro

/**
 * copy from
 * https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
 *
 * missing string printf
 * this is safe and convenient but not exactly efficient
 */
inline std::string FMT(const char* fmt, ...){
    int size = 512;
    char* buffer = 0;
    buffer = new char[size];
    va_list vl;
    va_start(vl, fmt);
    int nsize = vsnprintf(buffer, size, fmt, vl);
    if(size<=nsize){ //fail delete buffer and try again
        delete[] buffer;
        buffer = 0;
        buffer = new char[nsize+1]; //+1 for /0
        nsize = vsnprintf(buffer, size, fmt, vl);
    }
    std::string ret(buffer);
    va_end(vl);
    delete[] buffer;
    return ret;
}

#endif /* end of include guard: LOG_H_EVNJHQW5 */
