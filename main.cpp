#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/time.h>
#include <pthread.h>

#include <semaphore.h>
#include <string.h>
#include <list>
#include <unistd.h>
#include <iostream>
using namespace std;

int n_vertex;
int n_edge;

int *input;
int *inDegree;

int *inputs;


/*--------------------------------------------------------------------*/
typedef volatile int atomic_t;
// 这是啥？向量加法？
static inline int atomic_add(atomic_t *v, const int c)
{
    register int i = c;
    __asm__ __volatile__("lock ; xaddl %0, %1;"
                         : "+r"(i), "+m"(*v)
                         : "m"(*v), "r"(i));

    return i;
}

static inline int atomic_inc(atomic_t *v) { return atomic_add(v, 1); }

static inline int atomic_dec(atomic_t *v) { return atomic_add(v, -1); }
/*--------------------------------------------------------------------*/

//jrf,串行：m条边，n个点复杂度：n*m
void calc_inDegree(int n_vert, int n_edge, int *input, int *InDegrees)
{
    int results[n_vert] = {0};
    for (int i = 0; i < n_vert; i++)
    {
        for (int j = n_vert; j < 2 * n_edge; j = j + 2)
        {
            if (i - input[j] == 0)
            {
                results[i] += 1;
                //cout << "this is input: " << input[j] << "  ";
            }
        }
        //cout << "this is input: " << results[i] << " ";
    }
    cout<<input[n_vert-1] << endl;
    InDegrees = results;
}
// 计算最长路径
void calc_longest_path(int n_vertex, int n_edge, int *input, int *output)
{

    printf("===TODO: calc longest path use cpu, better performance with multi thread.\n");
}

/*******************************************************************************************jrf begin***/
//void *readThread();

struct FptreadP
{
    /* data */
    //readFile temp;
    FILE *pf;
    char *FilePath;
    int seq;
    sem_t *_ptlock;

    int bias;
};


//static void *readThread(void *FptP){
static void *readThread(void *Fptp){
   
    //int *seq = (FptreadP *)FptP;
    // pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    // pthread_mutex_lock(&mtx);   
    //cout<<"in"<<endl;
    FptreadP *F = (FptreadP *)malloc(sizeof(FptreadP));
    //(FptreadP *)FptP;
    F = (FptreadP *)Fptp;
    sem_t *semlock = F->_ptlock;
    
    //FILE *pf = F->temp.fp;
    //F->fp = fopen("../../../dataset/dag_data_1000000_100000000.b","r");
    F->pf = fopen(F->FilePath,"r");
    // cout << "File Path is: "<<&F->pf<<endl;
    if(F->pf == NULL)
    {
        printf("Open error!\n");
        pthread_exit(0);
    }
    fseek(F->pf,0,SEEK_SET);
    fseek(F->pf, F->seq*sizeof(int)*F->bias,SEEK_SET);
    // cout<<"Threads begin: "<<F->bias<<endl;
    //cout<<"sizeof(int): "<<sizeof(int)<<endl;
    
   int *inp;// = (int *)malloc(sizeof(int)*F->bias);
//    sem_wait(semlock);
   //memset(inp,-1,sizeof(int)*F->bias);
    inp = inputs+(F->seq*F->bias);
    // inp = (int *)malloc(sizeof(int)*F->bias);
    // if (F->seq == 0)
    // {
    //     /* code */
    //     cout<<"First Groups\n";
    //     for (int i = 0; i < 100; i++)
    //     {
    //         /* code */
    //         cout<<inputs[i]<<" ";
    //     }
    //     cout<<"\nend groups"<<endl;
        
    // }
    

    
    //cout<<"begin to read\n";
    fread(inp,F->bias*sizeof(int),1,F->pf);
    // cout<<F->seq<<" read ok: "<< F->seq*F->bias<<endl;
    //F->temp.inputs[F->seq] = *inp;
    

   // pthread_mutex_unlock(&mtx);
    
    //memcpy((void *)inputs[F->bias*F->seq],(void *)inp, F->bias*sizeof(int));
    //cout<<"cpy\n";
    // free(inp);
    // free(Fptp);
    //cout<<"the thread id is: "<<pthread_self()<<endl;
    fclose(F->pf);
    // sem_post(semlock);
    //pthread_detach(pthread_self());
    pthread_exit(0);
}  


