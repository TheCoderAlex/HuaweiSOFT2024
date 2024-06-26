#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <cmath>
#include <vector>
#include <queue>
#include <map>
#include <fstream>
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

int dx[4] = {0, 1, -1, 0};
int dy[4] = {-1, 0, 0, 1};
string dir[4] = {"1", "3", "2", "0"};
bool has_cracked[10];
ofstream fout("out.txt");
int choice[10] = {0,1,2,3,4,5,6,7,8,9}; //机器人的泊位选择
map<pair<int,int>,int> berthid;   //泊位坐标到id的映射

bool isValid(int x, int y) {
    return x >= 0 && x < M_SIZE && y >= 0 && y < M_SIZE && mp[x][y] != '#' && mp[x][y] != '*';
}

vector<string> BFS(int startX, int startY, int endX, int endY) {
    queue<pair<int, int> > q;
    vector<vector<bool> > visited(M_SIZE, vector<bool>(M_SIZE, false));
    map<pair<int, int>, pair<int, int> > parent;
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
    bool flag;
    int num; //计算当前泊位拥有的货物数量
    Berth () {
        flag = false;
    }
}berth[B_SIZE];

struct Boat
{
    /*
    shipedFrame 表示船在第几帧进入的港口
    num 表示 船当前装了多少货物
    */
    int shipedFrame, id, status, num, myLastBerth;
    void ship(int shipId, int berthId) {
        printf("ship %d %d\n", shipId, berthId);
    }

    void go(int shipID) const {
        printf("go %d\n", shipID);
    }
}boat[BOAT_SIZE];

struct Goods
{
    int x,y;
    int value;
    int frame;
    bool operator<(const Goods& other) const {
        return value < other.value;
    }
    Goods (int frame, int x, int y, int val) {
        this->frame = frame;
        this->x = x;
        this->y = y;
        this->value = val;
    }
};
priority_queue<Goods> goods_queue;

struct Robot
{
    int x,y;
    int mbx, mby; //?
    bool has_goods;
    int goods_value;
    int status; //0 free 1 work 2 collision
    int st;
    vector<string> directions;
    priority_queue<Goods> myGoods;
}robot[10];

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
        berthid[{berth[id].x,berth[id].y}] = id;    //berth坐标到id的映射
        berth[i].flag = false;
    }
    scanf("%d", &boat_capacity);
    
    fout << "这一轮的 船容量 为: " << boat_capacity << "\n";
    fout << "######################\n";
    
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
        Goods goods = {frame_id, x, y, val};
        goods_queue.push(goods);
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

bool boatLastChance (int frameID, int boatID) {
    if (boat[boatID].id != -1 && boat[boatID].status != 0) {
        return (15000 - frameID == berth[boat[boatID].id].time);
    } else if (boat[boatID].id != -1 && boat[boatID].status == 0) {
        return (15000 - frameID == berth[boat[boatID].myLastBerth].time);
    }
    return false;
}

// 找到拥有货物数量最多的泊位
int getMaxGoodsBerthID () {
    int maxBerthID = 0;
    for (int i = 1; i < 10; i++) {
        if (!berth[i].flag)
            maxBerthID = (berth[maxBerthID].num > berth[i].num) ? maxBerthID : i;
    }
    berth[maxBerthID].flag = true;
    return maxBerthID;
}

bool isBoatFull (int boatID) {
    return boat[boatID].num == boat_capacity;
}

