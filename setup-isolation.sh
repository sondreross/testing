#!/bin/bash

ISOLATED_CORE=3
ALLOWED_CORES="0-2,"

echo "Setting up CPU isolation for core $ISOLATED_CORE"

# 1. Stop IRQ balancing
echo "Stopping irqbalance..."
sudo systemctl stop irqbalance
sudo systemctl disable irqbalance

# 2. Move IRQs
echo "Moving IRQs away from isolated core..."
for IRQ in $(ls /proc/irq/ | grep -E '^[0-9]+$'); do
    if [ -f "/proc/irq/$IRQ/smp_affinity_list" ]; then
        echo "$ALLOWED_CORES" | sudo tee /proc/irq/$IRQ/smp_affinity_list > /dev/null 2>&1
    fi
done

# 3. Set CPU governor
echo "Setting CPU governor to performance..."
echo performance | sudo tee /sys/devices/system/cpu/cpu$ISOLATED_CORE/cpufreq/scaling_governor

# 4. Verify
echo "Verification:"
echo "  Kernel cmdline:"
cat /proc/cmdline | grep -o "nohz_full=[^ ]*"
cat /proc/cmdline | grep -o "rcu_nocbs=[^ ]*"
cat /proc/cmdline | grep -o "isolcpus=[^ ]*"

echo "  IRQs on isolated core:"
for irq in /proc/irq/*/smp_affinity_list; do
    if grep -q "$ISOLATED_CORE" "$irq" 2>/dev/null; then
        echo "    $irq: $(cat $irq)"
    fi
done

echo "Setup complete!"