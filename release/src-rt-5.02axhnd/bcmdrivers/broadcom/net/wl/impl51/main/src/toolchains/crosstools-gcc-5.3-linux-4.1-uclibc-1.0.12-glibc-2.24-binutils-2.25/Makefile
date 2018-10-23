# Makefile for toolchain building
REL_VER_MAJOR=1
REL_VER_MINOR=9

BR_VER=2016.02
UCLIBC_VER=1.0.12
GLIBC_VER=2.24
LINUX_VER=4.1
GCC_VER=5.3
BINUTILS_VER=2.25

ARCH := $(TARCH)
ifeq ($(ARCH),)
ARCH := all
endif

# Set default libc type and version.
MIPS_LIBC = uclibc
MIPS_LIBC_VER = $(UCLIBC_VER)
ARM_LIBC = glibc
ARM_LIBC_VER = $(GLIBC_VER)
AARCH64_LIBC = glibc
AARCH64_LIBC_VER = $(GLIBC_VER)

# override the libc type and version for all targets with specified clib
ifneq ($(strip $(CLIB)),)
ifeq ($(CLIB), $(filter $(CLIB),glibc))
MIPS_LIBC = glibc
ARM_LIBC = glibc
AARCH64_LIBC = glibc
MIPS_LIBC_VER = $(GLIBC_VER)
ARM_LIBC_VER = $(GLIBC_VER)
AARCH64_LIBC_VER = $(GLIBC_VER)
else
ifeq ($(CLIB), $(filter $(CLIB),uclibc))
MIPS_LIBC = uclibc
ARM_LIBC = uclibc
AARCH64_LIBC = uclibc
MIPS_LIBC_VER = $(UCLIBC_VER)
ARM_LIBC_VER = $(UCLIBC_VER)
AARCH64_LIBC_VER = $(UCLIBC_VER)
endif
endif
endif

ifeq ($(strip $(BCM_TOOLCHAIN_VER)),)
BCM_TOOLCHAIN_VER=Rel$(REL_VER_MAJOR).$(REL_VER_MINOR)
endif
ifeq ($(strip $(TOOLCHAIN_TOP)),)
TOOLCHAIN_TOP=/opt/toolchains
endif

# Name of toolchain source archive
TOOLCHAIN_SRC_NAME=crosstools-gcc-$(GCC_VER)-linux-$(LINUX_VER)-uclibc-$(UCLIBC_VER)-glibc-$(GLIBC_VER)-binutils-$(BINUTILS_VER)-sources

# Name of toolchain tarball archive
TOOLCHAIN_FULL_TARBALL_NAME=crosstools-gcc-$(GCC_VER)-linux-$(LINUX_VER)-uclibc-$(UCLIBC_VER)-glibc-$(GLIBC_VER)-binutils-$(BINUTILS_VER)-$(BCM_TOOLCHAIN_VER)-full

# Names of individual toolchain directories
TOOLCHAIN_AARCH64_NAME=crosstools-aarch64-gcc-$(GCC_VER)-linux-$(LINUX_VER)-$(AARCH64_LIBC)-$(AARCH64_LIBC_VER)-binutils-$(BINUTILS_VER)
ifeq ($(ARM_LIBC), $(filter $(ARM_LIBC),glibc))
TOOLCHAIN_ARM_NAME=crosstools-arm-gcc-$(GCC_VER)-linux-$(LINUX_VER)-$(ARM_LIBC)-$(ARM_LIBC_VER)-binutils-$(BINUTILS_VER)
else
TOOLCHAIN_ARM_NAME=crosstools-arm-gcc-$(GCC_VER)-linux-$(LINUX_VER)-$(ARM_LIBC)-$(ARM_LIBC_VER)-binutils-$(BINUTILS_VER)-NPTL
endif
TOOLCHAIN_MIPS_NAME=crosstools-mips-gcc-$(GCC_VER)-linux-$(LINUX_VER)-$(MIPS_LIBC)-$(MIPS_LIBC_VER)-binutils-$(BINUTILS_VER)-NPTL
TOOLCHAIN_MIPSEL_NAME=crosstools-mipsel-gcc-$(GCC_VER)-linux-$(LINUX_VER)-$(MIPS_LIBC)-$(MIPS_LIBC_VER)-binutils-$(BINUTILS_VER)-NPTL

