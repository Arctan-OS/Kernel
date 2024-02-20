PREFIX_DIR := $(ARCTAN_PORTS_HOME)/mlibc

MESON_FLAGS :=  --cross-file arctan_crossfile.txt --prefix=$(PREFIX_DIR) \
				--sysconfdir=/etc --localstatedir=/var --libdir=lib --sbindir=bin --buildtype=release -Ddefault_library=shared \
				--buildtype=debugoptimized -Dmlibc_no_headers=true -Ddefault_library=both \
				-Ddisable_ansi_option=false -Ddisable_crypt_option=true -Ddisable_posix_option=false -Ddisable_linux_option=true \
				-Ddisable_iconv_option=true -Ddisable_intl_option=true -Ddisable_glibc_option=false -Ddisable_bsd_option=false \
				-Ddisable_libgcc_dependency=false

MESON_BUILD_DIR := build

.PHONY: all
all: clean
	git clone https://github.com/managarm/mlibc
	cp arctan_crossfile.txt mlibc/
	cp -r sysdeps/* mlibc/sysdeps
	cd mlibc/ && git apply ../meson.build.patch
	cd mlibc/ && meson setup $(MESON_BUILD_DIR) $(MESON_FLAGS)
	meson install -C $(shell pwd)/mlibc/$(MESON_BUILD_DIR)
	rm -rf mlibc/

.PHONY: clean
clean:
	rm -rf mlibc/
