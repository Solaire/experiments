#include <iostream>
#include <vector>
#include <memory>
#include <set>
#include <map>
#include <unordered_map>
#include <sstream>
#include <bitset>
#include <fstream>
#include <string>
#include <algorithm>
#include <functional>
#include <climits>
#include <chrono>

#define HACKER_RANK 0

constexpr char INPUT[] = "./input.txt";
constexpr int MAX_FISH = 10;

std::string ltrim(const std::string &);
std::string rtrim(const std::string &);
std::vector<std::string> split(const std::string &);

struct Node
{
    Node()
    : _fish(0)
    , _neighbours()
    { }

    int _fish;
    std::map<int, int> _neighbours;

    void debug(int label)
    {
        std::cout << "Shop: " << label << ", "
            << "Fish: " << _fish << ", "
            << "Connections: " << _neighbours.size() << "\n";

        for(const auto n : _neighbours)
        {
            std::cout << "\t"
                << "Label: " << n.first << ", "
                << "Cost: " << n.second << "\n";
        }
    }
};

struct State
{
    void simplify()
    {
        for(int i = 1; i < _shops.size() - 1; i++)
        {
            // Only interested in empty nodes
            if(_shops[i]._fish > 0)
            {
                continue;
            }

            auto & m = _shops[i]._neighbours;

            // Iterate the connections
            // - Remove all paths to 'current'
            // - Connect the paths to each other
            //   - if path already exists, check if the new one is faster - ignore if not
            for(auto it_l = m.begin(); it_l != m.end(); it_l++)
            {
                const int left_node = it_l->first;
                const int left_cost = it_l->second;

                _shops[left_node]._neighbours.erase(i);

                for(auto it_r = std::next(it_l); it_r != m.end(); it_r++)
                {
                    const int right_node = it_r->first;
                    const int right_cost = it_r->second;

                    _shops[right_node]._neighbours.erase(i);

                    if( _shops[left_node]._neighbours.count(right_node) > 0
                        && _shops[left_node]._neighbours.at(right_node) < (left_cost + right_cost) )
                    {
                        continue;
                    }

                    _shops[left_node]._neighbours[right_node] = (left_cost + right_cost);
                    _shops[right_node]._neighbours[left_node] = (left_cost + right_cost);
                }
            }

            m.clear();
        }
    }

    std::vector<Node> _shops;
    int _max_fish;
};

struct Queue
{
    Queue(const int node_count, const int fish_len)
    : _queue()
    , _paths(node_count)
    { 
        for(int i = 0; i < node_count; i++)
        {
            _paths[i].resize(fish_len);
            std::fill(_paths[i].begin(), _paths[i].end(), -1);
        }
    }

    // [cost, [shop, Bits]]
    std::set<std::pair<int, std::pair<int, int>>> _queue;

    // [shop, fish, cost]
    std::vector<std::vector<int>> _paths;
};

State parse_input()
{
#if HACKER_RANK == 1
    using namespace std;

    State s;
    string line;
    getline(cin, line);
    vector<string> tokens = split(rtrim(line));

    int n = stoi(tokens[0]); // shopping centers
    int m = stoi(tokens[1]); // roads
    int k = stoi(tokens[2]); // fish types

    s._max_fish = 0x3FF >> (MAX_FISH - k);
    s._shops.resize(n);

    // Fish types
    for(int i = 0; i < n; i++)
    {
        getline(cin, line);
        vector<string> tokens = split(rtrim(line));

        for(int j = 1; j < tokens.size(); j++)
        {
            const int bitmask = stoi(tokens[j]) - 1;
            s._shops[i]._fish |= (0x01 << bitmask);
        }
    }

    // Now make all connections
    for(int i = 0; i < m; i++)
    {
        getline(cin, line);
        vector<string> tokens = split(rtrim(line));

        const int left  = stoi(tokens[0]) - 1;
        const int right = stoi(tokens[1]) - 1;
        const int cost  = stoi(tokens[2]);

        s._shops[left]._neighbours[right] = cost;
        s._shops[right]._neighbours[left] = cost;
    }

    return s;
#else
    using namespace std;

    std::fstream file(INPUT);

    State s;
    string line;
    getline(file, line);
    vector<string> tokens = split(rtrim(line));

    int n = stoi(tokens[0]); // shopping centers
    int m = stoi(tokens[1]); // roads
    int k = stoi(tokens[2]); // fish types

    s._max_fish = 0x3FF >> (MAX_FISH - k);
    s._shops.resize(n);

    // Fish types
    for(int i = 0; i < n; i++)
    {
        getline(file, line);
        vector<string> tokens = split(rtrim(line));

        for(int j = 1; j < tokens.size(); j++)
        {
            const int bitmask = stoi(tokens[j]) - 1;
            s._shops[i]._fish |= (0x01 << bitmask);
        }
    }

    // Now make all connections
    for(int i = 0; i < m; i++)
    {
        getline(file, line);
        vector<string> tokens = split(rtrim(line));

        const int left  = stoi(tokens[0]) - 1;
        const int right = stoi(tokens[1]) - 1;
        const int cost  = stoi(tokens[2]);

        s._shops[left]._neighbours[right] = cost;
        s._shops[right]._neighbours[left] = cost;
    }

    //s.simplify();

    return s;
#endif // HACKER_RANK == 1
}