export BR_VER LINUX_VER GCC_VER BINUTILS_VER TOOLCHAIN_TOP ARCH MIPS_LIBC MIPS_LIBC_VER ARM_LIBC ARM_LIBC_VER AARCH64_LIBC AARCH64_LIBC_VER

all: sanity_check
	cd ./src;./build
ifeq ($(ARCH), $(filter $(ARCH),mips all))
ifeq ($(MIPS_LIBC), $(filter $(MIPS_LIBC),uclibc))
	echo "#define BCM_TOOLCHAIN_VERSION \"$(BCM_TOOLCHAIN_VER)\"" >  $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPS_NAME)/usr/include/bcm_toolver.h
	echo "#define BCM_TOOLCHAIN_MAJOR $(REL_VER_MAJOR)" >>  $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPS_NAME)/usr/include/bcm_toolver.h
	echo "#define BCM_TOOLCHAIN_MINOR $(REL_VER_MINOR)" >>  $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPS_NAME)/usr/include/bcm_toolver.h
	cp $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPS_NAME)/usr/include/bcm_toolver.h $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPS_NAME)/usr/mips-buildroot-linux-uclibc/sysroot/usr/include/bcm_toolver.h
else	
	@echo "glibc builds for MIPS are currently disabled in Makefile"
#	cp $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPS_NAME)/usr/include/bcm_toolver.h $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPS_NAME)/usr/mips-buildroot-linux-gnu/sysroot/usr/include/bcm_toolver.h
endif
endif

ifeq ($(ARCH), $(filter $(ARCH),arm all))
	echo "#define BCM_TOOLCHAIN_VERSION \"$(BCM_TOOLCHAIN_VER)\"" >  $(TOOLCHAIN_TOP)/$(TOOLCHAIN_ARM_NAME)/usr/include/bcm_toolver.h
	echo "#define BCM_TOOLCHAIN_MAJOR $(REL_VER_MAJOR)" >>  $(TOOLCHAIN_TOP)/$(TOOLCHAIN_ARM_NAME)/usr/include/bcm_toolver.h
	echo "#define BCM_TOOLCHAIN_MINOR $(REL_VER_MINOR)" >>  $(TOOLCHAIN_TOP)/$(TOOLCHAIN_ARM_NAME)/usr/include/bcm_toolver.h
ifeq ($(ARM_LIBC), $(filter $(ARM_LIBC),glibc))
	cp $(TOOLCHAIN_TOP)/$(TOOLCHAIN_ARM_NAME)/usr/include/bcm_toolver.h $(TOOLCHAIN_TOP)/$(TOOLCHAIN_ARM_NAME)/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bcm_toolver.h
else	
	cp $(TOOLCHAIN_TOP)/$(TOOLCHAIN_ARM_NAME)/usr/include/bcm_toolver.h $(TOOLCHAIN_TOP)/$(TOOLCHAIN_ARM_NAME)/usr/arm-buildroot-linux-uclibcgnueabi/sysroot/usr/include/bcm_toolver.h
endif
endif

ifeq ($(ARCH), $(filter $(ARCH),aarch64 all))
ifeq ($(AARCH64_LIBC), $(filter $(AARCH64_LIBC),glibc))
	echo "#define BCM_TOOLCHAIN_VERSION \"$(BCM_TOOLCHAIN_VER)\"" >  $(TOOLCHAIN_TOP)/$(TOOLCHAIN_AARCH64_NAME)/usr/include/bcm_toolver.h
	echo "#define BCM_TOOLCHAIN_MAJOR $(REL_VER_MAJOR)" >>  $(TOOLCHAIN_TOP)/$(TOOLCHAIN_AARCH64_NAME)/usr/include/bcm_toolver.h
	echo "#define BCM_TOOLCHAIN_MINOR $(REL_VER_MINOR)" >>  $(TOOLCHAIN_TOP)/$(TOOLCHAIN_AARCH64_NAME)/usr/include/bcm_toolver.h
	cp $(TOOLCHAIN_TOP)/$(TOOLCHAIN_AARCH64_NAME)/usr/include/bcm_toolver.h $(TOOLCHAIN_TOP)/$(TOOLCHAIN_AARCH64_NAME)/usr/aarch64-buildroot-linux-gnu/sysroot/usr/include/bcm_toolver.h
