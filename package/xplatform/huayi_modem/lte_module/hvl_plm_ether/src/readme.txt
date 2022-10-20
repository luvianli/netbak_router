copy all the files to drivers/net/usb
add to Makefile

plm_ether-objs +=  qmi_oper.o \
		plm_cdc_ether.o 
                  
                  
obj-m +=  plm_ether.o