void queue_push(Queue & q, const int node, const int & fish, const int cost)
{
    const bool exists = q._paths[node][fish] >= 0;
    const int current_cost = (exists)
        ? q._paths[node][fish]
        : 0;

    // Already visited at lower cost
    if(exists && current_cost < cost)
    {
        return;
    }

    std::pair<int, std::pair<int, int>> current = {current_cost, {node, fish}};
    if(auto it = q._queue.find(current); it != q._queue.end())
    {
        q._queue.erase(current);
    }

    q._paths[node][fish] = cost;
    
    current.first = cost;
    q._queue.insert(current);
}

std::vector<std::pair<int, int>> dijkstra(State & s, const int start)
{
    using namespace std;

    vector<pair<int, int>> paths;

    Queue q(s._shops.size(), s._max_fish + 1);
    queue_push(q, start, s._shops[start]._fish, 0);
    int i = 0;

    while(!q._queue.empty())
    {
        const int cost = q._queue.begin()->first;
        const int fish = q._queue.begin()->second.second;
        const int node = q._queue.begin()->second.first;

        q._queue.erase(q._queue.begin());
        i++;

        // Found end of queue
        if(node == s._shops.size() - 1)
        {
            paths.push_back({fish, cost});
        }

        for(const auto & neighbour : s._shops[node]._neighbours)
        {
            queue_push(q, neighbour.first, fish | s._shops[neighbour.first]._fish, cost + neighbour.second);
        }
    }

    cout << i << "\n";

    return paths;
}

int main()
{
#if HACKER_RANK == 1
    std::ofstream fout(getenv("OUTPUT_PATH"));

    State s = parse_input();

    const auto paths = dijkstra(s, 0);
    int shortest = INT_MAX;

    if(paths.size() == 1)
    {
        shortest = std::min(shortest, paths[0].second);
    }
    else
    {
        for(int i = 0; i < paths.size(); i++)
        {
            for(int j = 1; j < paths.size(); j++)
            {
                if(s._max_fish == (paths[i].first | paths[j].first) )
                {
                    shortest = std::min(shortest, std::max(paths[i].second, paths[j].second));
                }
            }
        }
    }

    fout << shortest << "\n";

    fout.close();

    return 0;
#else
    auto time_1 = std::chrono::high_resolution_clock::now();

    State s = parse_input();

    auto time_2 = std::chrono::high_resolution_clock::now();

    const auto paths = dijkstra(s, 0);
    int shortest = INT_MAX;

    auto time_3 = std::chrono::high_resolution_clock::now();

    std::cout 
        << "Time to parse: " << std::chrono::duration_cast<std::chrono::duration<float>>(time_2 - time_1).count() << "\n"
        << "Time of dijkstra: " << std::chrono::duration_cast<std::chrono::duration<float>>(time_3 - time_2).count()  << "\n"
        << "Total time:" << std::chrono::duration_cast<std::chrono::duration<float>>(time_3 - time_1).count()  << "\n";

    if(paths.size() == 1)
    {
        shortest = std::min(shortest, paths[0].second);
    }
    else
    {
        for(int i = 0; i < paths.size(); i++)
        {
            for(int j = 1; j < paths.size(); j++)
            {
                if(s._max_fish == (paths[i].first | paths[j].first) )
                {
                    shortest = std::min(shortest, std::max(paths[i].second, paths[j].second));
                }
            }
        }
    }

    std::cout << shortest << std::endl;

    return 0;
#endif // HACKER_RANK
}

#pragma region // utils

std::string ltrim(const std::string & str)
{
    std::string s(str);

    s.erase(
        s.begin(),
        std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(isspace)))
    );

    return s;
}

std::string rtrim(const std::string & str)
{
    std::string s(str);

    s.erase(
        std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(isspace))).base(),
        s.end()
    );

    return s;
}

std::vector<std::string> split(const std::string & str)
{
    std::vector<std::string> tokens;

    std::string::size_type start = 0;
    std::string::size_type end = 0;

    while ((end = str.find(" ", start)) != std::string::npos)
    {
        tokens.push_back(str.substr(start, end - start));

        start = end + 1;
    }

    tokens.push_back(str.substr(start));

    return tokens;
}

inline void new_line()
{
#if PRINT == 1
    std::cout << "\n";
#endif // PRINT
}

std::string vector_string(const std::vector<int> & list)
{
    std::stringstream ss;

    for(int i = 0; i < list.size(); i++)
    {
        ss << i;
        if (i < list.size() - 1)
        {
            ss << ",";
        }
    }

    return ss.str();
}

#pragma endregion // utils
