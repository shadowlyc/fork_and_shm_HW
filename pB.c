#include <sys/ipc.h> // 行程通訊
#include <sys/shm.h> 
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#define SIZE 4096

struct data   // 與 shm 共用的 struct
{
    int u_in;
    char bin[SIZE];
    char allstr[SIZE];
    int quan;
    int flag;
};

struct pBonly{ // 從 shm 拿出來的東西，供運算用
    int num;
    char oz[SIZE];
    int many;
    int Bflag;
};

// 找最多1
int findMax(struct pBonly *_list, int bmax, int list_count){
    int j;
    for(j=1;j<list_count;j++){
        if(_list[j].many > bmax){
            bmax = _list[j].many;
        }
    }
    return bmax;
}

// 找最少1
int findMin(struct pBonly *_list, int bmin, int list_count){
    int j;
    for(j=1;j<list_count;j++){
        if(_list[j].many < bmin){
            bmin = _list[j].many;
        }
    }
    return bmin;
}

// 組合答案
char *match(int pn, char *bn){
    char *sn;
    sprintf(sn,"%d",pn);
    int ll = strlen(sn) + strlen(bn);
    char *result = (char *)malloc(sizeof(char) * ll);
    strcpy(result, sn);
    strcat(result, ":");
    strcat(result, bn);
    strcat(result, "; ");

    return result;
}

int main(int argc, char *argv[])
{
    int shm_id;
    struct data *shm_addr; // pointer to shm
    int t, turn, count, i;
    char d2b[SIZE]; // Dec2Bin 暫存
    int struct_count = 0;
    int execute_key = 1; // 判斷如果 pB 收到的字是 -3 就設回 0
    struct pBonly cal[50];

    shm_id = shm_open("text_buff", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    shm_addr = (struct data * )mmap(NULL, sizeof(struct data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);

    if(shm_id < 0){
        perror("shmget error");
        return 0;
    }

    if(shm_addr == (struct data *)-1){
        perror("shmat error");
        return 0;
    }


    while(execute_key){
        if(shm_addr->flag == 1){
            t = shm_addr->u_in; // 從 shm 拿出 parent 存的資料
            cal[struct_count].num = shm_addr->u_in;
            count = 0; // 每次都要重置 count
            i = 0;
            // 十轉二
            while(t > 0){
                turn = t%2;
                if(turn == 1){
                    d2b[i] = '1';
                    count++;
                }else{
                    d2b[i] = '0';
                }
                i++;
                t = t/2;
            } // while 迴圈結束之後，count 裡會存著該整數的'1'個數，d2b 會存反過來的二進位字串
            cal[struct_count].many = count; // 存 1 的個數

            int j;
            for(j=i-1;j>=0;j--){ // 把二進位轉過來印
                cal[struct_count].oz[i-j-1] = d2b[j];
                shm_addr->bin[i-j-1] = d2b[j];
            }
            struct_count++;
            shm_addr->flag = 2; // 運算完畢，flag 設為 2 請 pA 印出字串
        }
        else if(shm_addr->flag == 3){ // 當輸入為 -1
            int max = cal[0].many;
            int i;
            char *result;
            max = findMax(cal,max,struct_count);
            for(i=0;i<struct_count;i++){
                if(cal[i].many == max){
                    result = match(cal[i].num, cal[i].oz);
                    strcat(shm_addr->allstr,result);
                }
            }
            shm_addr->flag = 6; // 運算完畢，flag 設為 6 請 pA 印出多筆組合
        }
        else if(shm_addr->flag == 4){ // 當輸入為 -2
            int min = cal[0].many;
            int i;
            char *result;
            min = findMin(cal,min,struct_count);
            for(i=0;i<struct_count;i++){
                if(cal[i].many == min){
                    result = match(cal[i].num, cal[i].oz);
                    strcat(shm_addr->allstr,result);
                }
            }
            shm_addr->flag = 6; // 運算完畢，flag 設為 6 請 pA 印出多筆組合
        }
        else if(shm_addr->flag == 5){ // 輸入為 -3
            execute_key = 0;
        }
    }
}

