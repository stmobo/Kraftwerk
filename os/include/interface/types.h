#pragma once
#include <stddef.h>
#include <stdint.h>

typedef uint64_t  pmem_t;
typedef uintptr_t vmem_t;

// void int_handler(uint64_t err, vmem_t eip, uint16_t cs)
typedef void(*int_handler)(uint64_t, vmem_t, uint16_t);

typedef unsigned short irq_num_t;
typedef bool(*irq_handler)(irq_num_t);