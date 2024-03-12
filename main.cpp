#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <cmath>
#include <vector>
#include <queue>
#include <map>
#include <ctime>
#include <utility>
#include <cstdlib>

using namespace std;
const int M_SIZE = 200;
const int B_SIZE = 10;
const int BOAT_SIZE = 5;


char mp[M_SIZE][M_SIZE];
int boat_capacity;
int money,frame_id;
int k;

// queue<pair<int, int>> goods_queue;

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
                path.push_back(move[cur]);
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
    /*
    shipedFrame 表示船在第几帧进入的港口
    */
    int shipedFrame, id, status;
    void ship(int shipId, int berthId) {
        printf("ship %d %d\n", shipId, berthId);
    }

    void go(int shipID) const {
        printf("go %d\n", shipID);
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
    bool operator<(const Goods& other) const {
        return value < other.value;
    }
}goods[10];
priority_queue<Goods> goods_queue;

/*
 * 优先队列用法
    while (!pq.empty()) {
        Item item = pq.top();
        pq.pop();
        std::cout << "Item value: " << item.value << std::endl;
    }
*/

void Init()
{
    for(int i = 0; i < M_SIZE; i ++)
        scanf("%s", mp[i]);
    for(int i = 0; i < B_SIZE; i ++)
    {
        int id;
        scanf("%d", &id);
        scanf("%d%d%d%d", &berth[id].x, &berth[id].y, &berth[id].time, &berth[id].velocity);
    }
    scanf("%d", &boat_capacity);
    char okk[100];
    scanf("%s", okk);
    printf("OK\n");
    fflush(stdout);
}

int Input()
{
    scanf("%d%d", &frame_id, &money);
    scanf("%d", &k);
    for(int i = 0; i < k; i ++)
    {
        int x, y, val;
        scanf("%d%d%d", &x, &y, &val);
        goods[i].x = x;
        goods[i].y = y;
        goods[i].value = val;
    }
    for(int i = 0; i < 10; i ++)
    {
        scanf("%d%d%d%d", &robot[i].has_goods, &robot[i].x, &robot[i].y, &robot[i].st);
    }
    for(int i = 0; i < 5; i ++)
        scanf("%d%d\n", &boat[i].status, &boat[i].id);
    char okk[100];
    scanf("%s", okk);
    return frame_id;
}

void boatAction (int frameID) {
    /* boat.status:
        0 : moving
        1 : finishing shipping OR waiting for loading
        2 : waiting for berth

        boat.id: id is the berth id; -1 means virtual point.
    */
    for (int i = 0; i < BOAT_SIZE; i ++) {
        // 判断是不是最后一次送货机会
        if (boatLastChance(frameID, i)) { //可能会导致时间上的开销增加
            boat[i].go(i);
            continue;
        }

        if (boat[i].status == 0)
            continue;
        if (boat[i].id == -1 && boat[i].status != 0) {
            int berthID = 2 * i + frame_id % 2;
            boat[i].ship(i, berthID);
            boat[i].shipedFrame = frameID + berth[berthID].time;
        } else if (boat[i].id != -1 && boat[i].status == 1) {
            // 理想情况下 最快的装满货时间
            if (frameID - boat[i].shipedFrame >= boat_capacity / berth[boat[i].id].velocity) {
                boat[i].go(i);
            }
        }
    }
}

bool boatLastChance (int frameID, int boatID) {
    if (boat[boatID].id != -1 && boat[boatID].status == 1) {
        return (15000 - frameID == berth[boat[boatID].id].time);
    }
    return false;
}

void robot_move(int robot_id){
    if (!robot[robot_id].directions.empty()){
        const char* direc = robot[robot_id].directions.back().c_str();
        printf("move %d %s\n", robot_id,direc);
        robot[robot_id].directions.pop_back();
    } else {
        if (robot[robot_id].x != robot[robot_id].mbx || robot[robot_id].y != robot[robot_id].mby) {
            robot[robot_id].status = 0;
            return;
        }
    }
    return;
}

