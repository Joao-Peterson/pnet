#include "pnet_matrix.h"
#include "pnet_error_priv.h"
#include "pnet.h"

pnet_matrix_t *v_pnet_matrix_new(size_t x, size_t y, va_list *args){
    pnet_matrix_t *matrix = (pnet_matrix_t*)calloc(1, sizeof(pnet_matrix_t));
    matrix->x = x;
    matrix->y = y;
    matrix->m = (int**)calloc(y, sizeof(int*));

    for(size_t i = 0; i < y; i++){
        matrix->m[i] = (int*)calloc(x, sizeof(int));
        for(size_t j = 0; j < x; j++){
            matrix->m[i][j] = va_arg(*args, int); 
        }
    }

    pnet_set_error(pnet_info_ok);
    return matrix;
}

pnet_matrix_t *pnet_matrix_new(size_t x, size_t y, ...){
    va_list args;
    va_start(args, y);

    pnet_matrix_t *matrix = v_pnet_matrix_new(x, y, &args);

    va_end(args);

    pnet_set_error(pnet_info_ok);
    return matrix;
}

int pnet_matrix_get(pnet_matrix_t *m, size_t x, size_t y){
    if(m == NULL){
        pnet_set_error(pnet_error_matrix_passed_is_null);
        return 0;
    };

    if(x >= m->x || y >= m->y){
        pnet_set_error(pnet_error_matrix_index_x_y_out_of_range);
        return 0;
    };

    pnet_set_error(pnet_info_ok);
    return m->m[y][x];
}

void pnet_matrix_set(pnet_matrix_t *m, size_t x, size_t y, int value){
    if(m == NULL){
        pnet_set_error(pnet_error_matrix_passed_is_null);
        return;
    };

    if(x >= m->x || y >= m->y){
        pnet_set_error(pnet_error_matrix_index_x_y_out_of_range);
        return;
    };

    pnet_set_error(pnet_info_ok);
    m->m[y][x] = value;
}

void pnet_matrix_modify(pnet_matrix_t *m, size_t new_x, size_t new_y){
    if(m == NULL){
        pnet_set_error(pnet_error_matrix_passed_is_null);
        return;
    };
    size_t max_row = (m->y > new_y) ? m->y : new_y;
    size_t min_row = (m->y < new_y) ? m->y : new_y;
    size_t min_col = (m->x < new_x) ? m->x : new_x;

    // realloc remaining rows
    for(size_t i = 0; i < min_row-1; i++){
        int *tmp_cols = realloc(m->m[i], new_x * sizeof(int));
        free(m->m[i]);
        m->m[i] = tmp_cols;
        m->x = new_x;
        
        // zero created columns
        for(size_t j = min_col; j < m->x-1; j++){
            m->m[i][j] = 0;
        }
    }

    // less rows
    if(new_y < m->y){
        // delete exceding rows
        for(size_t i = min_row; i < max_row-1; i++){
            free(m->m[i]);
        }
        
        m->y = new_y;

        // shrink matrix rows count
        int **tmp_rows = realloc(m->m, m->y * sizeof(int*));
        free(m->m);
        m->m = tmp_rows;
    }
    // more rows
    else if(new_y > m->y){
        m->y = new_y;

        // expand matrix rows count
        int **tmp_rows = realloc(m->m, m->y * sizeof(int*));
        free(m->m);
        m->m = tmp_rows;

        // create new rows
        for(size_t i = min_row; i < max_row-1; i++){
            m->m[i] = calloc(m->x, sizeof(int));
        }
    }

    pnet_set_error(pnet_info_ok);
}

pnet_matrix_t *pnet_matrix_new_zero(size_t x, size_t y){
    pnet_matrix_t *matrix = (pnet_matrix_t*)calloc(1, sizeof(pnet_matrix_t));
    matrix->x = x;
    matrix->y = y;
    matrix->m = (int**)calloc(y, sizeof(int*));

    for(size_t i = 0; i < y; i++){
        matrix->m[i] = (int*)calloc(x, sizeof(int));
        for(size_t j = 0; j < x; j++){
            matrix->m[i][j] = 0; 
        }
    }

    pnet_set_error(pnet_info_ok);
    return matrix;
}

void pnet_matrix_delete(pnet_matrix_t *matrix){
    if(matrix == NULL)
        return;
    
    for(size_t i = 0; i < matrix->y; i++)
        free(matrix->m[i]);

    free(matrix->m);
    free(matrix);
}

void pnet_matrix_print(pnet_matrix_t *matrix, char *name){
    if(matrix == NULL){
        pnet_set_error(pnet_error_matrix_passed_is_null);
        return;
    };

    printf("\"%s\" =\n", name);
    for(size_t i = 0; i < matrix->y; i++){
        printf("[");
        for(size_t j = 0; j < matrix->x; j++){
            printf(" %i", matrix->m[i][j]);        
        }
        printf(" ]\n");
    }
}

