// SPDX-License-Identifier: GPL-2.0-only
/*
 * Tegra host1x driver
 *
 * SPDX-FileCopyrightText: Copyright (c) 2010-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#include <nvidia/conftest.h>

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>

#include <soc/tegra/common.h>

#define CREATE_TRACE_POINTS
#include <trace/events/host1x.h>
#undef CREATE_TRACE_POINTS

#if IS_ENABLED(CONFIG_ARM_DMA_USE_IOMMU)
#include <asm/dma-iommu.h>
#endif

#include "bus.h"
#include "channel.h"
#include "context.h"
#include "debug.h"
#include "dev.h"
#include "intr.h"

#include "hw/host1x01.h"
#include "hw/host1x02.h"
#include "hw/host1x04.h"
#include "hw/host1x05.h"
#include "hw/host1x06.h"
#include "hw/host1x07.h"
#include "hw/host1x08.h"

void host1x_common_writel(struct host1x *host1x, u32 v, u32 r)
{
	writel(v, host1x->common_regs + r);
}

u32 host1x_common_readl(struct host1x *host1x, u32 r)
{
	return readl(host1x->common_regs + r);
}

void host1x_hypervisor_writel(struct host1x *host1x, u32 v, u32 r)
{
	writel(v, host1x->hv_regs + r);
}

u32 host1x_hypervisor_readl(struct host1x *host1x, u32 r)
{
	return readl(host1x->hv_regs + r);
}

void host1x_sync_writel(struct host1x *host1x, u32 v, u32 r)
{
	void __iomem *sync_regs = host1x->regs + host1x->info->sync_offset;

	writel(v, sync_regs + r);
}

u32 host1x_sync_readl(struct host1x *host1x, u32 r)
{
	void __iomem *sync_regs = host1x->regs + host1x->info->sync_offset;

	return readl(sync_regs + r);
}

void host1x_ch_writel(struct host1x_channel *ch, u32 v, u32 r)
{
	writel(v, ch->regs + r);
}

u32 host1x_ch_readl(struct host1x_channel *ch, u32 r)
{
	return readl(ch->regs + r);
}

static const struct host1x_info host1x01_info = {
	.nb_channels = 8,
	.nb_pts = 32,
	.nb_mlocks = 16,
	.nb_bases = 8,
	.init = host1x01_init,
	.sync_offset = 0x3000,
	.dma_mask = DMA_BIT_MASK(32),
	.has_wide_gather = false,
	.has_hypervisor = false,
	.num_sid_entries = 0,
	.sid_table = NULL,
	.reserve_vblank_syncpts = true,
};

static const struct host1x_info host1x02_info = {
	.nb_channels = 9,
	.nb_pts = 32,
	.nb_mlocks = 16,
	.nb_bases = 12,
	.init = host1x02_init,
	.sync_offset = 0x3000,
	.dma_mask = DMA_BIT_MASK(32),
	.has_wide_gather = false,
	.has_hypervisor = false,
	.num_sid_entries = 0,
	.sid_table = NULL,
	.reserve_vblank_syncpts = true,
};

static const struct host1x_info host1x04_info = {
	.nb_channels = 12,
	.nb_pts = 192,
	.nb_mlocks = 16,
	.nb_bases = 64,
	.init = host1x04_init,
	.sync_offset = 0x2100,
	.dma_mask = DMA_BIT_MASK(34),
	.has_wide_gather = false,
	.has_hypervisor = false,
	.num_sid_entries = 0,
	.sid_table = NULL,
	.reserve_vblank_syncpts = false,
};

static const struct host1x_info host1x05_info = {
	.nb_channels = 14,
	.nb_pts = 192,
	.nb_mlocks = 16,
	.nb_bases = 64,
	.init = host1x05_init,
	.sync_offset = 0x2100,
	.dma_mask = DMA_BIT_MASK(34),
	.has_wide_gather = false,
	.has_hypervisor = false,
	.num_sid_entries = 0,
	.sid_table = NULL,
	.reserve_vblank_syncpts = false,
};

static const struct host1x_sid_entry tegra186_sid_table[] = {
	{
		/* VIC */
		.base = 0x1af0,
		.offset = 0x30,
		.limit = 0x34
	},
	{
		/* NVENC */
		.base = 0x1af8,
		.offset = 0x30,
		.limit = 0x34
	},
	{
		/* NVDEC */
		.base = 0x1b00,
		.offset = 0x30,
		.limit = 0x34
	},
	{
		/* NVJPG */
		.base = 0x1b08,
		.offset = 0x30,
		.limit = 0x34
	},
};

