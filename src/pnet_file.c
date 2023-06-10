#include "pnet.h"
#include "pnet_error_priv.h"
#include "crc32.h"

#define PNET_FILE_SERIALIZED_MATRICES_QTY 12

/**
 * @brief version for the pnet file
 */
typedef enum{
    pnet_file_version_first         = 0x0001   
}pnet_file_version_t;

/**
 * @brief header for the pnet file
 */
#pragma pack(push,1)
typedef struct{
    char magic[4];                                                                  /**< magic file number. Always: "PNET" */
    uint16_t version;                                                               /**< version, for future proofing and conversion handling */
    uint8_t valid;                                                                  /**< true if pnet passed the pnet_check() validation call */
    uint8_t matrix_size;                                                            /**< size for the serialized matrices. Ex: 32, 16 (bits) */
    uint32_t size;                                                                  /**< totol size of the matrices data, everything after the header, num_outputs */
    uint32_t crc32;                                                                 /**< crc32 checksum */
    uint32_t num_places;                                                            /**< number of places */
    uint32_t num_transitions;                                                       /**< number of transitions */
    uint32_t num_inputs;                                                            /**< number of inputs */
    uint32_t num_outputs;                                                           /**< number of outputs */

    uint32_t neg_arcs_map_size;                                                     /**< size for first matrix on the pnet */
    uint8_t neg_arcs_map_first_byte;                                                /**< first byte of the serialized matrix */
    /**
     * ...
     * neg_arcs_map
     * pos_arcs_map
     * inhibit_arcs_map
     * reset_arcs_map
     * places_init
     * transitions_delay
     * inputs_map
     * outputs_map
     * places
     * sensitive_transitions
     * outputs
     * inputs_last
    */
}pnet_file_header_t;
#pragma pack(pop)

void *pnet_serialize(pnet_t *pnet, size_t *size){
    if(pnet == NULL){
        pnet_set_error(pnet_error_pnet_struct_pointer_passed_as_argument_is_null);
        if(size != NULL)
            *size = 0;
        return NULL;
    }
    
    size_t data_size = 0;
    uint8_t *data;
    
    pnet_matrix_t *matrices[PNET_FILE_SERIALIZED_MATRICES_QTY] = {                  // array of matrices to be serialized
        pnet->neg_arcs_map,
        pnet->pos_arcs_map,
        pnet->inhibit_arcs_map,
        pnet->reset_arcs_map,
        pnet->places_init,
        pnet->transitions_delay,
        pnet->inputs_map,
        pnet->outputs_map,
        pnet->places,
        pnet->sensitive_transitions,
        pnet->outputs,
        pnet->inputs_last
    };

    void *m_serials[PNET_FILE_SERIALIZED_MATRICES_QTY];
    size_t m_sizes[PNET_FILE_SERIALIZED_MATRICES_QTY];

    for(int i = 0; i < PNET_FILE_SERIALIZED_MATRICES_QTY; i++){                     // serialize every matrix
        if(matrices[i] == NULL){
            data_size += sizeof(uint32_t);
            m_serials[i] = NULL;
            continue;
        }
        
        size_t msize = 0;
        void *m = pnet_matrix_serialize(matrices[i], &msize);

        if(m == NULL){
            for(int z = 0; z < PNET_FILE_SERIALIZED_MATRICES_QTY; z++){             // on error free all
                if(m_serials[z] == NULL)
                    break;

                free(m_serials[z]);
            }

            if(size != NULL)
                *size = 0;
            return NULL;
        }

        data_size += sizeof(uint32_t) + msize;                                      // add to total size
        m_sizes[i] = msize;                                                         // save serialized and size
        m_serials[i] = m;
    }

    data = calloc(data_size + sizeof(pnet_file_header_t), 1);                       // allocate memory for the file

    pnet_file_header_t *header = (pnet_file_header_t*)data;                         // write data to memory as pnet_file_header
    header->num_places      = pnet->num_places;
    header->num_transitions = pnet->num_transitions;
    header->num_inputs      = pnet->num_inputs;
    header->num_outputs     = pnet->num_outputs;
    header->valid           = pnet->valid;
    header->matrix_size     = 32;
    header->version         = (uint16_t)pnet_file_version_first;
    header->size            = data_size;
    memcpy(header->magic, "PNET", 4);

    uint32_t *cursor = &(header->neg_arcs_map_size); 

    for(int i = 0; i < PNET_FILE_SERIALIZED_MATRICES_QTY; i++){                     // run through saved serialized matrices and copy to file
        if(m_serials[i] == NULL){                                                   // null serialize means empty matrix
            cursor[0] = 0;
            cursor++;
        }
        else{
            cursor[0] = m_sizes[i];                                                 // save size
            memcpy(&(cursor[1]), m_serials[i], m_sizes[i]);                         // save serialized matrix
            cursor += (m_sizes[i] / sizeof(*cursor)) + 1;                           // place cursor after saved matrix 
            free(m_serials[i]);
        }
    }

    header->crc32 = crc32((uint8_t*)&(header->neg_arcs_map_size), data_size, 0xFFFFFFFF);    // crc32

    if(size != NULL)                                                                // final size is all matrix data + header size (minus first neg_arcs size and first byte)
        *size = (data_size + sizeof(pnet_file_header_t) - sizeof(uint32_t) - sizeof(uint8_t));

    if(!pnet->valid)
        pnet_set_error(pnet_info_pnet_not_valid_to_serialize);
    else
        pnet_set_error(pnet_info_ok);

    return (void*)data;
}

