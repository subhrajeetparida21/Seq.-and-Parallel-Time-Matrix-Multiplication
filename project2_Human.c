#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define LIMIT_PROC 8

// MATRIX MEMORY SECTION

double **makeGrid(int dim)
{
    double **grid = malloc(dim * sizeof(double *));
    for (int r = 0; r < dim; r++)
        grid[r] = malloc(dim * sizeof(double));
    return grid;
}

void destroyGrid(double **grid, int dim)
{
    for (int r = 0; r < dim; r++)
        free(grid[r]);
    free(grid);
}

void cloneGrid(double **src, double **dest, int dim)
{
    for (int r = 0; r < dim; r++)
        for (int c = 0; c < dim; c++)
            dest[r][c] = src[r][c];
}

void swapColumn(double **grid, double *vec, int colIndex, int dim)
{
    for (int r = 0; r < dim; r++)
        grid[r][colIndex] = vec[r];
}

// DETERMINANT SECTION

double calcDet(double **grid, int dim)
{
    double result = 1.0;

    for (int i = 0; i < dim; i++)
    {
        if (fabs(grid[i][i]) < 1e-9)
            return 0;

        for (int j = i + 1; j < dim; j++)
        {
            double factor = grid[j][i] / grid[i][i];
            for (int k = 0; k < dim; k++)
                grid[j][k] -= factor * grid[i][k];
        }
        result *= grid[i][i];
    }
    return result;
}

// SEQUENTIAL CRAMER SECTION

void linearSolveSeq(double **coeff, double *constVec, double *solVec, int dim)
{
    double **tmp = makeGrid(dim);
    cloneGrid(coeff, tmp, dim);

    double mainDet = calcDet(tmp, dim);
    destroyGrid(tmp, dim);

    if (mainDet == 0)
    {
        printf("No unique solution\n");
        return;
    }

    for (int var = 0; var < dim; var++)
    {
        tmp = makeGrid(dim);
        cloneGrid(coeff, tmp, dim);

        swapColumn(tmp, constVec, var, dim);
        double detVar = calcDet(tmp, dim);

        solVec[var] = detVar / mainDet;
        destroyGrid(tmp, dim);
    }
}

//PARALLEL CRAMER SECTION

void linearSolvePar(double **coeff, double *constVec, double *solVec, int dim)
{
    double **tmp = makeGrid(dim);
    cloneGrid(coeff, tmp, dim);

    double mainDet = calcDet(tmp, dim);
    destroyGrid(tmp, dim);

    if (mainDet == 0)
    {
        printf("No unique solution\n");
        return;
    }

    for (int var = 0; var < dim; var++)
    {
        pid_t proc = fork();

        if (proc == 0)
        {
            double **localGrid = makeGrid(dim);
            cloneGrid(coeff, localGrid, dim);

            swapColumn(localGrid, constVec, var, dim);
            double detVar = calcDet(localGrid, dim);

            solVec[var] = detVar / mainDet;

            destroyGrid(localGrid, dim);
            exit(0);
        }
    }

    for (int i = 0; i < dim; i++)
        wait(NULL);
}

//MAIN DRIVER SECTION

int main()
{
    int size;
    printf("Enter matrix dimension: ");
    scanf("%d", &size);

    srand(time(NULL));

    double **matrixA = makeGrid(size);
    double *vectorB = malloc(size * sizeof(double));
    double *resultX = malloc(size * sizeof(double));

    for (int i = 0; i < size; i++)
    {
        vectorB[i] = rand() % 10;
        for (int j = 0; j < size; j++)
            matrixA[i][j] = rand() % 10;
    }

    printf("\nSequential Solver Running...\n");
    clock_t t1 = clock();
    linearSolveSeq(matrixA, vectorB, resultX, size);
    clock_t t2 = clock();

    double seqDuration = (double)(t2 - t1) / CLOCKS_PER_SEC;
    printf("Sequential Time: %f sec\n", seqDuration);

    printf("\nParallel Solver Running...\n");
    t1 = clock();
    linearSolvePar(matrixA, vectorB, resultX, size);
    t2 = clock();

    double parDuration = (double)(t2 - t1) / CLOCKS_PER_SEC;
    printf("Parallel Time: %f sec\n", parDuration);

    if (parDuration > 0)
        printf("Speedup: %f\n", seqDuration / parDuration);

    destroyGrid(matrixA, size);
    free(vectorB);
    free(resultX);

    printf("\nExecution Completed.\n");
    return 0;
}
