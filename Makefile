# COMP 3430 Operating Systems
# Winter 2025
# Franklin Bristow
#
# Students registered in this offering of the course are explicitly permitted
# to copy and use this Makefile for their own work.

CC = clang
CFLAGS = -Wall -Werror -Wextra -Wpedantic -g -D_FORTIFY_SOURCE=3

# if you want to run this on macOS under Lima, you should run
# make NQP_EXFAT=nqp_exfat_arm.o
NQP_EXFAT ?= nqp_exfat.o

.PHONY: clean

all: nqp_shell

nqp_shell: $(NQP_EXFAT)

clean:
	rm -rf nqp_shell nqp_shell.dSYM
