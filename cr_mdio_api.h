#ifndef __CR_MDIO_H__
#define __CR_MDIO_H__

//#include "cr_config.h"
#include "cr_type.h"

#define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.

#ifdef __cplusplus
extern "C" {
#endif
#if 0
    DLL_PUBLIC extern CR_STATUS cr_mdio_init(CR_U8 phy_addr, CR_U8 dev_addr, int sel);
    DLL_PUBLIC extern CR_STATUS cr_mdio_close(void);
    DLL_PUBLIC extern CR_STATUS cr_mdio_write(CR_U16 reg_addr, CR_U16 reg_val);
    DLL_PUBLIC extern CR_U16 cr_mdio_read(CR_U16 reg_addr);
    DLL_PUBLIC extern CR_STATUS cr_spi_init(int sel);
    DLL_PUBLIC extern CR_STATUS cr_spi_close(void);
    DLL_PUBLIC extern unsigned char cr_spi_GetBoardId(void);
    DLL_PUBLIC extern CR_U16 cr_spi_read_cs(unsigned short addr, unsigned char CodingCsb);
    DLL_PUBLIC extern CR_STATUS cr_spi_write_cs(unsigned short addr,unsigned short value, unsigned char CodingCsb);
#endif
    #if defined(_WIN32) || defined(__CYGWIN__)
    #include <windows.h>
    #define MSLEEP Sleep
    #else
    #include <unistd.h>
    #include <string.h>
    #define MSLEEP(t) usleep((t) * 1000)
    #endif
    
    #include "ftd2xx.h"

#ifdef __cplusplus
}
#endif

#endif // __CR_MDIO_H__
