# Assignment: EE542 Lab Assignment 04: Fast, Reliable File Transfer TCP

# Group: Zifan Wang & Yuhan Qiu

# Notice:
    This Lab modifies kernel level tcp and requires to recompile the linux kernel.

# Steps:
    1. Download the linux kernel (fileName.tar) from https://www.kernel.org/ 
    2. In Linux terminal: tar xvf fileName.tar
    3. In Linux terminal: cd <linux-kernel-folder>
    4. Modify the codes based on the following Modified Codes section.
    5. In Linux terminal: make menuconfig
    6. In Linux terminal: make -j $(nproc) deb-pkg LOCALVERSION=-custom
    7. In Linux terminal: dpkg -i linux-*.deb
    8. update-grub

# Idea: Remove Exponential Backoff in TCP

# Modified Codes:
    tcp.h: increase MAX_TCP_WINDOW and TCP_INIT_CWND
    tcp_timer.c: remove exponential backoff in retransmits_timed_out() function.
    tcp.c: remove timeout << 1 in secs_to_retrans() & retrans_to_secs() function.

# Reference:
    The idea to improve TCP throughput is based on the paper below
    1. Amit Mondal and Aleksandar Kuzmanovic. Removing exponential backoff from tcp. SIGCOMM Comput. Commun. Rev. , 38:17–28, September 2008.
    
