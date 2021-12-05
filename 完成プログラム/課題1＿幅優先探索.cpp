
//このプログラムでは座標を(縦方向, 横方向), [縦方向][横方向]で書きます

#include <iostream>
#include <vector>
#include <queue>
#define ll long long
#define rep(i, n) for (ll i = 0; i < (ll)(n); i++)
using namespace std;

ll vec[][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

bool record_mode = true;   //trueにすると結果が表示できる代わりに時間がかかります

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
    
    //deepcopy、参照配列などの面倒な問題を回避するため
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

    //コンストラクタ
    State(Array_2d<ll> a, ll b){
        arr = a.copy();
        depth = b;
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


//見たことがある盤面ならtrue, 見たことがない盤面ならfalse、同じ盤面を複数回見ないようにするため
bool judge_seen(Array_2d<ll> arr, vector<Array_2d<ll> > seen_vec){
    rep(i, seen_vec.size()){
        Array_2d<ll> x = seen_vec[i];
        if(compare(arr, x) == true) return true;
    }

    return false;
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

    //結果を出力
    if(record_mode == true){
        rep(i, state.trans_vec.size()){
            cout << "depth = " << i << endl;
            state.trans_vec[i].print();
            cout << endl;
        }
    }
    else{
        cout << "recode_modeがfalseなので結果を出力できません" << endl;
        cout << endl;
    }

    cout << "----- 終了 -----" << endl;
    cout << endl;
}


//幅優先探索
void bfs(Array_2d<ll> first_arr, Array_2d<ll> goal){
    queue<State> que;   //幅優先探索用のqueue

    //初期状態作成
    State first_state(first_arr, 0);
    vector<Array_2d<ll> > empty_vec;   //今までの経路の記録、最初なので当然空
    first_state.make_trans_vec(empty_vec, first_arr);

    que.push(first_state);   //初期状態をqueueにセット
    vector<Array_2d<ll> > seen_vec;   //一度見た盤面を記録
    seen_vec.push_back(first_arr);
    ll tate = first_arr.tate;   //盤面の縦の長さ
    ll yoko = first_arr.yoko;   //盤面の横の長さ

    ll count = 1;   //出力用

    while(que.empty() == false){
        State state = que.front();   que.pop();
        Array_2d<ll> arr = state.arr;   //現在の盤面

        ll depth = state.depth;
        Place empty_place = find_empty(arr);   //空白の座標をget
        ll x = empty_place.x;
        ll y = empty_place.y;

        //経過を出力
        cout << "depth = " << depth << " count = " << count << endl;
        cout << "--- 探索中の盤面 ---" << endl;
        arr.print();
        cout << endl;
        count++;

        //探索状態がゴールだった時の処理
        if(compare(arr, goal) == true){   
            print_when_goal(state, goal);   //結果を出力
            return;   //この時点で探索を終了
        }

        //次の手を探索していく
        rep(k, 4){   //空白の上下左右のマスを見る、見る順番は下->右->上->左
            ll dx = x + vec[k][0];
            ll dy = y + vec[k][1];
            if(judge_inside(dy, dx, tate, yoko) == false) continue;   //(dy, dx)が盤面外なら処理を終了

            Array_2d<ll> next_arr = arr.copy();
            swap(next_arr[y][x], next_arr[dy][dx]);   //空白と(dy, dx)を入れ替える

            if(judge_seen(next_arr, seen_vec) == true) continue;   //見たことがある盤面ならその時点で処理を終了
            else{ 
                seen_vec.push_back(next_arr);   //見たことがない盤面ならseen_vecに追加
                State next_state(next_arr, depth + 1);
                if(record_mode == true) next_state.make_trans_vec(state.trans_vec, next_arr);
                que.push(next_state);   //queueに次の状態を追加
            }
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
    cout << "----- 幅優先探索開始 -----" << endl;
    cout << endl;

    bfs(first_arr, goal_arr);
}