make clean
make
sudo dmesg --clear
sudo insmod page_table.ko
dmesg
sudo rmmod page_table
make clean
