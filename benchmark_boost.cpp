// https://www.boost.org/doc/libs/1_76_0/libs/geometry/doc/html/geometry/spatial_indexes/rtree_examples/index_stored_in_mapped_file_using_boost_interprocess.html
// To add storage: https://stackoverflow.com/questions/31320633/packing-algorithm-in-rtree-in-boost

#include <iostream>
#include <filesystem>
#include <memory>

#include <boost/interprocess/managed_mapped_file.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>

#include "utils.hpp"

using namespace std;
namespace bi = boost::interprocess;
namespace bg = boost::geometry;
namespace bgm = bg::model;
namespace bgi = bg::index;
namespace sf = std::filesystem;

int main()
{
    typedef bgm::point<float, 3, bg::cs::cartesian> point_t;
    typedef bgm::box<point_t> box_t;
    typedef uint32_t data_t;
    typedef std::pair<box_t, data_t> value_t;
    typedef bgi::rstar<16> params_t;
    typedef bgi::indexable<value_t> indexable_t;
    typedef bgi::equal_to<value_t> equal_to_t;
    typedef bi::allocator<value_t, bi::managed_mapped_file::segment_manager> allocator_t;
    typedef bgi::rtree<value_t, params_t, indexable_t, equal_to_t, allocator_t> rtree_t;

    const char filename[] = "storage_boost.bin";
    sf::remove(sf::path(filename));

    bi::managed_mapped_file file(bi::open_or_create, filename, 1<<24); // reserve enough space (16MB)
    allocator_t alloc(file.get_segment_manager());
    rtree_t * rtree_ptr = file.find_or_construct<rtree_t>("rtree")(params_t(), indexable_t(), equal_to_t(), alloc);

    auto insert_test = [&rtree_ptr, &filename] (boxes3d_t const& boxes, bool silent = false)
    {
        if (!silent)
            cout << "===== Insertion test with " << boxes.size() << " boxes =====" << endl;

        auto tstart = timer_start();
        for (size_t i = 0; i < boxes.size(); ++i)
        {
            auto const& box = boxes[i];
            point_t const p1 = { get<0>(box), get<2>(box), get<4>(box) };
            point_t const p2 = { get<1>(box), get<3>(box), get<5>(box) };
            box_t const b(p1, p2);

            rtree_ptr->insert(std::make_pair(b, (data_t)1234));
        }
        
        if (!silent)
        {
            cout << "Insertion time: " << timer_end_ms(tstart, boxes.size()) << " ms/box" << endl;

            bi::managed_mapped_file::shrink_to_fit(filename);
            auto fsize = sf::file_size(sf::path(filename));
            cout << "Current file size after shrink: " << fsize << endl;
            bi::managed_mapped_file::grow(filename, (1<<24) - fsize);
        }
    };

    auto const query_test = [&rtree_ptr] (boxes3d_t const& boxes)
    {
        uint64_t result;
        cout << "===== Query test with " << boxes.size() << " boxes =====" << endl;
        auto tstart = timer_start();
        for (size_t i = 0; i < boxes.size(); ++i)
        {
            auto const& box = boxes[i];
            point_t const p1 = { get<0>(box) - 10, get<2>(box) - 10, get<4>(box) - 10 };
            point_t const p2 = { get<1>(box) + 10, get<3>(box) + 10, get<5>(box) + 10 };
            box_t const bq(p1, p2);

            for (auto it = rtree_ptr->qbegin(bgi::within(bq)); it != rtree_ptr->qend() ;++it)
                result += it->second;
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

    return 0;
}