else
	@echo "uClibc does not support AARCH64"
endif
endif

ifeq ($(ARCH), $(filter $(ARCH),mipsel all))
ifeq ($(MIPS_LIBC), $(filter $(MIPS_LIBC),uclibc))
	echo "#define BCM_TOOLCHAIN_VERSION \"$(BCM_TOOLCHAIN_VER)\"" >  $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPSEL_NAME)/usr/include/bcm_toolver.h
	echo "#define BCM_TOOLCHAIN_MAJOR $(REL_VER_MAJOR)" >>  $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPSEL_NAME)/usr/include/bcm_toolver.h
	echo "#define BCM_TOOLCHAIN_MINOR $(REL_VER_MINOR)" >>  $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPSEL_NAME)/usr/include/bcm_toolver.h
	cp $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPSEL_NAME)/usr/include/bcm_toolver.h $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPSEL_NAME)/usr/mipsel-buildroot-linux-uclibc/sysroot/usr/include/bcm_toolver.h
else	
	@echo "glibc builds for MIPS are currently disabled in Makefile"
#	cp $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPSEL_NAME)/usr/include/bcm_toolver.h $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPSEL_NAME)/usr/mipsel-buildroot-linux-gnu/sysroot/usr/include/bcm_toolver.h
endif	
endif
	@echo toolchain build done!

sanity_check:
	@if [ ! -e ./src/buildroot-$(BR_VER) ]; then \
		echo Untarring buildroot-$(BR_VER) source...; \
		(tar xfjk $(TOOLCHAIN_SRC_NAME).tar.bz2 2> /dev/null || true); \
	fi

