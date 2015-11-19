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
#define SIZE 50

// struct 裡面存有 user 輸入的整數、二進位字串、1的個數
struct data
{
    int u_in;
    char bin[SIZE];
    int quan;
};

int main()
{
    key_t key = 1235; // shm key
    int shm_id = 0;
    int shm_flag;
    int* flag_ptr;
    /*
    shm_flag_buff 旗標控制暫存，程式自己暫存share memory內的flag用
    shm_flag_buff[0]:process flag : 0 server_input process, 1 client process, 2 server_print process
    */
    int shm_flag_buff[1]={0};

    struct data *shm_addr; // pointer to shm
    pid_t pid;
    char shm_strid[10];
    
    struct data usr_input[100];

    // share memory建立與控制
    shm_id = shm_open("text_buff", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    shm_flag = shm_open("flag_buff", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    // 要求配置空間的大小(單位byte)
    ftruncate(shm_id, sizeof(struct data));
    ftruncate(shm_flag, 8);

    // 取得配置到的記憶體位置
    shm_addr = (struct data * )mmap(NULL, sizeof(struct data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
    flag_ptr = (int* )mmap(NULL, 8, PROT_READ | PROT_WRITE, MAP_SHARED, shm_flag, 0);

    if(shm_id < 0 || shm_flag < 0){
        perror("shmget error");
        return 0;
    }

    if(shm_addr < 0){
        perror("shmat error");
        return 0;
    }

    // 將分配到的記憶體空間重置
    int q;
    for (q = 0; q < 2; ++q){
        shm_flag_buff[q]=0;
        flag_ptr[q]=0;
    }

    int i = 0;
    int control = 1; // 1:user 要輸入值、2: pB 有更新 shm、3: 輸入為-1 or -2
    memset(shm_addr,1,1);
    char str_control[1];

    // fork another process
    pid = fork();

    if(pid != 0){ // In parent process, pid > 0
        int q;
        for (q = 0; q < 2; q++) {
            shm_flag_buff[q] = flag_ptr[q];
        }
        while(shm_flag_buff[0] == 0){ // flag = 0 時只能執行 input_parent
            printf("Enter a number: ");
            scanf("%d",&usr_input->u_in);
            char pt[10];
            if(usr_input->u_in == -1 || usr_input->u_in == -2){
                memset(shm_addr,3,1);
                control = 3;
            }
            sprintf(pt,"%d",usr_input->u_in); // 把輸入的整數轉成字串，用以更新 shm
            memcpy(shm_addr+1,pt,sizeof(usr_input->u_in));
                     
            shm_flag_buff[0] = 1; // go to child process
            int q;
            for(q=0;q<2;q++){ // update the flag to shm
                flag_ptr[q] = shm_flag_buff[q];
            }
        }
        while(shm_flag_buff[0] == 2){ // flag = 2 時只能執行 print_parent
            memcpy(str_control,shm_addr,4); // 把 control 拿出來，確認要 print 的東西
            control = atoi(str_control);
            if(control == 2){ // normal input 的 print
                char change[10];
                memcpy(change,shm_addr+1,sizeof(shm_addr));
                printf("%d:%s;\n", usr_input->u_in,change);
                memset(shm_addr,1,1);
            }else{ // print -1 or -2 的結果
                struct data last[sizeof(shm_addr)];
                memcpy(last,shm_addr+1,sizeof(shm_addr));
                int j;
                for(j=0;j<sizeof(last);j++){
                    printf("%d:%s; ",last[j].u_in,last[j].bin);
                }
                printf("\n");
            }
            shm_flag_buff[0] = 0; // back to parent_input process
            int q;
            for(q=0;q<2;q++){ // update the flag to shm
                flag_ptr[q] = shm_flag_buff[q];
            }
            
        }
        
        shmdt(shm_addr);
        shmctl(shm_id, IPC_RMID, NULL);
        
        return 0;
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
}