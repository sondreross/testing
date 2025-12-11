# CPU Isolation Configuration Guide

## Overview
This guide assumes you're isolating CPU core 3, with cores 0-2 and 4-N available for system tasks.

## 1. Kernel Boot Parameters (Already Done)
You mentioned these are already configured:
```
nohz_full=3 rcu_nocbs=3 irqaffinity=0-2,4-N housekeeping=0-2,4-N isolcpus=3
```

Verify in `/proc/cmdline`:
```bash
cat /proc/cmdline
```

## 2. BIOS Settings

### Disable Turbo Boost
**Intel:**
- Enter BIOS (usually F2, F12, or Del during boot)
- Look for: Advanced → CPU Configuration → Intel Turbo Boost Technology → Disabled
- Alternative names: "Turbo Mode", "Performance Mode"

**AMD:**
- Look for: CPU Configuration → Core Performance Boost → Disabled

### Disable P-States (Intel SpeedStep / AMD Cool'n'Quiet)
**Intel:**
- BIOS: Advanced → CPU Configuration → Intel SpeedStep → Disabled
- Or via Linux: `echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo`

**AMD:**
- BIOS: Advanced → CPU Configuration → Cool'n'Quiet → Disabled

### Set CPU Governor to Performance
```bash
# Check current governor
cat /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor

# Set to performance (runtime, needs to be done after each boot)
echo performance | sudo tee /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor

# Or set for all CPUs
for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
    echo performance | sudo tee $cpu
done
```

Make permanent with systemd service (create `/etc/systemd/system/cpu-performance.service`):
```ini
[Unit]
Description=Set CPU governor to performance

[Service]
Type=oneshot
ExecStart=/bin/bash -c 'echo performance | tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor'

[Install]
WantedBy=multi-user.target
```

Enable it:
```bash
sudo systemctl enable cpu-performance.service
sudo systemctl start cpu-performance.service
```

### Hyperthreading Note
If you can't find it in BIOS, check if it exists:
```bash
# Check for sibling threads
cat /sys/devices/system/cpu/cpu3/topology/thread_siblings_list

# If output shows something like "3,7" then core 3 and 7 are siblings
# You would need: isolcpus=3,7 nohz_full=3,7 rcu_nocbs=3,7
```

## 3. User-Space Optimizations

### Disable IRQ Balancing (systemd)

**First, check if irqbalance is installed:**
```bash
systemctl status irqbalance
```

**If it exists, stop and disable it:**
```bash
sudo systemctl stop irqbalance
sudo systemctl disable irqbalance
sudo systemctl mask irqbalance
```

**If you get "Unit irqbalance.service could not be found"** - that's fine! It means irqbalance isn't installed/running on your system, which is actually good for CPU isolation. You can skip this step.

### Verify and Fix IRQ Affinity

**Check current IRQ assignments:**
```bash
# List all IRQs and their affinity
for irq in /proc/irq/*/smp_affinity_list; do
    echo "$irq: $(cat $irq)"
done | grep -v "N/A"
```

**Find IRQs assigned to your isolated core (3):**
```bash
for irq in /proc/irq/*/smp_affinity_list; do
    if grep -q "3" "$irq" 2>/dev/null; then
        echo "$irq: $(cat $irq)"
    fi
done
```

**Move IRQs away from isolated core:**
```bash
# Create a script to move all IRQs to cores 0-2
#!/bin/bash

ISOLATED_CORE=3
ALLOWED_CORES="0-2"  # Adjust based on your system

for IRQ in $(ls /proc/irq/ | grep -E '^[0-9]+$'); do
    # Skip special IRQs
    if [ ! -f "/proc/irq/$IRQ/smp_affinity_list" ]; then
        continue
    fi
    
    # Get current affinity
    CURRENT=$(cat /proc/irq/$IRQ/smp_affinity_list 2>/dev/null)
    
    # If IRQ can be moved (not all can be)
    if echo "$ALLOWED_CORES" > /proc/irq/$IRQ/smp_affinity_list 2>/dev/null; then
        echo "Moved IRQ $IRQ from $CURRENT to $ALLOWED_CORES"
    fi
done
```

Save as `/usr/local/bin/isolate-irqs.sh`, make executable:
```bash
sudo chmod +x /usr/local/bin/isolate-irqs.sh
```