static const struct host1x_info host1x06_info = {
	.nb_channels = 63,
	.nb_pts = 576,
	.nb_mlocks = 24,
	.nb_bases = 16,
	.init = host1x06_init,
	.sync_offset = 0x0,
	.dma_mask = DMA_BIT_MASK(40),
	.has_wide_gather = true,
	.has_hypervisor = true,
	.num_sid_entries = ARRAY_SIZE(tegra186_sid_table),
	.sid_table = tegra186_sid_table,
	.reserve_vblank_syncpts = false,
};

static const struct host1x_sid_entry tegra194_sid_table[] = {
	{
		/* VIC */
		.base = 0x1af0,
		.offset = 0x30,
		.limit = 0x34
	},
	{
		/* NVENC */
		.base = 0x1af8,
		.offset = 0x30,
		.limit = 0x34
	},
	{
		/* NVDEC */
		.base = 0x1b00,
		.offset = 0x30,
		.limit = 0x34
	},
	{
		/* NVJPG */
		.base = 0x1b08,
		.offset = 0x30,
		.limit = 0x34
	},
	{
		/* NVDLA */
		.base = 0x1ba8,
		.offset = 0x30,
		.limit = 0x34
	},
	{
		/* NVDLA1 */
		.base = 0x1bb0,
		.offset = 0x30,
		.limit = 0x34
	},
	{
		/* NVENC1 */
		.base = 0x1bb8,
		.offset = 0x30,
		.limit = 0x34
	},
	{
		/* NVDEC1 */
		.base = 0x1bc0,
		.offset = 0x30,
		.limit = 0x34
	},
};

static const struct host1x_info host1x07_info = {
	.nb_channels = 63,
	.nb_pts = 704,
	.nb_mlocks = 32,
	.nb_bases = 0,
	.init = host1x07_init,
	.sync_offset = 0x0,
	.dma_mask = DMA_BIT_MASK(40),
	.has_wide_gather = true,
	.has_hypervisor = true,
	.num_sid_entries = ARRAY_SIZE(tegra194_sid_table),
	.sid_table = tegra194_sid_table,
	.reserve_vblank_syncpts = false,
};

/*
 * Tegra234 has two stream ID protection tables, one for setting stream IDs
 * through the channel path via SETSTREAMID, and one for setting them via
 * MMIO. We program each engine's data stream ID in the channel path table
 * and firmware stream ID in the MMIO path table.
 */
