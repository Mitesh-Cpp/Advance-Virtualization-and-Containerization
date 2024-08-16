#include <stdio.h>
#include <sys/mman.h>
#include <pthread.h>
#include <linux/kvm.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/ioctl.h>
#define _GNU_SOURCE
#include <unistd.h>
#include<time.h>
#include <signal.h>
#include <string.h>
// ----------------------------
// taken from the reference provided in assignment doc: "https://man7.org/linux/man-pages/man2/timer_create.2.html"
// from the declarations present inside the main() function in the given example
struct itimerspec its;
timer_t timerid;
struct sigevent sev;
struct sigaction sa;
// ----------------------------

void timer_handler(int sig) {
    // queried about it on moodle (title: "Does signal handler function clears the pending signal's bit..??"), received no reply
    // not 100% sure that whether it should be left empty or something should be done here
    // anyways the program works as expected on leaving it empty
}

#define KVM_DEVICE "/dev/kvm"
#define RAM_SIZE 512000000
#define CODE_START 0x1000
#define BINARY_FILE1 "guest1-b.bin"
#define BINARY_FILE2 "guest2-b.bin"
#define CURRENT_TIME ((double)clock() / CLOCKS_PER_SEC)

struct vm
{
    int dev_fd;
    int kvm_version;
    int vm_fd;
    struct kvm_userspace_memory_region mem;
    struct vcpu *vcpus;
    __u64 ram_size;
    __u64 ram_start;
    int vcpu_number;
};

struct vcpu
{
    int vcpu_id;
    int vcpu_fd;
    pthread_t vcpu_thread;
    struct kvm_run *kvm_run;
    int kvm_run_mmap_size;
    struct kvm_regs regs;
    struct kvm_sregs sregs;
    void *(*vcpu_thread_func)(void *);
};

void kvm_init(struct vm *vm1, struct vm *vm2)
{
    int dev_fd = open(KVM_DEVICE, O_RDWR);

    if (dev_fd < 0)
    {
        perror("open /dev/kvm");
        exit(1);
    }

    int kvm_version = ioctl(dev_fd, KVM_GET_API_VERSION, 0);

    if (kvm_version < 0)
    {
        perror("KVM_GET_API_VERSION");
        exit(1);
    }

    if (kvm_version != KVM_API_VERSION)
    {
        fprintf(stderr, "Got KVM api version %d, expected %d\n",
                kvm_version, KVM_API_VERSION);
        exit(1);
    }

    vm1->dev_fd = dev_fd;
    vm2->dev_fd = dev_fd;
    vm1->kvm_version = kvm_version;
    vm2->kvm_version = kvm_version;
}

int kvm_create_vm(struct vm *vm, int ram_size)
{
    int ret = 0;
    vm->vm_fd = ioctl(vm->dev_fd, KVM_CREATE_VM, 0);

    if (vm->vm_fd < 0)
    {
        perror("can not create vm");
        return -1;
    }

    vm->ram_size = ram_size;
    vm->ram_start = (__u64)mmap(NULL, vm->ram_size,
                                PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE,
                                -1, 0);

    if ((void *)vm->ram_start == MAP_FAILED)
    {
        perror("can not mmap ram");
        return -1;
    }

    vm->mem.slot = 0;
    vm->mem.guest_phys_addr = 0;
    vm->mem.memory_size = vm->ram_size;
    vm->mem.userspace_addr = vm->ram_start;

    ret = ioctl(vm->vm_fd, KVM_SET_USER_MEMORY_REGION, &(vm->mem));

    if (ret < 0)
    {
        perror("can not set user memory region");
        return ret;
    }

    return ret;
}

void load_binary(struct vm *vm, char *binary_file)
{
    int fd = open(binary_file, O_RDONLY);

    if (fd < 0)
    {
        fprintf(stderr, "can not open binary file\n");
        exit(1);
    }

    int ret = 0;
    char *p = (char *)vm->ram_start;

    while (1)
    {
        ret = read(fd, p, 4096);
        if (ret <= 0)
        {
            break;
        }
        printf("VMFD: %d, Loaded Program with size: %d\n", vm->vm_fd, ret);
        p += ret;
    }
}

