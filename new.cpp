#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <cmath>
#include <stack>
#include <set>
#include <vector>
#include <queue>
#include <map>
#include <ctime>
#include <utility>
#include <cstdlib>
#include <algorithm>
#include <fstream>

using namespace std;
const int M_SIZE = 200;
const int B_SIZE = 10;
const int BOAT_SIZE = 5;


char mp[M_SIZE][M_SIZE];
int boat_capacity;
int money,frame_id;
int k;
ofstream f("out.txt");  

// queue<pair<int, int>> goods_queue;

int dx[4] = {-1, 1, 0, 0};
int dy[4] = {0, 0, -1, 1};
string dir[4] = {"2", "3", "1", "0"};
int has_cracked[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// bool isValid(int x, int y) {
//     return x >= 0 && x < M_SIZE && y >= 0 && y < M_SIZE && mp[x][y] != '#' && mp[x][y] != '*';
// }

// vector<string> BFS(int startX, int startY, int endX, int endY) {
//     queue<pair<int, int>> q;
//     vector<vector<bool>> visited(M_SIZE, vector<bool>(M_SIZE, false));
//     map<pair<int, int>, pair<int, int>> parent;
//     map<pair<int, int>, string> move;

//     q.push({startX, startY});
//     visited[startX][startY] = true;

//     while (!q.empty()) {
//         auto [x,y] = q.front();
//         q.pop();

//         if (x == endX && y == endY) {
//             vector<string> path;
//             pair<int, int> cur = {endX, endY};
//             while (!(cur.first == startX && cur.second == startY)) {
//                 path.push_back(move[cur]);
//                 cur = parent[cur];
//             }
//             return path;
//         }

//         for (int i = 0; i < 4; ++i) {
//             int nx = x + dx[i], ny = y + dy[i];

//             if (isValid(nx, ny) && !visited[nx][ny]) {
//                 visited[nx][ny] = true;
//                 q.push({nx, ny});
//                 parent[{nx, ny}] = {x, y};
//                 move[{nx, ny}] = dir[i];
//             }
//         }
//     }

//     return {};
// }

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
    int mbx, mby; // target
    bool has_goods;
    int goodPos; //每次get货物之后更新
    int berth_id;
    int status; //0 free 1 work 2 init
    int st;
    int dir; //0去泊位 1去货物
    // vector<string> directions;
    Robot(){
        status = 2;
        dir = 0;
    }
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

//Dijkstra
const int N = 200;
const int maxn = N * N;
const int INF = 0x3f3f3f3f;
const vector<pair<int, int>> directions{{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

int currentPos[10];    //当前机器人位置集合
int avoidto[10];   //key对value执行了碰撞避免
bool hasAvoid[10];  //当前回合那些机器人执行了避让，接下来不再执行任何操作
int choice[10] = {0,1,2,3,4,5,6,7,8,9};

vector<int> dist(maxn, INF);    //临时距离
vector<int> pre(maxn, -1);      //临时路径
vector<vector<int>> minlen(10,vector<int>(maxn,INF));   //泊位i到任意点的最短距离
vector<vector<int>> berthPre(10,vector<int>(maxn,-1));    //泊位i的pre数组
vector<vector<vector<int>>> pathto(10,vector<vector<int>>(maxn));
 
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

/*
如何查找路径
vector<int> path;
for (; end != -1; end = pre[end]) {
    path.push_back(end);
}
reverse(path.begin(), path.end());
cout << "Path is: ";
for (int &p : path) {
    cout << p << ' ';
}
*/

//坐标变换
pair<int,int> pos2xy(int pos) {
    int x = pos / N , y = pos % N;
    return {x,y};
}

int xy2pos(pair<int,int> xy) {
    return xy.first * N + xy.second;
}

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
    
    //进行Dijkstra初始化最短路
    for (int i = 0;i < 10; i++) {
        int pos = berth[i].x * N + berth[i].y;
        dijkstra(pos);
        minlen[i] = dist;
        berthPre[i] = pre;
        for (int j = 0;j < maxn; ++j) {
            vector<int>& path = pathto[i][j];
            int end = j;
            for (; end != -1; end = pre[end]) {
                path.push_back(end);
            }
            reverse(path.begin(), path.end());
        }
    }

    //test path
    // cout << "The 1 berth.\n" << xy2pos({berth[1].x,berth[1].y});
    // for (int i = 0;i < maxn; ++i) {
    //     if (minlen[1][i] == INF)    continue;
    //     cout << "The path to point " << i << " :";
    //     for (auto &p : pathto[1][i]) {
    //         cout << p << " ";
    //     }
    //     cout << endl;
    // }

    //定好每个robot选择哪个港口
    for (int i = 0;i < 10; ++i){ 
        robot[i].mbx = berth[choice[i]].x, robot[i].mby = berth[choice[i]].y;
        robot[i].berth_id = choice[i];
    }
        
    //将机器人的goodsPos设置为初始位置方便回退
    int cnt = 0;
    for (int i = 0; i < 200; ++i) {
        for (int j = 0; j < 200; ++j) {
            if (mp[i][j] == 'A') {
                robot[cnt++].goodPos = xy2pos({i,j});
            }
        }
    }

    char okk[100];
    scanf("%s", okk);
    printf("OK\n");
    fflush(stdout);
}

int Input()
{
    //清空避让数组
    memset(avoidto, -1, sizeof avoidto);
    memset(hasAvoid, 0, sizeof hasAvoid);
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
        currentPos[i] = xy2pos({robot[i].x,robot[i].y});
    }
    for(int i = 0; i < 5; i ++)
        scanf("%d%d\n", &boat[i].status, &boat[i].id);
    char okk[100];
    scanf("%s", okk);
    return frame_id;
}

bool boatLastChance (int frameID, int boatID) {
    if (boat[boatID].id != -1 && boat[boatID].status == 1) {
        return (15000 - frameID == berth[boat[boatID].id].time);
    }
    return false;
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

void move(int rid, int nxt, int pos) {
    //cout << "nxt " << nxt << " pos " << pos << endl;
    if (nxt - pos == 1){
        printf("move %d 0\n",rid);
    } //向右
    else if (nxt - pos == -1) {
        printf("move %d 1\n",rid);
    } //向左
    else if (nxt - pos == 200) {
        printf("move %d 3\n",rid);
    }  //向下
    else if (nxt - pos == -200) {
        printf("move %d 2\n",rid);
    } //向上
    else
        return;
}

void robot_move(int robot_id, int d){
    //d = 0朝泊位移动
    //d = 1朝货物移动

    if (d == 0) {
        int now = xy2pos({robot[robot_id].x,robot[robot_id].y});
        int p = berthPre[robot[robot_id].berth_id][now];
        if (p == -1)
            return;
        // cout << robot_id << " move to Berth " << p << " now in " << now << "=========\n";
        move(robot_id, p, now);
    } else {
        // if(robot_id == 5)
            // cout << "robot_5 is moving to" << xy2pos({robot[5].mbx, robot[5].mby}) << " \n";
        int now = xy2pos({robot[robot_id].x,robot[robot_id].y});
        int target = xy2pos({robot[robot_id].mbx,robot[robot_id].mby});

        //如果不可达直接返回，将机器人状态换为空闲，否则会原地不动
        if(pathto[robot[robot_id].berth_id][target].size() == 1){
            robot[robot_id].status = 0;
            return;
        }
        
        // cout << robot_id << "\n move to target " << target << " now in " << now << "=========\n";
        int p = -1;
        int i;
        // 每个pathto[i][target]数组的最后一个应该都是坐标点，因此最后一个元素不需要查找
        for (i = 0;i < pathto[robot[robot_id].berth_id][target].size() - 1; ++i) {
            if (pathto[robot[robot_id].berth_id][target][i] == now) {
                p = pathto[robot[robot_id].berth_id][target][i+1];
                break;
            }
        }
        //p = -1代表到达终点，不执行move
        if (p == -1)
            return;
        //cout << " it want to move to " << p << endl;
        move(robot_id, p, now);
    }
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

//碰撞避免
void avoidCollision() {
    for (int i = 0; i < 10; ++i) {
        //检查该机器人下一步是否碰到另外的机器人
        if (robot[i].dir == 0) {
            //朝泊位移动的机器人
            int now = xy2pos({robot[i].x,robot[i].y});
            int p = berthPre[robot[i].berth_id][now];
            //找到它下一个位置及其下一个位置的周围是否存在其他人
            int postions[4] = {p - 1, p + 1, p - 200, p + 200};
            int j, will_colli = 0;
            for (j = 0; j < 10; ++j) {
                if (j == i) continue;
                if (currentPos[j] == p || currentPos[j] == postions[0] || currentPos[j] == postions[1] || currentPos[j] == postions[2] || currentPos[j] == postions[3]) {
                    //找到即将碰撞的机器人，不会存在其他机器人了
                    will_colli = 1;
                    break;
                }
            }
            if (will_colli == 0)    continue;
            //处理碰撞
            //如果对方已经避让了，就不再避让
            if (avoidto[j] == i)    continue;
            //j没有避让，i回退一格，记录i避让j，同时i该回合不再执行其他任何操作
            //因为是朝着泊位移动的，因此向货物方向退。如果已经退到了货物方向，就随便退
            int target = robot[i].goodPos;
            if (now == target) {
                //随便退一格
                int d[4] = {-1,1,200,-200};
                int c,newpos;
                pair<int,int> n;
                do {
                    c = rand() % 4;
                    newpos = now + d[c];
                    n = pos2xy(newpos);
                } while (newpos == p || mp[n.first][n.second] == '#' || mp[n.first][n.second] == '*');
                move(i,newpos,now);
                
            } else {
                //朝着货物方向走一格
                int ans = -1;
                for (int k = 0;k < pathto[robot[i].berth_id][target].size() - 1; ++k) {
                        if (pathto[robot[i].berth_id][target][k] == now) {
                        ans = pathto[robot[i].berth_id][target][k+1];
                        break;
                    }
                }
                //应该是能找到的
                // if (ans == -1)   continue;
                move(i,ans,now);
                //记录i避让j并且i hasAvoid
                avoidto[i] = j, hasAvoid[i] = 1;   
            }
            f << "=========robot " << i << " currentPos: "<< currentPos[i] <<
            " avoid: "<< avoidto[i] << "\n";
        } else {
            //朝货物移动的机器人
            int now = xy2pos({robot[i].x,robot[i].y});
            int p = berthPre[robot[i].berth_id][now];
            //找到它下一个位置及其下一个位置的周围是否存在其他人
            int postions[4] = {p - 1, p + 1, p - 200, p + 200};
            int j, will_colli = 0;
            for (j = 0; j < 10; ++j) {
                if (j == i) continue;
                if (currentPos[j] == p || currentPos[j] == postions[0] || currentPos[j] == postions[1] || currentPos[j] == postions[2] || currentPos[j] == postions[3]) {
                    //找到即将碰撞的机器人，不会存在其他机器人了
                    will_colli = 1;
                    break;
                }
            }
            if (will_colli == 0)    continue;
            //处理碰撞
            //如果对方已经避让了，就不再避让
            if (avoidto[j] == i)    continue;
            //j没有避让，i回退一格，记录i避让j，同时i该回合不再执行其他任何操作
            //因为是朝着货物移动的，回退就是向泊位移动一格，而每个机器人的泊位是固定的，因此退到泊位便肯定不会发生碰撞
            int target = xy2pos({berth[robot[i].berth_id].x,berth[robot[i].berth_id].y});
            move(i, target, now);
            //记录i避让j并且i hasAvoid
            avoidto[i] = j, hasAvoid[i] = 1;   
        }
    }
}
// void changeDirection(int rid) {
//     string tmp = robot[rid].directions.back();
    
//     //如果剩下三个个方向全是墙就不改变方向
//     int i;
//     for (i = 0;i < 4; ++i) {
//         if (dir[i] == tmp)  continue;
//         if (mp[robot[rid].x + dx[i]][robot[rid].y + dy[i]] != '#' && mp[robot[rid].x + dx[i]][robot[rid].y + dy[i]] != '*')
//             break;
//     }
//     if (i == 4) return;

//     //随机选择一个可以走的位置
//     int c,x,y,newx,newy;
//     do {
//         c = rand() % 4;
//         x = dx[c], y = dy[c];
//         newx = robot[rid].x + x, newy = robot[rid].y + y;
//     } while (dir[c] == tmp || mp[newx][newy] == '#' || mp[newx][newy] == '*');

//     //更新路径
//     vector<string> t = BFS(newx,newy,robot[rid].mbx,robot[rid].mby);
//     if (!t.empty()) {
//         //如果碰巧路径不可达那就等下次碰撞了再选一次，这里不处理
//         robot[rid].directions = t;
//         robot[rid].directions.push_back(dir[c]);
//     }
// }


int main() {
    Init();
    for(int frame = 1; frame <= 15000; frame++){
        int frame_id = Input();
        f << "frame_id:" << frame_id << "\n";

        //第一帧以及每500帧操作一下船
        // if (frame == 1 || frame % 500 == 0)
        // 船的操作， 传入当前帧ID
        boatAction(frame);

        //新增的货物入队
        for (int j = 0; j < k; ++j) {
            goods_queue.push(goods[j]);
        }

        //首先处理碰撞
        avoidCollision();

        //初始状态全部滚去港口
        for (int i = 0;i < 10; ++i) {
            if (hasAvoid[i])    continue;   //如果回退了本回合不进行操作
            if (robot[i].status == 2 && robot[i].st != 0) {
                robot_move(i ,0);
                //放到内部
                if (robot[i].x == robot[i].mbx && robot[i].y == robot[i].mby) {
                    robot[i].status = 0;
                } 
            }
        } 

        //分货
        for (int i = 0; i < 10; ++i) {
            //找到空闲的机器人就去分配货物
            if (hasAvoid[i])    continue;
            if (robot[i].status == 0 && robot[i].st != 0) {
                Goods tmp = goods_queue.top();
                goods_queue.pop();
                //转为一维坐标
                int pos = xy2pos({tmp.x,tmp.y});
                // 如果货物可达，设置为机器人的目标
                if (minlen[robot[i].berth_id][pos] != INF) {
                    robot[i].mbx = tmp.x;
                    robot[i].mby = tmp.y;
                    robot[i].status = 1;
                    robot[i].dir = 1;
                }
            }
        }

        for (int i = 0;i < 10; ++i) {
            if (hasAvoid[i])    continue;
            if (robot[i].status == 1 && !robot[i].has_goods) {//在拿货物的路上
                //刚才发生了碰撞
                //cout  << "move: " << i << ' ' << robot[i].x << ' ' << robot[i].y << endl;
                robot_move(i, 1);
                //点位重合则get
                //cout << robot[i].x << " " << robot[i].y << " " << robot[i].mbx << ' ' << robot[i].mby << endl;
                if(robot[i].x == robot[i].mbx && robot[i].y == robot[i].mby) {
                    getGoods(i);
                    robot[i].dir = 0;
                    robot[i].goodPos = xy2pos({robot[i].x,robot[i].y});
                }

            }else if(robot[i].status == 1 && robot[i].has_goods){//拿到货物去泊位
                
                robot_move(i, 0);

                if(robot[i].x == berth[robot[i].berth_id].x && robot[i].y == berth[robot[i].berth_id].y) {
                    pullGoods(i);
                    robot[i].dir = 1;
                }
            }
        }

        puts("OK");
        fflush(stdout);
    }
    return 0;
}