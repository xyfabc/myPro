/*
 *  linux/drivers/mmc/host/jz_mmc/msc/jz_mmc_msc.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#include <linux/dma-mapping.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/host.h>
#include <linux/scatterlist.h>

#include <asm/jzsoc.h>
#include "include/jz_mmc_msc.h"
#include "include/jz_mmc_pio.h"

#define MSC_STAT_ERR_BITS 0x3f
#define WAITMASK							\
	(MSC_STAT_CRC_RES_ERR |						\
	 MSC_STAT_CRC_READ_ERROR | MSC_STAT_CRC_WRITE_ERROR_MASK |	\
	 MSC_STAT_TIME_OUT_RES | MSC_STAT_TIME_OUT_READ)

#if 1

static int jzmmc_trace_level = 0;
static int jzmmc_trace_cmd_code = -1;
static int jzmmc_trace_data_len = -1;
static int jzmmc_trace_id = 0;
module_param(jzmmc_trace_level, int, 0644);
module_param(jzmmc_trace_cmd_code, int, 0644);
module_param(jzmmc_trace_data_len, int, 0644);
module_param(jzmmc_trace_id, int, 0644);

#define TRACE_CMD_REQ()							\
	({								\
	 if ( (jzmmc_trace_id & (1 << host->pdev_id)) && (jzmmc_trace_level & 0x1)) \
	 if ( (jzmmc_trace_cmd_code == -1) || (jzmmc_trace_cmd_code == cmd->opcode) ) \
	 printk("%s:     execute_cmd: opcode = %d cmdat = %#0x arg = %#0x data_flags = %#0x\n", \
		 mmc_hostname(host->mmc), cmd->opcode, REG_MSC_CMDAT(host->pdev_id), REG_MSC_ARG(host->pdev_id), \
		 host->curr_mrq->data ? host->curr_mrq->data->flags : 0); \
	 })

#define TRACE_CMD_RES()							\
	({								\
	 if ( (jzmmc_trace_id & (1 << host->pdev_id)) && (jzmmc_trace_level & 0x1)) \
	 if ( (jzmmc_trace_cmd_code == -1) || (jzmmc_trace_cmd_code == cmd->opcode) ) \
	 printk("%s:     cmd done: curr_res_type = %d resp[0] = %#0x err = %d state = %#0x\n", \
		 mmc_hostname(host->mmc), host->curr_res_type, cmd->resp[0], cmd->error, \
		 REG_MSC_STAT(host->pdev_id));	\
	 })

#define TRACE_DATA_REQ()						\
	({								\
	 if ((jzmmc_trace_id & (1 << host->pdev_id)) && (jzmmc_trace_level & 0x2) && host->curr_mrq->data ) { \
	 if ((jzmmc_trace_data_len == -1) ||		\
		 (jzmmc_trace_data_len == host->curr_mrq->data->blksz * host->curr_mrq->data->blocks) ) \
	 printk("%s:     blksz %d blocks %d flags %08x "	\
		 "tsac %d ms nsac %d\n",		\
		 mmc_hostname(host->mmc), host->curr_mrq->data->blksz, \
		 host->curr_mrq->data->blocks, host->curr_mrq->data->flags, \
		 host->curr_mrq->data->timeout_ns / 1000000, \
		 host->curr_mrq->data->timeout_clks); \
	 }							\
	 })

#define TRACE_DATA_DONE()						\
	({								\
	 if ((jzmmc_trace_id & (1 << host->pdev_id)) && (jzmmc_trace_level & 0x2)) \
	 if ((jzmmc_trace_data_len == -1) ||		\
		 (jzmmc_trace_data_len == data->blksz * data->blocks) ) \
	 printk("%s:     stat = 0x%08x error = %d bytes_xfered = %d stop = %p\n", \
		 mmc_hostname(host->mmc), stat, data->error, \
		 data->bytes_xfered, host->curr_mrq->stop); \
	 })

#define JZ_MMC_P_REG_BY_ID(reg_name, id) \
	printk("" #reg_name "(%d) = 0x%08x\n", id, reg_name(id))

void jz_mmc_dump_regs(int msc_id, int line) {
	printk("***** msc%d regs, line = %d *****\n", msc_id, line);

	JZ_MMC_P_REG_BY_ID(REG_MSC_STRPCL, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_STAT, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_CLKRT, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_CMDAT, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_RESTO, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_RDTO, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_BLKLEN, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_NOB, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_SNOB, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_IMASK, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_IREG, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_CMD, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_ARG, msc_id);
	//JZ_MMC_P_REG_BY_ID(REG_MSC_RES, msc_id);
	//JZ_MMC_P_REG_BY_ID(REG_MSC_RXFIFO, msc_id);
	//JZ_MMC_P_REG_BY_ID(REG_MSC_TXFIFO, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_LPM, msc_id);
#if defined(CONFIG_SOC_JZ4780) || defined(CONFIG_SOC_JZ4775)
	JZ_MMC_P_REG_BY_ID(REG_MSC_DMAC, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_DMANDA, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_DMADA, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_DMALEN, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_DMACMD, msc_id);
	JZ_MMC_P_REG_BY_ID(REG_MSC_RTCNT, msc_id);
#endif	
}
EXPORT_SYMBOL(jz_mmc_dump_regs);

#else
#define TRACE_CMD_REQ() do {  } while(0)
#define TRACE_CMD_RES() do {  } while(0)
#define TRACE_DATA_REQ() do {  } while(0)
#define TRACE_DATA_DONE() do {  } while(0)
#define jz_mmc_dump_regs(__mid, __ln) do {  } while(0)
#endif

#define SET_UNSTUFF_1BIT(resp,start,val)			\
	({							\
	 const int __off = 3 - ((start) / 32);		\
	 const int __shft = (start) & 31;		\
	 \
	 if (val)					\
	 (resp)[__off] |= (1 << __shft);		\
	 else						\
	 (resp)[__off] &= ~(1 << __shft);	\
	 })

void jz_mmc_set_clock(struct jz_mmc_host *host, int rate);
static int jz_mmc_data_done(struct jz_mmc_host *host);

static void msc_irq_mask_all(int msc_id)
{
	REG_MSC_IMASK(msc_id) = 0xffffffff;
	REG_MSC_IREG(msc_id) = 0xffffffff;
}

void jz_mmc_reset(struct jz_mmc_host *host)
{
	u32 clkrt = REG_MSC_CLKRT(host->pdev_id);

	//	while (REG_MSC_STAT(host->pdev_id) & MSC_STAT_CLK_EN);

	//	printk("b reset\n");
	REG_MSC_STRPCL(host->pdev_id) = MSC_STRPCL_RESET;
	while (REG_MSC_STAT(host->pdev_id) & MSC_STAT_IS_RESETTING);
	//	printk("a reset\n");
	/*need a delay */
	schedule_timeout(10); /* 100ms */

	// __msc_start_clk(host->pdev_id);
	REG_MSC_LPM(host->pdev_id) = 0x1;	// Low power mode
	msc_irq_mask_all(host->pdev_id);

	REG_MSC_RDTO(host->pdev_id) = 0xfffff;
	REG_MSC_RESTO(host->pdev_id) = 0xff;

	REG_MSC_CLKRT(host->pdev_id) = clkrt;

