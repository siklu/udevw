#include <iostream>

#include <udevw.hpp>

int main() {
  auto udev = udevw::Udev::create();
  auto enumerate = udevw::Enumerate::create(udev);

  enumerate.add_match_subsystem("input");
  auto devices_list = enumerate.scan_devices();

  for (const auto &device_entry : devices_list) {
    auto device = udevw::Device::create_from_syspath(udev, device_entry.name);
    auto syspath = device.get_syspath();
    if (syspath)
      std::cout << *syspath << std::endl;
  }

  auto monitor = udevw::Monitor::create_from_netlink(udev, "udev");
  monitor.filter_add_match_subsystem("input"); // devtype = nullptr implied
  monitor.enable_receiving();

  int fd = monitor.get_fd();
  for (;;) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    if (select(fd +1, &fds, nullptr, nullptr, nullptr) > 0 && FD_ISSET(fd, &fds)) {
      auto device = monitor.receive_device();

      auto syspath = device.get_syspath();
      if (!syspath)
        continue;

      auto action = device.get_action();
      if (!action)
        continue;

      if (*action == "add")
        std::cout << "[+] " << *syspath << std::endl;
      else if (*action == "remove")
        std::cout << "[-] " << *syspath << std::endl;
    }
  }
}