static const struct host1x_sid_entry tegra234_sid_table[] = {
	{
		/* SE1 MMIO */
		.base = 0x1650,
		.offset = 0x90,
		.limit = 0x90
	},
	{
		/* SE2 MMIO */
		.base = 0x1658,
		.offset = 0x90,
		.limit = 0x90
	},
	{
		/* SE4 MMIO */
		.base = 0x1660,
		.offset = 0x90,
		.limit = 0x90
	},
	{
		/* SE1 channel */
		.base = 0x1730,
		.offset = 0x90,
		.limit = 0x90
	},
	{
		/* SE2 channel */
		.base = 0x1738,
		.offset = 0x90,
		.limit = 0x90
	},
	{
		/* SE4 channel */
		.base = 0x1740,
		.offset = 0x90,
		.limit = 0x90
	},
	{
		/* VIC channel */
		.base = 0x17b8,
		.offset = 0x30,
		.limit = 0x30
	},
	{
		/* VIC MMIO */
		.base = 0x1688,
		.offset = 0x34,
		.limit = 0x34
	},
	{
		/* NVDEC channel */
		.base = 0x17c8,
		.offset = 0x30,
		.limit = 0x30,
	},
	{
		/* NVDEC MMIO */
		.base = 0x1698,
		.offset = 0x34,
		.limit = 0x34,
	},
	{
		/* NVENC channel */
		.base = 0x17c0,
		.offset = 0x30,
		.limit = 0x30,
	},
	{
		/* NVENC MMIO */
		.base = 0x1690,
		.offset = 0x34,
		.limit = 0x34,
	},
	{
		/* NVJPG channel */
		.base = 0x17d0,
		.offset = 0x30,
		.limit = 0x30,
	},
	{
		/* NVJPG MMIO */
		.base = 0x16a0,
		.offset = 0x34,
		.limit = 0x34,
	},
	{
		/* NVJPG1 channel */
		.base = 0x17a8,
		.offset = 0x30,
		.limit = 0x30,
	},
	{
		/* NVJPG1 MMIO */
		.base = 0x16b0,
		.offset = 0x34,
		.limit = 0x34,
	},
	{
		/* NVDLA channel */
		.base = 0x17e0,
		.offset = 0x30,
		.limit = 0x34
	},
	{
		/* NVDLA MMIO */
		.base = 0x16d8,
		.offset = 0x0030,
		.limit = 0x0034
	},
	{
		/* NVDLA1 channel */
		.base = 0x17e8,
		.offset = 0x30,
		.limit = 0x34
	},
	{
		/* NVDLA1 MMIO */
		.base = 0x16e0,
		.offset = 0x0030,
		.limit = 0x0034
	},
	{
		/* OFA channel */
		.base = 0x1768,
		.offset = 0x30,
		.limit = 0x30,
	},
	{
		/* OFA MMIO */
		.base = 0x16e8,
		.offset = 0x34,
		.limit = 0x34,
	},
	{
		/* TSEC MMIO */
		.base = 0x16a8,
		.offset = 0x30,
		.limit = 0x34,
	},
	{
		/* VI MMIO */
		.base = 0x16b8,
		.offset = 0x800,
		.limit = 0x800,
	},
	{
		/* VI_THI MMIO */
		.base = 0x16c0,
		.offset = 0x30,
		.limit = 0x34,
	},
	{
		/* ISP MMIO */
		.base = 0x1680,
		.offset = 0x800,
		.limit = 0x800,
	},
	{
		/* ISP_THI MMIO */
		.base = 0x16c8,
		.offset = 0x30,
		.limit = 0x34,
	},
	{
		/* VI2 MMIO */
		.base = 0x16f0,
		.offset = 0x800,
		.limit = 0x800,
	},
	{
		/* VI2_THI MMIO */
		.base = 0x16f8,
		.offset = 0x30,
		.limit = 0x34,
	},
};

static const struct host1x_actmon_entry tegra234_actmon_table[] = {
	{
		.classid = HOST1X_CLASS_NVENC,
		.name = "nvenc",
		.irq = 2,
		.offset = 0x00000,
		.num_modules = 1,
	},
	{
		.classid = HOST1X_CLASS_VIC,
		.name = "vic",
		.irq = 3,
		.offset = 0x10000,
		.num_modules = 1,
	},
	{
		.classid = HOST1X_CLASS_NVDEC,
		.name = "nvdec",
		.irq = 4,
		.offset = 0x20000,
		.num_modules = 1,
	},
	{
		.classid = HOST1X_CLASS_NVJPG,
		.name = "nvjpg",
		.irq = 5,
		.offset = 0x30000,
		.num_modules = 1,
	},
	{
		.classid = HOST1X_CLASS_NVJPG1,
		.name = "nvjpg1",
		.irq = 6,
		.offset = 0x40000,
		.num_modules = 1,
	},
	{
		.classid = HOST1X_CLASS_OFA,
		.name = "ofa",
		.irq = 7,
		.offset = 0x50000,
		.num_modules = 1,
	},
};

static const struct host1x_info host1x08_info = {
	.nb_channels = 63,
	.nb_pts = 1024,
	.nb_mlocks = 24,
	.nb_bases = 0,
	.init = host1x08_init,
	.sync_offset = 0x0,
	.dma_mask = DMA_BIT_MASK(40),
	.has_wide_gather = true,
	.has_hypervisor = true,
	.has_common = true,
	.num_sid_entries = ARRAY_SIZE(tegra234_sid_table),
	.sid_table = tegra234_sid_table,
	.num_actmon_entries = ARRAY_SIZE(tegra234_actmon_table),
	.actmon_table = tegra234_actmon_table,
	.streamid_vm_table = { 0x1004, 128 },
	.classid_vm_table = { 0x1404, 25 },
	.mmio_vm_table = { 0x1504, 25 },
	.reserve_vblank_syncpts = false,
};