struct vcpu *kvm_init_vcpu(struct vm *vm, int vcpu_id, void *(*fn)(void *))
{
    struct vcpu *vcpu = malloc(sizeof(struct vcpu));
    vcpu->vcpu_id = vcpu_id;
    vcpu->vcpu_fd = ioctl(vm->vm_fd, KVM_CREATE_VCPU, vcpu->vcpu_id);

    if (vcpu->vcpu_fd < 0)
    {
        perror("can not create vcpu");
        return NULL;
    }

    vcpu->kvm_run_mmap_size = ioctl(vm->dev_fd, KVM_GET_VCPU_MMAP_SIZE, 0);

    if (vcpu->kvm_run_mmap_size < 0)
    {
        perror("can not get vcpu mmsize");
        return NULL;
    }

    vcpu->kvm_run = mmap(NULL, vcpu->kvm_run_mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, vcpu->vcpu_fd, 0);

    if (vcpu->kvm_run == MAP_FAILED)
    {
        perror("can not mmap kvm_run");
        return NULL;
    }

    vcpu->vcpu_thread_func = fn;
    return vcpu;
}

void kvm_reset_vcpu(struct vcpu *vcpu)
{
    if (ioctl(vcpu->vcpu_fd, KVM_GET_SREGS, &(vcpu->sregs)) < 0)
    {
        perror("can not get sregs\n");
        exit(1);
    }

    vcpu->sregs.cs.selector = CODE_START;
    vcpu->sregs.cs.base = CODE_START * 16;
    vcpu->sregs.ss.selector = CODE_START;
    vcpu->sregs.ss.base = CODE_START * 16;
    vcpu->sregs.ds.selector = CODE_START;
    vcpu->sregs.ds.base = CODE_START * 16;
    vcpu->sregs.es.selector = CODE_START;
    vcpu->sregs.es.base = CODE_START * 16;
    vcpu->sregs.fs.selector = CODE_START;
    vcpu->sregs.fs.base = CODE_START * 16;
    vcpu->sregs.gs.selector = CODE_START;

    if (ioctl(vcpu->vcpu_fd, KVM_SET_SREGS, &vcpu->sregs) < 0)
    {
        perror("can not set sregs");
        exit(1);
    }

    vcpu->regs.rflags = 0x0000000000000002ULL;
    vcpu->regs.rip = 0;
    vcpu->regs.rsp = 0xffffffff;
    vcpu->regs.rbp = 0;

    if (ioctl(vcpu->vcpu_fd, KVM_SET_REGS, &(vcpu->regs)) < 0)
    {
        perror("KVM SET REGS\n");
        exit(1);
    }
}