#ifdef CONFIG_JZ_SDMA
	REG_MSC_DMAC(host->pdev_id) &= ~(MSC_DMAC_SEL_COMMON);  
#endif
#ifdef CONFIG_JZ_CDMA
	REG_MSC_DMAC(host->pdev_id) |= (MSC_DMAC_SEL_COMMON | MSC_DMAC_EN);
#endif
}

static inline int msc_calc_clkrt(int clk_src, u32 rate)
{
	int i;

#define JZ_MMC_MAX_CLKRT	7

	for (i = 0; i <= JZ_MMC_MAX_CLKRT; i++) {
		if ((clk_src >> i) <= rate)
			break;
	}

	//printk("===>clk_src = %d, clkrt = %d, actual_clk = %d\n",
	//       clk_src, i, clk_src / (1 << i));

	return i;
}

#if defined(CONFIG_SOC_JZ4750) || defined(CONFIG_SOC_JZ4750D)
void jz_mmc_set_clock(struct jz_mmc_host *host, int rate) {
	int clkrt;

	if (rate > SD_CLOCK_24M) {
		rate = SD_CLOCK_24M;
		__cpm_select_msc_clk_high(host->pdev_id,1);	/* select clock source from CPM */

		// __cpm_select_msc_clk(host->pdev_id,1);	/* select clock source from CPM */
		clkrt = msc_calc_clkrt(24000000, rate);
	} else {
		__cpm_select_msc_clk(host->pdev_id,1);	/* select clock source from CPM */
		clkrt = msc_calc_clkrt(24000000, rate);
	}

	REG_MSC_CLKRT(host->pdev_id) = clkrt;
}
#endif

#if defined(CONFIG_SOC_JZ4760) || defined(CONFIG_SOC_JZ4760B)
void jz_mmc_set_clock(struct jz_mmc_host *host, int rate) {
	int clkrt;

	/*
	 * NOTE: we do not support >24MHZ on JZ4760(B)
	 * add this check in case users who defined SD_CLOCK_MAX > 24MHZ
	 */
	if (rate > SD_CLOCK_24M)
		rate = SD_CLOCK_24M;

	cpm_set_clock(CGU_MSCCLK, SD_CLOCK_24M);
	clkrt = msc_calc_clkrt(cpm_get_clock(CGU_MSCCLK), rate);
	REG_MSC_CLKRT(host->pdev_id) = clkrt;
}
#endif

