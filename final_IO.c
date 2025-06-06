#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_PROCESSES 10
#define MAX_QUEUE_SIZE 100
#define MAX_GANTT_SIZE 100

// 프로세스 구조체
typedef struct{
    int pid; //ID
    int arrival_time; //도착시간
    int cpu_burst_time; // CPU BURST
    int io_burst_time; // IO BURST
    int io_request_time; // io request
    int io_total; // 총 IOtime 저장
    int priorty; // 우선순위 (작을수록 높음)
    int remaining_time; // 남은실행시간
    int waiting_time; // 대기시간
    int turnaround_time; // turnaround time

} Process;

// queue 구조체
typedef struct{
    Process* process_queue[MAX_QUEUE_SIZE];//process 포인터 배열 (100개)
    int front;
    int rear;
    int size;
} Queue;

typedef struct{
    int pid;
    int start_time;
    int end_time;
} Gantt;

// 전역변수 설정
Queue ready_queue;
Queue waiting_queue;
Process* processes[MAX_PROCESSES];
int process_count = 0;
Gantt gantt_chart[MAX_GANTT_SIZE];
int gantt_count=0;

void config(){ // 초기설정
    //ready queue 초기화
    ready_queue.front = 0;
    ready_queue.rear = -1; // enqueue가 먼저 수행되는데, 무조건 rear를 1 증가시키고 시작함.. 따라서 -1이어도 첫 rear는 0이 됨
    ready_queue.size = 0;

    //waiting queue 초기화
    waiting_queue.front = 0;
    waiting_queue.rear = -1;
    waiting_queue.size = 0;

    //gantt 초기화
    gantt_count = 0;

    //계속 다른 난수 생성을 위한 처리
    srand(time(NULL));
}

void enqueue(Queue* queue, Process* process){
    queue-> rear = (queue->rear +1) % MAX_QUEUE_SIZE;
    queue->process_queue[queue->rear] = process;
    queue-> size ++;
}

Process* dequeue(Queue* queue){
    if(queue->size >0){
        Process* process = queue->process_queue[queue->front];
        queue->front = (queue->front +1) % MAX_QUEUE_SIZE;
        queue -> size --;
        return process;
    }
    return NULL;
}

void add_gantt(int pid, int start_time, int end_time){
    if(gantt_count <MAX_GANTT_SIZE){
        gantt_chart[gantt_count].pid = pid;
        gantt_chart[gantt_count].start_time = start_time;
        gantt_chart[gantt_count].end_time = end_time;
        gantt_count++;
    }
}

void print_gantt(){
    printf("\ngantt chart\n");
    printf("0");
    for (int i=0 ; i <gantt_count ; i++){
        //맨 처음 실행시간이 0이 아닐때, idle 표시
        if(i==0 && gantt_chart[i].start_time !=0){ 
            printf(" ---idle--- %d",gantt_chart[i].start_time);
            printf (" ---P%d--- %d", gantt_chart[i].pid, gantt_chart[i].end_time);
        }
        // 전 프로세스가 끝난 시간과 후 프로세스가 시작하기전 시간이 다른 경우 : idle 발생
        else if(i !=0 && gantt_chart[i-1].end_time != gantt_chart[i].start_time){ 
            printf(" ---idle--- %d",gantt_chart[i].start_time);
            printf (" ---P%d--- %d", gantt_chart[i].pid, gantt_chart[i].end_time);
        }
        //idle 발생하지 않음
        else{
            printf (" ---P%d--- %d", gantt_chart[i].pid, gantt_chart[i].end_time);
        }
    }
}

