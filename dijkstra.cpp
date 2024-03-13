#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

const int N = 200;
const int maxn = N * N;
const int INF = 0x3f3f3f3f;

vector<int> dist(maxn, INF);
vector<int> pre(maxn, -1);
vector<vector<int>> minlen(10,vector<int>(maxn,INF));
vector<vector<vector<int>>> pathto(10,vector<vector<int>>(maxn));
 
const vector<pair<int, int>> directions{{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
char mp[N][N];

void dijkstra(int start) {
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;
    vector<bool> vis(maxn,false);

    fill(dist.begin(), dist.end(), INF);
    fill(pre.begin(), pre.end(), -1);

    dist[start] = 0;
    pq.push({0, start});

    while (!pq.empty()) {
        auto [cost, pos] = pq.top(); pq.pop();

        //cout << "now in point " << pos << endl;
        if (vis[pos])   continue;
        vis[pos] = 1;

        if (cost > dist[pos]) continue;

        int x = pos / N, y = pos % N;

        // 检查四个方向
        for (auto& dir : directions) {
            int newX = x + dir.first, newY = y + dir.second;
            if (newX >= 0 && newX < N && newY >= 0 && newY < N && mp[newX][newY] != '#' && mp[newX][newY] != '*') {
                int newIndex = newX * N + newY;
                int newCost = cost + 1;
                if (newCost < dist[newIndex]) {
                    dist[newIndex] = newCost;
                    pre[newIndex] = pos;
                    pq.push({newCost, newIndex});
                }
            }
        }
    }
}

void calcPath(int end) {
    cout << "The min length to " << end << " is " << dist[end] << endl;
    vector<int> path;
    for (; end != -1; end = pre[end]) {
        path.push_back(end);
    }
    reverse(path.begin(), path.end());
    cout << "Path is: ";
    for (int &p : path) {
        cout << p << ' ';
    }
    cout << endl;
}


int main() {
    vector<int> robot;
    for (int i = 0;i < 200;++i)
        scanf("%s",mp[i]);
    for (int i = 0;i < 200;i++) {
        for (int j = 0;j < 200;j++) {
            if (mp[i][j] == 'A')
                robot.push_back(i * N + j);
        }
    }
    cout << "size: " << robot.size() << endl;
    for (int i = 0; i < 10; ++i) {
        cout << "calc the " << i << " bot in " << robot[i] << ".\n";
        dijkstra(robot[i]);
        for (int i = 0;i < maxn;++i)
            calcPath(i);
    }

    return 0;
}