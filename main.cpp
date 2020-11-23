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

// int *input;
int *inDegree;

int *inputs;


/*--------------------------------------------------------------------*/
typedef volatile int atomic_t;
// 这是啥？向量加法？
static inline int atomic_add(atomic_t *v, const int c) // 原子加操作
{
    register int i = c;
    __asm__ __volatile__("lock ; xaddl %0, %1;"
                         : "+r"(i), "+m"(*v)
                         : "m"(*v), "r"(i));

    return i;
}

static inline int atomic_inc(atomic_t *v) { return atomic_add(v, 1); } // 原子自加操作

static inline int atomic_dec(atomic_t *v) { return atomic_add(v, -1); } // 原子自减操作
/*--------------------------------------------------------------------*/

//jrf,串行：m条边，n个点复杂度：n*m

void *calcIndeg(void *){

}

// 串行算法。。不太行
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
    //sem_t *_ptlock;

    int bias;
};


//static void *readThread(void *FptP){
static void *readThread(void *Fptp){
   

    FptreadP *F = (FptreadP *)malloc(sizeof(FptreadP));
    F = (FptreadP *)Fptp;
   // sem_t *semlock = F->_ptlock;
    
    F->pf = fopen(F->FilePath,"r");
    if(F->pf == NULL)
    {
        printf("Open error!\n");
        pthread_exit(0);
    }
    fseek(F->pf,0,SEEK_SET);
    fseek(F->pf, F->seq*sizeof(int)*F->bias,SEEK_SET);
    int *inp;// = (int *)malloc(sizeof(int)*F->bias);
    inp = inputs+(F->seq*F->bias);
    fread(inp,F->bias*sizeof(int),1,F->pf);
    fclose(F->pf);
    pthread_exit(0);
}  


int ReadFilemainThread(int n_vertex, int n_edges, char *FilePath, int TotalCounts)
{
    //sem_t semlock;
    //int TotalCounts = n_vertex + n_edges*2;
    int Thcounts = 6;//TotalCounts/n_vertex;
    FptreadP *Fpt[Thcounts];// = (FptreadP *)malloc(Thcounts*sizeof(FptreadP));

   // sem_init(&semlock,0,1);
   
    pthread_t Threads[Thcounts];
    inputs = (int *)malloc(sizeof(int)*TotalCounts);
    cout << "begin to open the threads\n";
    
    // jrf reading my datas
    if(TotalCounts%Thcounts == 0)
        for(int i = 0; i < Thcounts; i++)
        {
            Fpt[i] = (FptreadP *)malloc(sizeof(FptreadP));
            Fpt[i]->bias = TotalCounts/Thcounts;
            Fpt[i]->FilePath = FilePath;
            //Fpt[i]->_ptlock = &semlock;
            Fpt[i]->seq = i;
            pthread_create(&Threads[i], NULL, readThread, (void *)Fpt[i]);
            usleep(10);
        }
    else
         for(int i = 0; i < Thcounts; i++)
        {
            Fpt[i] = (FptreadP *)malloc(sizeof(FptreadP));
	    if(i == Thcounts-1){
	        Fpt[i]->bias = TotalCounts-(TotalCounts/Thcounts)*(Thcounts-1);
		cout<<"The last pthread data numbers: "<<Fpt[i]->bias<<" "<<TotalCounts/Thcounts<<endl;
	    }
	    else
                Fpt[i]->bias = TotalCounts/Thcounts;
            Fpt[i]->FilePath = FilePath;
            //Fpt[i]->_ptlock = &semlock;
            Fpt[i]->seq = i;
            pthread_create(&Threads[i], NULL, readThread, (void *)Fpt[i]);
            usleep(10);
        }   
    for (int j = 0; j < Thcounts; j++)
    {
        /* code */
        pthread_join(Threads[j],NULL);
        free(Fpt[j]);

    }
    
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

    size_t sz_v = n_vertex * sizeof(int);
    int data_nums = n_vertex + 2*n_edge;
    size_t sz = data_nums * sizeof(int);
    int *output = (int *)malloc(sz_v);
    unsigned long Unpara_time = 0 ,para_time = 0;
    FILE *fp;
    if((data_nums)<=33500000){
        // input
        gettimeofday(&tv[0],NULL);
        inputs = (int *)malloc(sz);
        // inputs = (int *)malloc(sz);
        fp = fopen(argv[3], "r");
       // cout << "function ================================================================= File Path is: "<<&fp<<endl;
        if (fp == NULL)
        {
            printf("Failed to open %s\n", argv[3]);
            return -1;
        }
        fread(inputs, sizeof(int), n_vertex + n_edge * 2, fp);
        fclose(fp);

        gettimeofday(&tv[1],NULL);

        Unpara_time = 1000000 * (tv[1].tv_sec - tv[0].tv_sec) + tv[1].tv_usec - tv[0].tv_usec;
    }
    else{
        gettimeofday(&tv[0],NULL);
       // readFile Obj(n_vertex, n_edge,argv[3]);
        ReadFilemainThread(n_vertex,n_edge,argv[3],data_nums);
        gettimeofday(&tv[1],NULL);
        para_time = 1000000 * (tv[1].tv_sec - tv[0].tv_sec) + tv[1].tv_usec - tv[0].tv_usec;
    }
   cout<<"unparal data: \n";
   for (int i = 0; i < n_vertex + n_edge * 2; i++)
   {
       /* code */
       if(i/n_vertex == 0)
           cout<<(inputs[i])<<" ";//<<endl;
   }
    cout<<endl;
    // do the job
    //struct timeval tv[2];
    gettimeofday(&tv[1], NULL);
    //TODO: do your work
    calc_longest_path(n_vertex, n_edge, inputs, output);

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
    //calc_inDegree(n_vertex, n_edge, input, Indegrees);
   // free(input);
    free(inputs);
    free(output);
    return 0;
}

// 原子加 线程测试函数
void *testfunction(void *value){
    
    for(int i = 0; i<10;i++)
        (*(int *)value)+=1;
        // atomic_inc((int *)value);
    int a = *(int *)value;
    printf("The Value changed and it is: %d\n", a);
    pthread_exit(0);
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

    int test =6;
    int *test_out;// = (int *)malloc(sizeof(int));
    cout<< "原子操作测试开始=====================================begin\n";
    test_out = &test;
    atomic_inc(&test);
    
    // test_out = atomic_inc(&test_out);
    cout << "the Value is: 直接函数返回 "<<atomic_inc(&test)<<endl;
    cout<<"赋值返回 is： "<<test<<endl;
    cout<<atomic_add(&test,2)<<endl;
    cout<<test<<endl;
    cout<<"test The mutithreads about 原子=============================begin";
    pthread_t testhrs[10];
    int *test_value = (int *)malloc(sizeof(int));
    *test_value = 0;
    for(int j = 0;j<10;j++)
    {
        pthread_create(&testhrs[j],NULL,testfunction,(void *)test_value);
        pthread_join(testhrs[j],NULL);
    }
    cout<<"at last the value is: "<<*test_value<<endl;
    cout<<"test The mutithreads about 原子=============================begin";
    cout<< "原子操作测试结束======================================end\n";

    pthread_exit(NULL);
    return 0;
}