void create_process(){
    if (process_count < MAX_PROCESSES) {  // 최대 프로세스 수 체크
        Process* new_process = (Process*)malloc(sizeof(Process)); // 프로세스 동적 할당
        new_process->pid = process_count + 1; //1부터 시작
        new_process->arrival_time = rand()%10 ; // 0-9 까지의 도착시간
        new_process->cpu_burst_time = rand()%10 +1 ; //1-10 사이의 cpu burst time
        new_process->io_burst_time = rand()%5 ; // 0-4사이의 io burst (0이면, 발생하지 않음. -> 랜덤한 발생)
        new_process->io_request_time = rand()%5 +1 ;  // arrival 이후 1-5틱 사이의 io request time
        new_process->priorty = rand()%10 +1; // 1-10 사이의 우선순위값(작을수록 높음)
        new_process->io_total = new_process->io_burst_time; // 최초의 io burst time 저장( waiting time 계산용)
        new_process->remaining_time = new_process->cpu_burst_time; // 아직 실행되지 않았음
        new_process->waiting_time = 0; // 초기값
        new_process->turnaround_time = 0; //초기값
        processes[process_count] = new_process; // Processes에 할당
        process_count++; // process count 증가
        printf("pid\tarrival time\tcpuburst\tio burst\tio request\tpriority\n");
        printf("%d\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n",new_process->pid,new_process->arrival_time, new_process->cpu_burst_time, new_process->io_burst_time,new_process->io_request_time,new_process->priorty);
    }
}

void manage_waiting_queue(int current_time){
    if(waiting_queue.size >0){
        int jobs = waiting_queue.size; // size는 계속 변하므로, 버그를 막기 위함

        // waiting queue의 모든 process에 대해 io burst 수행
        for (int i=0 ; i< jobs; i++){
            Process* process = dequeue(&waiting_queue);
            process->io_burst_time --;

            //I/O가 모두 완료된 경우 -> Ready queue로
            if(process->io_burst_time <=0){
                enqueue(&ready_queue,process);
            }
            //I/O가 완료되지 않은경우 -> 다시 waiting queue로
            else{
                enqueue(&waiting_queue,process);
            }
        }
    }
}

void TAT_WT (Process* current_process, int current_time){
    // (if) IO 실행된 경우 / (else) IO 실행 안된경우
        if(current_process->io_total>0 && current_process->io_burst_time ==0){
            current_process->turnaround_time = current_time - current_process->arrival_time;
            current_process->waiting_time = current_process->turnaround_time - current_process->cpu_burst_time - current_process->io_total;
            // 검토용
                //printf("P%d TAT(%d) = %d - %d\n", current_process->pid, current_process->turnaround_time, current_time, current_process->arrival_time);
                //printf("P%d WT(%d) = %d - %d -%d\n", current_process->pid,current_process->waiting_time, current_process->turnaround_time, current_process->cpu_burst_time, current_process->io_total);
        }
        else{
            current_process->turnaround_time = current_time - current_process->arrival_time;
            current_process->waiting_time = current_process->turnaround_time - current_process->cpu_burst_time;
            // 검토용
                //printf("P%d TAT(%d) = %d - %d\n", current_process->pid, current_process->turnaround_time, current_time, current_process->arrival_time);
                //printf("P%d WT(%d) = %d - %d\n", current_process->pid,current_process->waiting_time, current_process->turnaround_time, current_process->cpu_burst_time);
        }
}

