/*****************************************************************************************
 * PROGRAM: Performance Analysis of Cramer's Rule (Sequential vs Parallel using fork)
 *
 * OBJECTIVE:
 * This program solves a system of linear equations AX = B using Cramer's Rule and
 * compares the execution time of:
 *      1) Sequential computation
 *      2) Parallel computation using multiple processes (fork)
 *
 * The performance results are written into a CSV file for analysis and graph plotting.
 *
 * USAGE:
 *      ./final size1 size2 size3 ...
 * Example:
 *      ./final 200 400 600 800
 *
 * OUTPUT:
 *      results.csv → Contains:
 *          Matrix Size, Sequential Time, Parallel Time, Speedup
 *
 * NOTE:
 * Cramer's Rule has very high computational complexity (≈ O(n⁴)).
 * This program is meant for OS process parallelism performance study,
 * not for practical large-scale linear equation solving.
 *****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

/*****************************************************************************************
 * MATRIX MEMORY MANAGEMENT
 * These helper functions dynamically allocate, copy, and free matrices.
 *****************************************************************************************/

/* Dynamically allocate an n × n matrix */
double **makeGrid(int dim)
{
    double **grid = malloc(dim * sizeof(double *));
    for (int i = 0; i < dim; i++)
        grid[i] = malloc(dim * sizeof(double));
    return grid;
}

/* Free memory allocated to a matrix */
void destroyGrid(double **grid, int dim)
{
    for (int i = 0; i < dim; i++)
        free(grid[i]);
    free(grid);
}

/* Copy matrix src → dest */
void cloneGrid(double **src, double **dest, int dim)
{
    for (int i = 0; i < dim; i++)
        for (int j = 0; j < dim; j++)
            dest[i][j] = src[i][j];
}

/* Replace a column of matrix with vector B (Used in Cramer's Rule) */
void swapColumn(double **grid, double *vec, int colIndex, int dim)
{
    for (int i = 0; i < dim; i++)
        grid[i][colIndex] = vec[i];
}

/*****************************************************************************************
 * DETERMINANT CALCULATION
 *
 * Uses Gaussian Elimination to convert the matrix into upper triangular form.
 * Determinant = product of diagonal elements.
 *
 * Time Complexity: O(n³)
 *****************************************************************************************/
double calcDet(double **grid, int dim)
{
    double result = 1.0;

    for (int i = 0; i < dim; i++)
    {
        /* If pivot element is near zero, determinant becomes zero */
        if (fabs(grid[i][i]) < 1e-9)
            return 0;

        /* Eliminate elements below pivot */
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

/*****************************************************************************************
 * SEQUENTIAL CRAMER SOLVER
 *
 * Steps:
 * 1. Compute determinant of original matrix A
 * 2. For each variable Xi:
 *      - Replace column i of A with vector B
 *      - Compute determinant of modified matrix
 *      - Xi = det(Ai) / det(A)
 *****************************************************************************************/
void linearSolveSeq(double **A, double *B, double *X, int n)
{
    double **tmp = makeGrid(n);
    cloneGrid(A, tmp, n);

    double detA = calcDet(tmp, n);
    destroyGrid(tmp, n);

    if (detA == 0) return;  // No unique solution

    for (int i = 0; i < n; i++)
    {
        tmp = makeGrid(n);
        cloneGrid(A, tmp, n);

        swapColumn(tmp, B, i, n);
        X[i] = calcDet(tmp, n) / detA;

        destroyGrid(tmp, n);
    }
}

/*****************************************************************************************
 * PARALLEL CRAMER SOLVER (PROCESS-BASED PARALLELISM)
 *
 * fork() creates child processes.
 * Each child computes one variable Xi independently.
 *
 * Important Concept:
 * fork() creates separate memory spaces, so this demonstrates
 * process-level parallelism rather than shared-memory parallelism.
 *****************************************************************************************/
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
            exit(0);  // Child exits after its computation
        }
    }

    /* Parent waits for all children to finish */
    for (int i = 0; i < n; i++)
        wait(NULL);
}

/*****************************************************************************************
 * MAIN FUNCTION — PERFORMANCE DRIVER
 *
 * Accepts multiple matrix sizes from command line,
 * runs both sequential and parallel solvers,
 * measures execution time, and writes results to CSV.
 *****************************************************************************************/
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s size1 size2 size3 ...\n", argv[0]);
        return 1;
    }

    /* Open CSV file in current working directory */
    FILE *fp = fopen("results.csv", "w");
    if (!fp)
    {
        perror("Error opening results.csv");
        return 1;
    }

    fprintf(fp, "size,seq_time,par_time,speedup\n");
    fflush(fp);  // Flush header before any fork occurs

    srand(time(NULL));  // Seed random generator

    for (int arg = 1; arg < argc; arg++)
    {
        int n = atoi(argv[arg]);
        printf("\nRunning for matrix size %d\n", n);

        /* Allocate matrix and vectors */
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

        fflush(fp);  // Flush before fork to avoid duplicate buffer writes

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
        fflush(fp);  // Ensure data is written safely

        destroyGrid(A, n);
        free(B);
        free(X);
    }

    fclose(fp);
    printf("\nResults saved to results.csv\n");
    return 0;
}