mips_clean:
ifeq ($(ARCH), $(filter $(ARCH),mips all))
	rm -f $(TOOLCHAIN_MIPS_NAME)*.tar.bz2
	@if [ -e $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPS_NAME) ]; then \
		rm -r -f $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPS_NAME)/*; \
	fi
endif

arm_clean:
ifeq ($(ARCH), $(filter $(ARCH),arm all))
	rm -f $(TOOLCHAIN_ARM_NAME)*.tar.bz2
	@if [ -e $(TOOLCHAIN_TOP)/$(TOOLCHAIN_ARM_NAME) ]; then \
		rm -r -f $(TOOLCHAIN_TOP)/$(TOOLCHAIN_ARM_NAME)/*; \
	fi
endif

aarch64_clean:
ifeq ($(ARCH), $(filter $(ARCH),aarch64 all))
	rm -f $(TOOLCHAIN_ARM_NAME)*.tar.bz2
	@if [ -e $(TOOLCHAIN_TOP)/$(TOOLCHAIN_ARM_NAME) ]; then \
		rm -r -f $(TOOLCHAIN_TOP)/$(TOOLCHAIN_AARCH64_NAME)/*; \
	fi
endif

mipsel_clean:
ifeq ($(ARCH), $(filter $(ARCH),mipsel all))
	rm -f $(TOOLCHAIN_MIPSEL_NAME)*.tar.bz2
	@if [ -e $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPSEL_NAME) ]; then \
		rm -r -f $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPSEL_NAME)/*; \
	fi
endif

mips_release:
ifeq ($(ARCH), $(filter $(ARCH),mips all))
	@if [ -e $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPS_NAME) ]; then \
		rm -f $(TOOLCHAIN_MIPS_NAME)*.tar.bz2;\
		tar cfjP $(TOOLCHAIN_MIPS_NAME).$(BCM_TOOLCHAIN_VER).tar.bz2 $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPS_NAME)/; \
		tar --no-overwrite-dir -x -j -P -f $(TOOLCHAIN_MIPS_NAME).$(BCM_TOOLCHAIN_VER).tar.bz2; \
	fi
endif

arm_release:
ifeq ($(ARCH), $(filter $(ARCH),arm all))
	@if [ -e $(TOOLCHAIN_TOP)/$(TOOLCHAIN_ARM_NAME) ]; then \
		rm -f $(TOOLCHAIN_ARM_NAME)*.tar.bz2;\
		tar cfjP $(TOOLCHAIN_ARM_NAME).$(BCM_TOOLCHAIN_VER).tar.bz2 $(TOOLCHAIN_TOP)/$(TOOLCHAIN_ARM_NAME)/; \
		tar --no-overwrite-dir -x -j -P -f $(TOOLCHAIN_ARM_NAME).$(BCM_TOOLCHAIN_VER).tar.bz2; \
	fi
endif

aarch64_release:
ifeq ($(ARCH), $(filter $(ARCH),aarch64 all))
	@if [ -e $(TOOLCHAIN_TOP)/$(TOOLCHAIN_AARCH64_NAME) ]; then \
		rm -f $(TOOLCHAIN_AARCH64_NAME)*.tar.bz2;\
		tar cfjP $(TOOLCHAIN_AARCH64_NAME).$(BCM_TOOLCHAIN_VER).tar.bz2 $(TOOLCHAIN_TOP)/$(TOOLCHAIN_AARCH64_NAME)/; \
		tar --no-overwrite-dir -x -j -P -f $(TOOLCHAIN_AARCH64_NAME).$(BCM_TOOLCHAIN_VER).tar.bz2; \
	fi
endif

mipsel_release:
ifeq ($(ARCH), $(filter $(ARCH),mipsel all))
	@if [ -e $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPSEL_NAME) ]; then \
		rm -f $(TOOLCHAIN_MIPSEL_NAME)*.tar.bz2;\
		tar cfjP $(TOOLCHAIN_MIPSEL_NAME).$(BCM_TOOLCHAIN_VER).tar.bz2 $(TOOLCHAIN_TOP)/$(TOOLCHAIN_MIPSEL_NAME)/; \
		tar --no-overwrite-dir -x -j -P -f $(TOOLCHAIN_MIPSEL_NAME).$(BCM_TOOLCHAIN_VER).tar.bz2; \
	fi
endif

clean: mips_clean arm_clean aarch64_clean mipsel_clean
	@if [ -e ./src/buildroot-$(BR_VER) ]; then \
		rm -r -f ./src/buildroot-$(BR_VER); \
	fi

archive_src: mips_release arm_release aarch64_release mipsel_release
	rm -f $(TOOLCHAIN_SRC_NAME).tar.bz2
	$(MAKE) -i -C ./src/buildroot-$(BR_VER) clean 2> /dev/null
	tar \
	--exclude=src/buildroot-$(BR_VER)/.config \
	--exclude=src/buildroot-$(BR_VER)/.config.old \
	--exclude=src/buildroot-$(BR_VER)/..config.tmp \
	--exclude=src/buildroot-$(BR_VER)/package/uclibc/uclibc-$(UCLIBC_VER).brcm.config \
	-c -j -f $(TOOLCHAIN_SRC_NAME).tar.bz2 Makefile src/

build_full_tarball: archive_src
	rm -f $(TOOLCHAIN_FULL_TARBALL_NAME).tar.bz2
	cp README.txt.tmpl README.txt
	tar -c -j -f $(TOOLCHAIN_FULL_TARBALL_NAME).tar.bz2 *.bz2 README.txt
	rm -f README.txt

release: build_full_tarball

uclibc-menuconfig: sanity_check
	$(MAKE) -C ./src/buildroot-$(BR_VER) uclibc-menuconfig

srcdl: sanity_check
	$(MAKE) -C ./src/buildroot-$(BR_VER) source
