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

/**
 * 2018-03-29: 这是一个线程安全的日志库, 可以直接在其他Linux程序上复用
 */

#include <string.h>
#include "lock.h"
#include <pthread.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <cstdarg>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <iomanip>      // std::setw
#include <stdexcept>    // for std::runtime_error



#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

enum Level { ERROR, INFO, WARNING, DEBUG };
using std::runtime_error;

#define C_RED   "\033[31m"
#define C_GREEN "\033[32m"
#define C_BLUE  "\033[34m"
#define C_PINK  "\033[35m"
#define C_CLEAR "\033[0m"

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
            case Level::WARNING:
                return "[WARNING]";
            case Level::INFO:
                return "[  INFO ]";
            case Level::DEBUG:
                return "[ DEBUG ]";
            default:
                return "[ ERROR ]";
        }
    }

    const char* lColorStart(Level l) {
        if(this->m_out != &std::cout) return "";
        switch (l) {
            case Level::ERROR:
                return C_RED;
            case Level::WARNING:
                return C_PINK;
            case Level::INFO:
                return C_GREEN;
            case Level::DEBUG:
                return C_BLUE;
            default:
                return C_PINK;
        }
    }

    const char* lColorStop(Level l) {
        if(this->m_out != &std::cout) return "";
        return C_CLEAR;
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
            << Logger::instance()->lColorStart(lvl)                            \
            << Logger::lName(lvl)                                              \
            << "[" <<  __FILENAME__  << ":" << std::setw(3) << __LINE__ << "] " \
            << syscall(SYS_gettid) << " "                                      \
            <<  msg                                                            \
            << Logger::instance()->lColorStop(lvl)                             \
        );                                                                     \
    }                                                                          \
  } while(0)

#define LOG_D(msg) LOG_L(Level::DEBUG   , msg)
#define LOG_W(msg) LOG_L(Level::WARNING , msg)
#define LOG_I(msg) LOG_L(Level::INFO    , msg)
#define LOG_E(msg) LOG_L(Level::ERROR   , msg)

//((*Logger::instance()) << "[" <<  __FILENAME__  << ":" <<__LINE__ << "] "<<  msg);\
// For quick access you could define a macro

/**
 * LOG_I(FMT("Hello world %d, %d\n", 1, 3)); 类似printf的功能
 *
 * copy from
 * https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
 *
 * missing string printf
 * this is safe and convenient but not exactly efficient
 * 线程安全但并不是很高效, 请尽量避免
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
