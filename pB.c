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

struct data
{
    int u_in;
    char bin[SIZE];
    int quan;
};

// 找最多1
int findMax(struct data *_list, int bmax, int list_count){
    int j;
    for(j=1;j<list_count;j++){
        if(_list[j].quan > bmax){
            bmax = _list[j].quan;
        }
    }
    return bmax;
}

// 找最少1
int findMin(struct data *_list, int bmin, int list_count){
    int j;
    for(j=1;j<list_count;j++){
        if(_list[j].quan < bmin){
            bmin = _list[j].quan;
        }
    }
    return bmin;
}

// 找所有符合最多or少1的
struct data * findMatch(struct data *_list, int num, int list_count){
    struct data last[10];
    int k=0, i;
    for(i=0;i<list_count;i++){
        if(_list[i].quan == num){
            last[k] = _list[i];
            // memcpy(last[k],_list[i],sizeof(_list[i]));
            k++;
        }
    }
    return last;
}

int main(int argc, char *argv[])
{
    key_t key; // shm key
    int shm_id = 0;
    int shm_flag;
    int* flag_ptr;
    struct data *shm_addr; // pointer to shm
    int t, turn, count, i;
    char d2b[SIZE]; // Dec2Bin 暫存
    int struct_count = 0;
    int shm_flag_buff[1]={0};

    struct data usr_input[50];

    shm_id = shm_open("text_buff", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    shm_flag = shm_open("flag_buff", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    shm_addr = (struct data * )mmap(NULL, sizeof(struct data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
    flag_ptr = (int* )mmap(NULL, 8, PROT_READ | PROT_WRITE, MAP_SHARED, shm_flag, 0);

    if(shm_id < 0 || shm_flag < 0){
        perror("shmget error");
        return 0;
    }

    if(shm_addr == (struct data *)-1){
        perror("shmat error");
        return 0;
    }
    char buffer[(SIZE-1)];
    int control = 1;
    // 1:user 要輸入值、2: pB 有更新 shm、3: 輸入為-1 or -2，程式結束

    int q;
    for (q = 0; q < 2; q++) {
        shm_flag_buff[q] = flag_ptr[q];
    }
    while(shm_flag_buff[0] == 1){ // flag = 1 時只能執行 child
        memcpy(buffer,shm_addr,sizeof(shm_addr)); // 從 shm 拿出 parent 存的資料
        control = atoi(&buffer[0]);
        t = atoi(&buffer[1]); // t 為 user 所輸入的整數
        if(control == 1){ // user 輸入非 -1,-2 的值
            count = 0;
            i = 0;
            usr_input[struct_count].u_in = t;
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
            }
            usr_input[struct_count].quan = count; // 存 1 的個數
            int j;
            for(j=i-1;j>=0;j--){ // 把二進位轉過來印
                usr_input[struct_count].bin[i-j-1] = d2b[j];
            }

            memset(shm_addr,2,1); // 改變control
            memcpy(shm_addr+1,usr_input[struct_count].bin,sizeof(usr_input[struct_count].bin)); // 更新 shm，把二進位字串存進去給 A print
            struct_count++;
        }
        else{ // 當輸入為 -1 or -2
            struct data *final_ans; // 用來存所有符合要搜尋的條件
            if(t == -1){
                int max = usr_input[0].quan;
                max = findMax(usr_input,max,struct_count);
                final_ans = findMatch(usr_input,max,struct_count);
            }else{
                int min = usr_input[0].quan;
                min = findMin(usr_input,min,struct_count);
                final_ans = findMatch(usr_input,min,struct_count);
            }
            memcpy(shm_addr+1,final_ans,sizeof(final_ans)); // 把符合的存進 shm 裡
        }
        shm_flag_buff[0] = 2; // go to parent_print process
        int q;
        for(q=0;q<2;q++){ // update the flag to shm
            flag_ptr[q] = shm_flag_buff[q];
        }
    }
}