static const struct of_device_id host1x_of_match[] = {
	{ .compatible = "nvidia,tegra234-host1x", .data = &host1x08_info, },
	{ .compatible = "nvidia,tegra194-host1x", .data = &host1x07_info, },
	{ .compatible = "nvidia,tegra186-host1x", .data = &host1x06_info, },
	{ .compatible = "nvidia,tegra210-host1x", .data = &host1x05_info, },
	{ .compatible = "nvidia,tegra124-host1x", .data = &host1x04_info, },
	{ .compatible = "nvidia,tegra114-host1x", .data = &host1x02_info, },
	{ .compatible = "nvidia,tegra30-host1x", .data = &host1x01_info, },
	{ .compatible = "nvidia,tegra20-host1x", .data = &host1x01_info, },
	{ },
};
MODULE_DEVICE_TABLE(of, host1x_of_match);

static void host1x_setup_virtualization_tables(struct host1x *host)
{
	const struct host1x_info *info = host->info;
	unsigned int i;

	if (!host->hv_regs)
		return;

	for (i = 0; i < info->num_sid_entries; i++) {
		const struct host1x_sid_entry *entry = &info->sid_table[i];

		host1x_hypervisor_writel(host, entry->offset, entry->base);
		host1x_hypervisor_writel(host, entry->limit, entry->base + 4);
	}

	for (i = 0; i < info->streamid_vm_table.count; i++) {
		/* Allow access to all stream IDs to all VMs. */
		host1x_hypervisor_writel(host, 0xff, info->streamid_vm_table.base + 4 * i);
	}

	for (i = 0; i < info->classid_vm_table.count; i++) {
		/* Allow access to all classes to all VMs. */
		host1x_hypervisor_writel(host, 0xff, info->classid_vm_table.base + 4 * i);
	}

	for (i = 0; i < info->mmio_vm_table.count; i++) {
		/* Use VM1 (that's us) as originator VMID for engine MMIO accesses. */
		host1x_hypervisor_writel(host, 0x1, info->mmio_vm_table.base + 4 * i);
	}
}

static bool host1x_wants_iommu(struct host1x *host1x)
{
	/*
	 * If we support addressing a maximum of 32 bits of physical memory
	 * and if the host1x firewall is enabled, there's no need to enable
	 * IOMMU support. This can happen for example on Tegra20, Tegra30
	 * and Tegra114.
	 *
	 * Tegra124 and later can address up to 34 bits of physical memory and
	 * many platforms come equipped with more than 2 GiB of system memory,
	 * which requires crossing the 4 GiB boundary. But there's a catch: on
	 * SoCs before Tegra186 (i.e. Tegra124 and Tegra210), the host1x can
	 * only address up to 32 bits of memory in GATHER opcodes, which means
	 * that command buffers need to either be in the first 2 GiB of system
	 * memory (which could quickly lead to memory exhaustion), or command
	 * buffers need to be treated differently from other buffers (which is
	 * not possible with the current ABI).
	 *
	 * A third option is to use the IOMMU in these cases to make sure all
	 * buffers will be mapped into a 32-bit IOVA space that host1x can
	 * address. This allows all of the system memory to be used and works
	 * within the limitations of the host1x on these SoCs.
	 *
	 * In summary, default to enable IOMMU on Tegra124 and later. For any
	 * of the earlier SoCs, only use the IOMMU for additional safety when
	 * the host1x firewall is disabled.
	 */
	if (host1x->info->dma_mask <= DMA_BIT_MASK(32)) {
		if (IS_ENABLED(CONFIG_TEGRA_HOST1X_FIREWALL))
			return false;
	}

	return true;
}

static struct iommu_domain *host1x_iommu_attach(struct host1x *host)
{
	struct iommu_domain *domain = iommu_get_domain_for_dev(host->dev);
	int err;

#if IS_ENABLED(CONFIG_ARM_DMA_USE_IOMMU)
	if (host->dev->archdata.mapping) {
		struct dma_iommu_mapping *mapping =
				to_dma_iommu_mapping(host->dev);
		arm_iommu_detach_device(host->dev);
		arm_iommu_release_mapping(mapping);

		domain = iommu_get_domain_for_dev(host->dev);
	}
#endif

