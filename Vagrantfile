# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.require_version ">= 2.0.0"

Vagrant.configure("2") do |config|
  config.vm.box = "bento/ubuntu-18.04"
  config.vm.box_check_update = false
  config.vm.synced_folder "./", "/workspace"
  config.vm.provision "shell", path: "scripts/install-deps.sh"
  config.vm.provision "shell", inline: <<-SHELL
    echo "cd /workspace" >> /home/vagrant/.bashrc
  SHELL
  config.vm.provider "virtualbox" do |vb|
    vb.cpus = "4"
    vb.memory = "4096"
  end
end
