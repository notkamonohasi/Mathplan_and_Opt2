
//このプログラムでは座標を(縦方向, 横方向), [縦方向][横方向]で書きます、基本的に全て縦->横の順番です
//自作のclassの最初の文字は大文字で、それ以外で大文字を使うことはないです

#include <iostream>
#include <vector>
#include <queue>
#define ll long long
#define rep(i, n) for (ll i = 0; i < (ll)(n); i++)
using namespace std;

ll vec[][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

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
        robo1 = p1;   
        robo2 = p2;

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

    void print(){
        cout << robo1.x << " " << robo1.y << ", " << robo2.x << " " << robo2.y << endl;
    }
};


//探索で使う、メインclass
class State{
public :
    Robot robot;   //現在位置
    ll depth;   //探索の深さ
    vector<Robot>   record_vec;   //今までのロボットの場所の推移を記録する
    ll score;   //ヒューリスティック関数の値

    //コンストラクタ
    State(Place p1, Place p2, ll d){
        Robot r(p1, p2);
        robot = r;
        depth = d;
    }
    State(Robot r, ll d){
        robot = r;
        depth = d;
    }

    //operator, sort用
    bool operator>(const State& state) const {
        if(score != state.score) return score > state.score;   //scoreが高い順に
        else{
            if(rand() % 2 == 0) return true;   //scoreが同じならどちらでもいいので、乱数を使う
            else return false;
        }
    }


    //今までの経路を記録
    void make_record_vec(vector<Robot> vec, Robot place){
        rep(i, vec.size()) record_vec.push_back(vec[i]);
        record_vec.push_back(place);
    }


    //scoreは移動回数+マンハッタン距離で計算
    //マンハッタン距離はロボットが占有する二つの場所の平均とゴールが占有する二つの場所の平均を使って計算
    //平均を計算する時に÷2をすると小数点となる可能性があるので、全体を二倍にして整数で処理する
    //(depth - 1) は移動回数
    void calc_score(Robot goal){
        Place robo1 = robot.robo1;
        Place robo2 = robot.robo2;
        Place goal1 = goal.robo1;
        Place goal2 = goal.robo2;
        score = (depth - 1) * 2 + abs((robo1.x + robo2.x - goal1.x - goal2.x)) + 
                    abs(robo1.y + robo2.y - goal1.y - goal2.y);
    }
};


// robotの場所について、今までに見たことがあるならtrue、見たことがないならfalse
bool judge_seen(Robot robot, vector<Robot> seen_vec){
    rep(i, seen_vec.size()){
        if(robot == seen_vec[i]) return true;
    }
    return false;
}


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


//ゴールした時の処理で、結果を出力する
void print_when_goal(State state, Robot start, Robot goal, Array_2d<char> environment){
    vector<Robot> record_vec = state.record_vec;
    cout << endl;   cout << endl;
    cout << "----- 結果を出力 -----" << endl;   cout << endl;
    rep(i, record_vec.size()){
        cout << "depth = " << i << endl;
        print_field(record_vec[i], start, goal, environment);
        cout << endl;
    }

    cout << endl;
    cout << "----- 終了 -----" << endl;
    cout << endl;
}


//class名の関係でわかりにくいが、例えば、(0,1)に対して、それに直交する単位ベクトル{(1,0), (-1, 0)}を返す
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


//A*探索
void A_star(Robot start, Robot goal, Array_2d<char> environment){
    priority_queue<State, vector<State>, greater<State> > pq;   //A*探索用のpriority_queue

    //fisrt_state作成
    State first_state(start, 0);
    vector<Robot> empty_vec;
    first_state.make_record_vec(empty_vec, start);
    first_state.calc_score(goal);

    pq.push(first_state);   //初期状態をqueueにセット
    vector<Robot> seen_vec;   //見たことがあるRobotを記録
    seen_vec.push_back(start);   //一番最初のRobotも忘れずに追加しておく

    ll count = 0;   //出力用、何回探索を行ったか

    while(pq.empty() == false){
        State state = pq.top();   pq.pop();
        Robot robot = state.robot;
        Place robo1 = robot.robo1;
        Place robo2 = robot.robo2;
        ll depth = state.depth;
        
        //出力
        count++;
        cout << "depth = " << depth << ", count = " << count << endl;
        cout << "--- 探索中の盤面 --- " << endl; 
        cout << "score : " << state.score << endl;
        print_field(robot, start, goal, environment);
        cout << endl;

        //Robotがゴール状態だった時の処理
        if(robot == goal){
            print_when_goal(state, start, goal, environment);   //結果を出力
            return;   //探索を終了
        }

        //並進
        rep(k, 4){
            Place next_robo1(robo1.y + vec[k][0], robo1.x + vec[k][1]);   //vecは最初の方に定義されている、隣接位置への単位ベクトル
            Place next_robo2(robo2.y + vec[k][0], robo2.x + vec[k][1]);
            Robot next_robot(next_robo1, next_robo2);
            if(judge_env(next_robot, environment) == false) continue;   //ルール違反なら終了

            if(judge_seen(next_robot, seen_vec) == true) continue;   //見たことがあるなら終了 
            else{
                seen_vec.push_back(next_robot);   //見たことがないなら見たことがあるリストに追加
                State next_state(next_robot, depth + 1);
                next_state.make_record_vec(state.record_vec, next_robot);
                next_state.calc_score(goal);
                pq.push(next_state);   
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

                if(judge_seen(next_robot, seen_vec) == true) continue;   //見たことがあるなら終了 
                else{
                    seen_vec.push_back(next_robot);   //見たことがないなら見たことがあるリストに追加
                    State next_state(next_robot, depth + 1);
                    next_state.make_record_vec(state.record_vec, next_robot);
                    next_state.calc_score(goal);
                    pq.push(next_state);   
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

                if(judge_seen(next_robot, seen_vec) == true) continue;   //見たことがあるなら終了 
                else{
                    seen_vec.push_back(next_robot);   //見たことがないなら見たことがあるリストに追加
                    State next_state(next_robot, depth + 1);
                    next_state.make_record_vec(state.record_vec, next_robot);
                    next_state.calc_score(goal);
                    pq.push(next_state);   
                }
            }
        }
    }
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
    cout << "----- A*探索開始 -----" << endl; 
    cout << endl;

    A_star(start, goal, environment);
}