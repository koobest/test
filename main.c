#include<stdio.h>

int start_bit = 0;

unsigned char buf[12];

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

int u(int bits, uint8_t *pbuf, uint32_t *start_bit)
{
    return 0;
}

int se(uint8_t *pbuf, int bits, uint32_t *start_bit)
{
    return 0;
}

int ue(uint8_t *pbuf, int bits, uint32_t *start_bit)
{
    return 0;
}

void hrd_parameters(uint8_t *buf, int nlen)
{
    uint32_t start_bit = 0;
    int cpb_cnt_minus1 = ue(buf, nlen, &start_bit);
    int bit_rate_scale = u(4, buf, &start_bit);
    int cpb_size_scale = u(4, buf, &start_bit);
    int *bit_rate_value_minus1 = malloc(sizeof(int)*(cpb_cnt_minus1+1));
    int *cpb_size_value_minus1 = malloc(sizeof(int)*(cpb_cnt_minus1+1));
    int *cbr_flag = malloc(sizeof(int)*(cpb_cnt_minus1+1));
    for (int SchedSelIdx = 0; SchedSelIdx <= cpb_cnt_minus1; SchedSelIdx++) {
        bit_rate_value_minus1[ SchedSelIdx ] = ue(buf, nlen, &start_bit);
        cpb_size_value_minus1[ SchedSelIdx ] = ue(buf, nlen, &start_bit);
        cbr_flag[ SchedSelIdx ] = u(1, buf, &start_bit);
    }
    free(bit_rate_value_minus1);
    free(cpb_size_value_minus1);
    free(cbr_flag);
    int initial_cpb_removal_delay_length_minus1 = u(5, buf, &start_bit);
    int cpb_removal_delay_length_minus1 = u(5, buf, &start_bit);
    int dpb_output_delay_length_minus1 = u(5, buf, &start_bit);
    int time_offset_length = u(5, buf, &start_bit);
}

void vui_parameters(uint8_t *buf, int nlen)
{
    uint32_t start_bit = 0;
    int aspect_ratio_info_present_flag = u(1, buf, &start_bit);
    const int Extended_SAR = 255;
    if (aspect_ratio_info_present_flag) {
        int aspect_ratio_idc = u(8, buf, &start_bit);
        if (aspect_ratio_idc == Extended_SAR ) {
            int sar_width = u(16, buf, &start_bit);
            int sar_height = u(16, buf, &start_bit);
        }
    }
    int overscan_info_present_flag = u(1, buf, &start_bit);
    if (overscan_info_present_flag) {
        int overscan_appropriate_flag = u(1, buf, &start_bit);
    }
    int video_signal_type_present_flag = u(1, buf, &start_bit);
    if (video_signal_type_present_flag) {
        int video_format = u(3, buf, &start_bit);
        int video_full_range_flag = u(1, buf, &start_bit);
        int colour_description_present_flag = u(1, buf, &start_bit);
        if (colour_description_present_flag ) {
            int colour_primaries = u(8, buf, &start_bit);
            int transfer_characteristics = u(8, buf, &start_bit);
            int matrix_coefficients = u(8, buf, &start_bit);
        }
    }
    int chroma_loc_info_present_flag = u(1, buf, &start_bit);
    if (chroma_loc_info_present_flag ) {
        int chroma_sample_loc_type_top_field = ue(buf, nlen, &start_bit);
        int chroma_sample_loc_type_bottom_field = ue(buf, nlen, &start_bit);
    }
    int timing_info_present_flag = u(1, buf, &start_bit);
    if (timing_info_present_flag) {
        int num_units_in_tick = u(32, buf, &start_bit);
        int time_scale = u(32, buf, &start_bit);
        int fixed_frame_rate_flag = u(1, buf, &start_bit);
    }
    int nal_hrd_parameters_present_flag = u(1, buf, &start_bit);
    if (nal_hrd_parameters_present_flag ){
        hrd_parameters(buf, nlen);
    }
    int vcl_hrd_parameters_present_flag = u(1, buf, &start_bit);
    if (vcl_hrd_parameters_present_flag ){
        hrd_parameters(buf, nlen);
    }
    if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag){
        int low_delay_hrd_flag = u(1, buf, &start_bit);
    }
    int pic_struct_present_flag = u(1, buf, &start_bit);
    int bitstream_restriction_flag = u(1, buf, &start_bit);
    if (bitstream_restriction_flag) {
        int motion_vectors_over_pic_boundaries_flag = u(1, buf, &start_bit);
        int max_bytes_per_pic_denom = ue(buf, nlen, &start_bit);
        int max_bits_per_mb_denom = ue(buf, nlen, &start_bit);
        int log2_max_mv_length_horizontal = ue(buf, nlen, &start_bit);
        int log2_max_mv_length_vertical = ue(buf, nlen, &start_bit);
        int max_num_reorder_frames = ue(buf, nlen, &start_bit);
        int max_dec_frame_buffering = ue(buf, nlen, &start_bit);
    }
}