int ReadFilemainThread(int n_vertex, int n_edges, char *FilePath)
{
    sem_t semlock;
    int TotalCounts = n_vertex + n_edges*2;
    // FptreadP *Fpt = (FptreadP *)malloc(sizeof(FptreadP));
    int Thcounts = TotalCounts/n_vertex;
    // int Thcounts = 1;
    FptreadP *Fpt[Thcounts];// = (FptreadP *)malloc(Thcounts*sizeof(FptreadP));

    sem_init(&semlock,0,1);
   
    pthread_t Threads[Thcounts];
    inputs = (int *)malloc(sizeof(int)*TotalCounts);
    cout << "begin to open the threads\n";
    
    for(int i = 0; i < Thcounts; i++)
    {
        /* code */
        // int j = i;
        // cout<<"just in loops\n";
        // cout << "had define a struct objects\n";
        //memset(Fpt,0,sizeof(FptreadP));
        Fpt[i] = (FptreadP *)malloc(sizeof(FptreadP));
        //memset(Fpt, 0, sizeof(FptreadP));
        // Fpt[i]->bias = n_vertex;//counts;
        Fpt[i]->bias = TotalCounts/Thcounts;
        Fpt[i]->FilePath = FilePath;
        Fpt[i]->_ptlock = &semlock;
        Fpt[i]->seq = i;
        // usleep(10);
        //cout<<"seq is "<<Fpt->seq<<endl;;
        pthread_create(&Threads[i], NULL, readThread, (void *)Fpt[i]);
        //cout << "Open the: " << i<<" thread\n";
        usleep(10);
        
    }
    for (int j = 0; j < Thcounts; j++)
    {
        /* code */
        // cout<<"reduce is: "<<&Fpt[j+1]->pf - &Fpt[j]<<endl;
        pthread_join(Threads[j],NULL);
        free(Fpt[j]);

    }
    
    // free(Fpt[Thcounts]);
    //pthread_exit(NULL);
    return 0;
}

/******************************************************************************************end***/
//算法验证
int RunTest(int argc, char *argv[])
{
    struct timeval tv[2];
    sscanf(argv[1], "%d", &n_vertex);
    sscanf(argv[2], "%d", &n_edge);
    printf("v: %d e: %d\n", n_vertex, n_edge);

    // input
    size_t sz = (n_vertex + n_edge * 2) * sizeof(int);
    size_t sz_v = n_vertex * sizeof(int);
    gettimeofday(&tv[0],NULL);
    input = (int *)malloc(sz);
    // inputs = (int *)malloc(sz);
    int *output = (int *)malloc(sz_v);
    FILE *fp;
    fp = fopen(argv[3], "r");
    cout << "function ================================================================= File Path is: "<<&fp<<endl;
    if (fp == NULL)
    {
        printf("Failed to open %s\n", argv[3]);
        return -1;
    }
    fread(input, sizeof(int), n_vertex + n_edge * 2, fp);
    fclose(fp);
    gettimeofday(&tv[1],NULL);
    unsigned long Unpara_time = 1000000 * (tv[1].tv_sec - tv[0].tv_sec) + tv[1].tv_usec - tv[0].tv_usec;

    cout<<"\n\n"<<endl;
    gettimeofday(&tv[0],NULL);
   // readFile Obj(n_vertex, n_edge,argv[3]);
    ReadFilemainThread(n_vertex,n_edge,argv[3]);
    gettimeofday(&tv[0],NULL);
    cout<<"unparal data: \n";
    for (int i = 0; i < 200; i++)
    {
        /* code */
        cout<<(input[sz/sizeof(int)-i])<<" ";//<<endl;
    }
    cout<<endl;
    cout<<"paral data: \n";
    for (int i = 0; i < 200; i++)
    {
        /* code */
        cout<<(inputs[sz/sizeof(int)-i])<<" ";//<<endl;
    }
    
    cout<<"\n\n"<<endl;
    // do the job
    //struct timeval tv[2];
    gettimeofday(&tv[1], NULL);
    unsigned long para_time = 1000000 * (tv[1].tv_sec - tv[0].tv_sec) + tv[1].tv_usec - tv[0].tv_usec;
    //TODO: do your work
    calc_longest_path(n_vertex, n_edge, input, output);

    gettimeofday(&tv[1], NULL);
    unsigned long time_in_micros = 1000000 * (tv[1].tv_sec - tv[0].tv_sec) + tv[1].tv_usec - tv[0].tv_usec;
    printf("calculate time: %ld\n", time_in_micros);
    printf("unparall time is: %ld\n",Unpara_time);
    printf("parall time is: %ld\n\n",para_time);
    //output
    fp = fopen(argv[4], "w");
    if (fp == NULL)
    {
        printf("Failed to open %s\n", argv[4]);
        return -1;
    }
    fwrite(output, sizeof(int), n_vertex, fp);
    fclose(fp);
    int Indegrees[n_vertex] = {0};
    calc_inDegree(n_vertex, n_edge, input, Indegrees);
    free(input);
    free(inputs);
    free(output);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        printf("Wrong number of arguments\n");
        printf("Usage:./cpu 6 12 /contest/dataset/dag_data_6_12.b result.b\n");
        printf("%s <number of vertices> <number of edges> <dag input file> <result file>\n", argv[0]);
        printf("dag input file: specify the path for dag input file\n");
        printf("result file: specify the output file name, the file will be overwritten if already exists\n");
        return -1;
    }

    struct timeval tv[2];
    gettimeofday(&tv[0], NULL);

    int status = RunTest(argc, argv);

    gettimeofday(&tv[1], NULL);
    unsigned long time_in_micros = 1000000 * (tv[1].tv_sec - tv[0].tv_sec) + tv[1].tv_usec - tv[0].tv_usec;
    printf("total time: %ld\n", time_in_micros);
    if (status == 0)
    {
        printf("SUCCESS!!\n");
    }
    else
    {
        printf("FAILED!!\n");
    }

    pthread_exit(NULL);
    return 0;
}
