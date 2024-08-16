#define IOCTL_SEND_SIGCHLD_SIGNAL _IOWR('m', 0, struct SIGCHLD_SIGNAL)


// required structure to take the pid of control_station from soldier program during ioctl call
struct SIGCHLD_SIGNAL {
  int new_parent_process_id;
};