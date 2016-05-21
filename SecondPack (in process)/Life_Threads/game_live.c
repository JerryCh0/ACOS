#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <ncurses.h>
#include <locale.h>

// В пустой (мёртвой) клетке, рядом с которой ровно три живые клетки, зарождается жизнь;
// Если у живой клетки есть две или три живые соседки, то эта клетка продолжает жить;
// D противном случае (если соседей меньше двух или больше трёх) клетка умирает («от одиночества» или «от перенаселённости»)

//функция получает на вход: текущее поле, размеры поля, координаты. И высчитывает след. положение выбранной ячейки.
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
	return field[i * w + j] ? (2 <= count && count <= 3) : count == 3;
}

//функция, вызываемая на каждом шаге нашей программы, она получает на вход: текущее поле, получаемое поле, размеры поля, координаты изменяемого прямоугольника
void step(bool *field, bool *next, int h, int w, int i0, int j0, int i1, int j1)
{
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
	
	if (ncurses)
		endwin();//приостанавливаем сеанс
}


//аналогично и многопоточной функции, только из-за того, что поток 1 - просто запускаем его на нужное кол-во шагов
void one_thread()
{
	for (int i = 0; i < number_steps; ++i)
	{
		step(curr, next, h, w, 0, 0, h - 1, w - 1);
		bool *temp = curr;
		curr = next;
		next = temp;
	}
}

typedef void (*func) ();
long measure(func f) //замеряет время работы определенной функции
{
	/*
	struct timespec {
		time_t   tv_sec;    секунды 
		long     tv_nsec;   наносекунды
	};
	*/
	struct timespec tps, tpe;
	//Функция clock_gettime() получает и устанавливают время указанных часов (первый аргумент), и устанавливает во второй,
	//если 2 аргумент не равно NULL, сохраняет её в struct timespec, указанную во втором аргументе.
	//CLOCK_REALTIME - Часы системы, отсчитывающие реальное (т. е., бытовое) время.
	clock_gettime(CLOCK_REALTIME, &tps);
	f();
	clock_gettime(CLOCK_REALTIME, &tpe);
	return (tpe.tv_sec - tps.tv_sec) * 1000000000 + (tpe.tv_nsec - tps.tv_nsec);
}

long measure_live(int h_, int w_, int number_threads_, int number_steps_, bool generate)//измеряет время 
{
	number_threads = number_threads_;
	number_steps = number_steps_;
	h = h_;
	w = w_;
	next = malloc_field(h, w);//выделяем память под след. элемент
	if (generate)
		curr = generate_field(h, w);//если элемент не создался - создаем его
	long time = measure(number_threads == 1 ? one_thread : run_live); //запускаем измерение времени, в зависимости от количества потоков
	if (generate)
		free_field(curr, h, w);//удаляем поля
	free_field(next, h, w);
	return time;
}

void warmup()
{
	printf("warmup...\n");
	int h = 840;
	int w = 1e3;
	measure_live(h, w, 1, 10, 1);
	measure_live(h, w, 4, 10, 1);
	measure_live(h, w, 2, 10, 1);
	for (int i = 0; i < 10; ++i)
		for (int number_threads = 1; number_threads <= 8; ++number_threads)
		{
			measure_live(h, w, 9 - number_threads, 10, 1);
			measure_live(h, w, number_threads, 100, 1);
			measure_live(h, w, number_threads, 10, 1);
			measure_live(h, w, 9 - number_threads, 100, 1);
		}
	printf("warmup done\n");
}

void copy_field(bool *to, bool *from, int h, int w) //копирует поле
{
	for (int i = 0; i < h * w; ++i)
		to[i] = from[i];
}

//По сути функция красивого вывода на экран. Также имееет счетчик времени measured_time.
void test()
{
	ncurses = 0;
	warmup();
	
	int h = 840;
	int w = 1e3;//задает поле
	int steps_min = 10;
	int steps_max = 210;
	int steps_delta = 10;
	bool *initial = generate_field(h, w); 
	//ниже идет просто красивый вывод на экран
	curr = malloc_field(h, w);
	printf("Поле %dx%d\n", h, w);
	
	printf("Число операций:  ");
	for (int steps_ = steps_min; steps_ <= steps_max; steps_ += steps_delta)
		printf("  %4d", steps_);
	printf("\n");
	
	int steps_count = (steps_max - steps_min) / steps_delta + 1;
	long times[10][steps_count];
	long one_thread_time[steps_count];
	for (int number_threads = 1; number_threads <= 8; ++number_threads)
	{
		printf("%d:                ", number_threads);
		for (int steps_ = steps_min; steps_ <= steps_max; steps_ += steps_delta)
		{
			int iStep = (steps_ - steps_min) / steps_delta;
			long measured_time = 0;
			for (int i = 0; i < 10; ++i)
			{
				copy_field(curr, initial, h, w);
				measured_time += measure_live(h, w, number_threads, steps_, 0); //счетчик времени
			}
			measured_time /= 10;
			times[number_threads][iStep] = measured_time;
			if (number_threads == 1)
				one_thread_time[iStep] = measured_time;
			printf("  %.2f", measured_time / (float) one_thread_time[(steps_ - steps_min) / steps_delta]);
			fflush(stdout);
		}
		printf("\n");
	}
	printf("\n");
	free_field(initial, h, w);
	free_field(curr, h, w);
	
	for (int number_threads = 1; number_threads <= 8; ++number_threads)
	{
		char name[10];
		sprintf(name, "plots/%d.txt", number_threads); //функция производит вывод не на экран, а в массив (первый аргумент)
		FILE *fout = fopen(name, "w");
		if (fout == NULL)
			error(1, errno, "fopen");
		fprintf(fout, "x y\n");
		for (int steps_ = steps_min; steps_ <= steps_max; steps_ += steps_delta)
		{
			int iStep = (steps_ - steps_min) / steps_delta;
			fprintf(fout, "%d %ld\n", steps_ + steps_max * (number_threads > 4), times[number_threads][iStep] / steps_ / 1000);
		}
		fclose(fout);
	}
}

void check() //просто пропроверяет корректрость работы всех функций
{
	int h = 840;
	int w = 1e3;
	curr = malloc_field(h, w);
	bool* initial = generate_field(h, w);
	bool *answer1 = malloc_field(h, w);
	bool *answer2 = malloc_field(h, w);
	
	copy_field(curr, initial, h, w);
	measure_live(h, w, 1, 100, 0);
	copy_field(answer1, curr, h, w);
	
	copy_field(curr, initial, h, w);
	measure_live(h, w, 2, 100, 0);
	copy_field(answer2, curr, h, w);
	
	for (int i = 0; i < h * w; ++i)
		assert(answer1[i] == answer2[i]);
	
	free_field(curr, h, w);
	free_field(initial, h, w);
	free_field(answer1, h, w);
	free_field(answer2, h, w);
}

int main()
{
	srand(time(0));
	setlocale(LC_ALL, "");
	
	int action = 2;
	if (!action)
	{
		printf("Выберете действие:\n");
		printf("[1] запустить измерение времени\n");
		printf("[2] проверить корректность\n");
		printf("[3] выход\n");
		scanf("%d", &action);
	}
	switch (action)
	{
		case 1:
			test();
			break;
		case 2:
			check();
			break;
		case 3:
			exit(0);
		default:
			printf("Такого действия нет!\n");
			exit(1);
	}
	return 0;
}
