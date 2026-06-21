#if (!defined(__DMA_H__))
#define __DMA_H__

// reset->xfer->enable
extern void resetDMA(void);
extern void enableDMA(void);
extern void xferM2GRAM(unsigned short src, unsigned short dst, unsigned short size);
#endif
