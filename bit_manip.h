
/* Simply loop until the bit reads as set */
static inline void wait_for_bit(void* reg, uint32_t val)
{
    while (!(mmio_read(reg) & val));
}

/* Set a bit, and wait for that bit to read as set */
static inline void set_bit_and_wait(void* reg, uint32_t val)
{
    uint32_t util_val;

    /* Set the necessary values, and then wait to make sure they stick */
    util_val = mmio_read(reg);
    util_val |= val;
    mmio_write(reg, util_val);
    wait_for_bit(reg, val);
}
