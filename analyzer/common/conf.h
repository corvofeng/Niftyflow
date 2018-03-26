/*
 *=======================================================================
 *    Filename:conf.h
 *
 *     Version: 1.0
 *  Created on: March 18, 2018
 *
 *      Author: corvo
 *=======================================================================
 */
#ifndef CONF_H_DIL08GJQ
#define CONF_H_DIL08GJQ

#include "cJSON/cJSON.h"
#include <stdlib.h>
#include <string.h>
#include "log.h"

/**
 *
 *
 * {
 *   "mysql": {
 *     "host": "127.0.0.1",
 *     "port": 3306,
 *     "user": "root",
 *     "password": "***"
 *   },
 *   "redis": {
 *     "host": "127.0.0.1",
 *     "port": 6379,
 *     "auth": "***",
 *     "subscribe_chanel": "ever_chanel",
 *     "alert_queue": "ever_queue"
 *   }
 * }%
 */

class Conf {
private:
    Conf(): mysql_host(NULL),
            mysql_user(NULL),
            mysql_password(NULL),
            mysql_database(NULL),
            redis_host(NULL),
            redis_auth(NULL),
            redis_chanel(NULL),
            redis_queue(NULL) {}
public:
    char* mysql_host;
    int   mysql_port;
    char* mysql_user;
    char* mysql_password;
    char* mysql_database;

    char* redis_host;
    int   redis_port;
    char* redis_auth;
    char* redis_chanel;
    char* redis_queue;

    static Conf* instance() {
        static Conf c;
        return &c;
    }

    void ConfRead(const char* jFile) {
        cJSON *jConf = cJSON_Parse(jFile);
        if(!jConf) {
            LOG_E("Read file error\n");
        }

        cJSON *jMysql = cJSON_GetObjectItem(jConf, "mysql");
        cJSON *host = cJSON_GetObjectItem(jMysql, "host");
        cJSON *port = cJSON_GetObjectItem(jMysql, "port");
        cJSON *user = cJSON_GetObjectItem(jMysql, "user");
        cJSON *password = cJSON_GetObjectItem(jMysql, "password");
        cJSON *database = cJSON_GetObjectItem(jMysql, "database");

        cJSON *jRedis = cJSON_GetObjectItem(jConf, "redis");
        cJSON *jhost = cJSON_GetObjectItem(jRedis, "host");
        cJSON *jport = cJSON_GetObjectItem(jRedis, "port");
        cJSON *jauth = cJSON_GetObjectItem(jRedis, "auth");
        cJSON *jchanel = cJSON_GetObjectItem(jRedis, "subscribe_chanel");
        cJSON *jqueue = cJSON_GetObjectItem(jRedis, "alert_queue");

        bool parse_ok = false;
        do {

            if(host->type==cJSON_String) {
                this->mysql_host = (char*)malloc(strlen(host->valuestring) + 1);
                strcpy(this->mysql_host, host->valuestring);
            } else break;

            if(port->type==cJSON_Number) {
                this->mysql_port = port->valueint;
            } else break;

            if(user->type==cJSON_String) {
                this->mysql_user = (char*)malloc(strlen(user->valuestring) + 1);
                strcpy(this->mysql_user, user->valuestring);
            } else break;

            if(password->type==cJSON_String) {
                this->mysql_password = (char*)malloc(strlen(password->valuestring)+1);
                strcpy(this->mysql_password, password->valuestring);
            } else break;

            if(database->type==cJSON_String) {
                this->mysql_database = (char*)malloc(strlen(database->valuestring) + 1);
                strcpy(this->mysql_database, database->valuestring);
            } else break;

            if(jhost->type==cJSON_String) {
                this->redis_host = (char*)malloc(strlen(jhost->valuestring) + 1);
                strcpy(this->redis_host, jhost->valuestring);
            } else break;

            if(jauth->type==cJSON_String) {
                this->redis_auth = (char*)malloc(strlen(jauth->valuestring) + 1);
                strcpy(this->redis_auth, jauth->valuestring);
            } else break;

            if(jport->type==cJSON_Number) {
                this->redis_port = jport->valueint;
            } else break;

            if(jchanel->type==cJSON_String) {
                this->redis_chanel = (char*)malloc(strlen(jchanel->valuestring) + 1);
                strcpy(this->redis_chanel, jchanel->valuestring);
            } else break;

            if(jqueue->type==cJSON_String) {
                this->redis_queue = (char*)malloc(strlen(jqueue->valuestring) + 1);
                strcpy(this->redis_queue, jqueue->valuestring);
            } else break;

            parse_ok = true;
        } while(0);

        cJSON_Delete(jConf);

        if(!parse_ok) {
            LOG_E("The file is not valid\n");
            exit(-1);
        }
    }

    void try_clean() {
        LOG_D("In conf delete\n");
        if(mysql_host)     free(mysql_host);
        if(mysql_user)     free(mysql_user);
        if(mysql_password) free(mysql_password);
        if(mysql_database) free(mysql_database);
        if(redis_host)     free(redis_host);
        if(redis_auth)     free(redis_auth);
        if(redis_chanel)   free(redis_chanel);
        if(redis_queue)    free(redis_queue);
    }

    ~Conf() {
        try_clean();
    }

};


#endif /* end of include guard: CONF_H_DIL08GJQ */

