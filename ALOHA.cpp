//============================================================================
// Name        : ALOHASimu.cpp
// Author      : tt
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cmath>
#include <random>
#include <cstdlib>

#define MAXTIME 100000	//スロットの数
using namespace std;

double LAMBDA = 0.03;	//通常時のラムダ
double DELAYLAMBDA = 0.1;	//遅延再送時のラムダ
int N = 100;	//放送局数
int MAXSTDRAND = 0;	//乱数の最大値

class Broadcaster{
public:
    int duration;		//遅延時間(スロット単位なので整数値で良い)
    int fireTime;		//発火までの残り時間(スロット単位なので整数値で良い)
    bool isDelaying;	//現在、遅延状態か
    
    Broadcaster(){
        duration = 0;
        fireTime = 0;
        isDelaying = false;
    }
    
    //インターバル計算
    double interval(double random, double lambda){
        
        //printf("RAND:%lf  \n", random);
        return( ( -1.0 / lambda) * log(random) ) +1.0;
    }
    
    //通常時のインターバル計算
    void setInterval(double random){
        fireTime = (int) interval(random, LAMBDA);
        isDelaying = false;
    }
    
    //遅延時のインターバル計算
    void setDelayInterval(double random){
        fireTime = (int) interval(random, DELAYLAMBDA);
        isDelaying = true;
    }
    
    //発火するか？
    bool isFiring(){
        return fireTime == 0;
    }
    
    //時間を進ませる
    void advancingTime(){
        if(fireTime>=1) fireTime--;
        //遅延状態かどうかで遅延時間をかえる
        if(isDelaying) duration ++;
        else duration = 0;
    }
};

void simu(){
    // 乱数シード生成のため
    std::random_device rndmm;
    // メルセンヌ・ツイスタ
    std::mt19937 mt(rndmm());
    std::uniform_real_distribution<double> rnd(0,1); //0~1の実数の乱数
    Broadcaster broadcaster[N];
    int ackSlot=0;
    int duration = 0;
    
    //最初の一回！
    for(int time=0; time<1; time++){
        //全放送局を走査
        for(int index_broadcast=0; index_broadcast<N; index_broadcast++){
            broadcaster[index_broadcast].setInterval(rnd(mt));	//通常インターバル設定
        }
    }
    
    //時間を進める
    for(int slot=1; slot<MAXTIME; slot++){
        int fired = 0;	//このスロットで送信した数
        int firstFiredBroadcaster = -1;//最初に送った局
        int firstFiredBroadcasterDuration = 0;	//最初に送った局の遅延時間
        //全放送局を走査
        for(int index_broadcast=0; index_broadcast<N; index_broadcast++){
            //各局の時間をすすめる
            broadcaster[index_broadcast].advancingTime();
            
            //発火するスロットだったならば
            if(broadcaster[index_broadcast].isFiring()){
                //初めに発火した局のインデックスや遅延時間をとっておく
                if(fired == 0){
                    firstFiredBroadcaster = index_broadcast;
                    firstFiredBroadcasterDuration = broadcaster[index_broadcast].duration;
                    broadcaster[index_broadcast].setInterval(rnd(mt));	//通常インターバル設定
                }
                //コリジョンが起きてる！
                if(fired == 1){
                    //初めに発火したやつと今発火したやつを遅延再送
                    broadcaster[firstFiredBroadcaster].setDelayInterval(rnd(mt));
                    broadcaster[index_broadcast].setDelayInterval(rnd(mt));
                }
                else if(fired >= 2){
                    //遅延再送
                    broadcaster[index_broadcast].setDelayInterval(rnd(mt));
                }
                fired++;
            }
        }
        //衝突がなくて送信が1回だけありゃこのスロットはOK。
        if(fired == 1){
            ackSlot++;
            duration += firstFiredBroadcasterDuration;
        }
    }
    
    printf("%d %lf %lf %lf %lf %d\n", N,N*LAMBDA ,LAMBDA, DELAYLAMBDA, (double)ackSlot/MAXTIME, duration);
    return;
}

int main(int argc,char *argv[]) {
    double MAXLAMBDA = 0.03 * N;
    if(argc == 1)N = 100;
    else if(argc >= 2){
        N = atoi(argv[1]);
    }

    //ラムダを変えてシミュレーション
    //for(LAMBDA = 0.0005; N*LAMBDA<=MAXLAMBDA; LAMBDA += 0.0005){
    for(LAMBDA = 0.0005; N*LAMBDA<=MAXLAMBDA; LAMBDA += 0.00005){
        if(argc == 3){
            DELAYLAMBDA = atof(argv[2]); 
        }
        else DELAYLAMBDA = LAMBDA;
        simu();
    }
}
