#include <iostream>
#include <memory>
#include <string>
#include <cstdio>
#include <cassert>
#include <filesystem>

#include <sqlite3.h>

#include "utils.hpp"

using namespace std;

int main()
{
    sqlite3 *db;
    int rc; char *errmsg;
    filesystem::remove(filesystem::path("storage_sqlite3.db3"));
    filesystem::remove(filesystem::path("storage_sqlite3.db3-journal"));
    sqlite3_open("storage_sqlite3.db3", &db);

    const string sql_create_table = "CREATE VIRTUAL TABLE IF NOT EXISTS si USING rtree (" \
        "id, minX, maxX, minY, maxY, minZ, maxZ, +data BLOB)";
    assert (sqlite3_exec(db, sql_create_table.c_str(), NULL, NULL, &errmsg) == SQLITE_OK);
    assert (sqlite3_exec(db, "PRAGMA page_size=16384;", NULL, NULL, &errmsg) == SQLITE_OK);

    // try
    // {
        auto insert_test = [&db] (boxes3d_t const& boxes, int id_start)
        {
            if (id_start != 0)
                cout << "===== Insertion test with " << boxes.size() << " boxes =====" << endl;

            auto tstart = timer_start();

            assert (sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL) == SQLITE_OK);
            const string sql_insert_box = "INSERT INTO si VALUES(?,?,?,?,?,?,?,?);";
            sqlite3_stmt *stmt;
            assert (sqlite3_prepare_v2(db, sql_insert_box.c_str(), -1, &stmt, NULL) == SQLITE_OK);

            for (size_t i = 0; i < boxes.size(); ++i)
            {
                auto const& box = boxes[i];
                assert (sqlite3_bind_int(stmt, 1, i + id_start) == SQLITE_OK);
                assert (sqlite3_bind_double(stmt, 2, get<0>(box)) == SQLITE_OK);
                assert (sqlite3_bind_double(stmt, 3, get<1>(box)) == SQLITE_OK);
                assert (sqlite3_bind_double(stmt, 4, get<2>(box)) == SQLITE_OK);
                assert (sqlite3_bind_double(stmt, 5, get<3>(box)) == SQLITE_OK);
                assert (sqlite3_bind_double(stmt, 6, get<4>(box)) == SQLITE_OK);
                assert (sqlite3_bind_double(stmt, 7, get<5>(box)) == SQLITE_OK);

                union{
                    uint32_t integer;
                    uint8_t data[4];
                } d;
                d.integer = i;
                assert (sqlite3_bind_blob(stmt, 8, d.data, 4, NULL) == SQLITE_OK);
                
                if (sqlite3_step(stmt) != SQLITE_DONE)
                {
                    cout << i << " " << sqlite3_errmsg(db) << endl;
                    assert (false);
                }
                assert (sqlite3_reset(stmt) == SQLITE_OK);
            }
            assert (sqlite3_finalize(stmt) == SQLITE_OK);
            assert (sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL) == SQLITE_OK);

            if (id_start != 0)
            {
                cout << "Insertion time: " << timer_end_ms(tstart, boxes.size()) << " ms/box" << endl;
                cout << "Current file size: " << filesystem::file_size(filesystem::path("storage_sqlite3.db3")) << endl;
            }
        };

        auto const query_test = [&db] (boxes3d_t const& boxes)
        {
            uint64_t result;
            cout << "===== Query test with " << boxes.size() << " boxes =====" << endl;
            auto tstart = timer_start();

            const string sql_query_box = "SELECT data FROM si WHERE minX>? AND maxX<?" \
                " AND minY>? AND maxY<? AND minZ>? AND maxZ<?;";
            sqlite3_stmt *stmt;
            assert (sqlite3_prepare_v2(db, sql_query_box.c_str(), -1, &stmt, NULL) == SQLITE_OK);
            for (size_t i = 0; i < boxes.size(); ++i)
            {
                auto const& box = boxes[i];
                assert (sqlite3_bind_double(stmt, 1, get<0>(box)-10) == SQLITE_OK);
                assert (sqlite3_bind_double(stmt, 2, get<1>(box)+10) == SQLITE_OK);
                assert (sqlite3_bind_double(stmt, 3, get<2>(box)-10) == SQLITE_OK);
                assert (sqlite3_bind_double(stmt, 4, get<3>(box)+10) == SQLITE_OK);
                assert (sqlite3_bind_double(stmt, 5, get<4>(box)-10) == SQLITE_OK);
                assert (sqlite3_bind_double(stmt, 6, get<5>(box)+10) == SQLITE_OK);

                int lrc;
                while ((lrc = sqlite3_step(stmt)) == SQLITE_ROW)
                {
                    const uint32_t *cur_result = reinterpret_cast<const uint32_t*>(sqlite3_column_blob(stmt, 0));
                    result += *cur_result;
                }
                assert (lrc == SQLITE_DONE);
                assert (sqlite3_reset(stmt) == SQLITE_OK);
            }
            assert (sqlite3_finalize(stmt) == SQLITE_OK);

            cout << "Query time: " << timer_end_ms(tstart, boxes.size()) << " ms/box" << endl;
            cout << "Query sum: " << result << endl;
            // return result;
        };

        // warm up
        boxes3d_t boxes = generate_boxes(100);
        insert_test(boxes, 0);

        // actual test
        boxes = generate_boxes(100);
        insert_test(boxes, 100);
        boxes = generate_boxes(100);
        query_test(boxes);
        boxes = generate_boxes(1000);
        insert_test(boxes, 200);
        boxes = generate_boxes(1000);
        query_test(boxes);
        boxes = generate_boxes(10000);
        insert_test(boxes, 1200);
        boxes = generate_boxes(10000);
        query_test(boxes);
        boxes = generate_boxes(100000);
        insert_test(boxes, 11200);
    // }
    // catch(Tools::Exception& e)
    // {
    //     cout << e.what() << endl;
    // }
    cout << "End" << endl;
    sqlite3_close(db);

    cout << "File size after flush: " << filesystem::file_size(filesystem::path("storage_sqlite3.db3")) << endl;
}