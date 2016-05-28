#include <iostream>
#include <cstdio>
#include <pthread.h>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
using namespace std;

#define Semaphore_Name "/gameoflife"


const int maxn = 100;
int proc_count;
int steps_count;
int part;
    
int offset[] = {-1, 0, 1};
sem_t* semap_end;

struct send_data
{
    int from_row, to_row;
    bool *from, *to;
};

struct life_data
{
    bool life[maxn * maxn], buf[maxn * maxn];
};


bool get_cell(int x, int y, bool *arr)
{
    return *(arr + x * maxn + y);
}

bool set_cell(int x, int y, bool *arr, bool what)
{
    *(arr + x * maxn + y) = what;
}

inline void set(int x, int y, bool *from, bool *to)
{
    int cnt = 0, tx, ty;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            tx = x + offset[i];
            if (tx >= maxn)
                tx = 0;
            else if (tx < 0)
                tx = maxn - 1;

            ty = y + offset[j];
            if (ty >= maxn)
                ty = 0;
            else if (ty < 0)
                ty = maxn - 1;

            if (tx == x && ty == y)
                continue;

            if (get_cell(tx, ty, from))
                cnt++;
        }
    }

    if (get_cell(x, y, from))
    {
        if (cnt == 2 || cnt == 3)
            set_cell(x, y, to, true);
        else
            set_cell(x, y, to, false);
    }
    else
    {
        if (cnt == 3)
            set_cell(x, y, to, true);
        else
            set_cell(x, y, to, false);
    }
} 

void* recalc(void * data)
{
    send_data* cur_data = (send_data*)data;
    int from_row = cur_data->from_row, to_row = cur_data->to_row;
    bool *from = cur_data->from, *to = cur_data->to;

    for (int i = from_row; i < to_row; i++)
    {
        for (int j = 0; j < maxn; j++)
            set(i, j, from, to);
    }
    delete cur_data;
    return NULL;
}


int main(int argc, char** argv)
{   
    int shm_handle;
    int cur_pid = getpid();

    cout.precision(8);
    cout.setf(cout.fixed);
    //создайм shm размера life_data
    shm_handle = shm_open("/gamelife", O_RDWR | O_CREAT, 0777);
    ftruncate(shm_handle, sizeof(life_data));
    if (shm_handle < 0)
    {
        cout << "Can't create shared memory segment" << endl;
        cout << strerror(errno) << endl;;
        return 0;
    }
    //отображаем cur_data на shm
    life_data* cur_data = (life_data*)mmap(
        NULL, sizeof(life_data), PROT_WRITE,
        MAP_SHARED,  shm_handle, 0);
    
    if (cur_data == (life_data*)-1)
    {
        cout << "Can't attach shared memory segment" << endl;
        return 0;
    }
    //инициализируем начальную доску
    srand(time(NULL));

    for (int i = 0; i < maxn; i++)
    {
        for (int j = 0; j < maxn; j++)
        {
            cur_data->life[i * maxn + j] = rand() % 2;
            cur_data->buf[i * maxn + j] = 0;
        }
    }
    
    bool *first = cur_data->life, *second = cur_data->buf;
    clock_t time_begin_calc, time_end_calc;
    //семафор для пересчитывания доски
    semap_end = sem_open(Semaphore_Name, O_CREAT, 0666, 0);
    
    part = maxn / proc_count;
    for(proc_count=1; j < 9; ++j) {
	cout << proc_count << " ";
	for(steps_count=1; steps_count < 6; steps_count++) {
	    time_begin_calc = clock();
	    for (int j=0; j < steps_count * 10; j++)
	    {
		if (semap_end == SEM_FAILED)
		{
		    cout << strerror(errno) << endl;
		    return 0;
		}
		//пересчитываем доску
		for (int i = 0; i < proc_count; i++)
		{
		    int fid = fork();
		    if (fid == 0)
		    {
		        int left = i * part, right;
		        if (i != proc_count - 1)
		            right = (i + 1) * part;
		        else
		            right = maxn;

		        send_data *sn_data = new send_data();
		        sn_data->from = first;
		        sn_data->to = second;
		        sn_data->from_row = left;
		        sn_data->to_row = right;
		        recalc(sn_data);
		        sem_post(semap_end);
		        return 0;
		    }
		}
		//ждём пока все процессы пересчитают свой кусок доски
		for (int i = 0; i < proc_count; i++)
		    sem_wait(semap_end);
		
		swap(first, second);
	    }
	    time_end_calc = clock();
	    cout << (time_end_calc - time_begin_calc) / (double)CLOCKS_PER_SEC << " ";
	}
	cout << endl;
    }

    return 0;
}
