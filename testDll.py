# -*- coding: utf-8 -*-
from ctypes import *
import random
import os
dll = cdll.LoadLibrary('cr_mdio.dll')

regAddr = 0x37ff#0xd

def mdioTest():
    #######################################################################
    #Mdio test
    dll.cr_mdio_init( 0, 1, 0, 0 )
    RetValDec = dll.cr_mdio_read( 0 )

    print ("RegAddr: 0,value:%x\n" % (RetValDec))

    dll.cr_mdio_close()
    return
'''
    opAdr=0x37ff
    wtVal=0x1111
    loop_times= 0x87ff-0x80ff
    for k in range(0, loop_times , 1):
        dll.cr_mdio_write( opAdr + k,wtVal )
        RetValDec = dll.cr_mdio_read( opAdr + k )
        print ("RegAddr: 0x%x,writed value:0x%x,read value:0x%x\n" % (opAdr+ k,wtVal,RetValDec))
'''
def i2c_16b_TestOnce():
    #######################################################################
    #I2C 16bit operation
    print("i2c_16b_Test ---------------->")
    dll.cr_mdio_init(0x00, 1, 0, 1)#0x50
    RetValDec = dll.cr_mdio_read(regAddr)#dll.cr_read_i2c(0)#dll.cr_8bits_read_i2c(0)

    print ("RegAddr: 0x%x,value:0x%x\n" % (regAddr,RetValDec))

    dll.cr_mdio_close()
    print("i2c_16b_Test <----------------")
    return

def Reg_readwrite_integrity_16bI2C_test(PhyId,loop_times):
    dll.cr_mdio_init(PhyId, 1, 0, 1)#0x50
    for k in range(1, loop_times + 1, 1):
        value_8a3c = random.randint(0, 0xff);#65535
        dll.cr_mdio_write(regAddr, value_8a3c); #wr 8A3c

        value_8a3d = random.randint(0, 0xff);
        dll.cr_mdio_write(regAddr+1, value_8a3d); #wr 8A3d

        value_8a3e = random.randint(0, 0xff);
        dll.cr_mdio_write(regAddr+2, value_8a3e); #wr 8A3e

        value_8a3f = random.randint(0, 0xff);
        dll.cr_mdio_write(regAddr+3, value_8a3f); #wr 8A3f

        value_8a40 = random.randint(0, 0xff);
        dll.cr_mdio_write(regAddr+4, value_8a40); #wr 8A40

        value_8a41 = random.randint(0, 0xff);
        dll.cr_mdio_write(regAddr+5, value_8a41); #wr 8A41

        value_8a96 = random.randint(0, 0xff);
        dll.cr_mdio_write(regAddr+6, value_8a96); #wr 8A96

        value_8a97 = random.randint(0, 0xff);
        dll.cr_mdio_write(regAddr+7, value_8a97); #wr 8A97

        value_8a98 = random.randint(0, 0xff);
        dll.cr_mdio_write(regAddr+8, value_8a98); #wr 8A98

        value_8a99 = random.randint(0, 0xff);
        dll.cr_mdio_write(regAddr+9, value_8a99); #wr 8A99

        read_8a3c = dll.cr_mdio_read( regAddr ); #rd 8A3c
        read_8a3d = dll.cr_mdio_read( regAddr+1) #  8A3d
        read_8a3e = dll.cr_mdio_read( regAddr+2); #  8A3e
        read_8a3f = dll.cr_mdio_read( regAddr+3); #  8A3f
        read_8a40 = dll.cr_mdio_read( regAddr+4); #  8A40
        read_8a41 = dll.cr_mdio_read( regAddr+5); #  8A41
        read_8a96 = dll.cr_mdio_read( regAddr+6); #  8A96
        read_8a97 = dll.cr_mdio_read( regAddr+7); #  8A97
        read_8a98 = dll.cr_mdio_read( regAddr+8); #  8A98
        read_8a99 = dll.cr_mdio_read( regAddr+9); #  8A99

        if (read_8a3c==value_8a3c) and (read_8a3d==value_8a3d) and (read_8a3e==value_8a3e) and (read_8a3f==value_8a3f) and (read_8a40==value_8a40) and \
           (read_8a41==value_8a41) and (read_8a96==value_8a96) and (read_8a97==value_8a97) and (read_8a98==value_8a98) and (read_8a99==value_8a99):
            if (k%100)==0:
                print ("%d"%(k));
        else:
            print ("Reg readwrite ERROR!")
