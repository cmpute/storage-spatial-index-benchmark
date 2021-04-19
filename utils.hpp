#include <vector>
#include <tuple>
#include <array>
#include <random>
#include <chrono>

typedef float coord_t;
typedef std::array<double, 3> coord_array_t;
typedef std::vector<coord_t> coords_t;
typedef std::tuple<coord_t, coord_t, coord_t> point3d_t;
typedef std::vector<point3d_t> points3d_t;
typedef std::tuple<coord_t, coord_t, coord_t, coord_t, coord_t, coord_t> box3d_t;
typedef std::vector<box3d_t> boxes3d_t;

template <typename T>
struct random_generator
{
    typedef typename std::uniform_real_distribution<T>::result_type result_type;

    T const max;
    std::mt19937 gen;
    std::uniform_real_distribution<T> dis;

    random_generator(std::size_t n)
        : max(static_cast<T>(n / 2))
        , gen(1) // generate the same succession of results for everyone
        , dis(-max, max)
    {}
    
    result_type operator()()
    {
        return dis(gen);
    }

private:
    random_generator(random_generator const&) /*= delete*/;
    random_generator& operator=(random_generator const&) /*= delete*/;
};


inline coords_t generate_coordinates(std::size_t n)
{
    random_generator<float> rg(n);
    coords_t coords;
    coords.reserve(n);
    for (decltype(n) i = 0; i < n; ++i)
    {
        coords.emplace_back(rg());
    }
    return std::move(coords);
}

inline points3d_t generate_points(std::size_t n)
{
    auto coords = generate_coordinates(n * 3);
    points3d_t points;
    points.reserve(n);
    auto s = coords.size();
    for (decltype(s) i = 0; i < s; i += 3)
    {
        points.emplace_back(coords[i], coords[i + 1], coords[i + 2]);
    }
    return std::move(points);
}

inline boxes3d_t generate_boxes(std::size_t n)
{
    random_generator<float> rg(n);
    
    boxes3d_t boxes;
    boxes.reserve(n);
    for (decltype(n) i = 0; i < n; ++i)
    {
        auto const x = rg();
        auto const y = rg();
        auto const z = rg();
        boxes.emplace_back(x - 0.5f, x + 0.5f, y - 0.5f, y + 0.5f, z - 0.5f, z + 0.5f);
    }
    return std::move(boxes);   
}

inline std::chrono::system_clock::time_point timer_start()
{
    return std::chrono::system_clock::now();
}

inline double timer_end_ms(std::chrono::system_clock::time_point tstart, size_t n = 1)
{
    auto d = std::chrono::system_clock::now() - tstart;
    std::chrono::milliseconds dms = std::chrono::duration_cast<std::chrono::milliseconds>(d);
    return (double)dms.count() / n;
}