void scaling_list(uint8_t *buf, int nlen, int *scalingList, int sizeOfScalingList, int *useDefaultScalingMatrixFlag)
{
    uint32_t start_bit = 0;
    int lastScale = 8;
    int nextScale = 8;
    for(int j = 0; j < sizeOfScalingList; j++ ) {
        if( nextScale != 0 ) {
            int delta_scale = se(buf, nlen, &start_bit);
            nextScale = ( lastScale + delta_scale + 256 ) % 256;
            *useDefaultScalingMatrixFlag = (j == 0 && nextScale == 0);
        }
        scalingList[j] = (nextScale == 0 ) ? lastScale : nextScale;
        lastScale = scalingList[j];
    }
}

void seq_parameter_set_data(uint8_t *buf, int nlen)
{
    int ScalingList4x4[16];
    int UseDefaultScalingMatrix4x4Flag[16];
    int ScalingList8x8[64];
    int UseDefaultScalingMatrix8x8Flag[64];
    uint32_t start_bit = 0;
    int profile_idc = u(8, buf, &start_bit);
    int constraint_set0_flag = u(1, buf, &start_bit);
    int constraint_set1_flag = u(1, buf, &start_bit);
    int constraint_set2_flag = u(1, buf, &start_bit);
    int constraint_set3_flag = u(1, buf, &start_bit);
    int constraint_set4_flag = u(1, buf, &start_bit);
    int constraint_set5_flag = u(1, buf, &start_bit);
    int reserved_zero_2bits  = u(2, buf, &start_bit);
    int level_idc = u(8, buf, &start_bit);
    int seq_parameter_set_id = ue(buf, nlen, &start_bit);
    if (profile_idc == 100 || profile_idc == 110 || profile_idc == 122 ||
        profile_idc == 244 || profile_idc == 44  || profile_idc == 83  ||
        profile_idc == 86  || profile_idc == 118 || profile_idc == 128 ||
        profile_idc == 138 || profile_idc == 139 || profile_idc == 134 || profile_idc == 135) {
        int chroma_format_idc = ue(buf, nlen, &start_bit);
        if (chroma_format_idc == 3){
            int separate_colour_plane_flag = u(1, buf, &start_bit);
        }
        int bit_depth_luma_minus8 = ue(buf, nlen, &start_bit);
        int bit_depth_chroma_minus8 = ue(buf, nlen, &start_bit);
        int qpprime_y_zero_transform_bypass_flag = u(1, buf, &start_bit);
        int seq_scaling_matrix_present_flag = u(1, buf, &start_bit);
        if (seq_scaling_matrix_present_flag ) {
            int seq_scaling_list_present_flag[8];
            for (int i = 0; i < ((chroma_format_idc!= 3 ) ? 8 : 12 ); i++) {
                seq_scaling_list_present_flag[i] = u(1, buf, &start_bit);
                if (seq_scaling_list_present_flag[i]) {
                    if( i < 6 ){
                        scaling_list(buf, nlen, ScalingList4x4[i], 16, UseDefaultScalingMatrix4x4Flag[i]);
                    } else {
                        scaling_list(buf, nlen, ScalingList8x8[i-6 ], 64,UseDefaultScalingMatrix8x8Flag[i-6]);
                    }
                }
            }
        }
    }
    int log2_max_frame_num_minus4 = ue(buf, nlen, &start_bit);
    int pic_order_cnt_type = ue(buf, nlen, &start_bit);
    if (pic_order_cnt_type == 0) {
        int log2_max_pic_order_cnt_lsb_minus4 = ue(buf, nlen, &start_bit);
    } else if (pic_order_cnt_type == 1) {
        int delta_pic_order_always_zero_flag = u(1, buf, &start_bit);
        int offset_for_non_ref_pic = se(buf, nlen, &start_bit);
        int offset_for_top_to_bottom_field = se(buf, nlen, &start_bit);
        int num_ref_frames_in_pic_order_cnt_cycle = ue(buf, nlen, &start_bit);
        int *offset_for_ref_frame = malloc(sizeof(int)*num_ref_frames_in_pic_order_cnt_cycle);
        for(int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++){
            offset_for_ref_frame[i] = se(buf, nlen, &start_bit);
        }
        free(offset_for_ref_frame);
    }
    int max_num_ref_frames = ue(buf, nlen, &start_bit);
    int gaps_in_frame_num_value_allowed_flag = u(1, buf, &start_bit);
    int pic_width_in_mbs_minus1 = ue(buf, nlen, &start_bit);
    int pic_height_in_map_units_minus1 = ue(buf, nlen, &start_bit);
    int frame_mbs_only_flag = u(1, buf, &start_bit);
    if (!frame_mbs_only_flag) {
        int mb_adaptive_frame_field_flag = u(1, buf, &start_bit);
    }
    int direct_8x8_inference_flag = u(1, buf, &start_bit);
    int frame_cropping_flag = u(1, buf, &start_bit);
    if (frame_cropping_flag) {
        int frame_crop_left_offset = ue(buf, nlen, &start_bit);
        int frame_crop_right_offset = ue(buf, nlen, &start_bit);
        int frame_crop_top_offset = ue(buf, nlen, &start_bit);
        int frame_crop_bottom_offset = ue(buf, nlen, &start_bit);
    }
    int vui_parameters_present_flag = u(1, buf, &start_bit);
    if (vui_parameters_present_flag) {
        vui_parameters(buf, nlen);
    }
}