#            raw_input();
    dll.cr_mdio_close()

def i2c_8b_TestOnce():

    #######################################################################
    # I2C 8bit operation
    dll.cr_mdio_init(0x50, 1, 0, 1)#0x50
    RetValDec = dll.cr_8bits_read_i2c(regAddr)
    print ("RegAddr: 0x%x,value:0x%x\n"  % (regAddr,RetValDec))
    
    RetValDec = dll.cr_8bits_read_i2c(regAddr + 1)
    print ("RegAddr: 0x%x,value:0x%x\n" % (regAddr + 1, RetValDec))

    RetValDec = dll.cr_8bits_read_i2c(regAddr + 2)
    print ("RegAddr: 0x%x,value:0x%x\n" % (regAddr + 2, RetValDec))

    RetValDec = dll.cr_8bits_read_i2c(regAddr + 3)
    print ("RegAddr: 0x%x,value:0x%x\n" % (regAddr + 3, RetValDec))

    RetValDec = dll.cr_8bits_read_i2c(regAddr + 4)
    print ("RegAddr: 0x%x,value:0x%x\n" % (regAddr + 4, RetValDec))

    RetValDec = dll.cr_8bits_read_i2c(regAddr + 5)
    print ("RegAddr: 0x%x,value:0x%x\n" % (regAddr + 5, RetValDec))

    RetValDec = dll.cr_8bits_read_i2c(regAddr + 6)
    print ("RegAddr: 0x%x,value:0x%x\n" % (regAddr + 6, RetValDec))

    RetValDec = dll.cr_8bits_read_i2c(regAddr + 7)
    print ("RegAddr: 0x%x,value:0x%x\n" % (regAddr + 7, RetValDec))

    dll.cr_8bits_write_i2c( regAddr,0xFF )
    RetValDec = dll.cr_8bits_read_i2c( regAddr )

    print ("RegAddr: 0x%x,writed value:0xFF,read value:0x%x\n" % (regAddr,RetValDec))

    dll.cr_mdio_close()

    return

def Reg_readwrite_integrity_8bI2C_test(PhyId,loop_times):
    dll.cr_mdio_init(PhyId, 1, 0, 1)#0x50
    for k in range(1, loop_times + 1, 1):
        value_8a3c = random.randint(0, 0xff);#65535
        dll.cr_8bits_write_i2c(regAddr, value_8a3c); #wr 8A3c

        value_8a3d = random.randint(0, 0xff);
        dll.cr_8bits_write_i2c(regAddr+1, value_8a3d); #wr 8A3d

        value_8a3e = random.randint(0, 0xff);
        dll.cr_8bits_write_i2c(regAddr+2, value_8a3e); #wr 8A3e

        value_8a3f = random.randint(0, 0xff);
        dll.cr_8bits_write_i2c(regAddr+3, value_8a3f); #wr 8A3f

        value_8a40 = random.randint(0, 0xff);
        dll.cr_8bits_write_i2c(regAddr+4, value_8a40); #wr 8A40

        value_8a41 = random.randint(0, 0xff);
        dll.cr_8bits_write_i2c(regAddr+5, value_8a41); #wr 8A41

        value_8a96 = random.randint(0, 0xff);
        dll.cr_8bits_write_i2c(regAddr+6, value_8a96); #wr 8A96

        value_8a97 = random.randint(0, 0xff);
        dll.cr_8bits_write_i2c(regAddr+7, value_8a97); #wr 8A97

        value_8a98 = random.randint(0, 0xff);
        dll.cr_8bits_write_i2c(regAddr+8, value_8a98); #wr 8A98

        value_8a99 = random.randint(0, 0xff);
        dll.cr_8bits_write_i2c(regAddr+9, value_8a99); #wr 8A99

        read_8a3c = dll.cr_8bits_read_i2c( regAddr ); #rd 8A3c
        read_8a3d = dll.cr_8bits_read_i2c( regAddr+1) #  8A3d
        read_8a3e = dll.cr_8bits_read_i2c( regAddr+2); #  8A3e
        read_8a3f = dll.cr_8bits_read_i2c( regAddr+3); #  8A3f
        read_8a40 = dll.cr_8bits_read_i2c( regAddr+4); #  8A40
        read_8a41 = dll.cr_8bits_read_i2c( regAddr+5); #  8A41
        read_8a96 = dll.cr_8bits_read_i2c( regAddr+6); #  8A96
        read_8a97 = dll.cr_8bits_read_i2c( regAddr+7); #  8A97
        read_8a98 = dll.cr_8bits_read_i2c( regAddr+8); #  8A98
        read_8a99 = dll.cr_8bits_read_i2c( regAddr+9); #  8A99

        if (read_8a3c==value_8a3c) and (read_8a3d==value_8a3d) and (read_8a3e==value_8a3e) and (read_8a3f==value_8a3f) and (read_8a40==value_8a40) and \
           (read_8a41==value_8a41) and (read_8a96==value_8a96) and (read_8a97==value_8a97) and (read_8a98==value_8a98) and (read_8a99==value_8a99):
            if (k%100)==0:
                print ("%d"%(k));
        else:
            print ("Reg readwrite ERROR!")
