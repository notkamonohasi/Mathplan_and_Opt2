
//LRTA
//このプログラムでは座標を(縦方向, 横方向), [縦方向][横方向]で書きます、基本的に全て縦->横の順番です
//自作のclassの最初の文字は大文字で、それ以外で大文字を使うことはないです

#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <random>
#define ll long long
#define INF (1 << 29)
#define rep(i, n) for (ll i = 0; i < (ll)(n); i++)
using namespace std;

ll vec[][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
random_device random_generator;   //rand()が確定的？なので非確定的？な乱数を使う

ll REPEAT_TIME = 100;   //何回LRTA_star_loopを回すか
vector<ll> record_times_vec;

class Place{
public : 
    ll x, y;   //yが縦方向、xが横方向

    //コンストラクタ
    Place(ll a, ll b){   //縦->横の順番でいれる
        y = a;
        x = b;
    }
    Place(){}

    //operator
    bool operator == (const Place& place) const{
        if(x == place.x && y == place.y) return true;
        else return false; 
    }

    //デバッグ用
    void print(){
        cout << x << " " << y << endl;
    }
};


//一つのロボットが占有する二つの場所の位置関係について記述する単位ベクトルclass、vector<>とは大文字小文字の区別
class Vector{
public : 
    ll dx, dy;

    //コンストラクタ
    Vector(ll a, ll b){   //縦->横の順で
        dy = a;
        dx = b;

        //デバッグ用、dx, dyの絶対値は1以下でなければいけない
        if(abs(a) > 1 || abs(b) > 1){
            cout << endl;
            cout << "----- ERROR -----" << endl;
            cout << "----- Vectorに1より大きい数字が入っている -----" << endl;
            cout << a << " " << b << endl;
            cout << endl;
        }
    }
    Vector(){}
};


template<class T>

//二次元の盤面、C++だと通常の二次元配列を引数や返り値にするのが面倒なため
//このclassは元々作ってあったのを流用したので、余計な機能あり
class Array_2d{
public : 
    vector<vector<T> > arr;
    ll tate;
    ll yoko;
    
    //コンストラクタ
    Array_2d(ll a, ll b){
        tate = a;
        yoko = b;
        vector<vector<T> > memo(a, vector<T>(b));
        arr = memo;
    }
    Array_2d(){}
    
    //operator
    vector<T>& operator[](ll a){return arr[a];}

    //デバッグ用
    void print(){
        rep(i, arr.size()){
            rep(j, arr[0].size()){
                cout << arr[i][j] << " ";
            }
            cout << endl;
        }
    }
    
    //deepcopy、参照配列などの面倒な問題を回避するため、計算量が余計に増えるので本当はあまり良くない
    Array_2d copy(){
        ll tate = arr.size();
        ll yoko = arr[0].size();
        Array_2d<T> copied_arr(tate, yoko);
        rep(i, tate){
            rep(j, yoko){
                copied_arr[i][j] = arr[i][j];
            }
        }
        return copied_arr;
    }
};


//一つのロボットが占有する場所を管理する
class Robot{
public : 
    Place robo1, robo2;

    //コンストラクタ
    Robot(Place p1, Place p2){
        if(p1.x < p2.x){
            robo1 = p1;
            robo2 = p2;
        }
        else if(p1.x == p2.x){
            if(p1.y < p2.y){
                robo1 = p1;
                robo2 = p2;
            }
            else{
                robo1 = p2;
                robo2 = p1;
            }
        }
        else{
            robo1 = p2;
            robo2 = p1;
        }

        //デバッグ用、p1とp2はつながっていなければならない
        if(p1.x != p2.x && p1.y != p2.y){
            cout << endl;
            cout << "----- ERROR -----" << endl;
            cout << "----- ロボットの位置設定がおかしい -----" << endl;
            cout << p1.x << " " << p1.y << ", " << p2.x << " " << p2.y << endl;
            cout << endl;
        }
    }
    Robot(){}

    //operator
    bool operator == (const Robot& robot) const{
        if(robo1 == robot.robo1 && robo2 == robot.robo2) return true;
        else if(robo1 == robot.robo2 && robo2 == robot.robo1) return true;
        else return false;
    }

    //mapに乗せたいので、大小関係を定義する
    //xが小さいほど、xが同じならyが小さいほど... ということにしている。大小関係を定義することが大事なのであって、どちらが大きいかに意味はない
    bool operator < (const Robot& robot) const{   
        ll me_x = robo1.x + robo2.x;
        ll you_x = robot.robo1.x + robot.robo2.x;
        ll me_y = robo1.y + robo2.y;
        ll you_y = robot.robo1.y + robot.robo2.y;

        if(me_x != you_x) return me_x < you_x;
        else return me_y < you_y;
    }

    void print(){
        cout << robo1.x << " " << robo1.y << ", " << robo2.x << " " << robo2.y << endl;
    }
};


//現在のRobotが枠内にいるか、壁の中にいないかを判定（つまりルールに従っているか）
bool judge_env(Robot robot, Array_2d<char> environment){
    Place robo1 = robot.robo1;
    Place robo2 = robot.robo2;
    ll tate = environment.tate;
    ll yoko = environment.yoko;

    //枠内にいるかを判定
    if(robo1.x < 0 || robo1.x >= yoko) return false;
    if(robo2.x < 0 || robo2.x >= yoko) return false;
    if(robo1.y < 0 || robo1.y >= tate) return false;
    if(robo2.y < 0 || robo2.y >= tate) return false;

    //次にRobotの位置が壁になっていないか
    if(environment[robo1.y][robo1.x] == '#') return false;
    if(environment[robo2.y][robo2.x] == '#') return false;

    return true;   //今までの条件を全てクリアできていればok
}


void print_field(Robot robot, Robot start, Robot goal, Array_2d<char> environment){
    Array_2d<char> field = environment.copy();

    Place start1 = start.robo1;
    Place start2 = start.robo2;
    field[start1.y][start1.x] = 'S';
    field[start2.y][start2.x] = 'S';

    Place goal1 = goal.robo1;
    Place goal2 = goal.robo2;
    field[goal1.y][goal2.x] = 'G';
    field[goal2.y][goal2.x] = 'G';

    Place robo1 = robot.robo1;
    Place robo2 = robot.robo2;
    field[robo1.y][robo1.x] = 'R';
    field[robo2.y][robo2.x] = 'R';

    field.print();
}


//class名の関係でわかりにくいが、例えば、(0,1)に対して、それに直交する単位ベクトル{(1,0), (-1, 0)}を返す
//90度回転のところで使う
vector<Vector> calc_vertical_vector(Vector robo_vector){
    if(robo_vector.dy != 0){   //y方向のベクトル
        Vector vector1(0, 1);
        Vector vector2(0, -1);
        vector<Vector> ret;
        ret.push_back(vector1);   ret.push_back(vector2);
        return ret;
    }
    else{   //x方向のベクトル
        Vector vector1(1, 0);
        Vector vector2(-1, 0);
        vector<Vector> ret;
        ret.push_back(vector1);   ret.push_back(vector2);
        return ret;
    }
}


//通常の比較では値が同じ時の処理が不十分
//値が同じ時は50%の確率で大小を選べるようにしたい
//左の方が小さい時trueを返す
bool compare(ll a, ll b){
    if(a < b) return true;
    else if(a > b) return false;
    else{
        if(random_generator() % 2 == 0) return true;
        else return false;
    }
}


//スコア計算
//推移コスト+ゴールまでのコスト、ゴールまでのコストが決まってない時はマンハッタン距離
//A＊の時と同様、ロボットの平均位置とゴールの関係のマンハッタン距離を使っており、小数が出てくるのを防ぐため、全て2倍して処理
ll calc_score(Robot robot, Robot goal, map<Robot, ll> mp){
    Place robo1 = robot.robo1;
    Place robo2 = robot.robo2;
    Place goal1 = goal.robo1;
    Place goal2 = goal.robo2;
    ll score;

    if(mp[robot] == 0){
        score = 2 + abs((robo1.x + robo2.x - goal1.x - goal2.x)) + 
                  abs(robo1.y + robo2.y - goal1.y - goal2.y);
    }
    else{
        score = 2 + mp[robot];
    }
    
    return score;
}


//ゴールに辿りついた時の出力
void print_when_goal(vector<Robot> record_robot_vec, vector<ll> record_score_vec, Array_2d<char> environment, ll count, ll search_num){
    cout << "--- " << search_num + 1 << "回目の探索の結果 ---" << endl;
    cout << "累計探索回数 : " << count << endl;
    cout << endl;
    rep(i, record_robot_vec.size()){
        Robot robot = record_robot_vec[i];
        Place robo1 = robot.robo1;
        Place robo2 = robot.robo2;
        Array_2d<char> arr = environment.copy();
        arr[robo1.y][robo1.x] = 'R';
        arr[robo2.y][robo2.x] = 'R';
        
        cout << "depth = " << i << endl;
        cout << "best_score : " << record_score_vec[i] << endl;
        arr.print();
        cout << endl;
    }

    cout << "--- " << search_num + 1 << "回目の探索の結果表示終了 ---" << endl;
    cout << endl;
}


//LRTA*探索
//スタート地点からゴールにたどり着くまで探索する
//countは何個目の探索か、search_numは何回目のLRTA_star_loopか
void LRTA_star_loop(Robot start, Robot goal, Array_2d<char> environment, map<Robot, ll>& mp, ll& count, ll search_num){
    Robot state = start;   //stateを更新していく
    vector<Robot> record_robot_vec;   //遷移を記録する
    record_robot_vec.push_back(start);
    vector<ll> record_score_vec;
    record_score_vec.push_back(0);   //最初なのでとりあえず0を入れておく

    while(true){
        Place robo1 = state.robo1;
        Place robo2 = state.robo2;
        Robot best_robot;   //最小scoreを出したRobotの配置を記録
        ll best_score = INF;   //最小scoreを記録

        //ゴールした時
        if(state == goal){
            record_times_vec.push_back(count);
            print_when_goal(record_robot_vec, record_score_vec, environment, count, search_num);
            return;
        }

        //並進
        rep(k, 4){   ////下->右->上->左の順で見ていく
            Place next_robo1(robo1.y + vec[k][0], robo1.x + vec[k][1]);   //vecは最初の方に定義されている、隣接位置への単位ベクトル
            Place next_robo2(robo2.y + vec[k][0], robo2.x + vec[k][1]);
            Robot next_robot(next_robo1, next_robo2);
            if(judge_env(next_robot, environment) == false) continue;   //ルール違反なら終了
            else{
                ll score = calc_score(next_robot, goal, mp);
                if(compare(score, best_score) == true){
                    best_score = score;
                    best_robot = next_robot;
                }
            }
        }

        //robo1を中心に90度回転
        if(true){   //スコープを決めておきたいだけなので、特に意味はない
            Vector robo_vector(robo1.y - robo2.y, robo1.x - robo2.x);   //robo2 -> robo1のベクトル
            vector<Vector> robo_vectors = calc_vertical_vector(robo_vector);
            rep(i, robo_vectors.size()){
                Place next_robo1(robo1.y, robo1.x);   //robo1は不動
                Place next_robo2(robo1.y + robo_vectors[i].dy, robo1.x + robo_vectors[i].dx);
                Robot next_robot(next_robo1, next_robo2);
                if(judge_env(next_robot, environment) == false) continue;   //ルール違反なら終了
                else{
                    ll score = calc_score(next_robot, goal, mp);
                    if(compare(score, best_score) == true){
                        best_score = score;
                        best_robot = next_robot;
                    }
                }
            }
        }

        //robo2を中心に90度回転
        if(true){   //スコープを決めておきたいだけなので、特に意味はない
            Vector robo_vector(robo1.y - robo2.y, robo1.x - robo2.x);   //robo2 -> robo1のベクトル
            vector<Vector> robo_vectors = calc_vertical_vector(robo_vector);
            rep(i, robo_vectors.size()){
                Place next_robo1(robo2.y + robo_vectors[i].dy, robo2.x + robo_vectors[i].dx);   
                Place next_robo2(robo2.y, robo2.x);   //robo2は不動
                Robot next_robot(next_robo1, next_robo2);
                if(judge_env(next_robot, environment) == false) continue;   //ルール違反なら終了
                else{
                    ll score = calc_score(next_robot, goal, mp);
                    if(compare(score, best_score) == true){
                        best_score = score;
                        best_robot = next_robot;
                    }
                }
            }
        }

        //更新
        mp[state] = best_score;
        state = best_robot;   //stateを更新
        record_robot_vec.push_back(best_robot);   //一番スコアの小さかったRobotを記録
        record_score_vec.push_back(best_score);   //一番小さかったスコアを記録
        count++;
    }
}


//LRTA_star_loopを繰り返し行う
//mpは参照渡しをしているため、前の探索が次の探索に活かされる
void LRTA_star(Robot start, Robot goal, Array_2d<char> environment){
    map<Robot, ll> mp;   //Robotの配置のコスト(授業資料でf(x))を管理している
    ll count = 1;
    rep(i, REPEAT_TIME) LRTA_star_loop(start, goal, environment, mp, count, i);   //繰り返し行うことによって精度を高めていく

    cout << endl;
    cout << "--- 探索回数を表示する ---" << endl;
    rep(i, record_times_vec.size()){
        if(i == 0) cout << i + 1 << "回目 : " << record_times_vec[i] << endl;
        else cout << i + 1 << "回目 : " << record_times_vec[i] - record_times_vec[i - 1] << endl;
    }
    cout << endl;
}


int main(){
    ll tate, yoko;
    cin >> tate >> yoko;

    Array_2d<char> environment(tate, yoko);
    rep(i, tate){
        rep(j, yoko){
            cin >> environment[i][j];
        }
    }

    //startを設定
    Robot start;
    Place start1, start2;
    bool set_start = false;   //一個'S'を見つけたらtrueに変える
    rep(i, tate){
        rep(j, yoko){
            if(environment[i][j] == 'S' && set_start == false){
                start1 = Place(i, j);
                set_start = true;
            }
            else if(environment[i][j] == 'S' && set_start == true){
                start2 = Place(i, j);
                start = Robot(start1, start2);
            }
        }
    }

    //goalを設定
    Robot goal;
    Place goal1, goal2;
    bool set_goal = false;   //一個'G'を見つけたらtrueに変える
    rep(i, tate){
        rep(j, yoko){
            if(environment[i][j] == 'G' && set_goal == false){
                goal1 = Place(i, j);
                set_goal = true;
            }
            else if(environment[i][j] == 'G' && set_goal == true){
                goal2 = Place(i, j);
                goal = Robot(goal1, goal2);
            }
        }
    }

    cout << endl;
    cout << "----- LRTA*探索開始 -----" << endl; 
    cout << endl;

    LRTA_star(start, goal, environment);
}
