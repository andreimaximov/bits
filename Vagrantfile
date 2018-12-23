# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.require_version ">= 2.0.0"

Vagrant.configure("2") do |config|
  config.vm.box = "bento/ubuntu-18.04"
  config.vm.box_check_update = false
  config.vm.synced_folder "./", "/workspace"
  config.vm.provision "shell", inline: <<-SHELL
    sudo apt-get update
    sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
        build-essential                                    \
        clang-format                                       \
        cmake                                              \
        doxygen                                            \
        graphviz                                           \
        libgoogle-glog-dev                                 \
        ninja-build                                        \
        pkg-config                                         \
        python3                                            \
        python3-pip                                        \
        tmux                                               \
        unzip                                              \
        wget

    sudo pip3 install meson

    cd /tmp &&                                                                 \
        wget https://github.com/abseil/googletest/archive/release-1.8.1.zip && \
        unzip release-1.8.1.zip &&                                             \
        cd googletest-release-1.8.1 &&                                         \
        mkdir build &&                                                         \
        cd build &&                                                            \
        cmake -DCMAKE_BUILD_TYPE=RELEASE .. &&                                 \
        make -j4 install

    cd /tmp &&                                                                 \
        wget https://github.com/google/benchmark/archive/v1.4.1.zip &&         \
        unzip v1.4.1.zip &&                                                    \
        cd benchmark-1.4.1 &&                                                  \
        mkdir build &&                                                         \
        cd build &&                                                            \
        cmake -DCMAKE_BUILD_TYPE=RELEASE .. &&                                 \
        make -j4 install

    echo "cd /workspace" >> /home/vagrant/.bashrc
  SHELL

  config.vm.provider "virtualbox" do |vb|
    vb.cpus = "4"
    vb.memory = "4096"
  end
end