// 1. FCFS 
void schedule_FCFS(){
    printf("FCFS Scheduling\n");
    int current_time = 0; // 시간은 0부터 시작
    int completed = 0; // 끝난 process 개수 체크 
    int is_in_ready[MAX_PROCESSES] ={0}; // 레디큐에 들어간 프로세스를 체크하기 위함 (pid가 인덱스)

    while(completed < process_count){// 모든 프로세스의 실행이 끝날때까지 반복

        //현재 시간까지 도착한 프로세스를 레디큐로 이동 (중복해서 추가하지 않도록 is_in_ready 유지)
        for (int i=0; i< process_count; i++){
            if (!is_in_ready[i] && processes[i]->arrival_time <= current_time){
                enqueue(&ready_queue,processes[i]);
                is_in_ready[i] = 1;
            }
        }
        //1. readyqueue에 process가 있으면 process 실행
        if(ready_queue.size > 0){ 
            Process* current_process = dequeue(&ready_queue);

            //(1) current process 실행, I/O 요청 시간까지 실행됨(request time이 0이 될때까지.)
            int start_time = current_time;
            while(current_process->remaining_time > 0 && current_process->io_request_time > 0){
                current_time++;
                current_process->remaining_time--;
                current_process->io_request_time--;
                manage_waiting_queue(current_time);
            }

            // (2) I/O 실행 -> watingqueue에 enqueue, gantt에 여태까지 실행한 정보 추가 / 다음으로 넘어감
            if(current_process->io_request_time ==0 && current_process->io_burst_time > 0 && current_process->remaining_time > 0 ){
                enqueue(&waiting_queue, current_process);
                add_gantt(current_process->pid, start_time, current_time);
                continue;
            }

            // (3) I/O실행이 완료된 프로세스이면서 remaining time이 존재하는 경우 - 끝까지 실행
            if(current_process->io_burst_time == 0 && current_process->remaining_time > 0){
                while(current_process->remaining_time > 0){
                    current_time++;
                    current_process->remaining_time--;
                    manage_waiting_queue(current_time);
                }
            }

            // (4) 모든게 완료된 프로세스 -> WT, TAT 계산후 저장, 간트차트 추가
            // (if) IO 실행된 경우 / (else) IO 실행 안된경우
            TAT_WT(current_process,current_time);
            add_gantt(current_process->pid, start_time, current_time);
            completed++;
        }
        // 2. ready queue가 비어있음 -> time을 1 늘려봄
        else{
            current_time++;
            manage_waiting_queue(current_time);
        }
    }
    printf("FCFS 실행결과");
    print_gantt();
}


//2. non-preemptive SJF
void schedule_np_SJF(){
    printf("non-preemptive SJF Scheduling\n");
    int current_time = 0;
    int completed = 0;
    int is_in_ready[MAX_PROCESSES] ={0}; // 레디큐에 들어간 프로세스를 체크하기 위함 (pid가 인덱스)

    //모든 프로세스 실행이 완료될때까지 반복실행
    while(completed < process_count){

        //현재 시간까지 도착한 프로세스를 레디큐로 (중복해서 추가하지 않도록.)
        for (int i=0; i< process_count; i++){
            if (!is_in_ready[i] && processes[i]->arrival_time <= current_time){
                enqueue(&ready_queue,processes[i]);
                is_in_ready[i] = 1;
            }
        }

        //1. ready queue가 비어있지 않음 -> 프로세스 실행
        if(ready_queue.size >0){ 
            //SJF : 레디큐에서 bursttime이 가장 작은순으로 정렬(버블정렬, circular queue)
            for(int i=0; i<ready_queue.size-1; i++){
                for (int j=0; j<ready_queue.size-1-i; j++){
                    int idx1 = (ready_queue.front + j) % MAX_QUEUE_SIZE;
                    int idx2 = (ready_queue.front + j + 1) % MAX_QUEUE_SIZE;
                    if(ready_queue.process_queue[idx1]->remaining_time > ready_queue.process_queue[idx2]->remaining_time){
                        Process* temp = ready_queue.process_queue[idx1];
                        ready_queue.process_queue[idx1] = ready_queue.process_queue[idx2];
                        ready_queue.process_queue[idx2] = temp;
                    }
                }
            }

            // 정렬 후 가장 앞쪽(남은 시간이 가장 적은 프로세스) dequeue
            Process* current_process = dequeue(&ready_queue);
            int start_time = current_time;

            //(1) 당장 실행 - I/O 요청 시간까지 실행됨
            while(current_process->remaining_time > 0 && current_process->io_request_time >0){
                current_time ++;
                current_process->remaining_time --;
                current_process->io_request_time --;
                manage_waiting_queue(current_time);
            }

            //(2) I/O 실행 - waiting queue에 enqueue, gantt 추가
            if(current_process->io_request_time==0 && current_process->io_burst_time >0 && current_process->remaining_time > 0){
                enqueue(&waiting_queue,current_process);
                add_gantt(current_process->pid, start_time, current_time);
                continue;
            }

            //(3) I/O실행이 완료된 프로세스이면서 remaining time이 존재하는 경우 - 끝까지 실행
            if(current_process->io_burst_time==0 && current_process->remaining_time >0){
                while(current_process->remaining_time >0){
                    current_time ++;
                    current_process->remaining_time --;
                    manage_waiting_queue(current_time);
                }
            }

            //(4) 모든게 완료된 프로세스 -> WT, TAT 계산후 저장, 간트차트 추가
            // (if) IO 실행된 경우 / (else) IO 실행 안된경우
            TAT_WT(current_process,current_time);
            add_gantt(current_process->pid, start_time, current_time);
            completed++;
        }
        // 2. readyqueue 비어있음
        else{ 
            current_time++;
            manage_waiting_queue(current_time);
        }
    }
    printf("non-preemptive SJF 실행결과\n");
    print_gantt();
}

