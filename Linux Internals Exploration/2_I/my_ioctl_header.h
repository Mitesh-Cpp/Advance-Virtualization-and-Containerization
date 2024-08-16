#define IOCTL_GET_PHYSICAL_ADDR _IOWR('m', 0, struct GET_P_ADDR)
#define IOCTL_WRITE_PHYSICAL_MEM _IOR('m', 1, struct WRITE_P_MEM)

// required structure for returning the physical address corresponding to a virtual address for the process doing IOCTL call
struct GET_P_ADDR {
  unsigned long virtual_addr;
  unsigned long physical_addr;
};

// required structure for taking the physical address from user process doing IOCTL call and writing on that address from the kernel 
struct WRITE_P_MEM {
  unsigned long physical_addr;
  unsigned char value;
};