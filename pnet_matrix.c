#include "pnet_matrix.h"

// matrix_string_t *v_matrix_string_new(size_t x, size_t y, va_list *args){
//     if(x == 0 || y == 0) return NULL;
    
//     matrix_string_t *matrix = (matrix_string_t*)calloc(1, sizeof(matrix_string_t));
//     matrix->x = x;
//     matrix->y = y;
//     matrix->m = (char***)calloc(y, sizeof(char**));

//     for(size_t i = 0; i < y; i++){
//         matrix->m[i] = (char**)calloc(x, sizeof(char*));
//         for(size_t j = 0; j < x; j++){
//             matrix->m[i][j] = va_arg(*args, char*); 
//         }
//     }

//     return matrix;
// }

// matrix_string_t *matrix_string_new(size_t x, size_t y, ...){
//     va_list args;
//     va_start(args, y);

//     matrix_string_t *matrix = v_matrix_string_new(x, y, &args);

//     va_end(args);

//     return matrix;
// }

// void matrix_string_delete(matrix_string_t *matrix){
//     for(size_t i = 0; i < matrix->y; i++)
//         free(matrix->m[i]);

//     free(matrix->m);
//     free(matrix);    
// }

// void matrix_string_print(matrix_string_t *matrix, char *name){
//     printf("\"%s\" =\n", name);
//     for(size_t i = 0; i < matrix->y; i++){
//         printf("[");
//         for(size_t j = 0; j < matrix->x; j++){
//             printf(" \"%s\"", matrix->m[i][j]);        
//         }
//         printf(" ]\n");
//     }
// }

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

    return matrix;
}

pnet_matrix_t *pnet_matrix_new(size_t x, size_t y, ...){
    va_list args;
    va_start(args, y);

    pnet_matrix_t *matrix = v_pnet_matrix_new(x, y, &args);

    va_end(args);

    return matrix;
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

    return matrix;
}

void pnet_matrix_delete(pnet_matrix_t *matrix){
    if(matrix == NULL) return;
    
    for(size_t i = 0; i < matrix->y; i++)
        free(matrix->m[i]);

    free(matrix->m);
    free(matrix);
}

void pnet_matrix_print(pnet_matrix_t *matrix, char *name){
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
    if(a->x != b->y)
        return NULL;

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

    return m;
}

pnet_matrix_t *pnet_matrix_mul_by_element(pnet_matrix_t *a, pnet_matrix_t *b){
    if(a->x != b->x || a->y != b->y) return NULL;

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
    
    return m;
}

pnet_matrix_t *pnet_matrix_mul_scalar(pnet_matrix_t *a, int c){
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

    return m;
}

pnet_matrix_t *pnet_matrix_add(pnet_matrix_t *a, pnet_matrix_t *b){
    if(a->x != b->x || a->y != b->y) return NULL;

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
    
    return m;
}

pnet_matrix_t *pnet_matrix_and(pnet_matrix_t *a, pnet_matrix_t *b){
    if(a->x != b->x || a->y != b->y) return NULL;

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
    
    return m;
}

pnet_matrix_t *pnet_matrix_neg(pnet_matrix_t *a){
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
    
    return m;
}

pnet_matrix_t *pnet_matrix_duplicate(pnet_matrix_t *a){
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
    
    return m;
}

void pnet_matrix_copy(pnet_matrix_t *dest, pnet_matrix_t *src){
    if(dest->x != src->x || dest->y != src->y) return;
    
    for (size_t i = 0; i < dest->y; i++){
        for (size_t j = 0; j < dest->x; j++){
            dest->m[i][j] = src->m[i][j];
        }
    }
}

pnet_matrix_t *pnet_matrix_transpose(pnet_matrix_t *matrix){
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

    return m;
}

bool pnet_matrix_cmp_eq(pnet_matrix_t *a, pnet_matrix_t *b){
    if(a->x != b->x || a->y != b->y) return false;
    bool res = true;

    for (size_t i = 0; i < a->y; i++){
        for (size_t j = 0; j < a->x; j++){
            if(a->m[i][j] != b->m[i][j])
                res = false;
        }
    }

    return res;
}

void pnet_matrix_set(pnet_matrix_t *m, int number){
    for(size_t i = 0; i < m->y; i++){
        for(size_t j = 0; j < m->x; j++){
            m->m[i][j] = number;
        }
    }
}