//3. priority
void schedule_Priority(){
    printf("Priority Scheduling\n");
    int current_time = 0;
    int completed = 0;
    int is_in_ready[MAX_PROCESSES] ={0}; // 레디큐에 들어간 프로세스를 체크하기 위함 (pid가 인덱스)

    while(completed < process_count){

        //현재 시간까지 도착한 프로세스를 레디큐로 ( 중복해서 추가하지 않도록.)
        for (int i=0; i< process_count; i++){
            if (!is_in_ready[i] && processes[i]->arrival_time <= current_time){
                enqueue(&ready_queue,processes[i]);
                is_in_ready[i] = 1;
            }
        }
        //1. ready queue가 비어있지 않음 -> 프로세스 실행
        if(ready_queue.size >0){ 

            //priority : 레디큐에서 priority가 가장 작은순으로 정렬
            for(int i=0; i<ready_queue.size-1; i++){
                for (int j=0; j<ready_queue.size-1-i; j++){
                    int idx1 = (ready_queue.front + j) % MAX_QUEUE_SIZE;
                    int idx2 = (ready_queue.front + j + 1) % MAX_QUEUE_SIZE;
                    if(ready_queue.process_queue[idx1]->priorty > ready_queue.process_queue[idx2]->priorty){
                        Process* temp = ready_queue.process_queue[idx1];
                        ready_queue.process_queue[idx1] = ready_queue.process_queue[idx2];
                        ready_queue.process_queue[idx2] = temp;
                    }
                }
            }

            //priority 가장 높은 것 하나 dequeue
            Process* current_process = dequeue(&ready_queue);
            int start_time = current_time;
            
            // (1) 바로 실행 가능 -> I/O 요청 시간까지 실행됨(request time이 0이 될때까지.)
            while(current_process->remaining_time > 0 && current_process->io_request_time > 0){
                current_time++;
                current_process->remaining_time--;
                current_process->io_request_time--;
                manage_waiting_queue(current_time);
            }

            // (2) I/O 실행 -> watingqueue에 enqueue, gantt에 여태까지 실행한 정보 추가 / 다음으로 넘어감
            if(current_process->io_request_time == 0 && current_process->io_burst_time > 0 && current_process->remaining_time > 0){
                enqueue(&waiting_queue, current_process);
                add_gantt(current_process->pid, start_time, current_time);
                continue;
            }

            // (3) I/O실행이 완료된 프로세스이면서 remaining time이 존재하는 경우
            if(current_process->io_burst_time == 0 && current_process->remaining_time > 0){
                while(current_process->remaining_time > 0){
                    current_time++;
                    current_process->remaining_time--;
                    manage_waiting_queue(current_time);
                }
            }

            //(4) 모든게 완료된 프로세스
            // (if) IO 실행된 경우 / (else) IO 실행 안된경우
            TAT_WT(current_process,current_time);
            add_gantt(current_process->pid, start_time, current_time);
            completed++;

        }
        //2. readyqueue 비어있음
        else{ 
            current_time++;
            manage_waiting_queue(current_time);
            
        }
    }
    printf("Priority 실행결과\n");
    print_gantt();
}

