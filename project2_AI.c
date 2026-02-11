/**********************************************************************
 * PROGRAM: Performance Analysis of Cramer's Rule (Sequential vs Parallel)
 *
 * PURPOSE:
 * This program solves the linear system AX = B using Cramer's Rule.
 * It compares execution time between:
 *      1) Sequential computation
 *      2) Parallel computation using fork()
 *
 * It accepts multiple matrix sizes from the command line and stores
 * performance results into a CSV file for plotting graphs.
 *
 * USAGE:
 *      ./final 200 400 600 800
 *
 * OUTPUT FILE:
 *      results.csv  → contains size, sequential time, parallel time, speedup
 *
 * NOTE:
 * Cramer's Rule is computationally expensive (O(n^4)).
 * This program is for performance analysis and OS parallelism study.
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

/**********************************************************************
 * MATRIX MEMORY MANAGEMENT SECTION
 *
 * These functions handle dynamic creation, copying, and deletion of
 * square matrices required during determinant and Cramer's rule steps.
 **********************************************************************/

/* Allocate an n × n matrix dynamically */
double **makeGrid(int dim)
{
    double **grid = malloc(dim * sizeof(double *));
    for (int i = 0; i < dim; i++)
        grid[i] = malloc(dim * sizeof(double));
    return grid;
}

/* Free memory allocated to matrix */
void destroyGrid(double **grid, int dim)
{
    for (int i = 0; i < dim; i++)
        free(grid[i]);
    free(grid);
}

/* Copy matrix contents */
void cloneGrid(double **src, double **dest, int dim)
{
    for (int i = 0; i < dim; i++)
        for (int j = 0; j < dim; j++)
            dest[i][j] = src[i][j];
}

/* Replace one column of matrix with vector B (Cramer's rule step) */
void swapColumn(double **grid, double *vec, int colIndex, int dim)
{
    for (int i = 0; i < dim; i++)
        grid[i][colIndex] = vec[i];
}

/**********************************************************************
 * DETERMINANT CALCULATION SECTION
 *
 * Uses Gaussian elimination to convert matrix into upper triangular
 * form. Determinant = product of diagonal elements.
 * Time Complexity = O(n³)
 **********************************************************************/

double calcDet(double **grid, int dim)
{
    double result = 1.0;

    for (int i = 0; i < dim; i++)
    {
        /* If pivot is zero → determinant becomes zero */
        if (fabs(grid[i][i]) < 1e-9)
            return 0;

        /* Eliminate elements below pivot */
        for (int j = i + 1; j < dim; j++)
        {
            double factor = grid[j][i] / grid[i][i];
            for (int k = 0; k < dim; k++)
                grid[j][k] -= factor * grid[i][k];
        }

        /* Multiply diagonal elements */
        result *= grid[i][i];
    }
    return result;
}

/**********************************************************************
 * SEQUENTIAL CRAMER SOLVER
 *
 * Steps:
 * 1. Compute determinant of A
 * 2. For each variable Xi:
 *      Replace column i with vector B
 *      Compute determinant of modified matrix
 *      Xi = det(Ai) / det(A)
 **********************************************************************/

void linearSolveSeq(double **A, double *B, double *X, int n)
{
    double **tmp = makeGrid(n);
    cloneGrid(A, tmp, n);

    double detA = calcDet(tmp, n);
    destroyGrid(tmp, n);

    if (detA == 0) return;

    for (int i = 0; i < n; i++)
    {
        tmp = makeGrid(n);
        cloneGrid(A, tmp, n);

        swapColumn(tmp, B, i, n);
        X[i] = calcDet(tmp, n) / detA;

        destroyGrid(tmp, n);
    }
}

/**********************************************************************
 * PARALLEL CRAMER SOLVER
 *
 * Uses fork() to create child processes.
 * Each child computes one variable Xi independently.
 *
 * NOTE: fork() creates separate memory spaces. This demonstrates
 * process parallelism, not shared-memory parallelism.
 **********************************************************************/

void linearSolvePar(double **A, double *B, double *X, int n)
{
    double **tmp = makeGrid(n);
    cloneGrid(A, tmp, n);
    double detA = calcDet(tmp, n);
    destroyGrid(tmp, n);

    if (detA == 0) return;

    for (int i = 0; i < n; i++)
    {
        if (fork() == 0)   // Child process
        {
            double **local = makeGrid(n);
            cloneGrid(A, local, n);

            swapColumn(local, B, i, n);
            X[i] = calcDet(local, n) / detA;

            destroyGrid(local, n);
            exit(0);
        }
    }

    /* Parent waits for all child processes */
    for (int i = 0; i < n; i++)
        wait(NULL);
}

/**********************************************************************
 * MAIN PERFORMANCE DRIVER
 *
 * Accepts matrix sizes from command line.
 * Runs sequential and parallel solvers.
 * Measures time and writes results to CSV.
 **********************************************************************/

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s size1 size2 size3 ...\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen("results.csv", "w");
    fprintf(fp, "size,seq_time,par_time,speedup\n");

    srand(time(NULL));

    for (int arg = 1; arg < argc; arg++)
    {
        int n = atoi(argv[arg]);
        printf("\nRunning for matrix size %d\n", n);

        double **A = makeGrid(n);
        double *B = malloc(n * sizeof(double));
        double *X = malloc(n * sizeof(double));

        /* Fill matrix with random values */
        for (int i = 0; i < n; i++)
        {
            B[i] = rand() % 10;
            for (int j = 0; j < n; j++)
                A[i][j] = rand() % 10;
        }

        /* Sequential timing */
        clock_t t1 = clock();
        linearSolveSeq(A, B, X, n);
        clock_t t2 = clock();
        double seqTime = (double)(t2 - t1) / CLOCKS_PER_SEC;

        /* Parallel timing */
        t1 = clock();
        linearSolvePar(A, B, X, n);
        t2 = clock();
        double parTime = (double)(t2 - t1) / CLOCKS_PER_SEC;

        double speedup = (parTime > 0) ? seqTime / parTime : 0;

        printf("Seq: %.3f sec | Par: %.3f sec | Speedup: %.2f\n",
               seqTime, parTime, speedup);

        fprintf(fp, "%d,%.5f,%.5f,%.2f\n",
                n, seqTime, parTime, speedup);

        destroyGrid(A, n);
        free(B);
        free(X);
    }

    fclose(fp);
    printf("\nResults saved to results.csv\n");
    return 0;
}