int more_rbsp_data(void)
{
    return 0;
}

void pic_parameter_set_rbsp(uint8_t *buf, int nlen)
{
    int chroma_format_idc = 3;
    uint32_t start_bit = 0;
    int ScalingList4x4[16];
    int ScalingList8x8[64];
    int UseDefaultScalingMatrix4x4Flag[16];
    int UseDefaultScalingMatrix8x8Flag[64];
    int pic_parameter_set_id = ue(buf, nlen, &start_bit);
    int seq_parameter_set_id = ue(buf, nlen, &start_bit);
    int entropy_coding_mode_flag = u(1, buf, &start_bit);
    int bottom_field_pic_order_in_frame_present_flag = u(1, buf, &start_bit);
    int num_slice_groups_minus1 = ue(buf, nlen, &start_bit);
    if (num_slice_groups_minus1 > 0 ) {
        int slice_group_map_type = ue(buf, nlen, &start_bit);
        if (slice_group_map_type == 0) {
            int *run_length_minus1 = malloc(sizeof(int)*(num_slice_groups_minus1+1));
            for (int iGroup = 0; iGroup <= num_slice_groups_minus1; iGroup++) {
                run_length_minus1[iGroup] = ue(buf, nlen, &start_bit);
            }
            free(run_length_minus1);
        } else if (slice_group_map_type == 2) {
            int *top_left = malloc(sizeof(int)*num_slice_groups_minus1);
            int *bottom_right = malloc(sizeof(int)*num_slice_groups_minus1);
            for (int iGroup = 0; iGroup < num_slice_groups_minus1; iGroup++) {
                top_left[ iGroup ] = ue(buf, nlen, &start_bit);
                bottom_right[ iGroup ] = ue(buf, nlen, &start_bit);
            }
            free(top_left);
            free(bottom_right);
        } else if( slice_group_map_type == 3 || slice_group_map_type == 4 || slice_group_map_type == 5 ) {
            int slice_group_change_direction_flag = u(1, buf, &start_bit);
            int slice_group_change_rate_minus1 = ue(buf, nlen, &start_bit);
        } else if( slice_group_map_type == 6 ) {
            int pic_size_in_map_units_minus1 = ue(buf, nlen, &start_bit);
            int *slice_group_id = malloc(sizeof(int)*(pic_size_in_map_units_minus1+1));
            for(int i = 0; i <= pic_size_in_map_units_minus1; i++ ){
                slice_group_id[i] = u(nlen, buf, &start_bit);
            }
            free(slice_group_id);
        }
    }
    int num_ref_idx_l0_default_active_minus1 = ue(buf, nlen, &start_bit);
    int num_ref_idx_l1_default_active_minus1 = ue(buf, nlen, &start_bit);
    int weighted_pred_flag = u(1, buf, &start_bit);
    int weighted_bipred_idc = u(2, buf, &start_bit);
    int pic_init_qp_minus26 = se(buf, nlen, &start_bit);
    int pic_init_qs_minus26 = se(buf, nlen, &start_bit);
    int chroma_qp_index_offset = se(buf, nlen, &start_bit);
    int deblocking_filter_control_present_flag = u(1, buf, &start_bit);
    int constrained_intra_pred_flag = u(1, buf, &start_bit);
    int redundant_pic_cnt_present_flag = u(1, buf, &start_bit);
    if (more_rbsp_data()) {
        int transform_8x8_mode_flag = u(1, buf, &start_bit);
        int pic_scaling_matrix_present_flag = u(1, buf, &start_bit);
        if (pic_scaling_matrix_present_flag ) {
            int pic_scaling_list_present_flag[12];
            for (int i = 0; i < 6 + ((chroma_format_idc != 3 ) ? 2 : 6 )*transform_8x8_mode_flag; i++ ) {
                pic_scaling_list_present_flag[i] = u(1, buf, &start_bit);
                if (pic_scaling_list_present_flag[i]) {
                    if( i < 6 ) {
                        scaling_list(buf, nlen, ScalingList4x4[i], 16, UseDefaultScalingMatrix4x4Flag[i]);
                    } else {
                        scaling_list(buf, nlen, ScalingList8x8[i-6], 64, UseDefaultScalingMatrix8x8Flag[i-6 ]);
                    }
                }
            }
        }
        int second_chroma_qp_index_offset = se(buf, nlen, &start_bit);
    }
}


int main(void)
{
;
}