// 4. RoundRobin
void schedule_RR(int time_quantum){
    printf("Round Robin Scheduling\n");
    int current_time = 0;
    int completed = 0;
    int is_in_ready[MAX_PROCESSES] = {0};

    // 현재시간까지 실행되는 프로세스 ready queue에 추가
    while(completed < process_count){
        for(int i=0; i<process_count; i++){
            if(!is_in_ready[i] && processes[i]->arrival_time <= current_time){
                enqueue(&ready_queue, processes[i]);
                is_in_ready[i]=1;
            }
        }

        // 1. ready queue에 프로세스 존재
        if(ready_queue.size >0){
            Process* current_process = dequeue(&ready_queue);
            int execution_time = 0;
            int start_time = current_time;

            // (1) 바로 실행 : I/O 시작 전인 프로세스를, timequantum과 remaining time중 작은 것만큼 실행
            while(execution_time < time_quantum && current_process->remaining_time > 0 && current_process->io_request_time > 0){
                current_time++;
                execution_time++; // timequantum과 같아지면 while문 끝남
                current_process->remaining_time--;
                current_process->io_request_time--;
                // current time 늘릴때마다, ready queue 업데이트 & waiting queue 업데이트
                for(int i=0; i<process_count; i++){
                        if(!is_in_ready[i] && processes[i]->arrival_time <= current_time){
                            enqueue(&ready_queue, processes[i]);
                            is_in_ready[i]=1;
                        }
                }
                manage_waiting_queue(current_time);
            }

            // (2) I/O 요청이 발생한 경우
            if(current_process->io_request_time == 0 && current_process->io_burst_time > 0 && current_process->remaining_time > 0){
                enqueue(&waiting_queue, current_process);
                add_gantt(current_process->pid, start_time, current_time);
                continue;
            }

            // (3) I/O가 완료된 프로세스이면서 remaining time이 존재하는 경우
            if(current_process->io_burst_time == 0 && current_process->remaining_time > 0){
                while(current_process->remaining_time > 0 && execution_time < time_quantum){
                    current_time++;
                    execution_time++;
                    current_process->remaining_time--;
                    for(int i=0; i<process_count; i++){
                        if(!is_in_ready[i] && processes[i]->arrival_time <= current_time){
                            enqueue(&ready_queue, processes[i]);
                            is_in_ready[i]=1;
                        }
                    }
                    manage_waiting_queue(current_time);
                }
            }

            // (4) time quantum이 끝나거나 프로세스가 완료된 경우 : 
            // (if) 아직 실행시간 남았으면 다시 enqueue / (else) 완전히 끝난경우, WT, TAT 계산
            if(current_process->remaining_time > 0){
                enqueue(&ready_queue, current_process);
            }
            else{
                // (if) IO 실행된 경우 / (else) IO 실행 안된경우
                TAT_WT(current_process,current_time);
                completed++;
            }
            add_gantt(current_process->pid, start_time, current_time);
        }
        else{
            current_time++;
            manage_waiting_queue(current_time);
        }
    }
    printf("Round Robin 실행결과\n");
    print_gantt();
}

