/******************************************************************************
	Copyright (C) 2016 Marvell International Ltd.

  If you received this File from Marvell, you may opt to use, redistribute
  and/or modify this File under the following licensing terms.
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  
  	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
  
  	* Redistributions in binary form must reproduce the above copyright
  	  notice, this list of conditions and the following disclaimer in the
  	  documentation and/or other materials provided with the distribution.
  
  	* Neither the name of Marvell nor the names of its contributors may be
  	  used to endorse or promote products derived from this software
	  without specific prior written permission.
  
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#ifndef __IO_H__
#define __IO_H__

#define local_irq_disable()	do{;} while(0)
#define local_irq_enable()	do{;} while(0)
#define local_irq_save(_flags)	do{_flags=0;} while(0)
#define local_irq_restore(_flags)	do{_flags=_flags;} while(0)

#define __iomem

#define dsb(opt)	asm volatile("dsb " #opt : : : "memory")

#define mb()		dsb(sy)
#define rmb()		dsb(ld)
#define wmb()		dsb(st)

#define __iormb()		rmb()
#define __iowmb()		wmb()

#if 1
/* TODO: improve writel/readl */
//static inline void writel(u32 val, uintptr_t addr)
static inline void writel(u32 val, volatile void __iomem *addr)
{
	__iowmb();
	*(volatile u32 *)addr = val;
}

//static inline u32 readl(uintptr_t addr)
static inline u32 readl(volatile void __iomem *addr)
{
	u32 val = *(volatile u32 *)addr;
	__iormb();
	return val;
}


#else
#if 0
/*
 * alternative assembly primitive:
 *
 * If any of these .org directive fail, it means that insn1 and insn2
 * don't have the same length. This used to be written as
 *
 * .if ((664b-663b) != (662b-661b))
 *      .error "Alternatives instruction length mismatch"
 * .endif
 *
 * but most assemblers die if insn1 or insn2 have a .inst. This should
 * be fixed in a binutils release posterior to 2.25.51.0.2 (anything
 * containing commit 4e4d08cf7399b606 or c1baaddf8861).
 */
#define __ALTERNATIVE_CFG(oldinstr, newinstr, feature, cfg_enabled)     \
        ".if "#cfg_enabled" == 1\n"                                     \
        "661:\n\t"                                                      \
        oldinstr "\n"                                                   \
        "662:\n"                                                        \
        ".pushsection .altinstructions,\"a\"\n"                         \
        " .word 661b - .\n"                                             \
        " .word 663f - .\n"                                             \
        " .hword "#feature"\n"                                          \
        " .byte 662b-661b\n"                                            \
        " .byte 664f-663f\n"                                            \
        ".popsection\n"                                                 \
        ".pushsection .altinstr_replacement, \"a\"\n"                   \
        "663:\n\t"                                                      \
        newinstr "\n"                                                   \
        "664:\n\t"                                                      \
        ".popsection\n\t"                                               \
        ".org   . - (664b-663b) + (662b-661b)\n\t"                      \
        ".org   . - (662b-661b) + (664b-663b)\n"                        \
        ".endif\n"

#define _ALTERNATIVE_CFG(oldinstr, newinstr, feature, cfg, ...) \
        __ALTERNATIVE_CFG(oldinstr, newinstr, feature, IS_ENABLED(cfg))

#define ALTERNATIVE(oldinstr, newinstr, ...)   _ALTERNATIVE_CFG(oldinstr, newinstr, __VA_ARGS__, 1)

static inline u8 __raw_readb(const volatile void __iomem *addr)
{
	u8 val;
	asm volatile(ALTERNATIVE("ldrb %w0, [%1]",
				 "ldarb %w0, [%1]",
				 ARM64_WORKAROUND_DEVICE_LOAD_ACQUIRE)
		     : "=r" (val) : "r" (addr));
	return val;
}

static inline u16 __raw_readw(const volatile void __iomem *addr)
{
	u16 val;

	asm volatile(ALTERNATIVE("ldrh %w0, [%1]",
				 "ldarh %w0, [%1]",
				 ARM64_WORKAROUND_DEVICE_LOAD_ACQUIRE)
		     : "=r" (val) : "r" (addr));
	return val;
}