**Make IRQ isolation permanent with systemd service** (`/etc/systemd/system/isolate-irqs.service`):
```ini
[Unit]
Description=Isolate IRQs from CPU 3
After=network.target

[Service]
Type=oneshot
ExecStart=/usr/local/bin/isolate-irqs.sh
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
```

Enable it:
```bash
sudo systemctl enable isolate-irqs.service
sudo systemctl start isolate-irqs.service
```

## 4. Running Your Program on the Isolated Core

### Method 1: Using taskset
```bash
# Run program on CPU 3
taskset -c 3 ./your_program [arguments]

# Verify it's running on correct CPU
taskset -cp $(pgrep your_program)
```

### Method 2: Using cset (CPU Shield)
```bash
# Install cset
sudo apt-get install cpuset  # Debian/Ubuntu
sudo yum install cpuset       # RHEL/CentOS

# Create an isolated CPU set
sudo cset shield --cpu 3

# Run program in the isolated set
sudo cset shield --exec ./your_program -- [arguments]
```

### Method 3: Setting CPU Affinity in Code (C/C++)

```c
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>

int main() {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(3, &cpuset);  // Isolate to CPU 3
    
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == -1) {
        perror("sched_setaffinity");
        return 1;
    }
    
    // Your program logic here
    return 0;
}
```

### Method 4: Set Process Priority (Optional)
```bash
# Run with real-time priority (requires root or CAP_SYS_NICE)
sudo chrt -f 99 taskset -c 3 ./your_program

# -f: SCHED_FIFO scheduling
# 99: highest real-time priority (1-99)
```

## 5. Verification Commands

**Check isolated core is idle:**
```bash
# Monitor CPU usage per core
mpstat -P ALL 1

# Or use htop (press F2 → Display → check "Detailed CPU time")
htop
```

**Check IRQ activity on isolated core:**
```bash
# Watch interrupts per CPU
watch -n 1 'cat /proc/interrupts | head -n 30'
```

**Verify RCU callbacks are offloaded:**
```bash
# Check for RCU threads not on isolated core
ps aux | grep rcu | grep -v "grep"
```

**Check scheduler ticks:**
```bash
# Verify nohz_full is active
cat /sys/devices/system/cpu/nohz_full
# Should show: 3

# Check if tick is disabled when your process runs
cat /proc/sys/kernel/nohz_full_enabled
# Should show: 1
```

## 6. Complete Setup Script

Save this as `setup-isolation.sh`:

```bash
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

check_param "isolcpus=$ISOLATED_CORE"
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
for IRQ in $(ls /proc/irq/ | grep -E '^[0-9]+

## 7. Running Your Program

```bash
# Basic
taskset -c 3 ./your_program

# With real-time priority
sudo chrt -f 99 taskset -c 3 ./your_program

# With nice value (if not using real-time)
taskset -c 3 nice -n -20 ./your_program
```

## Troubleshooting

**If nohz_full doesn't work:**
- Ensure `CONFIG_NO_HZ_FULL=y` in kernel config: `grep NO_HZ_FULL /boot/config-$(uname -r)`
- Requires kernel 3.10+

**If still seeing interrupts:**
- Some hardware interrupts can't be moved (like local APIC timer)
- Use `perf` to identify sources: `sudo perf stat -C 3 -e 'irq:*' sleep 10`

**Performance validation:**
```bash
# Run your program and check for interruptions
sudo perf stat -C 3 -e context-switches,cpu-migrations ./your_program
# Goal: minimal context-switches and zero cpu-migrations
```
); do
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

echo ""
echo "Setup complete! Run your program with:"
echo "  taskset -c $ISOLATED_CORE ./your_program"
```

## 7. Running Your Program

```bash
# Basic
taskset -c 3 ./your_program

# With real-time priority
sudo chrt -f 99 taskset -c 3 ./your_program

# With nice value (if not using real-time)
taskset -c 3 nice -n -20 ./your_program
```

## Troubleshooting

**If nohz_full doesn't work:**
- Ensure `CONFIG_NO_HZ_FULL=y` in kernel config: `grep NO_HZ_FULL /boot/config-$(uname -r)`
- Requires kernel 3.10+

**If still seeing interrupts:**
- Some hardware interrupts can't be moved (like local APIC timer)
- Use `perf` to identify sources: `sudo perf stat -C 3 -e 'irq:*' sleep 10`

**Performance validation:**
```bash
# Run your program and check for interruptions
sudo perf stat -C 3 -e context-switches,cpu-migrations ./your_program
# Goal: minimal context-switches and zero cpu-migrations
```