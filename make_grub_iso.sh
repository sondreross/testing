mkdir -p iso/boot/grub

# Copy only the IncludeOS binary (no chainloader)
cp build/hello_includeos iso/boot/hello_includeos.elf.bin
cp chainloader iso/boot/chainloader.elf

# Create GRUB config for direct boot
cat > iso/boot/grub/grub.cfg << 'EOF'
set timeout=10
set default=0

menuentry "IncludeOS - Direct Boot (No Chainloader)" {
    multiboot /boot/hello_includeos.elf.bin
}

menuentry "IncludeOS - With Chainloader" {
    multiboot /boot/chainloader.elf
    module /boot/hello_includeos.elf.bin
    boot
}
EOF

# Copy chainloader for comparison
cp chainloader iso/boot/chainloader.elf

# Create bootable ISO
grub-mkrescue --modules="multiboot iso9660 configfile normal" -o hello_includeos.iso iso

echo "ISO created: hello_includeos.iso"
echo "Files in ISO structure:"
find iso/ -type f
