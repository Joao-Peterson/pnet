struct pnet_header_t{
    char magic[0x04];
    u16 version;
    u8 valid;
    u8 matrix_size;
    u32 size;
    be u32 crc32;
    u32 num_places;
    u32 num_transitions;
    u32 num_inputs;
    u32 num_outputs;
};

struct pnet_matrix_t{
    u32 size;
    if(size > 0){
        u32 x;
        u32 y;
        s32 matrix[size/4-2];
    }
};

struct pnet_t{
    pnet_header_t header;
    pnet_matrix_t neg_arcs_map;
    pnet_matrix_t pos_arcs_map;
    pnet_matrix_t inhibit_arcs_map;
    pnet_matrix_t reset_arcs_map;
    pnet_matrix_t places_init;
    pnet_matrix_t transitions_delay;
    pnet_matrix_t inputs_map;
    pnet_matrix_t outputs_map;
    pnet_matrix_t places;
    pnet_matrix_t sensitive_transitions;
    pnet_matrix_t outputs;
    pnet_matrix_t inputs_last;
};

pnet_t pnet @ 0x00;