#if defined(CONFIG_SOC_JZ4770) || defined(CONFIG_SOC_JZ4780) || defined(CONFIG_SOC_JZ4775)
void jz_mmc_set_clock(struct jz_mmc_host *host, int rate)
{
	int clkrt;
	int clk_src;
	cgu_clock cgu_clk;
	unsigned int msc_cdr, src;

	switch (host->pdev_id) {
		case 0:
			cgu_clk = CGU_MSC0CLK;
			break;
		case 1:
			cgu_clk = CGU_MSC1CLK;
			break;
		case 2:
			cgu_clk = CGU_MSC2CLK;
			break;
		default:
			panic("%s: no existing MMC host(%d)\n", __func__, host->pdev_id);
	}

	if (rate > SD_CLOCK_24M) {

		clk_src = SD_CLOCK_MAX;
		if(SD_CLOCK_MAX > 24000000){
			REG_MSC_LPM(host->pdev_id) |= (0x2 << 30 | 1 << 29);	// send cmd and data at clock rising
		}else{
			REG_MSC_LPM(host->pdev_id) &= ~(0x3 << 30);	// send cmd and data at clock falling
		}
	} else{
		clk_src = SD_CLOCK_24M;
		REG_MSC_LPM(host->pdev_id) &= ~(0x3 << 30);	// send cmd and data at clock falling
	}


	//	cpm_set_clock(cgu_clk, clk_src);
#if 0
	printk("id = %d, set rat = %d\n", host->pdev_id, rate);
	msc_cdr = (1 << 31) | (1 << 29);
	msc_cdr |= (400 * 1000 * 1000) / 24000000 / 2 - 1;
	printk("DEBUG: old msc_cdr = %x\n", msc_cdr);

#else
	if (host->pdev_id == 0)
		msc_cdr = (2 << 30) | (1 << 29);
	else
		msc_cdr = (1 << 29);
//	msc_cdr |= (400 * 1000000) % (24000000 / 2) ? (400 * 1000000) / 24000000 / 2 : (400 * 1000000) / 24000000 / 2 - 1;
	src = cpm_get_clock(CGU_CCLK);
	msc_cdr |= src % (24000000 / 2) ? src / 24000000 / 2 : src / 24000000 / 2 - 1;
//	msc_cdr |= (1 << 15);

//	REG_MSC_LPM(host->pdev_id) |= (0x2 << 30 | 1 << 29);	// send cmd and data at clock rising

	if(host->pdev_id == 0){
		REG_CPM_MSC0CDR = msc_cdr;
		while (REG_CPM_MSC0CDR & (1 << 28)) ;
	}else if(host->pdev_id == 1){
		REG_CPM_MSC1CDR = msc_cdr;
		while (REG_CPM_MSC1CDR & (1 << 28)) ;
	}else{
		REG_CPM_MSC2CDR = msc_cdr;
		while (REG_CPM_MSC2CDR & (1 << 28)) ;
	}

	clkrt = msc_calc_clkrt(24000000, rate);
	REG_MSC_CLKRT(host->pdev_id) = clkrt;
//	printk("rate = %d\n", rate);
#endif
}
#endif

#if defined(CONFIG_FPGA)
void jz_mmc_set_clock(struct jz_mmc_host *host, int rate)
{
	if(rate <= 400000)
		REG_MSC_CLKRT(host->pdev_id) = 7;
	else
		REG_MSC_CLKRT(host->pdev_id) = 4;
}
#endif

static void jz_mmc_enable_irq(struct jz_mmc_host *host, unsigned int mask)
{
	REG_MSC_IMASK(host->pdev_id) &= ~mask;
}

static void jz_mmc_disable_irq(struct jz_mmc_host *host, unsigned int mask)
{
	REG_MSC_IMASK(host->pdev_id) |= mask;
}

static int jz_mmc_parse_cmd_response(struct jz_mmc_host *host, unsigned int stat)
{
	struct mmc_command *cmd = host->curr_mrq->cmd;
	int i, temp[16] = {0};
	unsigned char *buf;
	unsigned int res, v, w1, w2;

	if (!cmd)
		return -EINVAL;

	/* NOTE: we must flush the FIFO,  despite of fail or success*/
	buf = (u8 *) temp;
	switch (host->curr_res_type) {
		case 1:
			{
				/*
				 * Did I mention this is Sick.  We always need to
				 * discard the upper 8 bits of the first 16-bit word.
				 */

				res = REG_MSC_RES(host->pdev_id);
				buf[0] = (res >> 8) & 0xff;
				buf[1] = res & 0xff;

				res = REG_MSC_RES(host->pdev_id);
				buf[2] = (res >> 8) & 0xff;
				buf[3] = res & 0xff;

				res = REG_MSC_RES(host->pdev_id);
				buf[4] = res & 0xff;

				cmd->resp[0] =
					buf[1] << 24 | buf[2] << 16 | buf[3] << 8 |
					buf[4];

				// printk("opcode = %d, cmd->resp = 0x%08x\n", cmd->opcode, cmd->resp[0]);
				break;
			}
		case 2:
			{
				res = REG_MSC_RES(host->pdev_id);
				v = res & 0xffff;
				for (i = 0; i < 4; i++) {
					res = REG_MSC_RES(host->pdev_id);
					w1 = res & 0xffff;
					res = REG_MSC_RES(host->pdev_id);
					w2 = res & 0xffff;
					cmd->resp[i] = v << 24 | w1 << 8 | w2 >> 8;
					v = w2;
				}
				break;
			}
		case 0:
			break;
	}

#if 0
	if (cmd->opcode == MMC_SEND_CSD) {
		/* force read_partial to 0 */
		SET_UNSTUFF_1BIT(cmd->resp, 79, 0);

		/* force write partial to 0 */
		SET_UNSTUFF_1BIT(cmd->resp, 21, 0);
	}
#endif

	if (stat & MSC_STAT_TIME_OUT_RES) {
		/* :-( our customer do not want to see SO MANY timeouts :-(
		   so only CMD5 can return timeout error!!! */

		/*
		 * Note: we can not return timeout when CMD SD_SWITCH or MMC_SWITCH
		 * because we declared that out host->caps support MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA and MMC_CAP_MMC_HIGHSPEED
		 * if in the future some error occured because of this, we must add some code to remember
		 * which mode(SDIO/SD/MMC)  the MSC is in
		 */
		switch(cmd->opcode) {
			case SD_IO_SEND_OP_COND:
				//case SD_SWITCH:
				//case MMC_SWITCH:
			case SD_SEND_IF_COND:
			case MMC_APP_CMD:
				cmd->error = -ETIMEDOUT;
				break;
			default:
				/* silly, isn't it??? */
				printk("jz-msc%d: ignored MSC_STAT_TIME_OUT_RES, cmd=%d\n", host->pdev_id, cmd->opcode);
		}
	} else if (stat & MSC_STAT_CRC_RES_ERR && cmd->flags & MMC_RSP_CRC) {
		printk("jz-msc%d: MSC_STAT_CRC, cmd=%d\n", host->pdev_id, cmd->opcode);
		if (cmd->opcode == MMC_ALL_SEND_CID ||
				cmd->opcode == MMC_SEND_CSD ||
				cmd->opcode == MMC_SEND_CID) {
			/* a bogus CRC error can appear if the msb of
			   the 15 byte response is a one */
			if ((cmd->resp[0] & 0x80000000) == 0)
				cmd->error = -EILSEQ;
		}
	}

	TRACE_CMD_RES();

	return cmd->error;
}

