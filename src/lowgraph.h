// lowlevel graphic control for X1turboZ
#if (!defined(__LOWGRAPH_H__))
#define __LOWGRAPH_H__

extern void setScreen(int is80, int hires);
extern void clearText(void);

extern void initDigitalPalette(void);

// Control GraphicRAM bank
extern void setDispGRAM(unsigned char bank);
extern void setWriteGRAM(unsigned char bank);

// Fast Clear VRAM
extern void clearGRAM(void);

// Set 4096Color Mode(Screen Size 320*200)
extern void set4096Mode(void);

#endif
