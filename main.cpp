/*
 * @Author: thecoderalex tangzf2001@126.com
 * @Date: 2024-03-12 14:34:00
 * @LastEditors: thecoderalex tangzf2001@126.com
 * @LastEditTime: 2024-03-17 11:54:39
 * @FilePath: \HuaweiSOFT\main.cpp
 * @Description: 新增机器人负载参数，考虑机器人任务分配均匀；新增船舶开始时间判断
 * 
 * Copyright (c) 2024 by 菜就多练, All Rights Reserved. 
 */
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
const int dx[4] = {0, 1, -1, 0};
const int dy[4] = {-1, 0, 0, 1};
const string dir[4] = {"1", "3", "2", "0"};

char mp[M_SIZE][M_SIZE];    // 地图
bool has_cracked[10];   // 机器人是否发生碰撞
int boat_capacity;
int money,frame_id;
int k;
int choice[10] = {0,1,2,3,4,5,6,7,8,9}; // 机器人的泊位选择
map<pair<int,int>,int> berthid;   // 泊位坐标到id的映射
ofstream fout("out.txt");   // 调试输出

// 泊位
struct Berth {
    int x,y;
    int time;
    int velocity;
    bool flag;
    int num; //计算当前泊位拥有的货物数量
    Berth () {
        flag = false;
    }
}berth[B_SIZE];

// 船
struct Boat {
    int shipedFrame;    // 表示船在第几帧进入的港口
    int id;
    int status;
    int num;    // 表示船当前装了多少货物
    int myLastBerth;
    void ship(int shipId, int berthId) {
        printf("ship %d %d\n", shipId, berthId);
    }

    void go(int shipID) const {
        printf("go %d\n", shipID);
    }
}boat[BOAT_SIZE];

// 货物
struct Goods {
    int x,y;
    int value;
    int frame;
    double ratio;
    bool operator<(const Goods& other) const {
        return ratio < other.ratio;
    }
    Goods (int frame, int x, int y, int val) {
        this->frame = frame;
        this->x = x;
        this->y = y;
        this->value = val;
    }
};

// 机器人
struct Robot {
    int x,y;
    int mbx, mby; // 目标点坐标，可以是货物也可以是泊位
    bool has_goods;
    int goods_value;
    int status; // 0 free 1 work 2 initialized
    int st; // 读入的是否发生碰撞的标记
    vector<string> directions;
    priority_queue<Goods> myGoods;
}robot[10];

/**
 * @description: BFS判断障碍函数
 * @param {int} x   横坐标
 * @param {int} y   纵坐标
 * @return {bool}   (x,y)是否可走
 */
bool isValid(int x, int y) {
    return x >= 0 && x < M_SIZE && y >= 0 && y < M_SIZE && mp[x][y] != '#' && mp[x][y] != '*';
}

/**
 * @description: BFS寻路算法
 * @param {int} startX  起点横坐标
 * @param {int} startY  起点纵坐标
 * @param {int} endX    终点横坐标
 * @param {int} endY    终点纵坐标
 * @return {vector<string>} 倒序储存起点到重点的每一步的方向，即第一步在vector的尾部
 */
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

/**
 * @description: 初始化函数，读入地图、泊位、船只容量
 * @return {*}
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
    }
    scanf("%d", &boat_capacity);
    
    fout << "这一轮的 船容量 为: " << boat_capacity << "\n";
    fout << "######################\n";
    
    char okk[100];
    scanf("%s", okk);
    printf("OK\n");
    fflush(stdout);
}

/**
 * @description: 计算货物对机器人robot_id的价值
 * @param {int} value   货物价值
 * @param {double} distance 货物和泊位的曼哈顿距离
 * @param {int} robot_id    机器人id
 * @return {double} 货物对机器人的价值
 */
double goodsRatio(int value, double distance,int robot_id){
    //添加机器人的负载
    double load_coefficient = 0.8;
    double value_coefficient = 0.6;
    double distance_coefficient = 1.2;

    int load = robot[robot_id].myGoods.size() + 1;
    if (distance <= 0) return value * 1.0;
    
    //以曼哈顿距离为60为界限
    int p = 60;
    if(distance <= p){
        // 时间价值系数计算
        value_coefficient = 0.8 + (1 - 0.8) * (1 - sqrt(1-pow(1-distance / p, 2)));
        distance_coefficient = 0.8;
    }
    return value *  value_coefficient / (distance_coefficient * distance + load_coefficient * load);
}