extern void jz_mmc_start_pio(struct jz_mmc_host *host);
#if defined(CONFIG_JZ_SDMA)
extern int jz_mmc_start_sdma(struct jz_mmc_host *host);
#else

#endif
void jz_mmc_data_start(struct jz_mmc_host *host)
{
	struct mmc_data *data = host->curr_mrq->data;
	unsigned int nob = data->blocks;
	unsigned int block_size = data->blksz;

	/* NOTE: this flag is never test! */
	if (data->flags & MMC_DATA_STREAM)
		nob = 0xffff;

	REG_MSC_NOB(host->pdev_id) = nob;
#ifndef CONFIG_JZ_SDMA
	REG_MSC_BLKLEN(host->pdev_id) = (block_size + 3) & (~0x3);
#else
	REG_MSC_BLKLEN(host->pdev_id) = block_size;
#endif

	/* if you want to a little faster under this condition,
	 * use the following code
	 */
#ifndef CONFIG_JZ_SDMA
#ifdef CONFIG_JZ_CDMA
	if (unlikely((block_size & 0x3) || ((nob * block_size) <= 64))) {
//		printk("===>use pio1\n");
		jz_mmc_start_pio(host);
		return;
	}
	if (jz_mmc_start_dma(host) < 0) {
		printk("===>use pio2\n");
		jz_mmc_start_pio(host);
	}
#else
	jz_mmc_start_pio(host);
#endif

#else
	jz_mmc_start_sdma(host);
#endif
}

volatile u32 jz_mmc_junk = 0;
EXPORT_SYMBOL(jz_mmc_junk);

static void jz_mmc_send_stop_cmd(struct jz_mmc_host *host);

void jz_mmc_data_stop(struct jz_mmc_host *host) {
	int junked = 1;

	if (host->transfer_mode == JZ_TRANS_MODE_DMA)
		jz_mmc_stop_dma(host);
	else if (host->transfer_mode == JZ_TRANS_MODE_PIO)
		jz_mmc_stop_pio(host);

	host->transfer_mode = JZ_TRANS_MODE_NULL;

	/* What if the data not arrived imediately? our while exits, but data remain in fifo! */
	while (!(REG_MSC_STAT(host->pdev_id) & MSC_STAT_DATA_FIFO_EMPTY)) {
		if (junked)
			jz_mmc_dump_regs(host->pdev_id, __LINE__);
		junked = 0;
		jz_mmc_junk = REG_MSC_RXFIFO(host->pdev_id);
		printk("warning: fifo not empty when dma stopped!!! junk = 0x%08x\n", jz_mmc_junk);
	}
#if 1
	struct mmc_command stop;
	if (host->curr_mrq->stop == NULL)
		host->curr_mrq->stop = &stop;
	host->curr_mrq->stop->opcode = MMC_STOP_TRANSMISSION;
	host->curr_mrq->stop->arg = 0;
	host->curr_mrq->stop->flags = MMC_RSP_R1B | MMC_CMD_AC;
	jz_mmc_send_stop_cmd(host);
	if (host->curr_mrq->stop == &stop)
		host->curr_mrq->stop = NULL;
#else
	jz_mmc_reset(host);
#endif
}

static int need_wait_prog_done(struct mmc_command *cmd) {
	if (cmd->flags & MMC_RSP_BUSY) {
		return 1;
	} else {
		switch(cmd->opcode) { /* R1b cmds need wait PROG_DONE */
			case 12:
			case 28:
			case 29:
			case 38:
				return 1;
				break;
			default:
				/* do nothing */
				break;
		}
	}

	return 0;
}

