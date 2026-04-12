#include "stz2/web/server.h"

static inline Str path_extension(Str filename)
{
    char* dot = strrchr(filename.buf, '.');
    if (!dot || dot == filename.buf) return (Str){};
    return (Str){dot + 1, filename.len - (dot + 1 - filename.buf)};
}

int request_info_parse(Buf* a, RequestInfo* info, Str src)
{
    // TODO: Should not ignore empty, instead should count, so we can get body
    Strs lines = str_splitc(a, src, '\n', 64, STRS_SPLIT_IGNORE_EMPTY);
    if (!lines.len) return -1;

    // Parse `REQ_TYPE path protocol` (e.g. `GET / HTTP1.1`)
    Strs parts = str_splitc(a, lines.buf[0], ' ', 4, STRS_SPLIT_IGNORE_EMPTY);
    if (parts.len != 3) return -1;
    info->type = parts.buf[0];
    info->path = parts.buf[1]; // Ignoring protocol

    // Split path to parse
    info->path_parts = str_splitc(a, info->path, '/', 32, STRS_SPLIT_DEFAULT);
    if (!info->path_parts.len) return -1;

    // Last path should have filename
    info->filename = info->path_parts.buf[info->path_parts.len - 1];
    info->ext      = path_extension(info->filename);

    // Parse headers
    isize count = 0;
    // TODO:
    // info->headers = array_new(StrPairs, a, KeyVal, lines.len - 1, ALLOC_ZERO);
    RANGE(i, 1, lines.len)
    {
        KeyVal kv = str_split_keyval(lines.buf[i], ':', STRS_TRIM_DEFAULT | STRS_TRIM_NEWLINES);
        if (!kv.key.len) continue;
        if (kv.key.len == 1)
        {
            if (kv.key.buf[0] == '\r') continue;
        }

        info->headers.buf[count] = kv;
        count++;
    }
    // TODO:
    // array_shrink((&info->headers), (a), KeyVal, count);

    return 0;
}

// --------------- Data ---------------

int data_create(Data** dp, uv_work_cb process_cb, isize request_capacity, isize response_capacity)
{
    Data* d = calloc(1, sizeof(Data));
    OnError_Return(!d, -ENOMEM, "Failed: calloc Data");

    d->process_cb = process_cb;
    d->req.cap    = request_capacity;
    d->res.cap    = response_capacity;

    d->client.data    = d;
    d->timer.data     = d;
    d->req.work.data  = d;
    d->res.write.data = d;

    *dp = d;

    return 0;
}

int data_destroy(Data* d)
{
    if (!d) return 0;

    uv_close((uv_handle_t*)&d->client, cb_close);
    uv_close((uv_handle_t*)&d->timer, cb_close);

    if (d->req.buf) free(d->req.buf);

    if (d->res.buf) free(d->res.buf);

    free(d);

    return 0;
}

// --------------- Callbacks ---------------

void cb_close(uv_handle_t* /* handle */) {}

void cb_connect(uv_stream_t* server, int status)
{
    if (status == -1) { return; }

    Data* data;
    int   err = data_create(&data, server->data, 16 * MB_, 64 * MB_);
    if (err == -ENOMEM) { return; }

    data->timer.data = data;
    data->start      = time(NULL);

    uv_tcp_init(server->loop, &data->client);

    if (uv_accept(server, (uv_stream_t*)&data->client) == 0)
    {
        uv_timer_init(server->loop, &data->timer);
        uv_timer_set_repeat(&data->timer, 1);
        uv_timer_start(&data->timer, cb_req_timeout, 10000, 20000);
        uv_read_start((uv_stream_t*)&data->client, cb_req_alloc, cb_req_read);
    }
    else
    {
        data_destroy(data);
    }
}

void cb_req_timeout(uv_timer_t* handle)
{
    Data* data = handle->data;

    uv_timer_stop(handle);
    if (data->work_started) return;
    else
    {
        data_destroy(data);
    }
}

void cb_req_alloc(uv_handle_t* /* handle */, size_t size, uv_buf_t* buf)
{
    char* base = (char*)calloc(1, size);
    if (!base) *buf = uv_buf_init(NULL, 0);
    else *buf = uv_buf_init(base, size);
}

void cb_req_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
    Data* data = stream->data;

    // Bytes read in this 'round' of cb_req_read
    if (nread == -1 || nread == UV_EOF) goto __error;

    // Allocate req buffer only first time
    if (!data->req.buf)
    {
        data->req.buf = calloc(data->req.cap, sizeof(char));
        if (!data->req.buf) goto __error;
    }

    // Copy to request buffer
    if (data->req.len + nread >= data->req.cap) goto __error;
    memcpy(&data->req.buf[data->req.len], buf->base, nread);
    data->req.len += nread;

    // Free uv buffer, allocated with calloc in cb_req_alloc
    free(buf->base);

    // Process valid requests
    if (!data->work_started //
        && data->req.len    //
        && (strstr(data->req.buf, "\r\n") || strstr(data->req.buf, "\n\n")))
    {
        data->work_started = 1;
        uv_read_stop(stream);
        uv_queue_work(stream->loop, &data->req.work, cb_req_process, cb_req_write);
    }

    return; // !! Important !!

__error:
    free(buf->base);
    uv_timer_stop(&data->timer);
    data_destroy(data);
    return;
}

void cb_req_process(uv_work_t* req)
{
    Data* data = req->data;

    Buf temp = buf_new(MB_);

    int err = request_info_parse(&temp, &data->info, Str_(data->req));
    if (err) goto __error;

    // Allocate space for response
    data->res.buf = calloc(data->res.cap, sizeof(char));
    if (!data->res.buf) goto __error;
    data->res.len = 0;

    // NOTE: Hand off to handler -> main part
    if (data->process_cb) data->process_cb(req);

    // Free headers, path, etc and leave only response
    buf_free(&temp);

    return;

__error:
    p_line("\n--[Failed to parse request]--\n%.*s", _s(data->req));

    data->res.buf = strdup("Got error");
    data->res.len = strlen(data->res.buf);
    buf_free(&temp);
}

void cb_req_write(uv_work_t* req, int status)
{
    if (status == -1) { return; }
    Data* data = req->data;

    uv_timer_stop(&data->timer);

    uv_buf_t buf = uv_buf_init(data->res.buf, data->res.len + 1);

    uv_write(&data->res.write, (uv_stream_t*)&data->client, &buf, 1, cb_req_finish);
}

void cb_req_finish(uv_write_t* req, int status)
{
    if (status == -1) { return; }
    Data* data = req->data;

    data_destroy(data);
}