#            raw_input();
    dll.cr_mdio_close()


def Reg_readwrite_integrity_Mdio_test( loop_times ):
    dll.cr_mdio_init( 0, 1, 0, 0 )
    for k in range(1, loop_times + 1, 1):
        value_8a3c = random.randint(0, 65535);
        dll.cr_mdio_write( 0x37ff, value_8a3c ); #wr 8A3c

        value_8a3d = random.randint(0, 65535);
        dll.cr_mdio_write( 0x3800, value_8a3d ); #wr 8A3d

        value_8a3e = random.randint(0, 65535);
        dll.cr_mdio_write( 0x3801, value_8a3e ); #wr 8A3e

        value_8a3f = random.randint(0, 65535);
        dll.cr_mdio_write( 0x3802, value_8a3f ); #wr 8A3f

        value_8a40 = random.randint(0, 65535);
        dll.cr_mdio_write( 0x3803, value_8a40 ); #wr 8A40

        value_8a41 = random.randint(0, 65535);
        dll.cr_mdio_write( 0x3804, value_8a41 ); #wr 8A41

        value_8a96 = random.randint(0, 65535);
        dll.cr_mdio_write( 0x3805, value_8a96 ); #wr 8A96

        value_8a97 = random.randint(0, 65535);
        dll.cr_mdio_write( 0x3806, value_8a97 ); #wr 8A97

        value_8a98 = random.randint(0, 65535);
        dll.cr_mdio_write( 0x3807, value_8a98 ); #wr 8A98

        value_8a99 = random.randint(0, 65535);
        dll.cr_mdio_write( 0x3808, value_8a99 ); #wr 8A99

        read_8a3c = dll.cr_mdio_read( 0x37ff ); #rd 8A3c
        read_8a3d = dll.cr_mdio_read( 0x3800 ) #  8A3d
        read_8a3e = dll.cr_mdio_read( 0x3801 ); #  8A3e
        read_8a3f = dll.cr_mdio_read( 0x3802 ); #  8A3f
        read_8a40 = dll.cr_mdio_read( 0x3803 ); #  8A40
        read_8a41 = dll.cr_mdio_read( 0x3804 ); #  8A41
        read_8a96 = dll.cr_mdio_read( 0x3805 ); #  8A96
        read_8a97 = dll.cr_mdio_read( 0x3806 ); #  8A97
        read_8a98 = dll.cr_mdio_read( 0x3807 ); #  8A98
        read_8a99 = dll.cr_mdio_read( 0x3808 ); #  8A99
        ''' and (read_8a3d==value_8a3d) and (read_8a3e==value_8a3e) and (read_8a3f==value_8a3f) and (read_8a40==value_8a40) and \
       (read_8a41==value_8a41) and (read_8a96==value_8a96) and (read_8a97==value_8a97) and (read_8a98==value_8a98) and (read_8a99==value_8a99):'''
        if (read_8a3c==value_8a3c):
           if (k%100)==0:
               print ("%d"%(k));
        else:
            print ("Reg readwrite ERROR!")
            #raw_input();
    dll.cr_mdio_close()

    

if __name__ == '__main__':
    print("Test start:")
#    os.system("cmd")
#    os.system("ls")
#    command ="ver"
#    os.system(command)
################## 8bit I2C ##################
#    i2c_8b_TestOnce()
#    Reg_readwrite_integrity_8bI2C_test(0x50,100000);
################## 16bit I2C ####################
    i2c_16b_TestOnce()
#    Reg_readwrite_integrity_16bI2C_test(0x00, 2000);
####################################################
#    while(1):
#    mdioTest()

#    Reg_readwrite_integrity_Mdio_test(2000);
    raw_input("Test end!\n")
#######################################################################

