#pragma once

#include "stz2/types.h"

#include <sqlite3.h>

// --------------- sqlite3 tables ---------------

typedef struct
{
    Str   name;       ///< its name
    Str   type;       ///< data type if given, else ''
    bool  notnull;    ///< whether or not the column can be NULL
    void* dflt_value; ///< the default value for the column
    bool  pk;         ///< either zero for columns that are not part of the primary key,
                      ///   or the 1-based index of the column within the primary key
} DbColumn;

typedef void* DbRow;

DECLARE_ARRAY(DbColumns, DbColumn);
DECLARE_ARRAY(DbRows, DbRow);

typedef struct
{
    Str name; ///< its name
    Str type; ///< data type if given, else ''

    DbColumns cols; ///< typed columns
    DbRows    rows; ///< untyped rows

} DbTable;

DECLARE_ARRAY(DbTables, DbTable);

// --------------- Generic statement ---------------

typedef struct DbStmt DbStmt;

typedef int (*db_stmt_callback_fn)(DbStmt* s, isize idx, void* data);

typedef struct DbStmt
{
    Str0          path;
    Str0          sql;
    sqlite3*      db;
    sqlite3_stmt* stmt;

    db_stmt_callback_fn start;
    db_stmt_callback_fn step;
    db_stmt_callback_fn finish;

    Buf*  mem;     // Memory to keep in scope after statement
    isize limit;   // Often needed, so just stick it in here
    isize col_idx; // As above

} DbStmt;

// Generic
int db_stmt_exec(DbStmt* s, void* data);

// Db level
Strs db_list_tables(Buf* a, Str0 path, isize limit);

// Table level
Strs db_list_columns(Buf* a, Str0 path, Str table, isize limit);

// Various generic helpers
int db_stmt_start_strs(DbStmt* s, isize, void* data);
int db_stmt_step_strs(DbStmt* s, isize, void* data);
int db_stmt_finish_strs(DbStmt* s, isize, void* data);
