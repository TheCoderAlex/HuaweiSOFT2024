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
ofstream fout("out.txt");

char mp[210][210];
int boat_capacity;
int money,frame_id;
int k;
int total_k = 0 ;   //用于统计一局一共会给多少货物
int total_val = 0; //用于统计一局一共会给多少米
int total_berth_value = 0;
int total_berth_num = 0;
int goods_sum = 0;
int goods_ignore = 0;
bool start_flag = false;

// queue<pair<int, int>> goods_queue;

int dx[4] = {0, 1, -1, 0};
int dy[4] = {-1, 0, 0, 1};
string dir[4] = {"1", "3", "2", "0"};
bool has_cracked[10];
int choice[10] = {0,1,2,3,4,5,6,7,8,9};  //机器人的泊位选择
map<pair<int,int>,int> berthid;   //泊位坐标到id的映射

bool isValid(int x, int y) {
    return x >= 0 && x < M_SIZE && y >= 0 && y < M_SIZE && mp[x][y] != '#' && mp[x][y] != '*';
}

vector<string> BFS(int startX, int startY, int endX, int endY) {
    queue<pair<int, int> > q;
    vector<vector<bool> > visited(M_SIZE, vector<bool>(M_SIZE, false));
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

const int N = 200;
const int maxn = N * N;
const int INF = 0x3f3f3f3f;

vector<int> dist(maxn, INF);
vector<vector<int>> minlen(10,vector<int>(maxn,INF));
 const vector<pair<int, int>> directions{{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

void dijkstra(int start) {
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;
    vector<bool> vis(maxn,false);

    fill(dist.begin(), dist.end(), INF);

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
                    pq.push({newCost, newIndex});
                }
            }
        }
    }
}

struct Berth
{
    /* 泊位 */
    int x,y;
    int time;
    int velocity;
    bool flag;
    int num; //计算当前泊位拥有的货物数量
    int value; //计算当前泊位的价值
    queue<int> goodsList;
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

struct Robot
{
    int x,y;
    int mbx, mby; //?
    bool has_goods;
    int goods_value;
    int status; //0 free 1 work 2 collision
    int st;
    int waiting_frame;
    vector<string> directions;
    priority_queue<Goods> myGoods;
}robot[10];

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
        fout << "Berth :" << id << " TIME is " << berth[id].time << "装卸速度: " << berth[id].velocity << endl;
    }
    scanf("%d", &boat_capacity);
    fout << "-----------Boat_Capacity is" << boat_capacity << endl;
    
    for (int i = 0; i < 10; ++i) {
        dijkstra(berth[i].x * 200 + berth[i].y);
        minlen[i] = dist;
    }

    char okk[100];
    scanf("%s", okk);
    printf("OK\n");
    fflush(stdout);
}

//计算货物的性价比
double goodsRatio(int value, int distance,int robot_id){
    //添加机器人的负载
    double load_coefficient = 0.6;
    double value_coefficient = 0.8;
    double distance_coefficient = 1.2;

    int load = robot[robot_id].myGoods.size() + 1;
    if (distance <= 0) return value * 1.0;
    

    //以距离为60为界限
    int p = 60;
    if(distance <= p){
        // 时间价值系数计算
        value_coefficient = value_coefficient + (1 - value_coefficient) * (1 - sqrt(1-pow(1- (double) distance / p, 2)));
        // distance_coefficient = 1;
        distance_coefficient = 1.2;
    } else {
         distance_coefficient = 1.2 + (distance - p) / 10.0;
    }
    return value *  value_coefficient / (distance_coefficient * distance + load_coefficient * load);
}


// 为当前生成的货物寻找一个机器人
void chooseRobot(Goods goods){
    double maxR = 0;
    int maxRoboId = -1;
    double threshold = 0.1;
    // 遍历机器人
    for(int i = 0; i < B_SIZE; i ++){
        // 找到当前机器人对应的泊位
        int berth_id = choice[i];
        int berth_x = berth[berth_id].x, berth_y = berth[berth_id].y;
        // 计算当前货物到这个泊点的曼哈顿距离
        int dist = minlen[berth_id][goods.x * 200 + goods.y];
        // 计算当前货物对于该泊点的性价比
        double ratio = goodsRatio(goods.value, dist, i);
        // 如果robot的货物队列中的货物太多了，这里是多于50，那么判断当前的货物价值是不是大于这个队列中的最大值，大于才入队，小于就不入
        // if (robot[i].myGoods.size() >= 50 && robot[i].myGoods.top().ratio >= ratio )
        //     continue;
        if (ratio > maxR){
            maxR = ratio;
            maxRoboId = i;
        }
    }   
    if (maxR < threshold){
        goods_ignore ++;
    }else{
        goods.ratio = maxR;
        robot[maxRoboId].myGoods.push(goods);
    }
}