/**
 * @description: 为机器人分配货物
 * @param {Goods} goods 待分配的货物
 * @return {*}
 */
void chooseRobot(Goods goods){
    double maxR = 0;
    int maxRoboId = -1;
    // 遍历机器人
    for(int i = 0; i < B_SIZE; i ++){
        // 找到当前机器人对应的泊位
        int berth_id = choice[i];
        int berth_x = berth[berth_id].x, berth_y = berth[berth_id].y;
        // 计算当前货物到这个泊点的曼哈顿距离
        double dist = sqrt(pow(abs(berth_x - goods.x), 2) + pow(abs(berth_y - goods.y), 2));
        // 计算当前货物对于该泊点的性价比
        double ratio = goodsRatio(goods.value, dist, i);
        if (ratio > maxR){
            maxR = ratio;
            maxRoboId = i;
        }
    }
    if(maxRoboId == -1) 
        fout << "!!!!!!!something wrong with your chooseRobot() !!!\n goods information: ( " 
        << goods.x << " , " << goods.y << " )\n";
    else{
        goods.ratio = maxR;
        robot[maxRoboId].myGoods.push(goods);
    }
}

/**
 * @description: 每一帧的输入，输入货物id，得分，新增的货物，机器人位置和状态，船的位置和状态
 * @return {int}    帧id
 */