static void jz_mmc_set_cmdat(struct jz_mmc_host *host) {
	struct mmc_request *mrq = host->curr_mrq;
	struct mmc_command *cmd = mrq->cmd;
	u32 cmdat;

	cmdat = host->cmdat;
	rmb();
	host->cmdat &= ~MSC_CMDAT_INIT;

	if(mrq->cmd->opcode != 0)
		cmdat &= ~MSC_CMDAT_INIT;

	if(mrq->data) {
		cmdat &= ~MSC_CMDAT_BUSY;

		if ((cmd->opcode == 51) || (cmd->opcode == 8)) {
			cmdat &= ~MSC_CMDAT_BUS_WIDTH_MASK;
			cmdat |= MSC_CMDAT_BUS_WIDTH_1BIT | MSC_CMDAT_DATA_EN;
		} else
			cmdat |= MSC_CMDAT_DATA_EN;

#if defined(CONFIG_SOC_JZ4780) || defined(CONFIG_SOC_JZ4775)
		cmdat |= MSC_CMDAT_DMA_EN;
#endif

		if (mrq->data->flags & MMC_DATA_WRITE)
			cmdat |= MSC_CMDAT_WRITE;

		if (mrq->data->flags & MMC_DATA_STREAM)
			cmdat |= MSC_CMDAT_STREAM_BLOCK;
	}

	if (cmd->flags & MMC_RSP_BUSY)
		cmdat |= MSC_CMDAT_BUSY;

	switch (RSP_TYPE(mmc_resp_type(cmd))) {
		case RSP_TYPE(MMC_RSP_R1):	// r1, r1b, r5, r6, r7
			cmdat |= MSC_CMDAT_RESPONSE_R1;
			host->curr_res_type = 1;
			break;
		case RSP_TYPE(MMC_RSP_R3):	// r3, r4
			cmdat |= MSC_CMDAT_RESPONSE_R3;
			host->curr_res_type = 1;
			break;
		case RSP_TYPE(MMC_RSP_R2):	// r2
			cmdat |= MSC_CMDAT_RESPONSE_R2;
			host->curr_res_type = 2;
			break;
		default:
			break;
	}

	// Multi-read || Multi-write
	//if(cmd->opcode == MMC_READ_MULTIPLE_BLOCK || cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK)
	if (host->curr_mrq->stop)
		cmdat |= MSC_CMDAT_SEND_AS_STOP;

	cmdat |= MSC_CMDAT_RTRG_EQUALT_16 | MSC_CMDAT_TTRG_LESS_16;

	REG_MSC_CMDAT(host->pdev_id) = cmdat;
}

static void jz_mmc_set_cmdarg(struct jz_mmc_host *host) {
	struct mmc_command *cmd = host->curr_mrq->cmd;

	if(host->plat->bus_width == 1) {
		if (cmd->opcode == 6) {
			/* set  1 bit sd card bus*/
			if (cmd->arg == 2) {
				REG_MSC_ARG(host->pdev_id) = 0;
			}

			/* set  1 bit mmc card bus*/
			if (cmd->arg == 0x3b70101) {
				REG_MSC_ARG(host->pdev_id) = 0x3b70001;
			}
		} else
			REG_MSC_ARG(host->pdev_id) = cmd->arg;
	} else if(host->plat->bus_width == 8) {
		if (cmd->opcode == 6) {
			/* set  8 bit mmc card bus*/
			if (cmd->arg == 0x3b70101) {
				REG_MSC_ARG(host->pdev_id) = 0x3b70201;
			} else
				REG_MSC_ARG(host->pdev_id) = cmd->arg;
		} else
			REG_MSC_ARG(host->pdev_id) = cmd->arg;
	} else
		REG_MSC_ARG(host->pdev_id) = cmd->arg;
}

#if 0
static void jz_mmc_status_checker(unsigned long arg) {
	struct jz_mmc_host *host = (struct jz_mmc_host *)arg;

	host->status = REG_MSC_STAT(host->pdev_id);
	if ((host->status & host->st_mask) || (host->eject)) {
		if (host->en_usr_intr)
			wake_up_interruptible(&host->status_check_queue);
		else
			wake_up(&host->status_check_queue);
	} else if ((host->st_check_timeout < 0) ||
			(host->st_check_timeout > host->st_check_interval)) {
		if (host->st_check_timeout < 0)
			host->st_check_timeout -= host->st_check_interval;
		host->status_check_timer.expires = jiffies + host->st_check_interval;
		host->status_check_timer.data = (unsigned long)host;


		add_timer(&host->status_check_timer);
	} else {
		host->st_check_timeout = 0;
		wake_up_interruptible(&host->status_check_queue);
	}
}


/**
 * timeout: -1 for wait forever until contition meet, otherwise the timeout value in jiffies
 * en_usr_intr: if allow user interrupt
 * Warning: if timeout == 0 && en_usr_intr == 0, this will wait forever if the condition never meet
 **/
static u32 jz_mmc_wait_status(struct jz_mmc_host *host, u32 st_mask,
		int timeout, int interval, int en_usr_intr) {
	int ret = 0;

	init_timer(&host->status_check_timer);
	host->status_check_timer.expires = jiffies + interval;
	host->status_check_timer.data = (unsigned long)host;
	host->status = 0;
	host->st_mask = st_mask;
	host->st_check_timeout = timeout;
	host->st_check_interval = interval;
	host->en_usr_intr = en_usr_intr;

	add_timer(&host->status_check_timer);

	if (en_usr_intr)
		ret = wait_event_interruptible(host->status_check_queue,
				(host->status & st_mask) ||
				(host->st_check_timeout == 0) ||
				(host->eject));
	else
		wait_event(host->status_check_queue,
				(host->status & st_mask) ||
				(host->st_check_timeout == 0) ||
				(host->eject));

	/* in case when the condition is meet before wait_event, the timer must del right away */
	del_timer_sync(&host->status_check_timer);
	return ret;
}
#endif

