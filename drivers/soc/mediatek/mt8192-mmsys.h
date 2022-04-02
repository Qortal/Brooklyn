/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __SOC_MEDIATEK_MT8192_MMSYS_H
#define __SOC_MEDIATEK_MT8192_MMSYS_H

#define MT8192_MMSYS_OVL_MOUT_EN		0xf04
#define MT8192_DISP_OVL1_2L_MOUT_EN		0xf08
#define MT8192_DISP_OVL0_2L_MOUT_EN		0xf18
#define MT8192_DISP_OVL0_MOUT_EN		0xf1c
#define MT8192_DISP_RDMA0_SEL_IN		0xf2c
#define MT8192_DISP_RDMA0_SOUT_SEL		0xf30
#define MT8192_DISP_CCORR0_SOUT_SEL		0xf34
#define MT8192_DISP_AAL0_SEL_IN			0xf38
#define MT8192_DISP_DITHER0_MOUT_EN		0xf3c
#define MT8192_DISP_DSI0_SEL_IN			0xf40
#define MT8192_DISP_OVL2_2L_MOUT_EN		0xf4c

#define MT8192_DISP_OVL0_GO_BLEND			BIT(0)
#define MT8192_DITHER0_MOUT_IN_DSI0			BIT(0)
#define MT8192_OVL0_MOUT_EN_DISP_RDMA0			BIT(0)
#define MT8192_OVL2_2L_MOUT_EN_RDMA4			BIT(0)
#define MT8192_DISP_OVL0_GO_BG				BIT(1)
#define MT8192_DISP_OVL0_2L_GO_BLEND			BIT(2)
#define MT8192_DISP_OVL0_2L_GO_BG			BIT(3)
#define MT8192_OVL1_2L_MOUT_EN_RDMA1			BIT(4)
#define MT8192_OVL0_MOUT_EN_OVL0_2L			BIT(4)
#define MT8192_RDMA0_SEL_IN_OVL0_2L			0x3
#define MT8192_RDMA0_SOUT_COLOR0			0x1
#define MT8192_CCORR0_SOUT_AAL0				0x1
#define MT8192_AAL0_SEL_IN_CCORR0			0x1
#define MT8192_DSI0_SEL_IN_DITHER0			0x1

static const struct mtk_mmsys_routes mmsys_mt8192_routing_table[] = {
	{
		DDP_COMPONENT_OVL_2L0, DDP_COMPONENT_RDMA0,
		MT8192_DISP_OVL0_2L_MOUT_EN, MT8192_OVL0_MOUT_EN_DISP_RDMA0,
		MT8192_OVL0_MOUT_EN_DISP_RDMA0
	}, {
		DDP_COMPONENT_OVL_2L2, DDP_COMPONENT_RDMA4,
		MT8192_DISP_OVL2_2L_MOUT_EN, MT8192_OVL2_2L_MOUT_EN_RDMA4,
		MT8192_OVL2_2L_MOUT_EN_RDMA4
	}, {
		DDP_COMPONENT_DITHER, DDP_COMPONENT_DSI0,
		MT8192_DISP_DITHER0_MOUT_EN, MT8192_DITHER0_MOUT_IN_DSI0,
		MT8192_DITHER0_MOUT_IN_DSI0
	}, {
		DDP_COMPONENT_OVL_2L0, DDP_COMPONENT_RDMA0,
		MT8192_DISP_RDMA0_SEL_IN, MT8192_RDMA0_SEL_IN_OVL0_2L,
		MT8192_RDMA0_SEL_IN_OVL0_2L
	}, {
		DDP_COMPONENT_CCORR, DDP_COMPONENT_AAL0,
		MT8192_DISP_AAL0_SEL_IN, MT8192_AAL0_SEL_IN_CCORR0,
		MT8192_AAL0_SEL_IN_CCORR0
	}, {
		DDP_COMPONENT_DITHER, DDP_COMPONENT_DSI0,
		MT8192_DISP_DSI0_SEL_IN, MT8192_DSI0_SEL_IN_DITHER0,
		MT8192_DSI0_SEL_IN_DITHER0
	}, {
		DDP_COMPONENT_RDMA0, DDP_COMPONENT_COLOR0,
		MT8192_DISP_RDMA0_SOUT_SEL, MT8192_RDMA0_SOUT_COLOR0,
		MT8192_RDMA0_SOUT_COLOR0
	}, {
		DDP_COMPONENT_CCORR, DDP_COMPONENT_AAL0,
		MT8192_DISP_CCORR0_SOUT_SEL, MT8192_CCORR0_SOUT_AAL0,
		MT8192_CCORR0_SOUT_AAL0
	}, {
		DDP_COMPONENT_OVL0, DDP_COMPONENT_OVL_2L0,
		MT8192_MMSYS_OVL_MOUT_EN, MT8192_DISP_OVL0_GO_BG,
		MT8192_DISP_OVL0_GO_BG
	}, {
		DDP_COMPONENT_OVL_2L0, DDP_COMPONENT_RDMA0,
		MT8192_MMSYS_OVL_MOUT_EN, MT8192_DISP_OVL0_2L_GO_BLEND,
		MT8192_DISP_OVL0_2L_GO_BLEND
	}
};

#endif /* __SOC_MEDIATEK_MT8192_MMSYS_H */
