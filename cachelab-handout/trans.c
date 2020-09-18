/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);


/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    // Split the original matrices into 4x4 blocks
    int sub_n = 0;
    int sub_m = 0;

    int sub_sub_n, sub_sub_m;
    
    int block_size = 4;
    if (N == 32) { block_size = 8; }
    if (N == 61) { block_size = 16; }
    int sub_block_size;
    int n_sub_blocks ;
    int n_b = N / block_size;
    int m_b = M / block_size;
    int tmp;
    int row_remainder = N % block_size;
    int column_remainder = M % block_size;
    if (row_remainder > 0){ n_b += 1;}
    if (column_remainder > 0){ m_b += 1; }


    sub_block_size = block_size / 2;
    n_sub_blocks= block_size / sub_block_size;


    for (int i = 0; i < n_b; i++){
        sub_n = i*block_size;
        for (int j = 0; j < m_b; j++){
            sub_m = j*block_size;

            // Specific optimization for diagnal access
            if (i == j){
                for (int iii = 0; iii < n_sub_blocks; iii++){
                    sub_sub_n = iii * sub_block_size + sub_n;
                    for (int jjj = 0; jjj < n_sub_blocks; jjj++){
                        sub_sub_m = jjj*sub_block_size + sub_m;

                        for (int iiii = sub_sub_n; iiii < sub_sub_n + sub_block_size; iiii++){
                            for (int jjjj = sub_sub_m; jjjj < sub_sub_m + sub_block_size; jjjj++){
                                tmp = A[iiii][jjjj];
                                B[jjjj][iiii] = tmp;
                            }
                        }
                    }
                }
            }

            else{
                for (int ii = sub_n; ii < sub_n + ((i < n_b - 1) || (row_remainder == 0) ? block_size: row_remainder); ii++){
                    for (int jj = sub_m; jj < sub_m + ((j < m_b - 1) || (column_remainder == 0)?block_size:column_remainder); jj++){
                        tmp = A[ii][jj];
                        B[jj][ii] = tmp;
                    }
                }
            }

        } 
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;
    
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    
}

char trans_c_desc[] = "Simple columnwise scan transpose";
void trans_c(int M, int N, int A[N][M], int B[M][N]){
    int i, j, tmp;
    for (j = 0; j < M; j++){
        for (i = 0; i < N; i++){
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}


/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

    registerTransFunction(trans_c, trans_c_desc);

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