//int wait_cmd_done = 0;
//extern volatile int error_may_happen;

static u32 jz_mmc_wait_cmd_done(struct jz_mmc_host *host) {
	u32 timeout = 0x7fffffff;
	struct mmc_command *cmd = host->curr_mrq->cmd;
	int cmd_succ = 0;
	u32 stat = 0;

#if 0
	/* this may slow down the card response from the usrs' view, but more friendly to other kernel parts */
	jz_mmc_wait_status(host, MSC_STAT_END_CMD_RES | MSC_STAT_TIME_OUT_RES | MSC_STAT_CRC_RES_ERR,
			-1, 1, 0); /* interval: 1jiffie = 10ms */
#else
	//wait_cmd_done = 1;
	while (!(REG_MSC_STAT(host->pdev_id) & (MSC_STAT_END_CMD_RES | MSC_STAT_TIME_OUT_RES | MSC_STAT_CRC_RES_ERR)) &&
			(host->eject == 0)) {
#if 0
		if (error_may_happen)
			jz_mmc_dump_regs(host->pdev_id, __LINE__);
#endif
	}
	//error_may_happen = 0;
	//wait_cmd_done = 0;
#endif

	if (REG_MSC_STAT(host->pdev_id) & MSC_STAT_TIME_OUT_RES){
//		printk("%s, %s, stat:%x\n",__FILE__,__func__,REG_MSC_STAT(host->pdev_id));
		cmd->error = -ETIMEDOUT;
	}
	if (host->eject) {
		/* wait response timeout */
		printk("===>eject!!! state = 0x%08x\n", REG_MSC_STAT(host->pdev_id));
		//while (!(REG_MSC_STAT(host->pdev_id) & (MSC_STAT_END_CMD_RES | MSC_STAT_TIME_OUT_RES | MSC_STAT_CRC_RES_ERR)));
		cmd->error = -ENOMEDIUM;
	}

	/* Check for status, avoid be cleaned by following command*/
	stat = REG_MSC_STAT(host->pdev_id);
	if ((stat & MSC_STAT_END_CMD_RES) &&
			!(stat & (MSC_STAT_TIME_OUT_RES | MSC_STAT_CRC_RES_ERR)))
		cmd_succ = 1;

	REG_MSC_IREG(host->pdev_id) = MSC_IREG_END_CMD_RES;	/* clear irq flag */

	if (cmd_succ && need_wait_prog_done(cmd)) {
		timeout = 0x7fffffff;
		while (--timeout && !(REG_MSC_IREG(host->pdev_id) & MSC_IREG_PRG_DONE) && (host->eject == 0))
			;

		stat |= (REG_MSC_STAT(host->pdev_id) & MSC_STAT_ERR_BITS);
		REG_MSC_IREG(host->pdev_id) = MSC_IREG_PRG_DONE;	/* clear status */
		if ((timeout == 0) || (host->eject)) {
			cmd->error = -ETIMEDOUT;
			printk("JZ-MSC%d: wait prog_done error when execute_cmd!, state = 0x%08x\n", host->pdev_id, stat);
		}
	}

	return stat;
}

#if 1
static void jz_mmc_send_stop_cmd(struct jz_mmc_host *host) {
	struct mmc_command *stop_cmd = host->curr_mrq->stop;

	REG_MSC_CMD(host->pdev_id) = stop_cmd->opcode;
	REG_MSC_ARG(host->pdev_id) = stop_cmd->arg;

	REG_MSC_CMDAT(host->pdev_id) = MSC_CMDAT_BUSY | MSC_CMDAT_RESPONSE_R1;

	REG_MSC_RESTO(host->pdev_id) = 0xff;

	REG_MSC_STRPCL(host->pdev_id) |= MSC_STRPCL_START_OP;

	/* Becarefull, maybe endless */
	while(!(REG_MSC_STAT(host->pdev_id) & (MSC_STAT_PRG_DONE | MSC_STAT_ERR_BITS)) &&
			!host->eject) ;

	if (REG_MSC_STAT(host->pdev_id) | MSC_STAT_ERR_BITS)
		stop_cmd->error = -ETIMEDOUT;

	REG_MSC_IREG(host->pdev_id) = MSC_IREG_PRG_DONE;
}
#endif

