#!/bin/bash

# This is the same as done in first round of real_second

# Used in experiments 4.2-26
ISOLATED_CORE=3
ALLOWED_CORES="0-2"

echo "=== CPU Isolation Setup for Core $ISOLATED_CORE ==="
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

# 1. Check kernel parameters
echo "[1/4] Checking kernel parameters..."
CMDLINE=$(cat /proc/cmdline)

check_param() {
    if echo "$CMDLINE" | grep -q "$1"; then
        echo "  ✓ $1 is set"
        return 0
    else
        echo "  ✗ $1 is MISSING - add to kernel boot parameters"
        return 1
    fi
}

check_param "nohz_full=$ISOLATED_CORE"
check_param "rcu_nocbs=$ISOLATED_CORE"
echo ""

# 2. Check/disable irqbalance
echo "[2/4] Checking irqbalance..."
if systemctl list-unit-files 2>/dev/null | grep -q irqbalance; then
    if systemctl is-active --quiet irqbalance; then
        systemctl stop irqbalance
        systemctl disable irqbalance
        echo "  ✓ irqbalance stopped and disabled"
    else
        echo "  ✓ irqbalance already disabled"
    fi
else
    echo "  ✓ irqbalance not installed (good)"
fi
echo ""

# 3. Move IRQs away from isolated core
echo "[3/4] Moving IRQs away from core $ISOLATED_CORE..."
MOVED=0
for IRQ in $(ls /proc/irq/ | grep -E '^[0-9]+$'); do
    if [ -f "/proc/irq/$IRQ/smp_affinity_list" ]; then
        if echo "$ALLOWED_CORES" > /proc/irq/$IRQ/smp_affinity_list 2>/dev/null; then
            ((MOVED++))
        fi
    fi
done
echo "  ✓ Moved $MOVED IRQs to cores $ALLOWED_CORES"
echo ""

# 4. Set CPU governor to performance
echo "[4/4] Setting CPU governor..."
if [ -f "/sys/devices/system/cpu/cpu$ISOLATED_CORE/cpufreq/scaling_governor" ]; then
    echo performance > /sys/devices/system/cpu/cpu$ISOLATED_CORE/cpufreq/scaling_governor
    GOVERNOR=$(cat /sys/devices/system/cpu/cpu$ISOLATED_CORE/cpufreq/scaling_governor)
    echo "  ✓ CPU $ISOLATED_CORE governor set to: $GOVERNOR"
else
    echo "  ⚠ CPU frequency scaling not available"
fi
echo ""

# Final check - count IRQs still on isolated core
echo "=== Verification ==="
IRQ_COUNT=0
for irq in /proc/irq/*/smp_affinity_list; do
    if grep -q "^$ISOLATED_CORE$\|,$ISOLATED_CORE,\|,$ISOLATED_CORE$\|^$ISOLATED_CORE," "$irq" 2>/dev/null; then
        ((IRQ_COUNT++))
    fi
done

if [ $IRQ_COUNT -eq 0 ]; then
    echo "✓ No IRQs on isolated core $ISOLATED_CORE"
else
    echo "⚠ Warning: $IRQ_COUNT IRQs still assigned to core $ISOLATED_CORE"
    echo "  (Some hardware IRQs cannot be moved)"
fi

echo "Setup complete! Run your program with:"
echo "  taskset -c $ISOLATED_CORE ./your_program"