void *kvm_cpu_thread(void *data)
{
    // Copy the code from this function to your code implementation in kvm_run_vm() and modify it accordingly
    struct vm *vm = (struct vm *)data;
    int ret = 0;
    kvm_reset_vcpu(vm->vcpus);

    while (1)
    {
        printf("VMFD: %d started running\n", vm->vm_fd);
        ret = ioctl(vm->vcpus->vcpu_fd, KVM_RUN, 0);

        printf("VMFD: %d stopped running - exit reason: %d\n", vm->vm_fd, vm->vcpus->kvm_run->exit_reason);

        switch (vm->vcpus->kvm_run->exit_reason)
        {
        case KVM_EXIT_UNKNOWN:
            printf("VMFD: %d KVM_EXIT_UNKNOWN\n", vm->vm_fd);
            break;
        case KVM_EXIT_DEBUG:
            printf("VMFD: %d KVM_EXIT_DEBUG\n", vm->vm_fd);
            break;
        case KVM_EXIT_IO:
            printf("VMFD: %d KVM_EXIT_IO\n", vm->vm_fd);
            printf("VMFD: %d out port: %d, data: %d\n", vm->vm_fd, vm->vcpus->kvm_run->io.port, *(int *)((char *)(vm->vcpus->kvm_run) + vm->vcpus->kvm_run->io.data_offset));
            sleep(1);
            break;
        case KVM_EXIT_MMIO:
            printf("VMFD: %d KVM_EXIT_MMIO\n", vm->vm_fd);
            break;
        case KVM_EXIT_INTR:
            printf("VMFD: %d KVM_EXIT_INTR\n", vm->vm_fd);
            break;
        case KVM_EXIT_SHUTDOWN:
            printf("VMFD: %d KVM_EXIT_SHUTDOWN\n", vm->vm_fd);
            goto exit_kvm;
            break;
        default:
            printf("VMFD: %d KVM PANIC\n", vm->vm_fd);
            printf("VMFD: %d KVM exit reason: %d\n", vm->vm_fd, vm->vcpus->kvm_run->exit_reason);
            goto exit_kvm;
        }

        if (ret < 0 && vm->vcpus->kvm_run->exit_reason != KVM_EXIT_INTR)
        {
            fprintf(stderr, "VMFD: %d KVM_RUN failed\n", vm->vm_fd);
            printf("VMFD: %d KVM_RUN return value %d\n", vm->vm_fd, ret);
            exit(1);
        }
    }

exit_kvm:
    return 0;
}

void kvm_run_vm(struct vm *vm1, struct vm *vm2)
{
    int ret = 0;
    kvm_reset_vcpu(vm1->vcpus);
    kvm_reset_vcpu(vm2->vcpus);
    // used simple logic to switch between both the VMs
    // turn variable 0 means its vm1's turn and 1 means its vm2's turn
    struct vm *vm_id[2] = {vm1, vm2};
    int turn = 0;
    //-----------------------------------------------------------------
    // portion coverec in this section taken from 2 major references:
    // First reference for creating and setting timer: "https://man7.org/linux/man-pages/man2/timer_create.2.html "
    // The above reference was provided in the assignment document.
    // Second reference for correctly using KVM_SET_SIGNAL_MASK ioctl call: "https://elixir.bootlin.com/linux/v6.1/source/tools/testing/selftests/kvm/dirty_log_test.c#L526"
    // searched for "KVM_SET_SIGNAL_MASK" in elixir bootling and found out the above reference
    sa.sa_handler = timer_handler;
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
        perror("sigaction");
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGUSR1;
    
    if (timer_create(CLOCK_MONOTONIC, &sev, &timerid) == -1)
        perror("timer_create");
    its.it_value.tv_sec = 1;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    if (timer_settime(timerid, 0, &its, NULL) == -1)
        perror("timer_settime");

	struct kvm_signal_mask *sigmask = (struct kvm_signal_mask *)malloc(sizeof(struct kvm_signal_mask));
    sigmask->len = 8;
    
    ioctl(vm1->vcpus->vcpu_fd, KVM_SET_SIGNAL_MASK, sigmask);
    ioctl(vm2->vcpus->vcpu_fd, KVM_SET_SIGNAL_MASK, sigmask);
    //-----------------------------------------------------------------
    
    while (1)
    {
        printf("VMFD: %d started running\n", vm_id[turn]->vm_fd);
        ret = ioctl(vm_id[turn]->vcpus->vcpu_fd, KVM_RUN, 0);
        printf("Time: %f\n", CURRENT_TIME);
        printf("VMFD: %d stopped running - exit reason: %d\n", vm_id[turn]->vm_fd, vm_id[turn]->vcpus->kvm_run->exit_reason);

        switch (vm_id[turn]->vcpus->kvm_run->exit_reason)
        {
        case KVM_EXIT_UNKNOWN:
            printf("VMFD: %d KVM_EXIT_UNKNOWN\n", vm_id[turn]->vm_fd);
            break;
        case KVM_EXIT_DEBUG:
            printf("VMFD: %d KVM_EXIT_DEBUG\n", vm_id[turn]->vm_fd);
            break;
        case KVM_EXIT_IO:
            printf("VMFD: %d KVM_EXIT_IO\n", vm_id[turn]->vm_fd);
            printf("VMFD: %d out port: %d, data: %d\n", vm_id[turn]->vm_fd, vm_id[turn]->vcpus->kvm_run->io.port, *(int *)((char *)(vm_id[turn]->vcpus->kvm_run) + vm_id[turn]->vcpus->kvm_run->io.data_offset));
            sleep(1);
            break;
        case KVM_EXIT_MMIO:
            printf("VMFD: %d KVM_EXIT_MMIO\n", vm_id[turn]->vm_fd);
            break;
        case KVM_EXIT_INTR:
            printf("VMFD: %d KVM_EXIT_INTR\n", vm_id[turn]->vm_fd);
            break;
        case KVM_EXIT_SHUTDOWN:
            printf("VMFD: %d KVM_EXIT_SHUTDOWN\n", vm_id[turn]->vm_fd);
            goto exit_kvm;
            break;
        default:
            printf("VMFD: %d KVM PANIC\n", vm_id[turn]->vm_fd);
            printf("VMFD: %d KVM exit reason: %d\n", vm_id[turn]->vm_fd, vm_id[turn]->vcpus->kvm_run->exit_reason);
            goto exit_kvm;
        }

        if (ret < 0 && vm_id[turn]->vcpus->kvm_run->exit_reason != KVM_EXIT_INTR)
        {
            fprintf(stderr, "VMFD: %d KVM_RUN failed\n", vm_id[turn]->vm_fd);
            printf("VMFD: %d KVM_RUN return value %d\n", vm_id[turn]->vm_fd, ret);
            exit(1);
        }

        if (ret < 0)
        {
            // in case the reason for KVM_RUN termination is INTR, set turn to (turn+1)%2
            // this will simply flip the turn (0->1 and 1->0)
            turn = (turn+1)%2;
            
            // Initializing the signal set, adding SIGUSR1 signal to it and then waiting for it in the hyperviser
            // This is basically done to clear the pending signal before running the guest again
            int sig = -1;
            sigset_t sgst;
            sigemptyset(&sgst);
            sigaddset(&sgst, SIGUSR1);
            sigwait(&sgst, &sig);
            continue;
        }

    }

exit_kvm:
}

