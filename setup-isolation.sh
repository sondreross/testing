#!/bin/bash

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

ISOLATED_CPU=$ISOLATED_CORE
CPUSET_ROOT="/dev/cpuset"

# Detect total CPUs
TOTAL_CPUS=$(nproc)
LAST_CPU=$((TOTAL_CPUS - 1))

if [ "$ISOLATED_CPU" -gt "$LAST_CPU" ]; then
    echo "Error: CPU $ISOLATED_CPU doesn't exist. Valid range: 0-$LAST_CPU"
    exit 1
fi

# Build housekeeping CPU list
HOUSEKEEPING_CPUS="0-2"


echo "=== CPU Isolation with cpusets ==="
echo "Isolated CPU: $ISOLATED_CPU"
echo "Housekeeping CPUs: $HOUSEKEEPING_CPUS"
echo ""

# Mount cpuset if not already mounted
if ! mountpoint -q $CPUSET_ROOT 2>/dev/null; then
    echo "Mounting cpuset..."
    mkdir -p $CPUSET_ROOT
    mount -t cpuset none $CPUSET_ROOT
fi

# Create housekeeping cpuset
echo "Creating housekeeping cpuset..."
mkdir -p $CPUSET_ROOT/housekeeping
echo $HOUSEKEEPING_CPUS > $CPUSET_ROOT/housekeeping/cpus
echo 0 > $CPUSET_ROOT/housekeeping/mems

# Create isolated cpuset
echo "Creating isolated cpuset..."
mkdir -p $CPUSET_ROOT/isolated
echo $ISOLATED_CPU > $CPUSET_ROOT/isolated/cpus
echo 0 > $CPUSET_ROOT/isolated/mems
echo 1 > $CPUSET_ROOT/isolated/cpu_exclusive

# Move all user tasks to housekeeping
echo "Moving user tasks to housekeeping cpuset..."
for pid in $(cat $CPUSET_ROOT/tasks); do
    # Skip kernel threads (they can't be moved anyway)
    if [ -d /proc/$pid/task ]; then
        comm=$(cat /proc/$pid/comm 2>/dev/null || echo "")
        # Only move if not a kernel thread (kernel threads have brackets)
        if [[ ! "$comm" =~ ^\[.*\]$ ]]; then
            echo $pid > $CPUSET_ROOT/housekeeping/tasks 2>/dev/null || true
        fi
    fi
done

# Disable printk
echo "Disabling printk console output..."
echo 0 > /proc/sys/kernel/printk

echo ""
echo "=== Setup Complete ==="
echo ""
echo "Kernel threads are controlled by kernel boot parameters:"
echo "  irqaffinity=$HOUSEKEEPING_CPUS - keeps IRQ threads off isolated CPU"
echo "  rcu_nocbs=$ISOLATED_CPU - offloads RCU callbacks"
echo ""
echo "To run a program on isolated CPU $ISOLATED_CPU:"
echo "  sudo sh -c 'echo \$\$ > $CPUSET_ROOT/isolated/tasks && exec chrt -f 99 ./your_program'"
echo ""
echo "Required kernel boot parameters:"
echo "  nohz_full=$ISOLATED_CPU rcu_nocbs=$ISOLATED_CPU irqaffinity=$HOUSEKEEPING_CPUS nmi_watchdog=0"
echo ""
echo "Check interrupts:"
echo "  watch -d -n 1 \"cat /proc/interrupts | head -20\""