pnet_matrix_t *pnet_matrix_mul(pnet_matrix_t *a, pnet_matrix_t *b){
    if(a == NULL || b == NULL){
        pnet_set_error(pnet_error_matrices_should_be_tranposed_equivalents);
        return NULL;
    };

    if(a->x != b->y){
        pnet_set_error(pnet_error_matrices_should_be_of_the_same_size);
        return NULL;
    }

    pnet_matrix_t *m = (pnet_matrix_t*)calloc(1, sizeof(pnet_matrix_t));
    m->x = b->x;
    m->y = a->y;
    m->m = (int**)calloc(m->y, sizeof(int*));

    for(size_t i = 0; i < m->y; i++){
        m->m[i] = (int*)calloc(m->x, sizeof(int));
        for(size_t j = 0; j < m->x; j++){
            int acc = 0;
            
            for(size_t x = 0; x < a->x; x++){
                acc += a->m[i][x] * b->m[x][j];
            }

            m->m[i][j] = acc; 
        }
    }

    pnet_set_error(pnet_info_ok);
    return m;
}

pnet_matrix_t *pnet_matrix_mul_by_element(pnet_matrix_t *a, pnet_matrix_t *b){
    if(a == NULL || b == NULL){
        pnet_set_error(pnet_error_matrix_passed_is_null);
        return NULL;
    };

    if(a->x != b->x || a->y != b->y){ 
        pnet_set_error(pnet_error_matrices_should_be_of_the_same_size);
        return NULL;
    }

    pnet_matrix_t *m = (pnet_matrix_t*)calloc(1, sizeof(pnet_matrix_t));
    m->x = a->x;
    m->y = a->y;
    m->m = (int**)calloc(m->y, sizeof(int*));

    for (size_t i = 0; i < m->y; i++){
        m->m[i] = (int*)calloc(m->x, sizeof(int));
        for (size_t j = 0; j < m->x; j++){
            m->m[i][j] = a->m[i][j] * b->m[i][j];
        }
    }
    
    pnet_set_error(pnet_info_ok);
    return m;
}

pnet_matrix_t *pnet_matrix_mul_scalar(pnet_matrix_t *a, int c){
    if(a == NULL){
        pnet_set_error(pnet_error_matrix_passed_is_null);
        return NULL;
    };

    pnet_matrix_t *m = (pnet_matrix_t*)calloc(1, sizeof(pnet_matrix_t));
    m->x = a->x;
    m->y = a->y;
    m->m = (int**)calloc(m->y, sizeof(int*));

    for(size_t i = 0; i < m->y; i++){
        m->m[i] = (int*)calloc(m->x, sizeof(int));
        for(size_t j = 0; j < m->x; j++){
            m->m[i][j] = m->m[i][j] * c; 
        }
    }

    pnet_set_error(pnet_info_ok);
    return m;
}

pnet_matrix_t *pnet_matrix_add(pnet_matrix_t *a, pnet_matrix_t *b){
    if(a == NULL || b == NULL){
        pnet_set_error(pnet_error_matrix_passed_is_null);
        return NULL;
    };

    if(a->x != b->x || a->y != b->y){ 
        pnet_set_error(pnet_error_matrices_should_be_of_the_same_size);
        return NULL;
    }

    pnet_matrix_t *m = (pnet_matrix_t*)calloc(1, sizeof(pnet_matrix_t));
    m->x = a->x;
    m->y = a->y;
    m->m = (int**)calloc(m->y, sizeof(int*));

    for (size_t i = 0; i < m->y; i++){
        m->m[i] = (int*)calloc(m->x, sizeof(int));
        for (size_t j = 0; j < m->x; j++){
            m->m[i][j] = a->m[i][j] + b->m[i][j];
        }
    }

    pnet_set_error(pnet_info_ok);    
    return m;
}

pnet_matrix_t *pnet_matrix_and(pnet_matrix_t *a, pnet_matrix_t *b){
    if(a == NULL || b == NULL){
        pnet_set_error(pnet_error_matrix_passed_is_null);
        return NULL;
    };

    if(a->x != b->x || a->y != b->y){ 
        pnet_set_error(pnet_error_matrices_should_be_of_the_same_size);
        return NULL;
    }

    pnet_matrix_t *m = (pnet_matrix_t*)calloc(1, sizeof(pnet_matrix_t));
    m->x = a->x;
    m->y = a->y;
    m->m = (int**)calloc(m->y, sizeof(int*));

    for (size_t i = 0; i < m->y; i++){
        m->m[i] = (int*)calloc(m->x, sizeof(int));
        for (size_t j = 0; j < m->x; j++){
            m->m[i][j] = a->m[i][j] & b->m[i][j];
        }
    }

    pnet_set_error(pnet_info_ok);    
    return m;
}