static int jz_mmc_data_done(struct jz_mmc_host *host)
{
	struct mmc_data *data = host->curr_mrq->data;
	int stat = 0;
	u32 timeout = 0x7fffffff;

	/* tran_done in back of in dma_end */
	while(!((REG_MSC_STAT(host->pdev_id) & MSC_STAT_DATA_TRAN_DONE) || (host->eject)));
	stat = REG_MSC_STAT(host->pdev_id);
#ifdef CONFIG_JZ_SDMA
	if (!(stat & MSC_STAT_DMAEND)) {
		printk("\n=========> CODE bug, dma no end when write operate, stat = %x\n", stat);
		printk("opcode = %d\n", host->curr_mrq->cmd->opcode);
		jz_mmc_dump_regs(host->pdev_id, __LINE__);
		while (1) ;
	}	
#endif

	stat = REG_MSC_STAT(host->pdev_id);
	/* clear status */
	REG_MSC_IREG(host->pdev_id) = MSC_IREG_DATA_TRAN_DONE;
#ifdef CONFIG_JZ_SDMA
	REG_MSC_IREG(host->pdev_id) = MSC_IREG_DMAEND;
#endif

	if (host->curr_mrq && (host->curr_mrq->data->flags & MMC_DATA_WRITE)) {
		while (--timeout && !(REG_MSC_IREG(host->pdev_id) & MSC_IREG_PRG_DONE))
			;
		if (timeout == 0) {
			/* FIXME: aha, we never see this situation happen, what can we do if it happened???
			 * block.c will send cmd13??? */
			//host->curr.mrq->cmd->error = -ETIMEDOUT;
			printk(KERN_ERR"PRG_DONE not done!!!\n");
		}
		stat |= REG_MSC_STAT(host->pdev_id);
		REG_MSC_IREG(host->pdev_id) = MSC_IREG_PRG_DONE;	/* clear status */
	}

#ifndef CONFIG_HIGHMEM
	if (host->transfer_mode == JZ_TRANS_MODE_DMA)
		dma_unmap_sg(mmc_dev(host->mmc), data->sg, host->dma.len,
				host->dma.dir);
#else
	// should we invalid cache here when read?
#endif

	if (stat & MSC_STAT_TIME_OUT_READ) {
		printk("MMC/SD/SDIO timeout, MMC_STAT 0x%x opcode = %d data flags = 0x%0x blocks = %d blksz = %d\n",
				stat,
				host->curr_mrq? host->curr_mrq->cmd->opcode : -1,
				data->flags,
				data->blocks,
				data->blksz);
		data->error = -ETIMEDOUT;
	} else if (stat & (MSC_STAT_CRC_READ_ERROR | MSC_STAT_CRC_WRITE_ERROR | MSC_STAT_CRC_WRITE_ERROR_NOSTS)) {
		printk("jz-msc%d: MMC/SD/SDIO CRC error, MMC_STAT 0x%x, cmd=%d\n",
				host->pdev_id, stat,
				host->curr_mrq? host->curr_mrq->cmd->opcode : -1);
		data->error = -EILSEQ;
	}

	/*
	 * There appears to be a hardware design bug here.  There seems to
	 * be no way to find out how much data was transferred to the card.
	 * This means that if there was an error on any block, we mark all
	 * data blocks as being in error.
	 */
	if (data->error == 0)
		data->bytes_xfered = data->blocks * data->blksz;
	else
		data->bytes_xfered = 0;

	TRACE_DATA_DONE();

	// jz_mmc_disable_irq(host, MSC_IMASK_DATA_TRAN_DONE);
	if (host->curr_mrq->stop) {
		while(!(REG_MSC_STAT(host->pdev_id) & (MSC_STAT_AUTO_CMD_DONE | MSC_STAT_ERR_BITS)) &&
				!host->eject) ;
		REG_MSC_IREG(host->pdev_id) = MSC_IREG_AUTO_CMD_DONE;

		if(REG_MSC_STAT(host->pdev_id) & MSC_STAT_ERR_BITS) {
			printk("wait auto 12 done error, stat:%x\n",REG_MSC_STAT(host->pdev_id));
			/* need send cmd 12 by soft */
			jz_mmc_send_stop_cmd(host);
		}

		host->cmdat &= ~MSC_CMDAT_SEND_AS_STOP;
	}

	if (host->data_err) {
		data->bytes_xfered = 0;
		host->data_err = 0;
	}

	host->transfer_mode = JZ_TRANS_MODE_NULL;
	return 0;
}

#if defined(CONFIG_JZ_SDMA)
void jz_mmc_enable_sdma(struct jz_mmc_host *host);
extern char *copy_buf;
extern size_t buflen;
#endif

