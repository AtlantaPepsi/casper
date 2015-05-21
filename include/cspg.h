#ifndef CSPG_H_
#define CSPG_H_

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "csp.h"
#include "hash_table.h"

/* #define CSP_G_DEBUG */
#ifdef CSP_G_DEBUG
#define CSP_G_DBG_PRINT(str, ...) do { \
    fprintf(stdout, "[CSP-H][N-%d]"str, CSP_MY_NODE_ID, ## __VA_ARGS__); fflush(stdout); \
    } while (0)
#else
#define CSP_G_DBG_PRINT(str, ...) {}
#endif

#define CSP_G_ERR_PRINT(str...) do {fprintf(stderr, str);fflush(stdout);} while (0)

#define CSP_G_assert(EXPR) do { if (unlikely(!(EXPR))){ \
            CSP_G_ERR_PRINT("[CSP-H][N-%d, %d]  assert fail in [%s:%d]: \"%s\"\n", \
                    CSP_MY_NODE_ID, CSP_MY_RANK_IN_WORLD, __FILE__, __LINE__, #EXPR); \
            PMPI_Abort(MPI_COMM_WORLD, -1); \
        }} while (0)

typedef struct CSP_G_win {
    MPI_Aint *user_base_addrs_in_local;

    /* communicator including root user processes and all ghosts,
     * used for internal information exchange between users and ghosts */
    MPI_Comm ur_g_comm;

    /* communicator including local processes and ghosts */
    MPI_Comm local_ug_comm;
    MPI_Win local_ug_win;
    int max_local_user_nprocs;

    /* communicator including all the user processes and ghosts */
    MPI_Comm ug_comm;

    void *base;
    MPI_Win *ug_wins;
    int num_ug_wins;

    MPI_Win active_win;

    struct CSP_Win_info_args info_args;
    unsigned long csp_g_win_handle;
} CSP_G_win;

extern hashtable_t *csp_g_win_ht;
#define CSP_G_WIN_HT_SIZE 256

typedef struct CSP_G_func_info {
    CSP_Func_info info;
    int user_root_in_local;
} CSP_G_func_info;

static inline int csp_get_g_win(unsigned long key, CSP_G_win ** win)
{
    *win = (CSP_G_win *) ht_get(csp_g_win_ht, (ht_key_t) key);
    return 0;
}

static inline int csp_put_g_win(unsigned long key, CSP_G_win * win)
{
    return ht_set(csp_g_win_ht, (ht_key_t) key, win);
}

static inline int csp_remove_g_win(unsigned long key)
{
    return ht_remove(csp_g_win_ht, (ht_key_t) key);
}

static inline int csp_init_g_win_table()
{
    csp_g_win_ht = ht_create(CSP_G_WIN_HT_SIZE);

    if (csp_g_win_ht == NULL)
        return -1;

    return 0;
}

static inline void destroy_csp_g_win_table()
{
    return ht_destroy(csp_g_win_ht);
}

extern int CSP_G_win_allocate(int user_local_root, int user_nprocs, int user_local_nprocs);
extern int CSP_G_win_free(int user_local_root, int user_nprocs, int user_local_nprocs);

extern int CSP_G_finalize(void);

extern int CSP_G_func_start(CSP_Func * FUNC, int *user_local_root, int *user_nprocs,
                               int *user_local_nprocs);
extern int CSP_G_func_new_ur_g_comm(int user_local_root, MPI_Comm * ur_g_comm);
extern int CSP_G_func_get_param(char *func_params, int size, MPI_Comm ur_g_comm);

#endif /* CSPG_H_ */