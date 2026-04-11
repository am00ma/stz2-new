#include "stz2/db.h"

#include "stz2/test.h"

int main(int argc, char* argv[])
{
    TEST_SUITE("Db");

    TEST_CASE("Size") { EXPECT_EQ_LONG(sizeof(DbStmt), 96L); }

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

    return TEST_RESULTS();
}
