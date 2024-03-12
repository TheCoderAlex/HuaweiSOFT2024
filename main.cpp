#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <cmath>
#include <vector>
#include <queue>
#include <map>
#include <utility>

using namespace std;
const int M_SIZE = 200;
const int B_SIZE = 10;
const int BOAT_SIZE = 5;


char mp[M_SIZE][M_SIZE];
int boat_capacity;
int money,frame_id;
int k;
// queue<pair<int, int>> goods_queue;
FILE *p;

int dx[4] = {-1, 1, 0, 0};
int dy[4] = {0, 0, -1, 1};
string dir[4] = {"2", "3", "1", "0"};

bool isValid(int x, int y) {
    return x >= 0 && x < M_SIZE && y >= 0 && y < M_SIZE && mp[x][y] != '#' && mp[x][y] != '*';
}

vector<string> BFS(int startX, int startY, int endX, int endY) {
    queue<pair<int, int>> q;
    vector<vector<bool>> visited(M_SIZE, vector<bool>(M_SIZE, false));
    map<pair<int, int>, pair<int, int>> parent;
    map<pair<int, int>, string> move;

    q.push({startX, startY});
    visited[startX][startY] = true;

    while (!q.empty()) {
        auto [x,y] = q.front();
        q.pop();

        if (x == endX && y == endY) {
            vector<string> path;
            pair<int, int> cur = {endX, endY};
            while (!(cur.first == startX && cur.second == startY)) {
                path.insert(path.begin(), move[cur]);
                cur = parent[cur];
            }
            return path;
        }

        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i], ny = y + dy[i];

            if (isValid(nx, ny) && !visited[nx][ny]) {
                visited[nx][ny] = true;
                q.push({nx, ny});
                parent[{nx, ny}] = {x, y};
                move[{nx, ny}] = dir[i];
            }
        }
    }

    return {};
}

struct Berth
{
    /* 泊位 */
    int x,y;
    int time;
    int velocity;

}berth[B_SIZE];

struct Boat
{
    int num, id, status;

    void ship(int shipId, int berthId) {
        printf("ship %d %d\n", shipId, berthId);
    }

    void go() {
        printf("go %d\n", this->id);
    }

}boat[BOAT_SIZE];

struct Robot
{
    int x,y;
    int mbx, mby; //?
    bool has_goods;
    int goods_id;
    int status; //0 free 1 work 2 collision
    int st;
    vector<string> directions;
}robot[10];

struct Goods
{
    int x,y;
    int value;
    int status;
}goods[10];

void Init()
{
    for(int i = 0; i < M_SIZE; i ++)
        fscanf(p,"%s", mp[i]);
    for(int i = 0; i < B_SIZE; i ++)
    {
        int id;
        fscanf(p,"%d", &id);
        fscanf(p,"%d%d%d%d", &berth[id].x, &berth[id].y, &berth[id].time, &berth[id].velocity);
    }
    fscanf(p,"%d", &boat_capacity);
    char okk[100];
    fscanf(p,"%s", okk);
    printf("OK\n");
    fflush(stdout);
}

int Input()
{
    fscanf(p,"%d%d", &frame_id, &money);
    fscanf(p,"%d", &k);
    for(int i = 0; i < k; i ++)
    {
        int x, y, val;
        fscanf(p,"%d%d%d", &x, &y, &val);
        goods[i].x = x;
        goods[i].y = y;
        goods[i].value = val;
    }
    for(int i = 0; i < 10; i ++)
    {
        fscanf(p,"%d%d%d%d", &robot[i].has_goods, &robot[i].x, &robot[i].y, &robot[i].st);
    }
    for(int i = 0; i < 5; i ++)
        fscanf(p,"%d%d\n", &boat[i].status, &boat[i].id);
    char okk[100];
    fscanf(p,"%s", okk);
    return frame_id;
}


void robot_move(int robot_id){

    const char* direc = robot[robot_id].directions.front().c_str();

    printf("move %d %s\n", robot_id,direc);

    robot[robot_id].directions.erase(robot[robot_id].directions.begin());
}

void getGoods(int robot_id, int goods_id){
    robot[robot_id].has_goods = true;
    //修改goods状态
    goods[goods_id].value = 0;

    printf("get %d\n", robot_id);
}

void pullGoods(int robot_id, int goods_id){
    robot[robot_id].has_goods = false;
    robot[robot_id].status = 0;


    printf("pull %d\n", robot_id);
}

void boatAction () {
    /* boat.status:
        0 : moving
        1 : finishing shipping OR waiting for loading
        2 : waiting for berth

        boat.id: id is the berth id; -1 means virtual point.
    */
    for (int i = 0; i < BOAT_SIZE; i ++) {
        if (boat[i].id == -1 && boat[i].status != 0) {
            boat[i].ship(boat[i].id, rand() % 10);
        } else if (boat[i].id != -1 && boat[i].status == 2) {
            boat[i].ship(boat[i].id, rand() % 10);
        } else if (boat[i].id != -1 && boat[i].status == 1) {
            boat[i].go();
        }
    }
}

int main() {
    p = fopen(R"(C:\Users\alextang\CLionProjects\Huawei\log.txt)","r");
    Init();
    for(int frame = 1; frame <=15000; frame ++){
        int frame_id = Input();
        for(int i = 0; i < B_SIZE; i ++){
            if(robot[i].status == 0){ //空闲
                int minGoods = -1;
                int minLen = 65535;
                //遍历查找最近的货物点
                for(int j = 0; j < k ; j ++){
                    if (goods[j].status == 1)
                        continue;
                    vector<string> oneGoods = BFS(robot[i].x, robot[i].y, goods[j].x, goods[j].y);
                    if(oneGoods.size() != 0 && oneGoods.size() < minLen){
                        minLen = oneGoods.size();
                        robot[i].directions = oneGoods;
                        minGoods = j;
                    }
                }
                if (minGoods != -1){
                    goods[minGoods].status = 1;
                    //cout << i << " pick " << minGoods << endl;
                }

                if (robot[i].directions.size() == 0){
                    continue;
                }

                //修改机器人状态为工作中
                robot[i].status = 1;
                //修改机器人目的货物
                robot[i].goods_id = minGoods;

                robot_move(i);

                if(robot[i].directions.size() == 0) getGoods(i, robot[i].goods_id);

            }else if (robot[i].status == 1 && !robot[i].has_goods) //去拿货物的路上
            {
                robot_move(i);
                if(robot[i].directions.size() == 0) getGoods(i, robot[i].goods_id);
            }else if (robot[i].status == 1 && robot[i].has_goods) //拿到货物
            {
                robot[i].directions = BFS(robot[i].x, robot[i].y,berth[1].x,berth[1].y);
                robot_move(i);
                if(robot[i].directions.size() == 0) pullGoods(i, robot[i].goods_id);
            }
        }
        puts("OK");
        fflush(stdout);

    }
    return 0;
}