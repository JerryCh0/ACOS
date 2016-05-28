#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <locale.h>

// В пустой (мёртвой) клетке, рядом с которой ровно три живые клетки, зарождается жизнь;
// Если у живой клетки есть две или три живые соседки, то эта клетка продолжает жить;
// В противном случае (если соседей меньше двух или больше трёх) клетка умирает («от одиночества» или «от перенаселённости»)

//функция получает на вход: текущее поле, размеры поля, координаты. И высчитывает след. положение выбранной ячейки.

clock_t time_begin_calc;
clock_t time_end_calc;

bool getNextState(bool *field, int h, int w, int i, int j)
{
    int count = 0;
    for (int di = -1; di <= 1; ++di)
        for (int dj = -1; dj <= 1; ++dj)
            if (di || dj)
            {
                int ni = (i + di + h) % h;
                int nj = (j + dj + w) % w;
                count += field[ni * w + nj];
            }
    return field[i * w + j] ? (2 <= count && count <= 3) : (count == 3);
}

//функция, вызываемая на каждом шаге нашей программы, она получает на вход: текущее поле, получаемое поле, размеры поля, координаты изменяемого прямоугольника
void step(bool *field, bool *next, int h, int w, int i0, int j0, int i1, int j1)
{
    time_begin_calc = clock();
    for (int i = i0; i <= i1; ++i)
        for (int j = j0; j <= j1; ++j)
            next[i * w + j] = getNextState(field, h, w, i, j);
}

bool *malloc_field(int h, int w) //выделяем память под поле
{
    return malloc(h * w * sizeof(char*));
}

void free_field(bool *field, int h, int w)//удаляет поле
{
    free(field);
}

bool *generate_field(int h, int w) //создает поле, заполняет его рандомно
{
    bool *field = malloc_field(h, w);
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j)
            field[i * w + j] = rand() % 2;
    return field;
}

//Вывод поля
void print_field(bool *field, int h, int w) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            printf("%c", *(field + i * h + j) ? '+' : ' ');
        }
        printf("\n");
    }
}

int number_threads; //количество потоков
int number_steps; //количество шагов
int h;//высота
int w;//ширина
bool *curr; //текущее поле
bool *next;//следующее поле

sem_t *sem_managing; //семафор, который ждет, пока закончат работу все потоки, чтобы перейти к следующему шагу
sem_t *sem_threads; //семафор для каждого потока

//функция получает на вход порядковый номер семафора. Функция запускается для каждого потока. Каждый поток работает на своем прямоугольники
static void *thread_start(void *arg)
{
    int id = (size_t) arg;
    int i0 = h / number_threads * id; //высчитываем координаты прямоугольника (на котором будет работать поток с номером id).
    int j0 = 0;
    int i1 = i0 + h / number_threads - 1;
    int j1 = w - 1;
    
    for (int i = 0; i < number_steps; ++i) //запускаем изменение определенного прямоугольника на нужное кол-во шагов.
    {
        step(curr, next, h, w, i0, j0, i1, j1);
        sem_post(sem_managing); //освобождаем семафор
        sem_wait(&sem_threads[id]); //захватываем семафор
    }
    return NULL;
}

void run_live()//если потоков больше одного
{
    sem_init(sem_managing = malloc(sizeof(sem_t)), 1, 0); //функция инициализирует неименованный семафор
    sem_threads = malloc(number_threads * sizeof(sem_t));//выделяем память под семафоры
    for (size_t i = 0; i < number_threads; ++i)
        sem_init(&sem_threads[i], 1, 0); //инициализируем их
    
    pthread_t *ids = malloc(number_threads * sizeof(pthread_t)); //выделяем память под потоки
    
    for (size_t i = 0; i < number_threads; ++i)
        pthread_create(&ids[i], NULL, &thread_start, (void*) i);//создаем массив потоков и запускаем их
    
    for (int step = 0; step < number_steps; ++step)
    {
        for (int number_done = 0; number_done < number_threads; ++number_done)
            sem_wait(sem_managing); //захватываем семофор
        bool *temp = curr;//
        curr = next;//
        next = temp;// получили новое поле, свопаем с текущим
        time_end_calc = clock();
        print_field(next, h, w);
        printf("______________________________");
        printf("%s%f\n", "Time : ", ((time_end_calc - time_begin_calc) / (double) CLOCKS_PER_SEC));
        usleep(200 * 1000);
        printf("\n");
        
        
        for (int i = 0; i < number_threads; ++i)// освобождаем семафоры, которые захватывали в thread_start
            sem_post(&sem_threads[i]);
    }
    
    for (size_t i = 0; i < number_threads; ++i)
        pthread_join(ids[i], NULL); //ожидание завершения потока управления
    free(ids);
    
    sem_destroy(sem_managing);//удаление неименованного семафора
    for (size_t i = 0; i < number_threads; ++i)
        sem_destroy(&sem_threads[i]);
    free(sem_managing);
    free(sem_threads);
    
}
void copy_field(bool *to, bool *from, int h, int w) //копирует поле
{
    for (int i = 0; i < h * w; ++i)
        to[i] = from[i];
}

int main()
{
    number_threads = 3;
    number_steps = 10;
    h = 10;
    w = 10;
    curr = generate_field(h, w);
    next = malloc_field(h, w);
    
    run_live();

    free_field(curr, h, w);
    free_field(next, h, w);
    return 0;
}