	/*
	 * We may not always want to enable IOMMU support (for example if the
	 * host1x firewall is already enabled and we don't support addressing
	 * more than 32 bits of physical memory), so check for that first.
	 *
	 * Similarly, if host1x is already attached to an IOMMU (via the DMA
	 * API), don't try to attach again.
	 */
	if (!host1x_wants_iommu(host) || domain)
		return domain;

	host->group = iommu_group_get(host->dev);
	if (host->group) {
		struct iommu_domain_geometry *geometry;
		dma_addr_t start, end;
		unsigned long order;

		err = iova_cache_get();
		if (err < 0)
			goto put_group;

		host->domain = iommu_domain_alloc(&platform_bus_type);
		if (!host->domain) {
			err = -ENOMEM;
			goto put_cache;
		}

		err = iommu_attach_group(host->domain, host->group);
		if (err) {
			if (err == -ENODEV)
				err = 0;

			goto free_domain;
		}

		geometry = &host->domain->geometry;
		start = geometry->aperture_start & host->info->dma_mask;
		end = geometry->aperture_end & host->info->dma_mask;

		order = __ffs(host->domain->pgsize_bitmap);
		init_iova_domain(&host->iova, 1UL << order, start >> order);
		host->iova_end = end;

		domain = host->domain;
	}

	return domain;

free_domain:
	iommu_domain_free(host->domain);
	host->domain = NULL;
put_cache:
	iova_cache_put();
put_group:
	iommu_group_put(host->group);
	host->group = NULL;

	return ERR_PTR(err);
}

static int host1x_iommu_init(struct host1x *host)
{
	u64 mask = host->info->dma_mask;
	struct iommu_domain *domain;
	int err;

	domain = host1x_iommu_attach(host);
	if (IS_ERR(domain)) {
		err = PTR_ERR(domain);
		dev_err(host->dev, "failed to attach to IOMMU: %d\n", err);
		return err;
	}

	/*
	 * If we're not behind an IOMMU make sure we don't get push buffers
	 * that are allocated outside of the range addressable by the GATHER
	 * opcode.
	 *
	 * Newer generations of Tegra (Tegra186 and later) support a wide
	 * variant of the GATHER opcode that allows addressing more bits.
	 */
	if (!domain && !host->info->has_wide_gather)
		mask = DMA_BIT_MASK(32);

	err = dma_coerce_mask_and_coherent(host->dev, mask);
	if (err < 0) {
		dev_err(host->dev, "failed to set DMA mask: %d\n", err);
		return err;
	}

	return 0;
}

static void host1x_iommu_exit(struct host1x *host)
{
	if (host->domain) {
		put_iova_domain(&host->iova);
		iommu_detach_group(host->domain, host->group);

		iommu_domain_free(host->domain);
		host->domain = NULL;

		iova_cache_put();

		iommu_group_put(host->group);
		host->group = NULL;
	}
}

static int host1x_get_resets(struct host1x *host)
{
	int err;

	host->resets[0].id = "mc";
	host->resets[1].id = "host1x";
	host->nresets = ARRAY_SIZE(host->resets);

	err = devm_reset_control_bulk_get_optional_exclusive_released(
				host->dev, host->nresets, host->resets);
	if (err) {
		dev_err(host->dev, "failed to get reset: %d\n", err);
		return err;
	}

	return 0;
}

static int host1x_get_assigned_resources(struct host1x *host)
{
	struct device_node *np = host->dev->of_node;
	u32 vals[2];
	int err;

	err = of_property_read_u32_array(np, "nvidia,channels", vals, 2);
	if (err == 0) {
		host->channel_base = vals[0];
		host->num_channels = vals[1];
	} else if (err == -EINVAL) {
		host->channel_base = 0;
		host->num_channels = host->info->nb_channels;
	} else {
		dev_err(host->dev, "invalid nvidia,channels property: %d\n", err);
		return err;
	}

	err = of_property_read_u32_array(np, "nvidia,syncpoints", vals, 2);
	if (err == 0) {
		host->syncpt_base = vals[0];
		host->syncpt_end = vals[0] + vals[1];
	} else if (err == -EINVAL) {
		host->syncpt_base = 0;
		host->syncpt_end = host->info->nb_pts;
	} else {
		dev_err(host->dev, "invalid nvidia,syncpoints property: %d\n", err);
		return err;
	}

	return 0;
}

