#include <iostream>
#include <memory>
#include <string>
#include <cstdio>
#include <filesystem>

#include <spatialindex/SpatialIndex.h>

#include "utils.hpp"

using namespace std;
using namespace SpatialIndex;


struct query_visitor : public IVisitor
{
   query_visitor() : result(0) {}

    void visitNode(INode const& n) { }

    void visitData(IData const& d)
    {
        uint8_t *pd;
        uint32_t size;
        d.getData(size, &pd);
        assert (size == 4);

        union{
            uint32_t integer;
            uint8_t data[4];
        } ud;
        for (int i = 0; i < 4; i++)
            ud.data[i] = pd[i];
        result += ud.integer;
    }

    void visitData(vector<IData const*>& v)
    {
        assert (v.empty());
    }

    uint64_t result;
};

int main()
{
    uint32_t const index_capacity = 100;
    uint32_t const leaf_capacity = 100;
    uint32_t const dimension = 3;
    double const fill_factor = 0.3;
    RTree::RTreeVariant const variant = RTree::RV_RSTAR; // RV_LINEAR; RV_QUADRATIC; RV_RSTAR;
    id_type index_id;
    string basename("storage_lsl");

    // remove existing index
    filesystem::remove(filesystem::path(basename + ".idx"));
    filesystem::remove(filesystem::path(basename + ".dat"));

    try
    {
        unique_ptr<IStorageManager> diskfile(StorageManager::createNewDiskStorageManager(basename, 16384));
        unique_ptr<StorageManager::IBuffer> file(StorageManager::createNewRandomEvictionsBuffer(*diskfile, 10, false));
        unique_ptr<ISpatialIndex> rtree(RTree::createNewRTree(*file,
                    fill_factor, index_capacity, leaf_capacity, dimension, variant, index_id));

        auto insert_test = [&rtree, &basename] (boxes3d_t const& boxes, bool silent = false)
        {
            if (!silent)
                cout << "===== Insertion test with " << boxes.size() << " boxes =====" << endl;

            auto tstart = timer_start();
            for (size_t i = 0; i < boxes.size(); ++i)
            {
                auto const& box = boxes[i];
                coord_array_t const p1 = { get<0>(box), get<2>(box), get<4>(box) };
                coord_array_t const p2 = { get<1>(box), get<3>(box), get<5>(box) };
                Region region(
                    Point(p1.data(), p1.size()),
                    Point(p2.data(), p2.size()));
                id_type item_id(i);

                union{
                    uint32_t integer;
                    uint8_t data[4];
                } d;
                d.integer = i;

                rtree->insertData(4, d.data, region, item_id);
            }
            
            if (!silent)
            {
                cout << "Insertion time: " << timer_end_ms(tstart, boxes.size()) << " ms/box" << endl;
                cout << "Current file size: " << filesystem::file_size(filesystem::path(basename + ".dat"))
                    + filesystem::file_size(filesystem::path(basename + ".idx")) << endl;
            }
        };

        auto const query_test = [&rtree] (boxes3d_t const& boxes)
        {
            uint64_t result;
            cout << "===== Query test with " << boxes.size() << " boxes =====" << endl;
            auto tstart = timer_start();
            for (size_t i = 0; i < boxes.size(); ++i)
            {
                auto const& box = boxes[i];
                coord_array_t const p1 = { get<0>(box) - 10, get<2>(box) - 10, get<4>(box) - 10 };
                coord_array_t const p2 = { get<1>(box) + 10, get<3>(box) + 10, get<5>(box) + 10 };
                Region region(
                    Point(p1.data(), p1.size()),
                    Point(p2.data(), p2.size()));
                query_visitor qvisitor;
                rtree->intersectsWithQuery(region, qvisitor);

                result += qvisitor.result;
            }
            cout << "Query time: " << timer_end_ms(tstart, boxes.size()) << " ms/box" << endl;
            cout << "Query checksum: " << result << endl;
            // return result;
        };

        // warm up
        boxes3d_t boxes = generate_boxes(100);
        insert_test(boxes, true);

        boxes = generate_boxes(100);
        insert_test(boxes);
        boxes = generate_boxes(100);
        query_test(boxes);
        boxes = generate_boxes(1000);
        insert_test(boxes);
        boxes = generate_boxes(1000);
        query_test(boxes);
        boxes = generate_boxes(10000);
        insert_test(boxes);
        boxes = generate_boxes(10000);
        query_test(boxes);
        boxes = generate_boxes(100000);
        insert_test(boxes);
    }
    catch(Tools::Exception& e)
    {
        cout << e.what() << endl;
    }
    cout << "Benchmark End." << endl;
    cout << "File size after flush: " << filesystem::file_size(filesystem::path(basename + ".dat"))
        + filesystem::file_size(filesystem::path(basename + ".idx")) << endl;
}