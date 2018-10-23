#include "ru.h"

/******************************************************************************
 * Chip: XRDP_
 ******************************************************************************/
const ru_block_rec *RU_ALL_BLOCKS[] =
{
    &QM_BLOCK,
    &DQM_BLOCK,
    &RNR_MEM_BLOCK,
    &RNR_INST_BLOCK,
    &RNR_CNTXT_BLOCK,
    &RNR_PRED_BLOCK,
    &RNR_REGS_BLOCK,
    &RNR_QUAD_BLOCK,
    &PSRAM_BLOCK,
    &PSRAM_MEM_BLOCK,
    &FPM_BLOCK,
    &DSPTCHR_BLOCK,
    &UBUS_MSTR_BLOCK,
    &UBUS_SLV_BLOCK,
    &XLIF_RX_IF_BLOCK,
    &XLIF_RX_FLOW_CONTROL_BLOCK,
    &XLIF_TX_IF_BLOCK,
    &XLIF_TX_FLOW_CONTROL_BLOCK,
    &DEBUG_BUS_BLOCK,
    &XLIF_EEE_BLOCK,
    &SBPM_BLOCK,
    &DMA_BLOCK,
    &TCAM_BLOCK,
    &HASH_BLOCK,
    &BAC_IF_BLOCK,
    &CNPL_BLOCK,
    &NATC_BLOCK,
    &NATC_ENG_BLOCK,
    &NATC_CFG_BLOCK,
    &NATC_CTRS_BLOCK,
    &NATC_KEY_MASK_BLOCK,
    &NATC_DDR_CFG_BLOCK,
    &NATC_INDIR_BLOCK,
    &BBH_TX_BLOCK,
    &BBH_RX_BLOCK,
    NULL
};

/* End of file XRDP_.c */
