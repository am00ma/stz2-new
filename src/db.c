#include "stz2/db.h"

// --------------- The baseline ---------------

int db_stmt_exec(DbStmt* s, void* data)
{
    int err = 0;

    err = sqlite3_open_v2(s->path.buf, &s->db, SQLITE_OPEN_READONLY, 0);
    OnError_Goto(err, __close, "Failed: sqlite3_open_v2");

    err = sqlite3_prepare(s->db, s->sql.buf, -1, &s->stmt, 0);
    OnError_Goto(err, __close, "Failed: sqlite3_prepare");

    if (s->start)
    {
        err = s->start(s, -1, data);
        OnError_Goto(err, __close, "Failed: s->start");
    }

    isize count = 0;
    while ((sqlite3_step(s->stmt) != SQLITE_DONE) && (!err))
    {
        if (s->step)
        {
            err = s->step(s, count, data);
            if (err) break;
        } // Allow to break on error
        count++;
    }

    if (s->finish)
    {
        err = s->finish(s, count, data);
        OnError_Goto(err, __close, "Failed: s->finish");
    }

    err = sqlite3_finalize(s->stmt);
    OnError_Goto(err, __close, "Failed: sqlite3_finalize");

__close:
    int err2 = sqlite3_close(s->db);
    OnError_Warn(err2, "Failed: sqlite3_close");

    return err;
}

// --------------- Get columns as strings ---------------

int db_stmt_start_strs(DbStmt* s, isize, void* data)
{
    Strs* strs = data;
    *strs      = (Strs){
        .buf = make(s->mem, Str, s->limit, ALLOC_NOZERO),
        .len = s->limit,
    };
    return 0;
}

int db_stmt_step_strs(DbStmt* s, isize idx, void* data)
{
    if (idx >= s->limit) return -1;

    Strs* strs     = data;
    char* str      = (char*)sqlite3_column_text(s->stmt, s->col_idx);
    strs->buf[idx] = str_copy(s->mem, (Str){.buf = str, .len = strlen(str)}, false);
    return 0;
}

int db_stmt_finish_strs(DbStmt*, isize idx, void* data)
{
    Strs* strs = data;
    strs->len  = idx;
    return 0;
}

// --------------- Get rows as list of strings ---------------

int db_stmt_start_strsarr(DbStmt* s, isize, void* data)
{
    StrsArr* rows = data;
    *rows         = (StrsArr){
        .buf = make(s->mem, Strs, s->limit, ALLOC_NOZERO),
        .len = s->limit,
    };
    return 0;
}

int db_stmt_step_strsarr(DbStmt* s, isize idx, void* data)
{
    if (idx >= s->limit) return -1;

    StrsArr* rows = data;

    Strs row = (Strs){
        .buf = make(s->mem, Str, s->num_cols, ALLOC_ZERO),
        .len = s->num_cols,
    };

    RANGE(i, s->num_cols)
    {
        char* str = (char*)sqlite3_column_text(s->stmt, i);
        if (!str) str = "";
        row.buf[i] = str_copy(s->mem, (Str){.buf = str, .len = strlen(str)}, false);
    }

    rows->buf[idx] = row;

    return 0;
}

int db_stmt_finish_strsarr(DbStmt*, isize idx, void* data)
{
    StrsArr* rows = data;
    rows->len     = idx;
    return 0;
}

// --------------- Higher level operations ---------------

Strs db_list_tables(Buf* a, Str0 path, isize limit)
{
    int err = 0;

    Strs   tables = {};
    DbStmt s      = {
        .path    = path,
        .sql     = _0("SELECT name FROM sqlite_master WHERE type='table'"),
        .start   = db_stmt_start_strs,
        .step    = db_stmt_step_strs,
        .finish  = db_stmt_finish_strs,
        .mem     = a,
        .limit   = limit,
        .col_idx = 0,
    };

    err = db_stmt_exec(&s, &tables);
    if (err) {} // TODO:

    return tables;
}

Strs db_list_columns(Buf* a, Str0 path, Str table, isize limit)
{
    int err = 0;

    Str0 sql = str0_fmt(a, "SELECT name FROM pragma_table_info('%.*s')", _s(table));
    Assert(IsNullTerm(sql), "Unexepectedly not null terminated: %.*s", _s(sql));

    Strs   columns = {};
    DbStmt s       = {
        .path    = path,
        .sql     = sql,
        .start   = db_stmt_start_strs,
        .step    = db_stmt_step_strs,
        .finish  = db_stmt_finish_strs,
        .mem     = a,
        .limit   = limit,
        .col_idx = 0,
    };

    err = db_stmt_exec(&s, &columns);
    if (err) {} // TODO:

    return columns;
}

StrsArr db_list_rows(Buf* a, Str0 path, Str table, isize limit)
{
    int err = 0;

    Strs columns = db_list_columns(a, path, table, limit);
    if (!columns.len) return (StrsArr){};

    Str0 sql = str0_fmt(a, "SELECT * FROM '%.*s'", _s(table));
    Assert(IsNullTerm(sql), "Unexepectedly not null terminated: %.*s", _s(sql));

    StrsArr rows   = {};
    DbStmt  s_rows = {
        .path     = path,
        .sql      = sql,
        .start    = db_stmt_start_strsarr,
        .step     = db_stmt_step_strsarr,
        .finish   = db_stmt_finish_strsarr,
        .mem      = a,
        .limit    = limit,
        .num_cols = columns.len,
        .col_idx  = 0,
    };

    err = db_stmt_exec(&s_rows, &rows);
    if (err) return (StrsArr){}; // TODO:

    return rows;
}