// 5. preemptive SJF
void schedule_preemptive_SJF(){
    printf("Preemptive SJF Scheduling");
    int current_time = 0;
    int completed = 0;
    int is_in_ready[MAX_PROCESSES] = {0};

    while(completed < process_count){
        // arrival time이 된 것들 Readyqueue로.
        for(int i=0; i<process_count; i++){ 
            if(!is_in_ready[i] && processes[i]->arrival_time <= current_time){
                enqueue(&ready_queue, processes[i]);
                is_in_ready[i]=1;
            }
        }

        //1. ready queue가 비어있지 않음
        if(ready_queue.size >0){ 
            //레디큐에서 remaining time이 가장 작은순으로 정렬
            for(int i=0; i<ready_queue.size-1; i++){
                for (int j=0; j<ready_queue.size-1-i; j++){
                    int idx1 = (ready_queue.front + j) % MAX_QUEUE_SIZE;
                    int idx2 = (ready_queue.front + j + 1) % MAX_QUEUE_SIZE;
                    if(ready_queue.process_queue[idx1]->remaining_time > ready_queue.process_queue[idx2]->remaining_time){
                        Process* temp = ready_queue.process_queue[idx1];
                        ready_queue.process_queue[idx1] = ready_queue.process_queue[idx2];
                        ready_queue.process_queue[idx2] = temp;
                    }
                }
            }
            //(1) 바로 실행 : 제일 짧은 것 실행(1초)
            Process* current_process = dequeue(&ready_queue);
            int start_time = current_time;
            current_time++;
            current_process->remaining_time --;
            current_process->io_request_time--;
            for(int i=0; i<process_count; i++){
                        if(!is_in_ready[i] && processes[i]->arrival_time <= current_time){
                            enqueue(&ready_queue, processes[i]);
                            is_in_ready[i]=1;
                        }
                    }
            manage_waiting_queue(current_time);

            // (2) 방금 실행된 프로세스에서 IO요청이 발생한 경우
            if(current_process->io_request_time ==0 && current_process->remaining_time>0){
                enqueue(&waiting_queue, current_process);
                if(gantt_chart[gantt_count-1].pid == current_process->pid){// 전 틱에서 추가된 간트차트의 pid와 이번 pid가 같으면, endtime만 바꿔줌.
                gantt_chart[gantt_count-1].end_time = current_time;
                }
                else{//아니면 새로 추가.
                    add_gantt(current_process->pid, start_time, current_time);
                }
                continue;   
            }

            // (3) 1틱단위로 실행 후 남은 시간이 있는 프로세스/ 아예 완료된 경우 : 
            // (if) 아직 실행시간 남았으면 다시 enqueue / (else) 완전히 끝난경우, WT, TAT 계산
            if(current_process->remaining_time >0){
                enqueue(&ready_queue, current_process);
            }
            else{
                TAT_WT(current_process,current_time);
                completed++;
                // 검토용
                //printf("P%d TAT(%d) = %d - %d\n", current_process->pid, current_process->turnaround_time, current_time, current_process->arrival_time);
                //printf("P%d WT(%d) = %d - %d -%d\n", current_process->pid,current_process->waiting_time, current_process->turnaround_time, current_process->cpu_burst_time, current_process->io_total);
            }

            //  1틱 단위로 실행됨 -> 같은 프로세스가 연속적으로 실행되면 gantt display 시 연결해줌
            if(gantt_chart[gantt_count-1].pid == current_process->pid){// 전 틱에서 추가된 간트차트의 pid와 이번 pid가 같으면, endtime만 바꿔줌.
                gantt_chart[gantt_count-1].end_time = current_time;
            }
            else{//아니면 새로 추가.
                add_gantt(current_process->pid, start_time, current_time);
            }
        }
        else{
            current_time++;
            manage_waiting_queue(current_time);
        }
           
    }
    printf("Preemptive SJF 실행결과");
    print_gantt();
}

// 6. preemptive Priority
void schedule_preemptive_Priority(){
    printf("Preemptive Priority Scheduling");
    int current_time = 0;
    int completed = 0;
    int is_in_ready[MAX_PROCESSES] = {0};

    while(completed < process_count){
        for(int i=0; i<process_count; i++){ // arrival time이 된 것들 Readyqueue로.
            if(!is_in_ready[i] && processes[i]->arrival_time <= current_time){
                enqueue(&ready_queue, processes[i]);
                is_in_ready[i]=1;
            }
        }
        ///ready queue가 비어있지 않음 -> priority로 정렬
        if(ready_queue.size >0){ 
            //레디큐에서 priority가 가장 작은순으로 정렬
            for(int i=0; i<ready_queue.size-1; i++){
                for (int j=0; j<ready_queue.size-1-i; j++){
                    int idx1 = (ready_queue.front + j) % MAX_QUEUE_SIZE;
                    int idx2 = (ready_queue.front + j + 1) % MAX_QUEUE_SIZE;
                    if(ready_queue.process_queue[idx1]->priorty > ready_queue.process_queue[idx2]->priorty){
                        Process* temp = ready_queue.process_queue[idx1];
                        ready_queue.process_queue[idx1] = ready_queue.process_queue[idx2];
                        ready_queue.process_queue[idx2] = temp;
                    }
                }
            }
            //(1)제일 우선순위 높은 것 실행(1초)
            Process* current_process = dequeue(&ready_queue);
            int start_time = current_time;
            current_time++;
            current_process->remaining_time --;
            current_process->io_request_time--;
            for(int i=0; i<process_count; i++){
                        if(!is_in_ready[i] && processes[i]->arrival_time <= current_time){
                            enqueue(&ready_queue, processes[i]);
                            is_in_ready[i]=1;
                        }
                    }
            manage_waiting_queue(current_time);

            //(2) 방금 실행한 프로세스에서 IO 요청 발생한 경우
            if(current_process->io_request_time ==0 && current_process->remaining_time >0){
                enqueue(&waiting_queue,current_process);

                if(gantt_chart[gantt_count-1].pid == current_process->pid){// 전 틱에서 추가된 간트차트의 pid와 이번 pid가 같으면, endtime만 바꿔줌.
                gantt_chart[gantt_count-1].end_time = current_time;
                }
                else{//아니면 새로 추가.
                    add_gantt(current_process->pid, start_time, current_time);
                }
                continue;
            }

            // (3) 1틱단위로 실행 후 남은 시간이 있는 프로세스/ 아예 완료된 경우 : 
            // (if) 아직 실행시간 남았으면 다시 enqueue / (else) 완전히 끝난경우, WT, TAT 계산
            if(current_process->remaining_time >0){
                enqueue(&ready_queue,current_process);
            }
            else{
                TAT_WT(current_process,current_time);
                completed++;
            }

            //  1틱 단위로 실행됨 -> 같은 프로세스가 연속적으로 실행되면 gantt display 시 연결해줌
            if(gantt_chart[gantt_count-1].pid == current_process->pid){// 전 틱에서 추가된 간트차트의 pid와 이번 pid가 같으면, endtime만 바꿔줌.
                gantt_chart[gantt_count-1].end_time = current_time;
            }
            else{//아니면 새로 추가.
                add_gantt(current_process->pid, start_time, current_time);
            }
        }
        else{
            current_time++;
            manage_waiting_queue(current_time);
        }
           
    }
    printf("Preemptive Priority 실행결과");
    print_gantt();
}

