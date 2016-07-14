# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
  config.vm.box = "ubuntu/trusty64"
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
    apt-get install -y cmake gcc g++ clang-3.5 cppcheck valgrind git

    # Build
    cp -rf /vagrant/ /home/vagrant/
    mv /home/vagrant/vagrant /home/vagrant/slitherss
    chown -R vagrant:vagrant /home/vagrant/slitherss
  SHELL
end