pnet_matrix_t *pnet_matrix_neg(pnet_matrix_t *a){
    if(a == NULL){
        pnet_set_error(pnet_error_matrix_passed_is_null);
        return NULL;
    };

    pnet_matrix_t *m = (pnet_matrix_t*)calloc(1, sizeof(pnet_matrix_t));
    m->x = a->x;
    m->y = a->y;
    m->m = (int**)calloc(m->y, sizeof(int*));

    for (size_t i = 0; i < m->y; i++){
        m->m[i] = (int*)calloc(m->x, sizeof(int));
        for (size_t j = 0; j < m->x; j++){
            m->m[i][j] = !a->m[i][j];
        }
    }
    
    pnet_set_error(pnet_info_ok);    
    return m;
}

pnet_matrix_t *pnet_matrix_duplicate(pnet_matrix_t *a){
    if(a == NULL){
        pnet_set_error(pnet_error_matrix_passed_is_null);
        return NULL;
    };
    pnet_matrix_t *m = (pnet_matrix_t*)calloc(1, sizeof(pnet_matrix_t));
    m->x = a->x;
    m->y = a->y;
    m->m = (int**)calloc(m->y, sizeof(int*));

    for (size_t i = 0; i < m->y; i++){
        m->m[i] = (int*)calloc(m->x, sizeof(int));
        for (size_t j = 0; j < m->x; j++){
            m->m[i][j] = a->m[i][j];
        }
    }
    
    pnet_set_error(pnet_info_ok);    
    return m;
}

void pnet_matrix_copy(pnet_matrix_t *dest, pnet_matrix_t *src){
    if(dest == NULL || src == NULL){
        pnet_set_error(pnet_error_matrix_passed_is_null);
        return;
    };

    if(dest->x != src->x || dest->y != src->y){ 
        pnet_set_error(pnet_error_matrices_should_be_of_the_same_size);
        return;
    }

    
    for (size_t i = 0; i < dest->y; i++){
        for (size_t j = 0; j < dest->x; j++){
            dest->m[i][j] = src->m[i][j];
        }
    }

    pnet_set_error(pnet_info_ok);
}

pnet_matrix_t *pnet_matrix_transpose(pnet_matrix_t *matrix){
    if(matrix == NULL){
        pnet_set_error(pnet_error_matrix_passed_is_null);
        return NULL;
    };

    pnet_matrix_t *m = (pnet_matrix_t*)calloc(1, sizeof(pnet_matrix_t));
    m->x = matrix->y;
    m->y = matrix->x;
    m->m = (int**)calloc(m->y, sizeof(int*));

    for(size_t i = 0; i < m->y; i++){
        m->m[i] = (int*)calloc(m->x, sizeof(int));
        for(size_t j = 0; j < m->x; j++){
            m->m[i][j] = matrix->m[j][i]; 
        }
    }

    pnet_set_error(pnet_info_ok);
    return m;
}

bool pnet_matrix_cmp_eq(pnet_matrix_t *a, pnet_matrix_t *b){
    if(a == NULL || b == NULL){
        pnet_set_error(pnet_error_matrix_passed_is_null);
        return false;
    };

    if(a->x != b->x || a->y != b->y){ 
        pnet_set_error(pnet_error_matrices_should_be_of_the_same_size);
        return false;
    }

    bool res = true;

    for (size_t i = 0; i < a->y; i++){
        for (size_t j = 0; j < a->x; j++){
            if(a->m[i][j] != b->m[i][j])
                res = false;
        }
    }

    pnet_set_error(pnet_info_ok);
    return res;
}

void pnet_matrix_set_all(pnet_matrix_t *m, int number){
    if(m == NULL){
        pnet_set_error(pnet_error_matrix_passed_is_null);
        return;
    };

    for(size_t i = 0; i < m->y; i++){
        for(size_t j = 0; j < m->x; j++){
            m->m[i][j] = number;
        }
    }

    pnet_set_error(pnet_info_ok);
}

pnet_matrix_t *pnet_matrix_extract_col(pnet_matrix_t *m, size_t x){
    if(m == NULL){
        pnet_set_error(pnet_error_matrix_passed_is_null);
        return NULL;
    }

    if(x >= m->x){
        pnet_set_error(pnet_error_matrix_index_x_y_out_of_range);
        return NULL;
    }

    pnet_matrix_t *ret = pnet_matrix_new_zero(m->y, 1);

    for(size_t i = 0; i < m->y; i++){
        ret->m[0][i] = m->m[i][x];
    }

    pnet_set_error(pnet_info_ok);
    return ret;
}