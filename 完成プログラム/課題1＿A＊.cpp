
//このプログラムでは座標を(縦方向, 横方向), [縦方向][横方向]で書きます
//内部では、空白マスを0として扱っています

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

    Place(ll a, ll b){   //縦->横の順番でいれる
        y = a;
        x = b;
    }
};


template<class T>

//二次元の盤面、c++だと通常の二次元配列を引数や返り値にするのが面倒なため
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


class State{
public : 
    Array_2d<ll> arr;   //現在の盤面
    ll depth;   //何回操作したか 0-index
    vector<Array_2d<ll> > trans_vec;   //今までの盤面の履歴
    ll score;   //ヒューリスティック関数の値

    //コンストラクタ
    State(Array_2d<ll> a, ll b){
        arr = a.copy();
        depth = b;
    }

    //operator, sort用
    bool operator>(const State& state) const {
        if(score != state.score) return score > state.score;   //scoreが高い順に
        else{
            if(rand() % 2 == 0) return true;   //scoreが同じならどちらでもいいので、乱数を使う
            else return false;
        }
    }

    //盤面のヒューリスティック関数
    //(depth - 1)は操作回数
    void calc_score(Array_2d<ll> goal){
        ll distance = 0;   //goalと場所が違う数
        ll tate = arr.tate;
        ll yoko = arr.yoko;

        //distanceを計算
        rep(i, tate){
            rep(j, yoko){
                if(goal[i][j] != 0 && arr[i][j] != goal[i][j]) distance++;
            }
        }

        score = depth + distance;   //A*のヒューリスティック関数   
    }

    //trans_vecに履歴を入れる
    void make_trans_vec(vector<Array_2d<ll> > vec, Array_2d<ll> a){
        rep(i, vec.size()) trans_vec.push_back(vec[i]);
        trans_vec.push_back(a);
    }

    State copy(){
        State state(arr.copy(), depth);
        return state;
    }
};


//二つの盤面が同一かどうかを判断する
bool compare(Array_2d<ll> a, Array_2d<ll> b){
    ll tate = a.tate;
    ll yoko = a.yoko;
    bool ret = true;
    rep(i, tate){
        rep(j, yoko){
            if(a[i][j] != b[i][j]) ret = false;
        }
    }

    return ret;
}


//(a, b)が盤面の中かどうかを判断
bool judge_inside(ll a, ll b, ll tate, ll yoko){
    if(a < 0 || a >= tate) return false;
    if(b < 0 || b >= yoko) return false;
    return true;
}


//arrのどこが空白マスかを見つける、プログラムの使用上、空白マスは0として扱っているため
Place find_empty(Array_2d<ll> arr){
    ll tate = arr.tate;
    ll yoko = arr.yoko;
    rep(i, tate){
        rep(j, yoko){
            if(arr[i][j] == 0){
                Place place(i, j);
                return place;
            }
        }
    }

    Place place(tate, yoko);
    return place;   //基本的に使わない
}


//ゴールした時の処理
void print_when_goal(State state, Array_2d<ll> goal){
    cout << endl;
    cout << "----- 結果を出力 -----" << endl;
    cout << endl;

    rep(i, state.trans_vec.size()){
        cout << "depth = " << i << endl;
        state.trans_vec[i].print();
        cout << endl;
    }

    cout << "----- 終了 -----" << endl;
    cout << endl;
}


//A*探索
void A_star(Array_2d<ll> first_arr, Array_2d<ll> goal){
    priority_queue<State, vector<State>, greater<State> > pq;   //priority_queueを使って、ヒューリスティック関数の値が小さい順に探索する

    //初期状態作成
    State first_state(first_arr, 0);
    vector<Array_2d<ll> > empty_vec;   //今までの経路の記録、最初なので当然空
    first_state.make_trans_vec(empty_vec, first_arr);
    first_state.calc_score(goal);

    pq.push(first_state);   //初期状態
    ll tate = first_arr.tate;   //盤面の縦の長さ
    ll yoko = first_arr.yoko;   //盤面の横の長さ

    ll count = 1;   //出力用、探索回数の記録

    while(pq.empty() == false){
        State state = pq.top();   pq.pop();
        Array_2d<ll> arr = state.arr;   //現在の盤面

        ll depth = state.depth;
        Place empty_place = find_empty(arr);   //空白の座標をget
        ll x = empty_place.x;
        ll y = empty_place.y;

        //経過を出力
        cout << "depth = " << depth << " count = " << count << endl;
        cout << "--- 探索中の盤面 ---" << endl;
        cout << "score : " << state.score << endl;
        arr.print();
        cout << endl;
        count++;

        //ゴールした時の処理
        if(compare(arr, goal) == true){
            print_when_goal(state, goal);   //結果を出力
            return;   //探索を終了
        }

        //次の手を探索
        rep(k, 4){   //空白の上下左右のマスを見る、見る順番は下->右->上->左
            ll dx = x + vec[k][0];
            ll dy = y + vec[k][1];
            if(judge_inside(dy, dx, tate, yoko) == false) continue;   //(dy, dx)が盤面外なら処理を終了

            Array_2d<ll> next_arr = arr.copy();
            swap(next_arr[y][x], next_arr[dy][dx]);   //空白と(dy, dx)を入れ替える

            //次の状況をpriority_queueに入れる
            State next_state(next_arr, depth + 1);
            next_state.calc_score(goal);
            next_state.make_trans_vec(state.trans_vec, next_arr);
            pq.push(next_state);
        }
    }
}


int main(){
    ll tate, yoko;
    cin >> tate >> yoko;

    Array_2d<ll> first_arr(tate, yoko);
    rep(i, tate){
        rep(j, yoko){
            cin >> first_arr[i][j];
        }
    }

    Array_2d<ll> goal_arr(tate, yoko);
    rep(i, tate){
        rep(j, yoko){
            cin >> goal_arr[i][j];
        }
    }

    cout << endl;
    cout << "----- A*探索開始 -----" << endl;
    cout << endl;

    A_star(first_arr, goal_arr);
}