void schedule(){
    int num;
    printf("숫자를 입력해 scheduling algorithm을 선택하세요.\n");
    printf("(1)FCFS / (2)SJF / (3)Priority / (4)RR / (5)Preemptive SJF / (6)Preemptive Priority\n");
    scanf("%d",&num);

    switch(num) {
        case 1:
            schedule_FCFS();
            break;
        case 2:
            schedule_np_SJF();
            break;
        case 3: 
            schedule_Priority();
            break;
        case 4:
            schedule_RR(2);  // 기본 time_quantum = 2
            break;
        case 5:
            schedule_preemptive_SJF(); 
            break;
        case 6:
            schedule_preemptive_Priority(); 
            break;        
    }
}

// evaluation
void evaluation() {
    float avg_wating_time = 0;
    float avg_turnaround_time = 0;

    for(int i = 0; i < process_count; i++){
        avg_wating_time += processes[i]->waiting_time;
        avg_turnaround_time += processes[i]->turnaround_time;
    }
    avg_wating_time /= process_count;
    avg_turnaround_time /= process_count;
    // 소수점 둘째자리까지 print
    printf("\naverage waiting time : %.2f\n",avg_wating_time);
    printf("average turnaround time : %.2f\n",avg_turnaround_time);
}
int main(){
    config();
    
    for(int i = 0; i < MAX_PROCESSES; i++){
        create_process(); // MAX_PROCESSES의 개수만큼 Process 생성
    }

    //예시
    /*int at [3]={3,0,9},  cpu[3]={2,5,1},  ioB[3]={2,3,3},
    ioReq[3]={3,3,5}, prio[3]={8,1,7};
    for(int i=0;i<3;i++){
        processes[i]->arrival_time   = at[i];
        processes[i]->cpu_burst_time = cpu[i];
        processes[i]->io_burst_time  = ioB[i];
        processes[i]->io_total = ioB[i];
        processes[i]->io_request_time= ioReq[i];
        processes[i]->priorty        = prio[i];
        processes[i]->remaining_time = cpu[i];
        printf("pid\tarrival time\tcpuburst\tio burst\tio request\tpriority\n");
        printf("%d\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n",processes[i]->pid,processes[i]->arrival_time, processes[i]->cpu_burst_time, processes[i]->io_burst_time,processes[i]->io_request_time,processes[i]->priorty);
    
    }*/
    

    schedule();
    evaluation();

}