static inline u32 __raw_readl(const volatile void __iomem *addr)
{
	u32 val;
	asm volatile(ALTERNATIVE("ldr %w0, [%1]",
				 "ldar %w0, [%1]",
				 ARM64_WORKAROUND_DEVICE_LOAD_ACQUIRE)
		     : "=r" (val) : "r" (addr));
	return val;
}

static inline u64 __raw_readq(const volatile void __iomem *addr)
{
	u64 val;
	asm volatile(ALTERNATIVE("ldr %0, [%1]",
				 "ldar %0, [%1]",
				 ARM64_WORKAROUND_DEVICE_LOAD_ACQUIRE)
		     : "=r" (val) : "r" (addr));
	return val;
}

#else
static inline u8 __raw_readb(const volatile void __iomem *addr)
{
	u8 val;
	asm volatile("ldrb %w0, [%1]" : "=r" (val) : "r" (addr));
	return val;
}

static inline u16 __raw_readw(const volatile void __iomem *addr)
{
	u16 val;

	asm volatile("ldrh %w0, [%1]" : "=r" (val) : "r" (addr));
	return val;
}

static inline u32 __raw_readl(const volatile void __iomem *addr)
{
	u32 val;
	asm volatile("ldr %w0, [%1]" : "=r" (val) : "r" (addr));
	return val;
}

static inline u64 __raw_readq(const volatile void __iomem *addr)
{
	u64 val;
	asm volatile("ldr %0, [%1]" : "=r" (val) : "r" (addr));
	return val;
}
#endif /* 0 */

/*
 * Generic IO read/write.  These perform native-endian accesses.
 */
static inline void __raw_writeb(u8 val, volatile void __iomem *addr)
{
	asm volatile("strb %w0, [%1]" : : "r" (val), "r" (addr));
}

static inline void __raw_writew(u16 val, volatile void __iomem *addr)
{
	asm volatile("strh %w0, [%1]" : : "r" (val), "r" (addr));
}

static inline void __raw_writel(u32 val, volatile void __iomem *addr)
{
	asm volatile("str %w0, [%1]" : : "r" (val), "r" (addr));
}

static inline void __raw_writeq(u64 val, volatile void __iomem *addr)
{
	asm volatile("str %0, [%1]" : : "r" (val), "r" (addr));
}


/*
 * Relaxed I/O memory access primitives. These follow the Device memory
 * ordering rules but do not guarantee any ordering relative to Normal memory
 * accesses.
 */
#define readb_relaxed(c)	({ u8  __r = __raw_readb(c); __r; })
#define readw_relaxed(c)	({ u16 __r = le16toh(__raw_readw(c)); __r; })
#define readl_relaxed(c)	({ u32 __r = le32toh(__raw_readl(c)); __r; })
#define readq_relaxed(c)	({ u64 __r = le64toh(__raw_readq(c)); __r; })

#define writeb_relaxed(v,c)	((void)__raw_writeb((v),(c)))
#define writew_relaxed(v,c)	((void)__raw_writew(htole16(v),(c)))
#define writel_relaxed(v,c)	((void)__raw_writel(htole32(v),(c)))
#define writeq_relaxed(v,c)	((void)__raw_writeq(htole64(v),(c)))

/*
 * I/O memory access primitives. Reads are ordered relative to any
 * following Normal memory access. Writes are ordered relative to any prior
 * Normal memory access.
 */
#define readb(c)		({ u8  __v = readb_relaxed(c); __iormb(); __v; })
#define readw(c)		({ u16 __v = readw_relaxed(c); __iormb(); __v; })
#define readl(c)		({ u32 __v = readl_relaxed(c); __iormb(); __v; })
#define readq(c)		({ u64 __v = readq_relaxed(c); __iormb(); __v; })

#define writeb(v,c)		({ __iowmb(); writeb_relaxed((v),(c)); })
#define writew(v,c)		({ __iowmb(); writew_relaxed((v),(c)); })
#define writel(v,c)		({ __iowmb(); writel_relaxed((v),(c)); })
#define writeq(v,c)		({ __iowmb(); writeq_relaxed((v),(c)); })
#endif /* 0 */

#endif /* __IO_H__ */
