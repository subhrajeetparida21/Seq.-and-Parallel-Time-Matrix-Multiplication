PROJECT TITLE:
Performance Analysis of Sequential vs Parallel Cramer's Rule
Operating Systems Laboratory Project (Roll Numbers 41–50)

------------------------------------------------------------
PROGRAM EXECUTION MANUAL
------------------------------------------------------------

There are two ways to run the program.

METHOD 1 — AI_Code Version (Recommended)

This version runs multiple matrix sizes automatically
and stores performance results in a CSV file.

Compilation:
gcc project2_Human.c -o AI_Code

Execution:
./AI_Code 200 400 600 800 1000 1200 1400 1600

Each number represents a matrix dimension.

Output File Generated:
results.csv

CSV Format:
size,seq_time,par_time,speedup

------------------------------------------------------------

METHOD 2 — Basic GCC Execution

This method runs the program normally without CSV logging.

Compilation:
gcc project2_AI.c

Execution:
./a.out

The program will ask for matrix size manually.

------------------------------------------------------------
PROJECT OVERVIEW
------------------------------------------------------------

This project studies process-based parallelism in Operating Systems
by comparing Sequential Cramer's Rule and Parallel Cramer's Rule
using fork().

The system solved is:

AX = B

Where:
A = Coefficient matrix
B = Constant vector
X = Solution vector

The goal is to measure execution time and analyze performance
improvement due to parallel execution.

------------------------------------------------------------
CODE STRUCTURE EXPLANATION
------------------------------------------------------------

1. MATRIX MEMORY SECTION
   makeGrid()     → Allocates n×n matrix
   destroyGrid()  → Frees matrix memory
   cloneGrid()    → Copies matrix data
   swapColumn()   → Replaces a column for Cramer's Rule

2. DETERMINANT SECTION
   Uses Gaussian Elimination to compute determinant.
   Matrix is converted into upper triangular form.
   Determinant = Product of diagonal elements.
   Time Complexity = O(n³)

3. SEQUENTIAL SOLVER
   Step-by-step Cramer's Rule:
   - Compute det(A)
   - Replace column i with B
   - Compute det(Ai)
   - Xi = det(Ai) / det(A)

4. PARALLEL SOLVER
   Uses fork() system call.
   Each child process computes one Xi.
   Parent process waits using wait().
   Demonstrates process-level parallelism.

5. PERFORMANCE DRIVER (MAIN)
   - Accepts multiple matrix sizes
   - Measures execution time
   - Calculates speedup
   - Stores results in CSV

------------------------------------------------------------
OUTPUT DESCRIPTION
------------------------------------------------------------

Program prints:
- Sequential execution time
- Parallel execution time
- Speedup ratio

------------------------------------------------------------
LEARNING OUTCOMES
------------------------------------------------------------

- Process creation using fork()
- Parallel vs sequential performance
- System call overhead
- CPU utilization in multi-process execution
- Performance benchmarking

------------------------------------------------------------
IMPORTANT NOTE
------------------------------------------------------------

Cramer's Rule has time complexity O(n⁴).
This project focuses on Operating Systems parallelism study,
not optimal numerical solving.

------------------------------------------------------------
END OF README
------------------------------------------------------------