static void jz_mmc_execute_cmd(struct jz_mmc_host *host)
{
	struct mmc_request *mrq = host->curr_mrq;
	struct mmc_data *data = mrq->data;
	struct mmc_command *cmd = mrq->cmd;
	u32 old_cmd_arg = 0;
	unsigned int stat;
	int err = 0;

#ifndef CONFIG_JZ_SDMA
	if (unlikely((cmd->opcode == MMC_SET_BLOCKLEN) && (cmd->arg & 0x3))) {
		old_cmd_arg = cmd->arg;
		//printk("blksz is not multiple of 4(%d), round to 4x\n", old_cmd_arg);
		cmd->arg = (cmd->arg + 3) & (~0x3); /* round to multiple of 4 */
	}
#endif
	/* mask interrupts */
	REG_MSC_IMASK(host->pdev_id) = 0xffffffff;
	/* clear status */
	REG_MSC_IREG(host->pdev_id) = 0xffffffff;

	jz_mmc_set_cmdat(host);
	REG_MSC_CMD(host->pdev_id) = cmd->opcode;
	jz_mmc_set_cmdarg(host);

	//printk("cmd:%d, arg:%x, cmdat:%x\n",REG_MSC_CMD(host->pdev_id),REG_MSC_ARG(host->pdev_id),REG_MSC_CMDAT(host->pdev_id));
	TRACE_CMD_REQ();
#if 0
#ifdef CONFIG_JZ_SDMA	/* ??? */
	//host->transfer_end = 0;
	host->transfer_end = 1;
#else
	host->transfer_end = 1;
#endif
#endif

	if(data && (data->flags & MMC_DATA_READ)){
#if defined(CONFIG_JZ_SDMA)
		jz_mmc_enable_irq(host, MSC_IMASK_DMAEND);
#endif
		jz_mmc_data_start(host);
	}
	REG_MSC_RESTO(host->pdev_id) = 0xffff;
	/* Send command */
	REG_MSC_STRPCL(host->pdev_id) = MSC_STRPCL_START_OP;
	stat = jz_mmc_wait_cmd_done(host);
	if (cmd->error)
		goto cmd_err;

	TRACE_DATA_REQ();

	if (jz_mmc_parse_cmd_response(host, stat))
		goto cmd_err;

	if (host->curr_mrq->data) {
		int acked = 0;

		if(host->curr_mrq->data->flags & MMC_DATA_WRITE) {
			jz_mmc_enable_irq(host, MSC_IMASK_DATA_TRAN_DONE);
			jz_mmc_data_start(host);
		}

		err = wait_event_interruptible_timeout(host->data_wait_queue,
				((host->data_ack) || (host->eject)
				 || (REG_MSC_STAT(host->pdev_id) & WAITMASK)),
				6 * HZ);

		acked = host->data_ack;
		host->data_ack = 0;

		if (acked){
			jz_mmc_data_done(host);
		}else {
			if (err == -ERESTARTSYS) /* user cancelled */
				cmd->error = -ECANCELED;
			else if (!err) {
				printk("Timeout while IRQ_dma, opcode = %d\n", cmd->opcode);
				printk("host->eject = %d\n", host->eject);
				jz_mmc_dump_regs(host->pdev_id, __LINE__);
#if defined(CONFIG_JZ_CDMA)
				dump_jz_dma_channel(host->dma.channel);
#endif
				cmd->error = -ETIMEDOUT;
			}
			goto data_wait_err;
		}
	}
#ifndef CONFIG_JZ_SDMA
	/* restore cmd->arg for SET_BLOCKLEN */
	if (unlikely((cmd->opcode == MMC_SET_BLOCKLEN) && (old_cmd_arg & 0x3))) {
		cmd->arg = old_cmd_arg;
	}
#endif
	return;

cmd_err:
//	printk("%s, %s, cmd err lable\n",__FILE__, __func__);
#if 0
	if (host->eject)
		printk("WARNNING: media eject when sending cmd, opcode = %d\n", cmd->opcode);
#endif
data_wait_err:
	if (host->curr_mrq->data){
		host->curr_mrq->data->bytes_xfered = 0;

#if 0
		if (host->eject)
			printk("WARNNING: media eject when transfering data, opcode = %d err = %d\n", cmd->opcode, err);
#endif
	}

	if (host->eject)
		cmd->error = -ENOMEDIUM;

	if (host->curr_mrq->data)
		jz_mmc_data_stop(host);

	/* restore cmd->arg for SET_BLOCKLEN */
	if (unlikely((cmd->opcode == MMC_SET_BLOCKLEN) && (old_cmd_arg & 0x3))) {
		cmd->arg = old_cmd_arg;
	}
}

static irqreturn_t jz_mmc_irq(int irq, void *devid)
{
	struct jz_mmc_host *host = devid;
	unsigned int ireg = 0, mask;

	ireg = REG_MSC_IREG(host->pdev_id);
#if defined(CONFIG_JZ_SDMA)
	if (ireg & MSC_IREG_DMAEND)
		REG_MSC_DMAC(host->pdev_id) &= ~(MSC_DMAC_EN);
#endif
	mask = REG_MSC_IMASK(host->pdev_id);
	ireg = ireg & ~mask;

	if (ireg & (MSC_IREG_DATA_TRAN_DONE | MSC_IREG_DMAEND)) {
		jz_mmc_disable_irq(host, ireg);
		BUG_ON(host->data_ack);
		host->data_ack = 1;
		wmb();
		wake_up_interruptible(&host->data_wait_queue);
	}

	return IRQ_HANDLED;
}

static char *msc_trans_irq_name[] = {
	"msc_trans_0",
	"msc_trans_1",
	"msc_trans_2",
};

static int jz_mmc_msc_init(struct jz_mmc_host *host)
{
	int ret = 0;

	jz_mmc_reset(host);

	host->data_ack = 0;
	host->transfer_end = 1;
	host->transfer_mode = JZ_TRANS_MODE_NULL;
	init_waitqueue_head(&host->data_wait_queue);
#if 0
	init_waitqueue_head(&host->status_check_queue);
	init_timer(&host->status_check_timer);
	host->status_check_timer.function = jz_mmc_status_checker;
#endif

	ret = request_irq(host->irq, jz_mmc_irq, 0, msc_trans_irq_name[host->pdev_id], host);
	if (ret) {
		printk(KERN_ERR "MMC/SD: can't request MMC/SD IRQ\n");
		return ret;
	}

	return 0;
}

static void jz_mmc_msc_deinit(struct jz_mmc_host *host)
{
	free_irq(host->irq, &host);
}

int jz_mmc_msc_register(struct jz_mmc_msc *msc)
{
	if(msc == NULL)
		return -ENOMEM;

	msc->init = jz_mmc_msc_init;
	msc->deinit = jz_mmc_msc_deinit;
	msc->set_clock = jz_mmc_set_clock;
	msc->execute_cmd = jz_mmc_execute_cmd;

	return 0;
}