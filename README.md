# Low Latency Market Data Processor

## Description

A high-performance, ultra-low latency market data processor leveraging DPDK for efficient packet handling and a custom TCP/IP stack for order transmission. This project implements market data parsing, order book management, and a basic trading strategy simulation with integrated network packet processing. It utilizes kernel bypass techniques and lock-free data structures to achieve minimal latency and maximum throughput.

## Features

- Efficient packet handling with DPDK kernel bypass
- Custom TCP/IP stack for order transmission
- AVX2 SIMD market data parsing
- Integrated network packet processing
- Lock-free data structures for maximum throughput
- Order book management
- Basic trading strategy simulation

## Requirements

- Ubuntu 20.04 or later
- GCC 9.3.0 or later
- CMake 3.16 or later
- DPDK 20.11 or later
- Git

## Setup

### Step 1: Install Dependencies

Update your package list and install the necessary packages:

    
    sudo apt update
    sudo apt install -y build-essential cmake git libnuma-dev
    

### Step 2: Install DPDK

Download and install DPDK:

    
    sudo apt install -y dpdk dpdk-dev libdpdk-dev 
    

Set up the environment variables for DPDK:

    
    echo "export RTE_SDK=/usr/share/dpdk" >> ~/.bashrc
    echo "export RTE_TARGET=x86_64-native-linuxapp-gcc" >> ~/.bashrc
    source ~/.bashrc
    

### Step 3: Configure Hugepages

Configure the system to use hugepages:

    
    sudo sysctl -w vm.nr_hugepages=1024
    sudo mkdir -p /mnt/huge
    sudo mount -t hugetlbfs nodev /mnt/huge
    echo "nodev /mnt/huge hugetlbfs defaults 0 0" | sudo tee -a /etc/fstab


### Step 4: Clone the Repository

Clone the project repository:

    
    git clone https://github.com/adelshaaban/low-latency-market-data-processor.git
    cd low-latency-market-data-processor
    
    

### Step 5: Configure Network Interface for DPDK

List your network interfaces and find the PCI address of the network interface you want to use with DPDK:

    
    sudo lspci | grep -i net
    

Bind the network interface to the DPDK-compatible driver (e.g., uio_pci_generic):

    
    sudo modprobe uio
    sudo modprobe uio_pci_generic
    sudo dpdk-devbind.py --bind=uio_pci_generic <PCI_ADDRESS>
    


### Step 6: Build the Project

Create a build directory and run CMake:

    
    mkdir cmake-build-debug
    cd cmake-build-debug
    cmake ..
    make
    

### Step 7: Run the Application

Run the application with root privileges:

    
    sudo ./Low_latency_DPDK
    

## Usage

The application will initialize DPDK, configure the network ports, and start processing market data. It will simulate market activity, process incoming network packets, and execute a basic trading strategy. The application prints statistics such as processed messages, message rates, and latencies.

## Enabling AVX2 SIMD

To enable AVX2 SIMD for performance optimization, ensure your CPU supports AVX2 instructions. Uncomment the SIMD code in `SIMDMessageParser.h`. By default, it is commented to ensure functionality across all devices. You can enable AVX2 SIMD in the compilation process by adding the following flags to your `CMakeLists.txt` or Makefile:

    
    -march=native -mavx2
    

## Troubleshooting


### Error: Interface is Active. Not Modifying

If the interface is active or cannot be modified:

List all network interfaces to find the interface name:

    ip link show

Check the interface status:

    ip link show <INTERFACE_NAME>

Bring down the interface:

    sudo ip link set <INTERFACE_NAME> down

Unbind the interface from the current driver:

    sudo dpdk-devbind.py --unbind <PCI_ADDRESS>

Bind the interface to the DPDK-compatible driver:

    sudo dpdk-devbind.py --bind=uio_pci_generic <PCI_ADDRESS>

### Error: Cannot Get Hugepage Information

If you see the error `Cannot get hugepage information`, ensure that hugepages are configured correctly and are available:

Check the current hugepage settings:

    
    grep HugePages /proc/meminfo
    

Ensure `HugePages_Total` is greater than 0 and `HugePages_Free` is not 0.

Reconfigure hugepages if necessary:

    
    sudo sysctl -w vm.nr_hugepages=1024
    sudo mkdir -p /mnt/huge
    sudo mount -t hugetlbfs nodev /mnt/huge
    

If hugepages are still not available, you can flush and reconfigure them:

    
    sudo umount /mnt/huge
    sudo rm -rf /mnt/huge/*
    sudo mkdir -p /mnt/huge
    sudo mount -t hugetlbfs nodev /mnt/huge
    sudo sh -c 'echo 0 > /proc/sys/vm/nr_hugepages'
    sudo sh -c 'echo 1024 > /proc/sys/vm/nr_hugepages'
    sudo rm -rf /var/run/dpdk/*
    

### Error: Port Not Available

If you see the error `Port 0 is not available`, ensure that the network interface is correctly bound to the DPDK driver:

Check the current status of your network devices:

    
    sudo dpdk-devbind.py --status
    

Bind the network interface to the DPDK-compatible driver:

    
    sudo dpdk-devbind.py --bind=uio_pci_generic <PCI_ADDRESS>
    

If you need to revert the binding:

    
    sudo dpdk-devbind.py --bind=e1000 <PCI_ADDRESS>
    

### General Debugging

Ensure all dependencies are installed and environment variables are correctly set:

    
    echo $RTE_SDK
    echo $RTE_TARGET
    

Check DPDK EAL logs for more detailed error messages. Running the application with a higher log level can provide additional insights:

    
    sudo ./Low_latency_DPDK --log-level=pmd.net_e1000:8
    

Verify that no other applications are using the network interface you are trying to bind to DPDK.

## Note
- You may need to adjust the configuration parameters in the source code to match your environment and requirements.