void boatAction_cycle (int frameID) {
    /* boat.status:
        0 : moving
        1 : finishing shipping OR waiting for loading
        2 : waiting for berth

        boat.id: id is the berth id; -1 means virtual point.
    */
    for (int i = 0; i < BOAT_SIZE; i ++) {
        //获取当前未被选择的货物最多的泊点
        int berthID = 0;
        if (boat[i].id == -1) {
            fout << "Boat " << i << " is going / at virtual point ! My Status is: " << boat[i].status << '\n';
        }

        //检查是否装满了
        if (isBoatFull(i)) {
            boat[i].go(i);
            boat[i].num = 0;
            continue;
        }
        
        // 判断是不是最后一次送货机会
        if (boatLastChance(frameID, i)) { //可能会导致时间上的开销增加
            boat[i].go(i);
            //取出港口中的货物
            fout << "!!!!!!!!!!!!Boat " << i << " now is the last chance : Berth time is " 
            << berth[boat[i].id].time << " Status is " << boat[i].status 
            << " Now the frame is " << frameID << "I have " << boat[i].num << "Goods\n";
            continue;
        }

        if (boat[i].status == 0) //正在运行中
            continue;
        // if (boat[i].status == 2) { //在泊位外堵塞
        //     // int berthID = getMaxGoodsBerthID();
        //     // if (berthID == boat[i].id)
        //     //     boat[i].shipedFrame += 1;
        //     // else {
        //     //     boat[i].ship(i, berthID);
        //     //     boat[i].shipedFrame = frameID + berth[berthID].time;
        //     boat[i].ship(i, berthID);
        //     boat[i].shipedFrame = frameID + berth[berthID].time;

        // }
        if (boat[i].id == -1 && boat[i].status != 0) { //没有在走且在虚拟点，则需分配泊点
            berthID = getMaxGoodsBerthID();
            // int berthID = getMaxGoodsBerthID();
            boat[i].ship(i, berthID);
            boat[i].shipedFrame = frameID + berth[berthID].time;
            fout << "****Boat: " << i << " choose Berth " << berthID << " boat[i].id is " << boat[i].id <<"\n";
        } else if (boat[i].id != -1 && boat[i].status == 1) { //没有在虚拟点，且正常运行中

            // 如果 此时泊位的货物数量小于泊位每帧装卸速度，那就直接讲 泊位num 赋值为0,否则就让让 nun - 装卸速度 velocity
            // 如果 船当前剩余容量小于泊位装卸速度 那么这一帧 最多装我还差的量
            if (berth[boat[i].id].num < berth[boat[i].id].velocity) {
                boat[i].num += berth[boat[i].id].num;
                berth[boat[i].id].num = 0;
            } else if (boat_capacity - boat[i].num < berth[boat[i].id].velocity) {
                boat[i].num = boat_capacity;
                berth[boat[i].id].num -= (boat_capacity - boat[i].num);
            } else {
                boat[i].num += berth[boat[i].id].velocity;
                berth[boat[i].id].num -= berth[boat[i].id].velocity;
            }
            
            // 如果我把这个港口装空了或者我装满了，俺就回家交货赚钱
            if (isBoatFull(i) || (berth[boat[i].id].num == 0 && frame_id > 2000)) {
                // boat[i].myLastBerth = boat[i].id;
                
                // boat[i].ship(i, berthID);
                // boat[i].shipedFrame = frameID + 500;

                boat[i].go(i);
                berth[boat[i].id].flag = false;
            }
        }
        // fout << "Boat: " << i << " Status is " << boat[i].status << " Now is at " << boat[i].id <<"\n";
    }
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

    //增加泊位的货物
    int bid = berthid[{robot[robot_id].mbx,robot[robot_id].mby}];
    berth[bid].num += 1;
    fout << "######### " << robot_id << " pull a goods in (" << robot[robot_id].x << "," <<robot[robot_id].y << ") its berth is (" << robot[robot_id].mbx << "," <<robot[robot_id].mby << ")\n";

    printf("pull %d\n", robot_id);
}

void changeDirection(int rid, int frame) {
    // string tmp = robot[rid].directions.back();
    
    // //如果剩下三个个方向全是墙就不改变方向
    // int i;
    // for (i = 0;i < 4; ++i) {
    //     if (dir[i] == tmp)  continue;
    //     if (mp[robot[rid].x + dx[i]][robot[rid].y + dy[i]] != '#' && mp[robot[rid].x + dx[i]][robot[rid].y + dy[i]] != '*')
    //         break;
    // }
    // if (i == 4) return;

    // if (frame % 20 == 0) {
        robot[rid].mbx = berth[rand() % 10].x;
        robot[rid].mby = berth[rand() % 10].y;
    // }
    
    //随机选择一个可以走的位置
    // int c,x,y,newx,newy;
    // do {
    //     c = rand() % 4;
    //     x = dx[c], y = dy[c];
    //     newx = robot[rid].x + x, newy = robot[rid].y + y;
    // } while (dir[c] == tmp || mp[newx][newy] == '#' || mp[newx][newy] == '*');

    int newx = robot[rid].x;
    int newy = robot[rid].y;

    //更新路径
    vector<string> t = BFS(newx,newy,robot[rid].mbx,robot[rid].mby);
    if (!t.empty()) {
        //如果碰巧路径不可达那就等下次碰撞了再选一次，这里不处理
        robot[rid].directions = t;
    // if (!t.empty())
    //     robot[rid].directions.push_back(dir[c]);
    }
}