void getGoods(int robot_id){
    //[robot_id].has_goods = true;
    //修改goods状态
    //goods[goods_id].value = 0;
    printf("get %d\n", robot_id);
}

void pullGoods(int robot_id){
    robot[robot_id].status = 0;

    printf("pull %d\n", robot_id);
}

void changeDirection(int rid) {
    string tmp = robot[rid].directions.back();
    
    //如果剩下三个个方向全是墙就不改变方向
    int i;
    for (i = 0;i < 4; ++i) {
        if (dir[i] == tmp)  continue;
        if (mp[robot[rid].x + dx[i]][robot[rid].y + dy[i]] != '#' && mp[robot[rid].x + dx[i]][robot[rid].y + dy[i]] != '*')
            break;
    }
    if (i == 4) return;

    //随机选择一个可以走的位置
    int c,x,y,newx,newy;
    do {
        c = rand() % 4;
        x = dx[c], y = dy[c];
        newx = robot[rid].x + x, newy = robot[rid].y + y;
    } while (dir[c] == tmp || mp[newx][newy] == '#' || mp[newx][newy] == '*');

    //更新路径
    vector<string> t = BFS(newx,newy,robot[rid].mbx,robot[rid].mby);
    if (!t.empty()) {
        //如果碰巧路径不可达那就等下次碰撞了再选一次，这里不处理
        robot[rid].directions = t;
        robot[rid].directions.push_back(dir[c]);
    }
}

int main() {
    Init();
    for(int frame = 1; frame <=15000; frame ++){
        int frame_id = Input();

        //第一帧以及每500帧操作一下船
        // if (frame == 1 || frame % 500 == 0)
        // 船的操作， 传入当前帧ID
        boatAction(frame);

        //新增的货物入队
        for (int j = 0; j < k; ++j) {
            goods_queue.push(goods[j]);
        }
        
        for (int i = 0; i < 10; ++i) {
            //找到空闲的机器人就去分配货物
            if (!goods_queue.empty() && robot[i].status == 0 && robot[i].st != 0) {
                Goods tmp = goods_queue.top();
                goods_queue.pop();
                vector<string> g = BFS(robot[i].x,robot[i].y,tmp.x,tmp.y);
                if (!g.empty()) {
                    robot[i].mbx = tmp.x;
                    robot[i].mby = tmp.y;
                    robot[i].directions = g;
                    robot[i].status = 1;
                }
            }
        }

        //机器人根据状态进行移动
        for (int i = 0;i < 10; ++i) {
            //如果没带货物碰撞
            if (robot[i].st == 0 && !robot[i].has_goods) {
                robot[i].status = 0;
                robot[i].directions = {};
                robot[i].mbx = robot[i].mby = 0;
                continue;
            }

            //带着货物碰撞
            if (robot[i].st == 0 && robot[i].has_goods) {
                const int setp = 3;
                if (rand() % 5 == setp)
                    changeDirection(i);
            }

            if (robot[i].status == 1 && !robot[i].has_goods) {
                //在拿货物的路上
                robot_move(i);
                if (robot[i].directions.empty()) {
                    //去拿货物
                    getGoods(i);
                } 
            } else if (robot[i].status == 1 && robot[i].has_goods) {
                //拿到货物去泊位
                
                // int minLen = 1000000;
                
                // vector<string> tmp;
                // //泊位选择
                // for(int j = 0; j < 10; j ++){
                //     vector<string> one = BFS(robot[i].x, robot[i].y,berth[j].x,berth[j].y);
                //     if(!one.empty() && one.size() < minLen){
                //         minLen = one.size();
                        
                //         tmp = one;
                //     }
                // }
                vector<string> tmp = BFS(robot[i].x, robot[i].y,berth[i].x,berth[i].y);

                if (!tmp.empty()) {
                    robot[i].directions = tmp;
                    robot[i].mbx = berth[i].x;
                    robot[i].mby = berth[i].y;
                    robot_move(i);
                    if (robot[i].directions.empty())
                        pullGoods(i);
                }
            }
        }
        puts("OK");
        fflush(stdout);
    }
    return 0;
}