#include "stz2/db.h"

#include "stz2/test.h"

int main(int argc, char* argv[])
{
    TEST_SUITE("Db");

    TEST_CASE("Size") { EXPECT_EQ_LONG(sizeof(DbStmt), 104L); }

    TEST_CASE("tables, columns")
    {
        Str0 path = _0("tests/data/chinook.db");
        buf_stack(b, 1024);
        Strs tables = db_list_tables(&b, path, 32);
        PrintLn(tables);

        buf_reset(&b);
        Str  table   = tables.buf[0];
        Strs columns = db_list_columns(&b, path, table, 32);
        PrintLn(columns);
    }

    TEST_CASE("rows")
    {
        Buf b = buf_new(MB_);

        Str0 path   = _0("tests/data/chinook.db");
        Strs tables = db_list_tables(&b, path, 32);

        RANGE(t, tables.len)
        {
            Buf     a     = b;
            Str     table = tables.buf[t];
            Strs    cols  = db_list_columns(&a, path, table, 32);
            StrsArr rows  = db_list_rows(&a, path, table, 32);

            PrintLn(cols);
            RANGE(i, rows.len) { PrintLn(rows.buf[i]); }
        }

        buf_free(&b);
    }

    return TEST_RESULTS();
}
