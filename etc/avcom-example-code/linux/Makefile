MNT_PATH=/mnt/api/source/
API_PATH=api_v2/
DEBUGGER_PATH=dev-debugger_v2/
CUSTOMER_PATH=customer-example_v2/
USBTEST_PATH=usb-test/
FTD2XX_PATH=~/libftd2xx1.0.0/

API_C=aovspecan
DEBUGGER_C=debugger
CUSTOMER_C=example
USBTEST_C=main

VERSION=0.1
LIBNAME=avcom
FTD2XX_LIBNAME=ftd2xx

CC=gcc


# In case all I want to do is copy the files over, the following make conditions will do the job
# without doing any other operation.
all:	api debugger customer usbtest

copy:	copy-api copy-debugger copy-customer copy-usbtest

copy-api:	
	mkdir -p ${API_PATH}
	cp ${MNT_PATH}${API_PATH}*.[ch] ./${API_PATH}
	cp ${FTD2XX_PATH}ftd2xx.h ./${API_PATH}
	cp ${FTD2XX_PATH}WinTypes.h ./${API_PATH}

copy-debugger:
	mkdir -p ${DEBUGGER_PATH}
	cp ${MNT_PATH}${DEBUGGER_PATH}*.[ch] ./${DEBUGGER_PATH}

copy-customer:
	mkdir -p ${CUSTOMER_PATH}
	cp ${MNT_PATH}${API_PATH}aovspecan.h ./${CUSTOMER_PATH}
	cp ${MNT_PATH}${CUSTOMER_PATH}*.[ch] ./${CUSTOMER_PATH}

copy-usbtest:
	mkdir -p ${USBTEST_PATH}
	cp ${MNT_PATH}${USBTEST_PATH}*.[ch] ./${USBTEST_PATH}
	cp ${FTD2XX_PATH}ftd2xx.h ./${USBTEST_PATH}
	cp ${FTD2XX_PATH}WinTypes.h ./${USBTEST_PATH}



# api is the Avcom API's, or Avcom shared library
api:	copy-api compile-api

api-nocopy:	compile-api

compile-api:
	$(CC) -Wall -fPIC -c ${API_PATH}${API_C}.c
	$(CC) -shared -Wl,-soname,lib${LIBNAME}.so.0 -o lib${LIBNAME}.so.${VERSION} ${API_C}.o
	/sbin/ldconfig -n .
	ln -sf lib${LIBNAME}.so.0 lib${LIBNAME}.so



# debugger is a project used to debug all features at individual intervals
# through a command-line utility
debugger:	copy-debugger compile-debugger

debugger-nocopy:	compile-debugger

compile-debugger:
	$(CC) -Wall -L. ${DEBUGGER_PATH}${DEBUGGER_C}.c -l${LIBNAME} -l${FTD2XX_LIBNAME} -o ${DEBUGGER_C}


# This is the customer example program that is included with the API's
customer:	copy-customer compile-customer

customer-nocopy:	compile-customer

compile-customer:
	$(CC) -Wall -L. ${CUSTOMER_PATH}${CUSTOMER_C}.c -l${LIBNAME} -l${FTD2XX_LIBNAME} -o ${CUSTOMER_C}

# A USB Connection testing project... probably gonna be obsolete soon
usbtest:	copy-usbtest compile-usbtest

usbtest-nocopy:		compile-usbtest

compile-usbtest:
	 $(CC) -Wall -L. ${USBTEST_PATH}${USBTEST_C}.c -l${LIBNAME} -l${FTD2XX_LIBNAME} -o ${USBTEST_C}


# And we gots to have clean!
clean:
	rm -rf ${API_PATH} ${DEBUGGER_PATH} ${CUSTOMER_PATH} ${USBTEST_PATH} *.so *.[01o] ${API_C} ${DEBUGGER_C} ${CUSTOMER_C} ${USBTEST_C}

