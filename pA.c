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

// struct 裡面存有 user 輸入的整數、二進位字串、flag
struct data
{
    int u_in;
    char bin[SIZE];
    char allstr[SIZE]; // 專門給 -1,-2 印多筆資料用
    int flag;
    /* flag 說明如下
    1: 有使用者輸入數值
    2: 需要印出單一字串
    3: 輸入 -1 尋找多筆組合
    4: 輸入 -2 尋找多筆組合
    5: 輸入 -3 結束程式
    6: 需要印出多筆字串
    0: flag 原始狀態，為 0 時才可輸入數值
    */
};

int main()
{
    int shm_id = 0;

    struct data *shm_addr; // pointer to shm
    pid_t pid;
    int execute_key = 1; // 判斷程式何時要結束，本程式設定為輸入 -3 時結束（此時 key 為 0）

    // share memory建立與控制
    shm_id = shm_open("text_buff", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    // 要求配置空間的大小(單位byte)
    ftruncate(shm_id, sizeof(struct data));

    // 取得配置到的記憶體位置
    shm_addr = (struct data * )mmap(NULL, sizeof(struct data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);

    if(shm_id < 0){
        perror("shmget error");
        return 0;
    }

    if(shm_addr < 0){
        perror("shmat error");
        return 0;
    }

    int i;
    for (i = 0;i < sizeof(struct data);i++) {
        shm_addr->bin[i] = '\0';
        shm_addr->allstr[i] = '\0';
        shm_addr->u_in = 0;
    }

    // fork another process
    pid = fork();

    if(pid > 0){ // In parent process, pid > 0
        
        printf("程式使用說明：\n--------------\n");
        printf("1. 輸入正整數時會印出該整數與其二進位字串\n");
        printf("2. 輸入 -1 會印出二進位字串中，'1'的數量最多的數字\n");
        printf("3. 輸入 -2 會印出二進位字串中，'1'的數量最少的數字\n");
        printf("4. 輸入 -3 則代表程式結束。\n\n稍後即可開始輸入\n\n");
        sleep(3);
        shm_addr->flag = 0;
        while(execute_key){

            if(shm_addr->flag == 0){ // 只有 flag 為 0 時，使用者才能輸入數值
                printf("Enter a number: ");
                scanf("%d",&shm_addr->u_in);
                if(shm_addr->u_in == -1){
                    shm_addr->flag = 3; // flag 設為 3 通知 pB 要找相對應的條件組合
                }
                else if(shm_addr->u_in == -2){
                    shm_addr->flag = 4; // flag 設為 4 通知 pB 要找相對應的條件組合
                }
                else if(shm_addr->u_in == -3){
                    shm_addr->flag = 5; // flag 設為 5 通知兩個程式準備結束
                }
                else{
                    shm_addr->flag = 1; // flag 設為 1 通知 pB 有數值需要運作
                }
            }
            else if(shm_addr->flag == 2){
                printf("%d:%s;\n", shm_addr->u_in, shm_addr->bin);
                shm_addr->flag = 0;
            }
            else if(shm_addr->flag == 6){ // 印出多筆組合
                printf("%s\n",shm_addr->allstr);
                shm_addr->flag = 0;
            }
            else if(shm_addr->flag == 5){ // 輸入為 -3，程式結束
                execute_key = 0;
                printf("程式即將結束\n");
            }
        }
        
        
    }

    else if(pid == 0){ // In child process
        execlp("./pB", NULL, (char *)0);
        shmdt(shm_addr);
        exit(-1);
    }

    else{ /* error occurred */
        fprintf(stderr, "Fork Failed");
        exit(-1);
    }
    shmdt(shm_addr);
    shmctl(shm_id, IPC_RMID, NULL);
        
    return 0;
}