static int host1x_get_syncpt_pools(struct host1x *host)
{
	struct device_node *np = host->dev->of_node;
	int err, i;

	err = of_property_count_strings(np, "nvidia,syncpoint-pool-names");
	if (err == -EINVAL)
		return 0;
	if (err < 0) {
		dev_err(host->dev, "invalid nvidia,syncpoint-pool-names property: %d\n", err);
		return err;
	}

	host->num_pools = err;
	host->pools = devm_kcalloc(host->dev, host->num_pools, sizeof(struct host1x_syncpt_pool),
				   GFP_KERNEL);
	if (!host->pools)
		return -ENOMEM;

	for (i = 0; i < host->num_pools; i++) {
		struct host1x_syncpt_pool *pool = &host->pools[i];

		err = of_property_read_string_index(np, "nvidia,syncpoint-pool-names", i, &pool->name);
		if (err)
			return err;

		err = of_property_read_u32_index(np, "nvidia,syncpoint-pools", i*2+0, &pool->base);
		if (!err)
			err = of_property_read_u32_index(np, "nvidia,syncpoint-pools", i*2+1, &pool->end);
		if (err) {
			dev_err(host->dev, "invalid nvidia,syncpoint-pools property: %d\n", err);
			return err;
		}

		pool->end = pool->base + pool->end;
	}

	return 0;
}

static int host1x_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct host1x *host;
	int err, i;

	host = devm_kzalloc(&pdev->dev, sizeof(*host), GFP_KERNEL);
	if (!host)
		return -ENOMEM;

	host->info = of_device_get_match_data(&pdev->dev);

	if (host->info->has_hypervisor) {
		host->regs = devm_platform_ioremap_resource_byname(pdev, "vm");
		if (IS_ERR(host->regs))
			return PTR_ERR(host->regs);

		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "hypervisor");
		if (res) {
			host->hv_regs = devm_ioremap_resource(&pdev->dev, res);
			if (IS_ERR(host->hv_regs))
				return PTR_ERR(host->hv_regs);
		}

		if (res && host->info->has_common) {
			host->common_regs = devm_platform_ioremap_resource_byname(pdev, "common");
			if (IS_ERR(host->common_regs))
				return PTR_ERR(host->common_regs);
		}
	} else {
		host->regs = devm_platform_ioremap_resource(pdev, 0);
		if (IS_ERR(host->regs))
			return PTR_ERR(host->regs);
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "actmon");
	if (res) {
		host->actmon_regs = devm_ioremap_resource(&pdev->dev, res);
		if (IS_ERR(host->actmon_regs))
			return PTR_ERR(host->actmon_regs);
	}

	for (i = 0; i < ARRAY_SIZE(host->syncpt_irqs); i++) {
		char irq_name[] = "syncptX";
		sprintf(irq_name, "syncpt%d", i);

		err = platform_get_irq_byname_optional(pdev, irq_name);
		if (err == -ENXIO)
			break;
		if (err < 0)
			return err;

		host->syncpt_irqs[i] = err;
	}

	host->num_syncpt_irqs = i;

	/* Device tree without irq names */
	if (i == 0) {
		host->syncpt_irqs[0] = platform_get_irq(pdev, 0);
		if (host->syncpt_irqs[0] < 0)
			return host->syncpt_irqs[0];

		host->num_syncpt_irqs = 1;
	}

	host->general_irq = platform_get_irq_byname_optional(pdev, "host1x");

	mutex_init(&host->devices_lock);
	INIT_LIST_HEAD(&host->devices);
	spin_lock_init(&host->actmons_lock);
	INIT_LIST_HEAD(&host->actmons);
	INIT_LIST_HEAD(&host->list);
	host->dev = &pdev->dev;

	/* set common host1x device data */
	platform_set_drvdata(pdev, host);

	host->dev->dma_parms = &host->dma_parms;
	dma_set_max_seg_size(host->dev, UINT_MAX);

	if (host->info->init) {
		err = host->info->init(host);
		if (err)
			return err;
	}

	if (host->general_irq > 0 && host->common_regs) {
		err = host1x_hw_intr_init_host_general(host);
		if (err)
			return err;
	}

	err = host1x_get_assigned_resources(host);
	if (err)
		return err;

	err = host1x_get_syncpt_pools(host);
	if (err)
		return err;

	host->clk = devm_clk_get_optional(&pdev->dev, "host1x");
	if (!host->clk)
		host->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(host->clk))
		return dev_err_probe(&pdev->dev, PTR_ERR(host->clk),
				"failed to get host1x clock\n");

	host->actmon_clk = devm_clk_get_optional(&pdev->dev, "actmon");
	if (IS_ERR(host->actmon_clk))
		return dev_err_probe(&pdev->dev, PTR_ERR(host->actmon_clk),
				"failed to get actmon clock\n");

	err = host1x_get_resets(host);
	if (err)
		return err;

	host1x_bo_cache_init(&host->cache);

	err = host1x_iommu_init(host);
	if (err < 0) {
		dev_err(&pdev->dev, "failed to setup IOMMU: %d\n", err);
		goto destroy_cache;
	}

	err = host1x_channel_list_init(&host->channel_list,
				       host->info->nb_channels);
	if (err) {
		dev_err(&pdev->dev, "failed to initialize channel list\n");
		goto iommu_exit;
	}

	err = host1x_memory_context_list_init(host);
	if (err) {
		dev_err(&pdev->dev, "failed to initialize context list\n");
		goto free_channels;
	}

	err = host1x_syncpt_init(host);
	if (err) {
		dev_err(&pdev->dev, "failed to initialize syncpts\n");
		goto free_contexts;
	}

	err = host1x_intr_init(host);
	if (err) {
		dev_err(&pdev->dev, "failed to initialize interrupts\n");
		goto deinit_syncpt;
	}

	pm_runtime_enable(&pdev->dev);

