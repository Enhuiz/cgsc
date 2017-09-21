namespace cgsc
{
namespace test
{

template <class T>
std::vector<T> roll(const std::vector<T> &v, int n)
{
    std::vector<T> ret;
    n = n < 0 ? v.size() + n : n;

    for (int i = v.size() - n; i < v.size(); ++i)
    {
        ret.push_back(v[i]);
    }

    for (int i = 0; i < v.size() - n; ++i)
    {
        ret.push_back(v[i]);
    }

    return ret;
}

std::string to_string(const std::vector<model::Point> &v)
{
    std::string ret;
    std::ostringstream oss(ret);

    oss << "[";
    for (int i = 0; i < v.size(); ++i)
    {
        const auto &p = v[i];
        if (i)
        {
            oss << ", ";
        }
        oss << "[" << p.x() << ", " << p.y() << "]";
    }
    oss << "]";

    return oss.str();
}
}
}