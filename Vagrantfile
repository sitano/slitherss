# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
  config.vm.box = "ubuntu/xenial64"
  config.vm.box_check_update = false

  config.vm.provider "virtualbox" do |v|
    v.memory = 1024
    v.cpus = 2
  end

  config.vm.synced_folder "./", "/vagrant"

  config.vm.provision "shell", inline: <<-SHELL
    # Deps
    apt-get update
    apt-get install -y libboost-dev libboost-system-dev libboost-thread-dev libboost-program-options-dev zlib1g-dev
    apt-get install -y cmake gcc g++ clang-3.8 cppcheck valgrind git

    # Build
    ln -s /usr/bin/clang-3.8 /usr/bin/clang
    ln -s /usr/bin/clang++-3.8 /usr/bin/clang
    cp -rf /vagrant/ /home/ubuntu/
    mv /home/ubuntu/vagrant /home/ubuntu/slitherss
    chown -R ubuntu:ubuntu /home/ubuntu/slitherss
  SHELL
end
