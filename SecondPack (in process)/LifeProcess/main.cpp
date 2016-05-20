#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <vector>
#include <unistd.h>
using namespace std;


int n;
int m;

int num_threads;


int countNeighbours(vector<vector<int> >& matrix, int x, int y)
{
    int ans = 0;
    for (int i = -1; i < 2; ++i)
        for (int j = -1; j < 2; ++j)
            if (i != 0 || j != 0)
                ans += matrix[(x + i + n) % n][(y + j + m) % m];
    return ans;
}
// B3S23
bool checkAlive(vector<vector<int> >& matrix, int x, int y)
{
    int neigbours = countNeighbours(matrix, x, y);
    if (matrix[x][y] && (neigbours == 2 || neigbours == 3))
        return true;
    if (!matrix[x][y] && neigbours == 3)
        return true;
    return false;
}

struct Life
{
    vector<vector<int> > *old;
    vector<vector<int> > *fresh;
    int rows;
    int from;
};

void* refresh(void* arg)
{
    struct Life* temp = (Life*)arg;
    vector< vector<int> > * old = temp->old;
    vector< vector<int> > * fresh = temp->fresh;
    for (int i = temp->from; i < temp->from + temp->rows; ++i)
    {
        for (int j = 0; j < m; ++j)
            (*fresh)[i][j] = checkAlive(*old, i, j);
    }
    return nullptr;
}

void generateMap(vector<vector<int> >& map)
{
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            map[i][j] = rand() % 2;
}
void visual(vector<vector<int> >& map)
{
    char dead = 46;
    char alive = 254;
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < m; ++j)
            printf("%c ",  map[i][j] ? alive : dead);
        printf("\n");
    }
    printf("\n");
    printf("\n");
}

int main(int argc, char*  argv[])
{
    if(argc != 4)
    {
        n = 20;
        m = 20;
        num_threads = 4;
    } else
    {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        num_threads = atoi(argv[3]);
    }
    
    pthread_t threads[num_threads];
    Life parts[num_threads];
    vector<vector<int> > lifeMap(n, vector<int>(m));
    generateMap(lifeMap);
    while (true)
    {
        int status;
        int status_addr;
        vector<vector<int> > newLife(n, vector<int> (m));
        for (int i = 0; i < num_threads; i++) {
            parts[i].fresh = &newLife;
            parts[i].old = &lifeMap;
            parts[i].from = i * (n / num_threads);
            parts[i].rows = n / num_threads;
            if (i == num_threads - 1)
                parts[i].rows += n % num_threads;
        }
        
        for (int i = 0; i < num_threads; i++) {
            status = pthread_create(&threads[i], NULL, refresh, (void*)&parts[i]);
            
        }
        
        for (int i = 0; i < num_threads; i++) {
            status = pthread_join(threads[i], (void**)&status_addr);
        }
        swap(lifeMap, newLife);
        visual(lifeMap);
        sleep(1);
    }
    return 0;
}