int Input()
{
    scanf("%d%d", &frame_id, &money);
    scanf("%d", &k);
    for(int i = 0; i < k; i ++)
    {
        int x, y, val;
        scanf("%d%d%d", &x, &y, &val);
        Goods goods = {frame_id, x, y, val};
        // goods_queue.push(goods);
        // 选择一个性价比最高的机器人，然后进入它的货物队列
        chooseRobot(goods);
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

/**
 * @description: 计算船只来得及回到虚拟点的最后帧数
 * @param {int} frameID 当前帧数
 * @param {int} boatID  船只id
 * @return {bool}   这一帧是否是船来得及回到虚拟点的最后时刻
 */
bool boatLastChance (int frameID, int boatID) {
    if (boat[boatID].id != -1 && boat[boatID].status != 0) {
        return (15000 - frameID == berth[boat[boatID].id].time);
    } else if (boat[boatID].id != -1 && boat[boatID].status == 0) {
        return (15000 - frameID == berth[boat[boatID].myLastBerth].time);
    }
    return false;
}

/**
 * @description: 获取当前拥有最多货物的港口
 * @return {int}    港口id
 */
int getMaxGoodsBerthID () {
    int maxGoods = 0;
    int BerthID = -1;
    for (int i = 0; i < 10; i++) {
        if (!berth[i].flag)
            if (berth[i].num >= maxGoods) {
                maxGoods = berth[i].num;
                BerthID = i;
            }
    }
    return BerthID;
}

/**
 * @description: 船是否满载
 * @param {int} boatID  船只id
 * @return {bool}   船是否满载
 */
bool isBoatFull (int boatID) {
    return boat[boatID].num == boat_capacity;
}

/**
 * @description: 船只的移动策略
 * @param {int} frameID 当前帧数
 * @return {*}
 */
void boatAction (int frameID) {
    /* boat.status:
        0 : moving
        1 : finishing shipping OR waiting for loading
        2 : waiting for berth

        boat.id: id is the berth id; -1 means virtual point.
    */
    for (int i = 0; i < BOAT_SIZE; i ++) {
        if (boat[i].id == -1) {
            fout << "Boat " << i << " is going / at virtual point ! My Status is: " << boat[i].status << '\n';
        }
        if (isBoatFull(i)) {
            boat[i].go(i);
            berth[boat[i].myLastBerth].flag = false;
            boat[i].num = 0;
            continue;
        }
        // 判断是不是最后一次送货机会
        if (boatLastChance(frameID, i)) { //可能会导致时间上的开销增加
            boat[i].go(i);
            berth[boat[i].myLastBerth].flag = false;
            //取出港口中的货物
            fout << "!!!!!!!!!!!!Boat " << i << " now is the last chance : Berth time is " 
            << berth[boat[i].id].time << " Status is " << boat[i].status 
            << " Now the frame is " << frameID << "I have " << boat[i].num << "Goods\n";
            continue;
        }
        if (boat[i].status == 0)
            continue;
        if (boat[i].status == 2) {
            int berthID = getMaxGoodsBerthID();
            if (berthID == boat[i].id)
                boat[i].shipedFrame += 1;
            else {
                boat[i].ship(i, berthID);
                boat[i].shipedFrame = frameID + berth[berthID].time;
            }
        }
        if (boat[i].id == -1 && boat[i].status != 0) {
            int berthID = getMaxGoodsBerthID();
            fout << "$$$$$$$$$ Boat " << i << " chooses the " << berthID << endl;
            boat[i].myLastBerth = berthID;
            boat[i].ship(i, berthID);
            berth[berthID].flag = true;
            boat[i].shipedFrame = frameID + berth[berthID].time;
        } else if (boat[i].id != -1 && boat[i].status == 1) {
            boat[i].myLastBerth = boat[i].id;
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
            // 如果我把这个港口装空了 我就找下一个
            if (!isBoatFull(i) && berth[boat[i].id].num == 0) {
                berth[boat[i].myLastBerth].flag = false;
                int berthID = getMaxGoodsBerthID();
                fout << "$$$$$$$$$ Boat " << i << " chooses the " << berthID << endl;
                boat[i].ship(i, berthID);
                berth[berthID].flag = true;
                boat[i].shipedFrame = frameID + 500;
            }
        }
        fout << "Boat: " << i << " Status is " << boat[i].status << " Now is at " << boat[i].id <<"\n";
    }
}

/**
 * @description: 机器人移动一格的操作
 * @param {int} robot_id    机器人id
 * @return {*}
 */
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

/**
 * @description: 机器人拿取货物
 * @param {int} robot_id    机器人id
 * @return {*}
 */
void getGoods(int robot_id){
    printf("get %d\n", robot_id);
}

/**
 * @description: 机器人放置货物到泊位
 * @param {int} robot_id    机器人id
 * @return {*}
 */
void pullGoods(int robot_id){
    robot[robot_id].status = 0;
    //增加泊位的货物
    int bid = berthid[{robot[robot_id].mbx,robot[robot_id].mby}];
    berth[bid].num += 1;
    fout << "######### " << robot_id << " pull a goods in (" << robot[robot_id].x << "," <<robot[robot_id].y << ") its berth is (" << robot[robot_id].mbx << "," <<robot[robot_id].mby << ")\n";
    printf("pull %d\n", robot_id);
}

/**
 * @description: 机器人碰撞后随机更换方向以解除碰撞
 * @param {int} rid 机器人id
 * @param {int} frame   当前帧数
 * @return {*}
 */
void changeDirectionRandom(int rid, int frame) {
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

/**
 * @description: 机器人碰撞后更换泊位以解除碰撞
 * @param {int} rid 机器人id
 * @param {int} frame   当前帧
 * @return {*}
 */
void changeDirection_cgBerth(int rid, int frame) {
    //随机更换泊位
    robot[rid].mbx = berth[rand() % 10].x;
    robot[rid].mby = berth[rand() % 10].y;
    //更新路径
    int newx = robot[rid].x;
    int newy = robot[rid].y;
    vector<string> t = BFS(newx,newy,robot[rid].mbx,robot[rid].mby);
    if (!t.empty()) {
        //如果碰巧路径不可达那就等下次碰撞了再选一次，这里不处理
        robot[rid].directions = t;
    }
}

/**
 * @description: 机器人碰撞后顺时针转向以解除碰撞
 * @param {int} rid 机器人id
 * @param {int} frame   当前帧
 * @return {*}
 */
void changeDirection_turnAround(int rid, int frame) {
    string tmp = robot[rid].directions.back();
    string turnDir[4] = {"2", "0", "3", "1"};
    int dx[4] = {-1, 0, 1, 0};
    int dy[4] = {0, 1, 0, -1};
    //如果剩下三个个方向全是墙就不改变方向
    int i;
    int j = 0;
    for (i = 0;i < 4; ++i) {
        if (turnDir[i] == tmp)  continue;
        if (mp[robot[rid].x + dx[i]][robot[rid].y + dy[i]] != '#' && mp[robot[rid].x + dx[i]][robot[rid].y + dy[i]] != '*')
            break;
    }
    if (i == 4) return;
    //找到当前方向
    for(j = 0; j < 4; j ++){
        if (tmp == turnDir[j]) break;
    }
    //顺时针找一个能够走的位置
    int c = (j+1) % 4,x,y,newx,newy;
    do {
        x = dx[c], y = dy[c];    
        newx = robot[rid].x + x, newy = robot[rid].y + y;
        c = (c + 1) % 4;
    } while (turnDir[c] == tmp || mp[newx][newy] == '#' || mp[newx][newy] == '*');
    //更新路径
    vector<string> t = BFS(newx,newy,robot[rid].mbx,robot[rid].mby);
    if (!t.empty()) {
        //如果碰巧路径不可达那就等下次碰撞了再选一次，这里不处理
        robot[rid].directions = t;
        robot[rid].directions.push_back(turnDir[c]);
    }
}

int main() {
    Init();
    srand((unsigned)time(NULL));
    for(int frame = 1; frame <=15000; frame ++){
        int frame_id = Input();
        
        fout << "Robot_Goods_Size: Total: " << k << endl;
        for (int i = 0;i < 10; ++i) {
            fout << "[*]Robot " << i << " : " << robot[i].myGoods.size() << endl;
        }
        fout << "----------" << endl;

        // 船的操作， 传入当前帧ID，判断船什么时候出发
        if (frame > 7000)
            boatAction(frame_id);
        
        for (int i = 0; i < 10; ++i) {    
            // 解决原地get的问题，让机器人重新寻路
            if (robot[i].status == 0 && robot[i].st != 0 && robot[i].has_goods) {
                robot[i].status = 1;
            }   
            
            // 当机器人空闲且没拿货物且货物列表不空的时候，直接从自己的队列里面取出下一个要去拿的货物
            if (robot[i].status == 0 && robot[i].st != 0 && !robot[i].has_goods && !robot[i].myGoods.empty()){
                Goods tmp = robot[i].myGoods.top();
                robot[i].myGoods.pop();
                robot[i].directions = BFS(robot[i].x,robot[i].y,tmp.x,tmp.y);
                robot[i].mbx = tmp.x;
                robot[i].mby = tmp.y;
                robot[i].goods_value = tmp.value;
                robot[i].status = 1;
            }
            
            // 如果没带货物碰撞，机器人重新取货
            if (robot[i].st == 0 && !robot[i].has_goods) {
                robot[i].status = 0;
                robot[i].directions = {};
                robot[i].mbx = robot[i].mby = 0;
                continue;
            }

            // 带着货物碰撞，将碰撞标记打上，等待释放碰撞后解决碰撞
            if (robot[i].st == 0 && robot[i].has_goods) {
                has_cracked[i] = 1;
                continue;
            }

            // 机器人取货或者放货
            if (robot[i].status == 1 && !robot[i].has_goods) {
                // 在拿货物的路上
                if (robot[i].x == robot[i].mbx && robot[i].y == robot[i].mby) {
                    robot[i].status = 0;
                    robot[i].mbx = robot[i].mby = 0;
                }
                robot_move(i);
                if (robot[i].directions.empty()) {
                    getGoods(i);
                } 
            } else if (robot[i].status == 1 && robot[i].has_goods) {
                // 拿到货物去泊位
                if (has_cracked[i] == 1) {
                    // 如果刚从碰撞状态解除就更换方向
                    changeDirectionRandom(i, frame_id);
                    has_cracked[i] = 0;
                }
                
                // 只要没有计算过去港口的路径或者方向空了就计算路径
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
        puts("OK");
        fflush(stdout);

        //输出每一帧的泊位剩余货物大小
        fout << "frame_id: " << frame_id << endl;
        fout << "----------" << endl;
        for (int i = 0;i < 10; i++) {
            fout << "Berth " << i << ": " << berth[i].num << endl;
        }
        fout << "----------" << endl;
    }
    return 0;
}