int main() {
    Init();
    srand((unsigned)time(NULL));
    for(int frame = 1; frame <=15000; frame ++){
        int frame_id = Input();
        //第一帧以及每500帧操作一下船
        // if (frame == 1 || frame % 500 == 0)
        // 船的操作， 传入当前帧ID
        boatAction_cycle(frame_id);

        //新增的货物入队 集合到 Input 函数中了
        
        for (int i = 0; i < 10; ++i) {   
            if(i == 4) continue;      
            if (robot[i].status == 0 && robot[i].st != 0 && robot[i].has_goods) {
                robot[i].status = 1;
            }   
            priority_queue<Goods> temp;
            //找到空闲的机器人就去分配货物
            while (!goods_queue.empty() && robot[i].status == 0 && robot[i].st != 0 && !robot[i].has_goods) {
                Goods tmp = goods_queue.top();
                if (-60 <= tmp.x - robot[i].x && tmp.x - robot[i].x <= 60 && -60 <= tmp.y - robot[i].y && tmp.y - robot[i].y <=60) {
                    robot[i].myGoods.push(tmp);
                    goods_queue.pop();
                } else {
                    temp.push(goods_queue.top());
                    goods_queue.pop();
                }
            }
            while (!temp.empty()) {
                goods_queue.push(temp.top());
                temp.pop();
            }
            if (!robot[i].myGoods.empty()) {
                while (frame_id - robot[i].myGoods.top().frame >= 1000) {
                    robot[i].myGoods.pop();
                }
                Goods tmp = robot[i].myGoods.top();
                robot[i].myGoods.pop();
                robot[i].directions = BFS(robot[i].x,robot[i].y,tmp.x,tmp.y);
                robot[i].mbx = tmp.x;
                robot[i].mby = tmp.y;
                robot[i].goods_value = tmp.value;
                robot[i].status = 1;
            }
            

            //如果没带货物碰撞
            if (robot[i].st == 0 && !robot[i].has_goods) {
                robot[i].status = 0;
                robot[i].directions = {};
                robot[i].mbx = robot[i].mby = 0;
                continue;
            }

            //带着货物碰撞
            if (robot[i].st == 0 && robot[i].has_goods) {
                //changeDirection(i, frame_id);
                has_cracked[i] = 1;
                continue;
            }

            if (robot[i].status == 1 && !robot[i].has_goods) {
                //在拿货物的路上
                if (robot[i].x == robot[i].mbx && robot[i].y == robot[i].mby) {
                    robot[i].status = 0;
                    robot[i].mbx = robot[i].mby = 0;
                }
                
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
                if (has_cracked[i] == 1) {
                    changeDirection(i, frame_id);
                    has_cracked[i] = 0;
                }
                
                if (robot[i].directions.empty()) {
                    vector<string> tmp = BFS(robot[i].x, robot[i].y,berth[choice[i]].x,berth[choice[i]].y);
                    if (!tmp.empty()) {
                        robot[i].directions = tmp;
                        robot[i].mbx = berth[i].x;
                        robot[i].mby = berth[i].y;
                    }
                }
            
                robot_move(i);
                if (robot[i].directions.empty()) {
                    pullGoods(i);
                }
            }
        }
        while (!goods_queue.empty()) {
            goods_queue.pop();
        }
        puts("OK");
        //输出每一帧的泊位剩余货物大小
        fout << "frame_id: " << frame_id << endl;
        fout << "----------" << endl;
        for (int i = 0;i < 10; i++) {
            
            fout << "Berth " << i << ": " << berth[i].num << endl;
        }
        fout << "----------" << endl;
        fflush(stdout);
    }
    return 0;
}