void kvm_clean_vm(struct vm *vm)
{
    close(vm->vm_fd);
    munmap((void *)vm->ram_start, vm->ram_size);
}

void kvm_clean_vcpu(struct vcpu *vcpu)
{
    munmap(vcpu->kvm_run, vcpu->kvm_run_mmap_size);
    close(vcpu->vcpu_fd);
}

void kvm_clean(struct vm *vm)
{
    assert(vm != NULL);
    close(vm->dev_fd);
    free(vm);
}

int main(int argc, char **argv)
{
    struct vm *vm1 = malloc(sizeof(struct vm));
    struct vm *vm2 = malloc(sizeof(struct vm));

    kvm_init(vm1, vm2);

    if (kvm_create_vm(vm1, RAM_SIZE) < 0)
    {
        fprintf(stderr, "create vm fault\n");
        return -1;
    }

    if (kvm_create_vm(vm2, RAM_SIZE) < 0)
    {
        fprintf(stderr, "create vm fault\n");
        return -1;
    }

    load_binary(vm1, BINARY_FILE1);
    load_binary(vm2, BINARY_FILE2);

    vm1->vcpu_number = 1;
    vm1->vcpus = kvm_init_vcpu(vm1, 0, kvm_cpu_thread);

    vm2->vcpu_number = 1;
    vm2->vcpus = kvm_init_vcpu(vm2, 0, kvm_cpu_thread);

    kvm_run_vm(vm1, vm2);

    kvm_clean_vm(vm1);
    kvm_clean_vm(vm2);

    kvm_clean_vcpu(vm1->vcpus);
    kvm_clean_vcpu(vm2->vcpus);
    kvm_clean(vm1);
    kvm_clean(vm2);
}