int Input()
{
    scanf("%d%d", &frame_id, &money);
    scanf("%d", &k);
    goods_sum += k;
    for(int i = 0; i < k; i ++)
    {
        int x, y, val;
        scanf("%d%d%d", &x, &y, &val);
        Goods goods = {frame_id, x, y, val};
        total_val += val;
        // goods_queue.push(goods);
        // 选择一个性价比最高的机器人，然后进入它的货物队列
        chooseRobot(goods);
    }
    total_k += k;
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
//改一
int getMaxGoodsBerthID () {
    int maxGoodsRate = -1;
    int BerthID = -1;
    for (int i = 0; i < 10; i++) {
        if (!berth[i].flag)
            if (berth[i].num >= maxGoodsRate) {
                maxGoodsRate = berth[i].num;
                BerthID = i;
            }
        }   
    return BerthID == -1 ? rand() % 10 : BerthID;
}

// 找到拥有最高价值的泊位
int getMaxValueBerthID () {
    int maxValue = -1;
    int BerthID = -1;
    for (int i = 0; i < 10; i++) {
        if (!berth[i].flag)
            if (berth[i].value >= maxValue) {
                maxValue = berth[i].value;
                BerthID = i;
            }
    }
    return BerthID;
}

int getMaxRatioBerthID() {
    int maxRatio = -1;
    int BerthID = -1;
    for (int i = 0; i < 10; i++) {
        if (!berth[i].flag && berth[i].num > 0) 
            if (berth[i].value / berth[i].num >= maxRatio) {
                maxRatio = berth[i].value / berth[i].num;
                BerthID = i;
            }
    }
    return BerthID == -1 ? rand() % 10 : BerthID;
}

bool isBoatFull (int boatID) {
    return boat[boatID].num == boat_capacity;
}

void boatAction (int frameID) {
    /* boat.status:
        0 : moving
        1 : finishing shipping OR waiting for loading
        2 : waiting for berth

        boat.id: id is the berth id; -1 means virtual point.
    */
    for (int i = 0; i < BOAT_SIZE; i ++) {
        if (isBoatFull(i)) {
            boat[i].go(i);
            fout << "Boat: " << i << " is full" << endl;
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
            << berth[boat[i].id].time << " Status is " << boat[i].status << "I am now at " << boat[i].myLastBerth 
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
            boat[i].myLastBerth = berthID;
            boat[i].ship(i, berthID);
            fout << "Boat " << i << " Now is going to " << berthID << " have" << boat[i].num << "Goods" << endl;
            berth[berthID].flag = true;
            boat[i].shipedFrame = frameID + berth[berthID].time;
        } else if (boat[i].id != -1 && boat[i].status == 1) {
            boat[i].myLastBerth = boat[i].id;
            // 如果 此时泊位的货物数量小于泊位每帧装卸速度，那就直接将 泊位num 赋值为0,否则就让让 nun - 装卸速度 velocity
            // 如果 船当前剩余容量小于泊位装卸速度 那么这一帧 最多装我还差的量
            // load_num 为装货数量
            if (berth[boat[i].id].num < berth[boat[i].id].velocity) {
                boat[i].num += berth[boat[i].id].num;
                berth[boat[i].id].num = 0;
                berth[boat[i].id].value = 0;
                while (!berth[boat[i].id].goodsList.empty())
                    berth[boat[i].id].goodsList.pop();
            } else if (boat_capacity - boat[i].num < berth[boat[i].id].velocity) {
                int load_num = boat_capacity - boat[i].num;
                boat[i].num = boat_capacity;
                berth[boat[i].id].num -= load_num;
            } else {
                boat[i].num += berth[boat[i].id].velocity;
                berth[boat[i].id].num -= berth[boat[i].id].velocity;
                int load_num = berth[boat[i].id].velocity;
            }
            
            // 如果我把这个港口装空了 我就找下一个
            if (boat[i].id != -1 && !isBoatFull(i) && berth[boat[i].id].num == 0) {
                if (boat_capacity - boat[i].num >= 7) {
                    berth[boat[i].myLastBerth].flag = false;
                    int berthID = getMaxGoodsBerthID();
                    boat[i].ship(i, berthID);
                    berth[berthID].flag = true;
                    boat[i].shipedFrame = frameID + 500;
                    fout << "Boat : " << i << "我装的还不够，去下一个: " << berthID << endl;
                }
            }
        }
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
    berth[bid].value += robot[robot_id].goods_value;
    berth[bid].goodsList.push(robot[robot_id].goods_value);
    total_berth_num += 1;
    total_berth_value += robot[robot_id].goods_value;
    printf("pull %d\n", robot_id);
}

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

void changeDirection_cgBerth(int rid, int frame) {
    robot[rid].mbx = berth[rand() % 10].x;
    robot[rid].mby = berth[rand() % 10].y;
    int newx = robot[rid].x;
    int newy = robot[rid].y;
    //更新路径
    vector<string> t = BFS(newx,newy,robot[rid].mbx,robot[rid].mby);
    if (!t.empty()) {
        //如果碰巧路径不可达那就等下次碰撞了再选一次，这里不处理
        robot[rid].directions = t;
    }
}

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
        //第一帧以及每500帧操作一下船
        // if (frame == 1 || frame % 500 == 0)
        // 船的操作， 传入当前帧ID
        
        // 什么时候开始出发?
        // if (frame_id > 7000)
        //     boatAction(frame_id);

        if (start_flag)
            boatAction(frame_id);
        else {
            for (int i = 0; i < B_SIZE; i++){
                if (berth[i].num >= boat_capacity * 0.5) {
                    start_flag = true;
                    break;
                }
            }
        }

        //新增的货物入队 集合到 Input 函数中了
        
        for (int i = 0; i < 10; ++i) {    
            // if (i == 3) continue;   
            if (robot[i].status == 0 && robot[i].st != 0 && robot[i].has_goods) {
                robot[i].status = 1;
            }   
            //当机器人空闲且没拿货物且货物列表不空的时候，直接从自己的队列里面取出下一个要去拿的货物
            if (robot[i].status == 0 && robot[i].st != 0 && !robot[i].has_goods && !robot[i].myGoods.empty()){
                // 把前 1 个超时的货物 丢弃掉
                int flag = 1;
                while (flag > 0 && frame_id - robot[i].myGoods.top().frame >= 1000 && !robot[i].myGoods.empty()) {
                    robot[i].myGoods.pop();
                    flag--;
                }
                if (!robot[i].myGoods.empty()) { 
                    Goods tmp = robot[i].myGoods.top();
                    robot[i].myGoods.pop();
                    robot[i].directions = BFS(robot[i].x,robot[i].y,tmp.x,tmp.y);
                    robot[i].mbx = tmp.x;
                    robot[i].mby = tmp.y;
                    robot[i].goods_value = tmp.value;
                    robot[i].status = 1;
                }
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

            // 根据空闲帧数进行赋值
            if (robot[i].myGoods.empty()) {
                robot[i].waiting_frame += 1;
            } else {
                robot[i].waiting_frame = 0;
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
                if (has_cracked[i] == 1) {
                    changeDirectionRandom(i, frame_id);
                    // changeDirection_turnAround(i, frame_id);
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
        fout << "########frame_id: " << frame_id << endl;
        fout << "----------" << endl;
        for (int i = 0;i < 10; i++) {
            fout << "Berth " << i << ": " << berth[i].num << " 累计 Value is" << berth[i].value<< endl;
        }
        fout << "----------" << endl;
        fout << "?????total K is: " << total_k << "total value: " << total_val << endl;
        fout << "?????We get total K is: " << total_berth_num << "total value: " << total_berth_value << endl;
        fout << "----------" << endl;
        puts("OK");
        fflush(stdout);
    }
    return 0;
}
