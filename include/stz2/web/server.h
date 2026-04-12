#pragma once

#include "stz2/types.h"
#include <uv.h>

// ---------------  Helpers ---------------

typedef struct
{
    Str     type;
    Str     path;
    KeyVals headers;
    Str     body;

    Strs path_parts;
    Str  filename;
    Str  ext;
} RequestInfo;

int request_info_parse(Buf* a, RequestInfo* req, Str src);

// --------------- Data ---------------

typedef struct
{
    uv_work_cb process_cb;

    time_t     start;
    uv_tcp_t   client;
    uv_timer_t timer;

    int work_started;

    RequestInfo info;

    struct
    {
        char*     buf;
        isize     len;
        isize     cap;
        uv_work_t work;
    } req;

    struct
    {
        char*      buf;
        isize      len;
        isize      cap;
        uv_write_t write;
    } res;

} Data;

int data_create(Data** dp, uv_work_cb process_cb, isize request_capacity, isize response_capacity);
int data_destroy(Data* d);

// ---------------  Callbacks (in order) ---------------

void cb_close(uv_handle_t* handle);
void cb_connect(uv_stream_t* server, int status);

void cb_req_timeout(uv_timer_t* handle);
void cb_req_alloc(uv_handle_t* handle, size_t size, uv_buf_t* buf);
void cb_req_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
void cb_req_process(uv_work_t* req);
void cb_req_write(uv_work_t* req, int status);
void cb_req_finish(uv_write_t* req, int status);