#if defined(NV_DEVM_TEGRA_CORE_DEV_INIT_OPP_TABLE_COMMON_PRESENT) /* Linux v5.17 */
	err = devm_tegra_core_dev_init_opp_table_common(&pdev->dev);
	if (err)
		goto pm_disable;
#endif

	/* the driver's code isn't ready yet for the dynamic RPM */
	err = pm_runtime_resume_and_get(&pdev->dev);
	if (err)
		goto pm_disable;

	host1x_debug_init(host);

	err = host1x_register(host);
	if (err < 0)
		goto deinit_debugfs;

	err = devm_of_platform_populate(&pdev->dev);
	if (err < 0)
		goto unregister;

	return 0;

unregister:
	host1x_unregister(host);
deinit_debugfs:
	host1x_debug_deinit(host);

	pm_runtime_put_sync_suspend(&pdev->dev);
pm_disable:
	pm_runtime_disable(&pdev->dev);

	host1x_intr_deinit(host);
deinit_syncpt:
	host1x_syncpt_deinit(host);
free_contexts:
	host1x_memory_context_list_free(&host->context_list);
free_channels:
	host1x_channel_list_free(&host->channel_list);
iommu_exit:
	host1x_iommu_exit(host);
destroy_cache:
	host1x_bo_cache_destroy(&host->cache);

	return err;
}

static int host1x_remove(struct platform_device *pdev)
{
	struct host1x *host = platform_get_drvdata(pdev);

	host1x_unregister(host);
	host1x_debug_deinit(host);

	pm_runtime_force_suspend(&pdev->dev);

	host1x_intr_deinit(host);
	host1x_syncpt_deinit(host);
	host1x_memory_context_list_free(&host->context_list);
	host1x_channel_list_free(&host->channel_list);
	host1x_iommu_exit(host);
	host1x_bo_cache_destroy(&host->cache);

	return 0;
}