pnet_t *pnet_deserialize(void *data, size_t size, pnet_callback_t callback, void *callback_data){
    if(data == NULL || size < sizeof(pnet_file_header_t)) return NULL;

    pnet_file_header_t *header = (pnet_file_header_t*)data;

    if(memcmp(header->magic, "PNET", 4)){                                           // check magic
        pnet_set_error(pnet_error_file_invalid_filetype);
        return NULL;
    }

    uint32_t crc = crc32((uint8_t*)&(header->neg_arcs_map_size), header->size, 0xFFFFFFFF);

    if(header->crc32 != crc){                                                       // check crc32
        pnet_set_error(pnet_error_file_invalid_checksum);
        return NULL;
    }

    pnet_matrix_t *matrices[PNET_FILE_SERIALIZED_MATRICES_QTY] = {0};
    uint32_t *cursor = &(header->neg_arcs_map_size);

    for(int i = 0; i < PNET_FILE_SERIALIZED_MATRICES_QTY; i++){                     // read every matrix
        uint32_t matrix_size = *cursor;
        cursor++;

        if(matrix_size == 0){                                                       // if empty matrix
            matrices[i] = NULL;    
            continue;
        }

        matrices[i] = pnet_matrix_deserialize((void*)cursor, matrix_size);

        if(matrices[i] == NULL){                                                    // on error
            pnet_set_error(pnet_error_file_corrupted_data);
            return NULL;
        }

        cursor += matrix_size / sizeof(uint32_t);                                   // jump to next matrix

        if(cursor >= &(header->neg_arcs_map_size) + size) break;                    // exit after the end of the file
    }

    pnet_t *pnet = m_pnet_new(                                                      // create a new one
        matrices[0],
        matrices[1],
        matrices[2],
        matrices[3],
        matrices[4],
        matrices[5],
        matrices[6],
        matrices[7],
        callback,
        callback_data
    );

    if(pnet == NULL) return NULL;                                                   // on error

    if(matrices[8] != NULL){                                                        // write internal state only if not null
        pnet_matrix_delete(pnet->places);
        pnet->places = matrices[8];
    }
    if(matrices[9] != NULL){
        pnet_matrix_delete(pnet->sensitive_transitions);
        pnet->sensitive_transitions = matrices[9];
    }
    if(matrices[10] != NULL){
        pnet_matrix_delete(pnet->outputs);
        pnet->outputs = matrices[10];
    }
    if(matrices[11] != NULL){
        pnet_matrix_delete(pnet->inputs_last);
        pnet->inputs_last = matrices[11];
    }

    pnet_set_error(pnet_info_ok);
    return pnet;
}

void pnet_save(pnet_t *pnet, char *filename){
    size_t size;
    void *data = pnet_serialize(pnet, &size);
    if(data == NULL) return;
    FILE *file = fopen(filename, "w+b");
    
    if(file == NULL){
        free(data);
        return;
    }

    fwrite(data, size, 1, file);
    fclose(file);
    free(data);
}

void *filetomem(char *filename, size_t *filesize){
    FILE *file = fopen(filename, "r+b");
    if(file == NULL) return NULL;

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t *buffer = malloc(size + 1);
    fread(buffer, size, 1, file);

    buffer[size] = 0;

    if(filesize != NULL)
        *filesize = size;
    return buffer;
}

pnet_t *pnet_load(char *filename, pnet_callback_t callback, void *callback_data){
    size_t size = 0;
    void *file = filetomem(filename, &size);

    if(file == NULL || size == 0){
        pnet_set_error(pnet_error_file_corrupted_data);

        if(file != NULL)
            free(file);
            
        return NULL;    
    }

    pnet_t *pnet = pnet_deserialize(file, size, callback, callback_data);

    free(file);
    return pnet;
}