static int __maybe_unused host1x_runtime_suspend(struct device *dev)
{
	struct host1x *host = dev_get_drvdata(dev);
	int err;

	host1x_hw_intr_disable_all_general_intrs(host);
	host1x_channel_list_stop(&host->channel_list);
	host1x_intr_stop(host);
	host1x_syncpt_save(host);

	err = reset_control_bulk_assert(host->nresets, host->resets);
	if (err) {
		dev_err(dev, "failed to assert reset: %d\n", err);
		goto resume_host1x;
	}

	usleep_range(1000, 2000);

	clk_disable_unprepare(host->clk);
	clk_disable_unprepare(host->actmon_clk);
	reset_control_bulk_release(host->nresets, host->resets);

	return 0;

resume_host1x:
	host1x_setup_virtualization_tables(host);
	host1x_syncpt_restore(host);
	host1x_hw_intr_enable_general_intrs(host);
	host1x_intr_start(host);

	return err;
}

static int __maybe_unused host1x_runtime_resume(struct device *dev)
{
	struct host1x *host = dev_get_drvdata(dev);
	int err;

	err = reset_control_bulk_acquire(host->nresets, host->resets);
	if (err) {
		dev_err(dev, "failed to acquire reset: %d\n", err);
		return err;
	}

	err = clk_prepare_enable(host->clk);
	if (err) {
		dev_err(dev, "failed to enable host1x clock: %d\n", err);
		goto release_reset;
	}

	err = clk_prepare_enable(host->actmon_clk);
	if (err) {
		dev_err(dev, "failed to enable actmon clock: %d\n", err);
		goto disable_host1x_clk;
	}

	err = reset_control_bulk_deassert(host->nresets, host->resets);
	if (err < 0) {
		dev_err(dev, "failed to deassert reset: %d\n", err);
		goto disable_actmon_clk;
	}

	host1x_setup_virtualization_tables(host);
	host1x_syncpt_restore(host);
	host1x_hw_intr_enable_general_intrs(host);
	host1x_intr_start(host);

	return 0;

disable_actmon_clk:
	clk_disable_unprepare(host->actmon_clk);
disable_host1x_clk:
	clk_disable_unprepare(host->clk);
release_reset:
	reset_control_bulk_release(host->nresets, host->resets);

	return err;
}

static const struct dev_pm_ops host1x_pm_ops = {
	SET_RUNTIME_PM_OPS(host1x_runtime_suspend, host1x_runtime_resume,
			   NULL)
	SET_SYSTEM_SLEEP_PM_OPS(host1x_runtime_suspend, host1x_runtime_resume)
};

#if defined(NV_PLATFORM_DRIVER_STRUCT_REMOVE_RETURNS_VOID) /* Linux v6.11 */
static void host1x_remove_wrapper(struct platform_device *pdev)
{
	host1x_remove(pdev);
}
#else
static int host1x_remove_wrapper(struct platform_device *pdev)
{
	return host1x_remove(pdev);
}
#endif

static struct platform_driver tegra_host1x_driver = {
	.driver = {
		.name = "tegra-host1x",
		.of_match_table = host1x_of_match,
		.pm = &host1x_pm_ops,
	},
	.probe = host1x_probe,
	.remove = host1x_remove_wrapper,
};

static struct platform_driver * const drivers[] = {
	&tegra_host1x_driver,
	&tegra_mipi_driver,
};

static int __init tegra_host1x_init(void)
{
	int err;

	err = bus_register(&host1x_bus_type);
	if (err < 0)
		return err;

	err = platform_register_drivers(drivers, ARRAY_SIZE(drivers));
	if (err < 0)
		bus_unregister(&host1x_bus_type);

	return err;
}
module_init(tegra_host1x_init);

static void __exit tegra_host1x_exit(void)
{
	platform_unregister_drivers(drivers, ARRAY_SIZE(drivers));
	bus_unregister(&host1x_bus_type);
}
module_exit(tegra_host1x_exit);

/**
 * host1x_get_dma_mask() - query the supported DMA mask for host1x
 * @host1x: host1x instance
 *
 * Note that this returns the supported DMA mask for host1x, which can be
 * different from the applicable DMA mask under certain circumstances.
 */
u64 host1x_get_dma_mask(struct host1x *host1x)
{
	return host1x->info->dma_mask;
}
EXPORT_SYMBOL(host1x_get_dma_mask);

MODULE_AUTHOR("Thierry Reding <thierry.reding@avionic-design.de>");
MODULE_AUTHOR("Terje Bergstrom <tbergstrom@nvidia.com>");
MODULE_DESCRIPTION("Host1x driver for Tegra products");
MODULE_LICENSE("GPL");
